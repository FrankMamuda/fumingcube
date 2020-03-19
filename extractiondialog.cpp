/*
 * Copyright (C) 2017-2018 Factory #12
 * Copyright (C) 2019-2020 Armands Aleksejevs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

/*
 * includes
 */
#include "extractiondialog.h"
#include "extractionmodel.h"
#include "property.h"
#include "reagent.h"
#include "textedit.h"
#include "ui_extractiondialog.h"
#include <QBuffer>
#include <QButtonGroup>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "tag.h"
#include "propertywidget.h"
#include "imageutils.h"
#include "structurefragment.h"
#include "pixmaputils.h"
#include "htmlutils.h"
#include "listutils.h"
#include "cache.h"

/**
 * @brief ExtractionDialog::ExtractionDialog
 * @param parent
 */
ExtractionDialog::ExtractionDialog( QWidget *parent, const Id &reagentId, const int cid ) : QDialog( parent ), ui( new Ui::ExtractionDialog ), m_reagentId( reagentId ) {
    // setup ui and get rid of verticalHeader in property view
    this->ui->setupUi( this );
    this->ui->ExtractionDialogContents->setWindowFlags( Qt::Widget );

    //this->ui->propertyView->verticalHeader()->hide();

    // setup connections to the NetworkManager
    NetworkManager::connect( NetworkManager::instance(), SIGNAL( finished( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )), this, SLOT( replyReceived( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )));
    NetworkManager::connect( NetworkManager::instance(), SIGNAL( error( const QString &, NetworkManager::Types, const QString & )), this, SLOT( error( const QString &, NetworkManager::Types, const QString & )));

    // read id/name maps from disk if any
    this->readCache();

    // set reagent name
    this->ui->searchPage->setIdentifier( HTMLUtils::convertToPlainText( Reagent::instance()->name( reagentId )));

    // check name
    // NOTE: this also re-enables extract button after unsuccessful query
    //auto checkName = [ this ]() { this->ui->extractButton->setEnabled( !this->ui->nameEdit->text().isEmpty()); };
    //QLineEdit::connect( this->ui->identifierEdit, &QLineEdit::textChanged, checkName );
    //checkName();

#if 0


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
    QPushButton::connect( this->ui->addSelectedButton, &QPushButton::pressed, addProperties );

    this->buttonTest();

    // forced cid
    if ( cid > 0 ) {
        this->ui->nameEdit->setText( "cid_" + QString::number( cid ));
        this->ui->nameEdit->hide();
        this->ui->cidEdit->setText( QString::number( cid ));
        this->ui->cidEdit->setDisabled( true );

        // begin data request immediately
        this->getFormula( QString::number( cid ));

        // GET DATA
        if ( !this->readFromCache())
            NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/%1/JSON" ).arg( cid ), NetworkManager::DataRequest );
    } 
#endif











   // const QPixmap pixmap( QIcon::fromTheme( "info" ).pixmap( 16, 16 ));
    //const QList<QLabel*> tips( QList<QLabel*>() << this->ui->cacheTipIcon << this->ui->searchTipIcon << this->ui->valuesTipIcon << this->ui->propertyTipIcon << this->ui->structureTipIcon );
    //for ( QLabel *tip : tips )
   //     tip->setPixmap( pixmap );

    auto leftSpacer( new QWidget( this ));
    leftSpacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    auto rightSpacer( new QWidget( this ));
    rightSpacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    QToolBar *toolbar( this->ui->navigationBar );
    toolbar->insertWidget( this->ui->actionSearch, leftSpacer );
    toolbar->insertWidget( this->ui->actionClose, rightSpacer );

    const QList<QAction*> actions( QList<QAction*>() << this->ui->actionSearch << this->ui->actionStructureBrowser << this->ui->actionProperties );
    auto checkState = [ this, actions ]( QAction *action, bool checked ) {
        if ( !checked ) {
            action->blockSignals( true );
            action->setChecked( true );
            action->blockSignals( false );
            return;
        }

        for ( QAction *otherAction : actions ) {
            if ( otherAction != action ) {
                otherAction->blockSignals( true );
                otherAction->setChecked( false );
                otherAction->blockSignals( false );
            }
        }

        this->ui->viewStack->setCurrentIndex( actions.indexOf( action ));
        this->adjustSize();
    };

    for ( QAction *action : actions )
        QAction::connect( action, &QAction::toggled, this, [ action, checkState ]( bool checked ) { checkState( action, checked ); } );

    this->ui->actionSearch->setChecked( true );
}

