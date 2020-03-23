#include "propertyfragment.h"
#include "networkmanager.h"
#include "extractiondialog.h"
#include "searchfragment.h"
#include "structurefragment.h"
#include "ui_propertyfragment.h"
#include "cache.h"
#include "tag.h"
#include "propertywidget.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>

PropertyFragment::PropertyFragment(QWidget *parent) :
    Fragment(parent),
    ui(new Ui::PropertyFragment)
{


    ui->setupUi(this);


    //this->ui->propertyView->verticalHeader()->hide();

    /*



    auto addProperties = [ this ]( bool all = false ) {
        if ( all )
            this->ui->propertyView->selectAll();

        for ( const QModelIndex &index : this->ui->propertyView->selectionModel()->selectedRows()) {
            auto *widget( qobject_cast<PropertyWidget *>( this->ui->propertyView->cellWidget( index.row(), 1 )));
            if ( widget != nullptr ) {
                if ( widget->tagId() != Id::Invalid ) {
                    widget->add( this->reagentId());
                } else {
                    Row row = Row::Invalid;
                    const QPixmap pixmap( widget->pixmap());
                    if ( pixmap.isNull() || this->reagentId() == Id::Invalid )
                        return;

                    for ( int y = 0; y < Tag::instance()->count(); y++ ) {
                        const auto r = static_cast<Row>( y );
                        if ( Tag::instance()->type( r ) == Tag::Formula ) {
                            row = r;
                            break;
                        }
                    }

                    if ( row != Row::Invalid )
                        Property::instance()->add( ExtractionDialog::tr( "Structural formula" ), Tag::instance()->id( row ), PixmapUtils::convertToData( pixmap ), this->reagentId());
                }
            }
        }

        this->accept();
    };

    QPushButton::connect( this->ui->addAllButton, &QPushButton::pressed, std::bind( addProperties, true ));
    QPushButton::connect( this->ui->addSelectedButton, &QPushButton::pressed, addProperties );*/

}

PropertyFragment::~PropertyFragment()
{
    delete ui;
}

/**
 * @brief PropertyFragment::sendFormulaRequest
 */
void PropertyFragment::sendFormulaRequest() {
    qDebug() << "  request formula" << this->host()->searchFragment()->identifier();
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/%1/PNG" ).arg( this->host()->structureFragment()->cid()), NetworkManager::FormulaRequest );
}

/**
 * @brief PropertyFragment::sendDataRequest
 */
void PropertyFragment::sendDataRequest(){
    qDebug() << "  request data" << this->host()->searchFragment()->identifier();
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/%1/JSON" ).arg( this->host()->structureFragment()->cid()), NetworkManager::DataRequest );
}

/**
 * @brief PropertyFragment::parseFormulaRequest
 * @param data
 * @return
 */
bool PropertyFragment::parseFormulaRequest( const QByteArray &data ) {
    if ( !data.isEmpty()) {
        qDebug() << "    network->formula" << this->host()->searchFragment()->identifier();
        Cache::instance()->insert( Cache::FormulaContext, QString( "%1.png" ).arg( this->host()->structureFragment()->cid()), data );
        this->readFormula( data );
        return true;
    }

    return false;
}

/**
 * @brief ExtractionDialog::parseDataRequest
 * @param data
 * @return
 */
bool PropertyFragment::parseDataRequest( const QByteArray &data ) {
    if ( !data.isEmpty()) {
        qDebug() << "    network->data" << this->host()->searchFragment()->identifier();
        Cache::instance()->insert( Cache::DataContext, QString( "%1.dat" ).arg( this->host()->structureFragment()->cid()), data, true );
        this->readData( data );
        return true;
    }

    return false;
}

/**
 * @brief PropertyFragment::getDataAndFormula
 * @param id
 * @return
 */
bool PropertyFragment::getDataAndFormula( const int &id ) {
    //this->ui->structureFragment->setup( QList<int>() << id );

    qDebug() << "  getDataAndFormula";

    this->getFormula();
    this->getData();

    return true;
}

/**
 * @brief ExtractionDialog::getFormula
 */
void PropertyFragment::getFormula() {
    if ( Cache::instance()->contains( Cache::FormulaContext, QString( "%1.png" ).arg( this->host()->structureFragment()->cid()))) {
        qDebug() << "    cache->formula" << this->host()->searchFragment()->identifier();
        this->readFormula( Cache::instance()->getData( Cache::FormulaContext, QString( "%1.png" ).arg( this->host()->structureFragment()->cid())));
        return;
    }

    // formula not in the cache, fetch it
    this->sendFormulaRequest();
}

/**
 * @brief PropertyFragment::getData
 */