/**
 * @brief ExtractionDialog::~ExtractionDialog
 */
ExtractionDialog::~ExtractionDialog() {
    // disconnect from the network manager
    NetworkManager::disconnect( NetworkManager::instance(), SIGNAL( finished( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )), this, SLOT( replyReceived( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )));
    NetworkManager::disconnect( NetworkManager::instance(), SIGNAL( error( const QString &, NetworkManager::Types, const QString & )), this, SLOT( error( const QString &, NetworkManager::Types, const QString & )));

    // write out id cache
    this->writeCache();

    // delete ui
    delete this->ui;
}

/**
 * @brief ExtractionDialog::name
 * @return
 */
QString ExtractionDialog::name() const {
    return this->ui->searchPage->identifier();
}

/**
 * @brief ExtractionDialog::id
 * @return
 */
int ExtractionDialog::id() const {
    return this->ui->structurePage->cid();// cidEdit->text().toInt();
}

/**
 * @brief ExtractionDialog::readCache
 */
void ExtractionDialog::readCache() {
    // look for id map in cache
    if ( Cache::instance()->contains( ExtractionDialog::IdMapContext, "data.map" )) {
        // read serialized map
        QByteArray byteArray( Cache::instance()->getData( ExtractionDialog::IdMapContext, "data.map", true ));
        QBuffer buffer( &byteArray );
        buffer.open( QIODevice::ReadOnly );
        QDataStream in( &buffer );

        // check version to avoid segfaults
        // if we fail, new cache information will just be overwritten
        int version;
        in >> version;
        if ( version != ExtractionDialog::Version )
            return;

        // finally read in the maps
        in >> this->idNameMap >> this->nameIdMap;
    }
}

/**
 * @brief ExtractionDialog::writeCache
 */
void ExtractionDialog::writeCache() {
    // serialize maps
    QByteArray byteArray;
    QBuffer buffer( &byteArray );
    buffer.open( QIODevice::WriteOnly );
    QDataStream out( &buffer );
    out << ExtractionDialog::Version << this->idNameMap << this->nameIdMap;

    // store maps into disk cache
    Cache::instance()->insert( ExtractionDialog::IdMapContext, "data.map", byteArray, true );
}

/**
 * @brief ExtractionDialog::replyReceived
 * @param url
 * @param type
 * @param userData
 * @param data
 */
void ExtractionDialog::replyReceived( const QString &, NetworkManager::Types type, const QVariant &, const QByteArray &data ) {
    switch ( type ) {
    case NetworkManager::CIDRequestInitial:
        qDebug() << "network->idList" << this->name();
        if ( !this->parseIdListRequest( data )) {
            qDebug() << "  parseIdListRequest failed";

            // try similar structure search if initial request fails
            this->sendSimilarRequest();
            return;
        }
        break;

    case NetworkManager::CIDRequestSimilar:
        qDebug() << "network->idList (similar)" << this->name();
        if ( !this->parseIdListRequest( data )) {
            qDebug() << "  parseIdListRequest failed";

            // TODO: report error
            return;
        }
        break;

    case NetworkManager::FormulaRequest:
        this->parseFormulaRequest( data );
        break;

    case NetworkManager::DataRequest:
        this->parseDataRequest( data );
        break;

    default:
        ;
    }
}

/**
 * @brief ExtractionDialog::error
 * @param type
 */
void ExtractionDialog::error( const QString &, NetworkManager::Types type, const QString & ) {
    switch ( type ) {
    case NetworkManager::CIDRequestInitial:
        // try similar structure search if initial request fails
        this->sendSimilarRequest();
        break;

    default:
        ;
    }
}

/**
 * @brief ExtractionDialog::sendInitialRequest
 */
void ExtractionDialog::sendInitialRequest() {
    // TODO: set status message
    qDebug() << "  request initial" << this->name();
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/%1/cids/TXT" ).arg( this->name().replace( " ", "-" )), NetworkManager::CIDRequestInitial );
}

/**
 * @brief ExtractionDialog::sendSimilarRequest
 */
void ExtractionDialog::sendSimilarRequest() {
    // TODO: set status message
    qDebug() << "  request similar" << this->name();
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/%1/cids/TXT?name_type=word" ).arg( this->name().replace( " ", "-" )), NetworkManager::CIDRequestSimilar );
}

/**
 * @brief ExtractionDialog::sendFormulaRequest
 */
void ExtractionDialog::sendFormulaRequest() {
    qDebug() << "  request formula" << this->name();
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/%1/PNG" ).arg( this->id()), NetworkManager::FormulaRequest );
}

/**
 * @brief ExtractionDialog::sendDataRequest
 */
void ExtractionDialog::sendDataRequest(){
    qDebug() << "  request data" << this->name();
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/%1/JSON" ).arg( this->id()), NetworkManager::DataRequest );
}

/**
 * @brief ExtractionDialog::parseFormulaRequest
 * @param data
 * @return
 */
bool ExtractionDialog::parseFormulaRequest( const QByteArray &data ) {
    if ( !data.isEmpty()) {
        qDebug() << "    network->formula" << this->name();
        Cache::instance()->insert( ExtractionDialog::FormulaContext, QString( "%1.png" ).arg( this->id()), data );
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
bool ExtractionDialog::parseDataRequest( const QByteArray &data ) {
    if ( !data.isEmpty()) {
        qDebug() << "    network->data" << this->name();
        Cache::instance()->insert( ExtractionDialog::DataContext, QString( "%1.dat" ).arg( this->id()), data, true );
        this->readData( data );
        return true;
    }

    return false;
}

/**
 * @brief ExtractionDialog::parseIdListRequest
 * @param data
 * @return
 */
bool ExtractionDialog::parseIdListRequest( const QByteArray &data ) {
    // get id list from data
    const QList<int>idList( ListUtils::toNumericList<int>( QString( data ).split( "\n" )));

    if ( idList.isEmpty())
        return false;

    // store ids into cache
    for ( const int &id : idList )
        this->nameIdMap.insert( this->name(), id );

    // continue on
    return this->parseIdList( idList );
}

/**
 * @brief ExtractionDialog::parseIdList
 * @param idList
 */
bool ExtractionDialog::parseIdList( const QList<int> &idList ) {
#if 0
    qDebug() << "parseIdList";

    // abort on empty id lists
    if ( idList.isEmpty()) {
        // TODO: update status
        return false;
    }

    // select an id
    int id = 0;
    if ( idList.count() > 1 ) {
        // if we have multiple ids in the list, open the StructureBrowser and let the user decide
        //StructureBrowser sb( idList, this );
        if ( sb.exec() != QDialog::Accepted ) {
            // TODO: update status - ABORT
            return true;
        }
        id = sb.cid();
    } else {
        // if the list has only one entry, continue with that
        id = idList.first();
    }

    // make sure it is a valid id
    if ( id <= 0 ) {
        // TODO: update status
        return false;
    }

    // all ok, continue with data extraction
    this->getDataAndFormula( id );
#endif
    // return success
    return true;
}

/**
 * @brief ExtractionDialog::getDataAndFormula
 * @param id
 * @return
 */
bool ExtractionDialog::getDataAndFormula( const int &id ) {
    this->ui->structurePage->setup( QList<int>() << id );

    qDebug() << "  getDataAndFormula";

    this->getFormula();
    this->getData();

    return true;
}

/**
 * @brief ExtractionDialog::getFormula
 */
void ExtractionDialog::getFormula() {
    if ( Cache::instance()->contains( ExtractionDialog::FormulaContext, QString( "%1.png" ).arg( this->id()))) {
        qDebug() << "    cache->formula" << this->name();
        this->readFormula( Cache::instance()->getData( ExtractionDialog::FormulaContext, QString( "%1.png" ).arg( this->id())));
        return;
    }

    // formula not in the cache, fetch it
    this->sendFormulaRequest();
}

/**
 * @brief ExtractionDialog::getData
 */
void ExtractionDialog::getData() {
    if ( Cache::instance()->contains( ExtractionDialog::DataContext, QString( "%1.dat" ).arg( this->id()))) {
        qDebug() << "    cache->data" << this->name();
        this->readData( Cache::instance()->getData( ExtractionDialog::DataContext, QString( "%1.dat" ).arg( this->id()), true ));
        return;
    }

    // data not in the cache, fetch it
    this->sendDataRequest();
}

/**
 * @brief ExtractionDialog::readFormula
 * @param data
 */
void ExtractionDialog::readFormula( const QByteArray &data ) {
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

/**
 * @brief ExtractionDialog::readData
 * @param uncompressed
 * @return
 */
void ExtractionDialog::readData( const QByteArray &uncompressed ) const {
    QMutexLocker lock( &this->mutex );

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

#if 0

/**
 * @brief ExtractionDialog::buttonTest
 */
void ExtractionDialog::buttonTest() {
    this->ui->addAllButton->setDisabled( true );
    this->ui->addSelectedButton->setDisabled( true );
    this->ui->clearCacheButton->setDisabled( true );
    this->ui->extractButton->setDisabled( true );

    if ( this->status() == Idle ) {
        if ( this->ui->propertyView->model()->rowCount() > 1 ) {
            this->ui->addAllButton->setEnabled( true );
            this->ui->addSelectedButton->setEnabled( true );
        }

        this->ui->clearCacheButton->setEnabled( true );
        this->ui->extractButton->setEnabled( true );
        return;
    }
}
/**
 * @brief ExtractionDialog::on_extractButton_clicked
 */
void ExtractionDialog::on_extractButton_clicked() {
    this->setStatus( Busy );
    this->buttonTest();

    this->ui->propertyView->clear();
    this->ui->propertyView->setRowCount( 0 );
    this->ui->propertyView->setColumnCount( 2 );

    if ( !this->readFromCache()) {
        // get data by making an inital request
        NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/%1/cids/TXT" ).arg( this->ui->nameEdit->text().replace( " ", "-" )), NetworkManager::CIDRequestInitial );
    }
}

/**
 * @brief ExtractionDialog::on_clearCacheButton_clicked
 */
void ExtractionDialog::on_clearCacheButton_clicked() {
    if ( this->ui->nameEdit->text().isEmpty())
        return;

    const QString cachedName( this->cachedName());
    Cache::instance()->clear( ExtractionDialog::FormulaContext, cachedName + ".png" );
    Cache::instance()->clear( ExtractionDialog::DataContext, cachedName );
}


/**
 * @brief ExtractionDialog::error
 */
void ExtractionDialog::error( const QString &, NetworkManager::Types type, const QString &/*errorMessage*/ ) {
    //qDebug() << errorMessage;
    this->setStatus( Error );
    this->buttonTest();

    switch ( type ) {
    case NetworkManager::CIDRequestInitial:
    {
        this->ui->cidEdit->setText( ExtractionDialog::tr( "Could not get a valid CID, trying similar" ));

        // get formula
        const QString cachedName( this->cachedName() + ".cid" );
        if ( Cache::instance()->contains( ExtractionDialog::CIDContext, cachedName )) {
            this->cidList = QString( Cache::instance()->getData( ExtractionDialog::CIDContext, cachedName )).split( "\n" );

            QList<int> cidListInt( ListUtils::toNumericList<int>( qAsConst( this->cidList )));
            cidListInt.removeAll( 0 );
            std::sort( cidListInt.begin(), cidListInt.end());
            cidListInt.erase( std::unique( cidListInt.begin(), cidListInt.end()), cidListInt.end());

            if ( cidListInt.count() == 1 ) {
                NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/%1/JSON" ).arg( cidListInt.first()), NetworkManager::DataRequest );
                this->getFormula( QString::number( cidListInt.first()));
            } else
                this->getSimilar( qAsConst( cidListInt ));

            return;
        }

        // initial failed to yield a list, proceed to similar search
        NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/%1/cids/TXT?name_type=word" ).arg( this->ui->nameEdit->text().replace( " ", "-" )), NetworkManager::CIDRequestSimilar );
    }
        break;

    case NetworkManager::CIDRequestSimilar:
        this->ui->cidEdit->setText( ExtractionDialog::tr( "Could not get a valid CID" ));
        break;

    case NetworkManager::DataRequest:
        this->ui->cidEdit->setText( ExtractionDialog::tr( "Could retrieve data associated with CID" ));
        break;

    case NetworkManager::FormulaRequest:
        this->ui->cidEdit->setText( ExtractionDialog::tr( "Could retrieve formula associated with CID" ));
        break;

        // do not handle errors related to other classes
    case NetworkManager::IUPACName:
    case NetworkManager::NoType:
    case NetworkManager::FormulaRequestBrowser:
    case NetworkManager::FavIcon:
        break;
    }
}
#endif

/**
 * @brief ExtractionDialog::on_actionFetch_triggered
 */
void ExtractionDialog::on_actionFetch_triggered() {
    // TODO: clear previous
    // TODO: completer for cached entries?

    // check for id in cache
    if ( this->nameIdMap.contains( this->name())) {
        const QList<int> idList( this->nameIdMap.values( this->name()));
        if ( !idList.isEmpty()) {
            qDebug() << "cache->idList";
            if ( this->parseIdList( idList ))
                return;
        }
    }

    // send initial search request to get an id
    this->sendInitialRequest();
}