void PropertyFragment::getData() {
    if ( Cache::instance()->contains( Cache::DataContext, QString( "%1.dat" ).arg( this->host()->structureFragment()->cid()))) {
        qDebug() << "    cache->data" << this->host()->searchFragment()->identifier();
        this->readData( Cache::instance()->getData( Cache::DataContext, QString( "%1.dat" ).arg( this->host()->structureFragment()->cid()), true ));
        return;
    }

    // data not in the cache, fetch it
    this->sendDataRequest();
}

/**
 * @brief PropertyFragment::readData
 * @param uncompressed
 */
void PropertyFragment::readData( const QByteArray &uncompressed ) const {
    //QMutexLocker lock( &this->mutex );

    /**
     * @brief findTag finds a TOCHeading in json document
     */
    std::function<void( const QJsonValue &, const QString &heading, QList<QJsonArray> &matches )> findTag;
    findTag = [ &findTag ]( const QJsonValue &value, const QString &heading, QList<QJsonArray> &matches ) {
        if ( value.isObject()) {
            const QJsonObject object( value.toObject());

            const QStringList keys( object.keys());
            for ( const QString &key : keys ) {
                const QJsonValue keyValue( object.value( key ));

                if ( !QString::compare( key, "TOCHeading" )) {
                    if ( !keyValue.isArray() && !keyValue.isObject()) {
                        if ( !QString::compare( keyValue.toVariant().toString(), heading )) {
                            if ( object.contains( "Information" )) {
                                const QJsonValue infoValue( object.value( "Information" ));
                                if ( infoValue.isArray())
                                    matches << infoValue.toArray();
                            }
                        }
                    }
                }

                findTag( keyValue, heading, matches );
            }
        } else if ( value.isArray()) {
            const QJsonArray array( value.toArray());

            for ( const QJsonValue &arrayValue : array )
                findTag( arrayValue, heading, matches );
        }
    };

    /**
     * @brief extractValues gets string or numeric values from json
     */
    auto extractValues = []( const QList<QJsonArray> &matches, const QString &name, const QString &pattern, QList<QStringList> &out, bool global ) {
        QStringList values;

        for ( const QJsonArray &array : matches ) {
            for ( const QJsonValue &info : array ) {
                if ( info.isObject()) {
                    const QJsonObject infoObject( info.toObject());

                    if ( !name.isEmpty() && infoObject.contains( "Name" )) {
                        const QJsonValue nameValue( infoObject["Name"] );
                        if ( !nameValue.isArray() && !nameValue.isObject()) {
                            if ( QString::compare( nameValue.toString(), name ))
                                continue;
                        }
                    }

                    if ( infoObject.contains( "Value" )) {
                        const QJsonValue value( infoObject["Value"] );

                        if ( value.isObject()) {
                            const QJsonObject valueObject( value.toObject());
                            QString units;
                            if ( valueObject.contains( "Unit" ))
                                units = valueObject["Unit"].toVariant().toString();

                            if ( valueObject.contains( "Number" )) {
                                const QJsonValue numberTag( valueObject["Number"] );
                                if ( numberTag.isArray()) {
                                    const QJsonArray numberArray( numberTag.toArray());
                                    for ( const QJsonValue &number : numberArray )
                                        values << QString( "%1 %2" ).arg( number.toDouble()).arg( qAsConst( units ));
                                }
                            } else if ( valueObject.contains( "StringWithMarkup" )) {
                                const QJsonValue &stringTag( valueObject["StringWithMarkup"] );
                                if ( stringTag.isArray()) {

                                    const QJsonArray &stringArray( stringTag.toArray());
                                    for ( const QJsonValue &stringValue : stringArray ) {
                                        if ( stringValue.isObject()) {
                                            const QJsonObject &stringObject( stringValue.toObject());
                                            QStringList extra;
                                            if ( stringObject.keys().contains( "Markup" )) {
                                                const QJsonValue &markupTag( stringObject["Markup"] );
                                                if ( markupTag.isArray()) {
                                                    const QJsonArray &markupArray( markupTag.toArray());
                                                    if ( markupArray.count()) {
                                                        for ( const QJsonValue &markupValue : markupArray ) {
                                                            if ( markupValue.isObject()) {
                                                                const QJsonObject &markupObject( markupValue.toObject());

                                                                if ( markupObject.contains( "Type" )) {
                                                                    const QJsonValue &typeValue( markupObject["Type"] );
                                                                    if ( !typeValue.isArray() && !typeValue.isObject()) {
                                                                        if ( !QString::compare( typeValue.toString(), "PubChem Internal Link" )) {
                                                                            continue;
                                                                        }
                                                                    }
                                                                }

                                                                if ( markupObject.contains( "Extra" )) {
                                                                    const QJsonValue &extraValue( markupObject["Extra"] );
                                                                    if ( !extraValue.isArray() && !extraValue.isObject()) {
                                                                        extra << extraValue.toString();
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                            if ( !extra.isEmpty()) {
                                                values << extra.join( ", " );
                                                continue;
                                            }

                                            if ( stringObject.contains( "String" ))
                                                values << QString( "%1" ).arg( stringObject["String"].toVariant().toString());
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        values.removeDuplicates();

        if ( !pattern.isEmpty()) {
            const QRegularExpression re( pattern );

            if ( !re.isValid())
                return;

            for ( const QString &value : qAsConst( values )) {
                QStringList captured;

                const QString stripped( QString( value ).remove( QRegularExpression( R"(\(\w+, \d{4}\))" )));
                auto matcher = [ &captured, stripped, re, global, value ]( const QRegularExpressionMatch &match ) {
                    if ( match.hasMatch()) {
                        int k = 0;

                        if ( global )
                            k = 1;

                        for ( ; k < match.capturedTexts().count(); k++ ) {
                            const QString matched( match.captured( k ).simplified());
                            captured << matched;
                        }
                    }
                };

                if ( global ) {
                    QRegularExpressionMatchIterator i( re.globalMatch( stripped ));
                    captured << "";
                    while ( i.hasNext())
                        matcher( i.next());
                } else {
                    matcher( re.match( stripped ));
                }

                if ( !out.contains( captured ) && !captured.isEmpty())
                    out.append( captured );
            }
        } else {
            for ( const QString &value : qAsConst( values ))
                out << ( QStringList() << value << value );
        }


        // de-prioritize imperial temperature units
        std::sort( out.begin(), out.end(), []( const QStringList &l, const QStringList &r ) {
            if ( l.isEmpty() || r.isEmpty())
                return false;

            return l.first().contains( "°F" ) < r.first().contains( "°F" );
        } );
    };

    const QJsonDocument document( QJsonDocument::fromJson( uncompressed ));
    const QJsonValue value( document.isArray() ? QJsonValue( document.array()) : document.object());
    auto getPropertyFromJson = [ value, findTag, extractValues ]( const QString &tagName, const QString &valueName = QString(), const QString &pattern = QString(), bool global = false ) {
        QList<QJsonArray> matches;
        findTag( value, tagName, matches );
        QList<QStringList> values;
        extractValues( qAsConst( matches ), valueName, pattern, values, global );

        return values;
    };

    QMap<QString, PropertyWidget*>propList;

    for ( int y = 0; y < Tag::instance()->count(); y++ ) {
        const auto row = static_cast<Row>( y );

        const QString script( Tag::instance()->script( row ).toString());
        if ( !script.isEmpty()) {
            const QStringList args( script.split( ";" ));

            if ( args.isEmpty())
                continue;

            const QString tagName( args.count() >= 1 ? args.at( 0 ) : "" );
            const QString valueName( args.count() >= 2 ? args.at( 1 ) : "" );
            const QString pattern( args.count() >= 3 ? args.at( 2 ) : "" );
            const bool global( args.count() >= 4 ? args.at( 3 ).toInt() : false );

            const QList<QStringList> values( getPropertyFromJson( tagName, valueName, pattern, global ));
            if ( values.isEmpty())
                continue;

            PropertyWidget *group( new PropertyWidget( nullptr, values, Tag::instance()->id( row )));
            propList[Tag::instance()->name( row )] = group;
        }
    }
#if 0

    int row = this->ui->propertyView->rowCount();
    QStringList propListKeys( propList.keys());
    this->ui->propertyView->setRowCount( this->ui->propertyView->rowCount() + propListKeys.count());
    for ( const QString &name : propListKeys ) {
        this->ui->propertyView->setItem( row, 0, new QTableWidgetItem( name ));
        this->ui->propertyView->setCellWidget( row, 1, propList[name] );
        row++;
    }

    this->ui->propertyView->resizeRowsToContents();
    this->ui->propertyView->resizeColumnsToContents();
#endif
}

/**
 * @brief PropertyFragment::readFormula
 * @param data
 */
void PropertyFragment::readFormula(const QByteArray &data) {
#if 0
    QMutexLocker lock( &this->mutex );

    const int rows = this->ui->propertyView->rowCount();
    this->ui->propertyView->setRowCount( rows + 1 );
    QPixmap pixmap;
    if ( !pixmap.loadFromData( data ))
        return;

    if ( pixmap.isNull())
        return;

    const QPixmap cropped( PixmapUtils::autoCrop( qAsConst( pixmap ), QColor::fromRgb( 245, 245, 245, 255 )));
    this->ui->propertyView->setItem( rows, 0, new QTableWidgetItem( "Formula" ));
    this->ui->propertyView->setCellWidget( rows, 1, new PropertyWidget( nullptr, cropped ));
    this->ui->propertyView->resizeRowToContents( rows );
#endif
}
