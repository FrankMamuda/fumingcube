/*
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
#include "propertyfragment.h"
#include "networkmanager.h"
#include "extractiondialog.h"
#include "searchfragment.h"
#include "structurefragment.h"
#include "ui_propertyfragment.h"
#include "cache.h"
#include "tag.h"
#include "propertywidget.h"
#include "pixmaputils.h"
#include "property.h"
#include "tagselectiondialog.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QDesktopWidget>
#include <QWindow>
#include <QScreen>
#include "listutils.h"

/**
 * @brief PropertyFragment::PropertyFragment
 * @param parent
 */
PropertyFragment::PropertyFragment( QWidget *parent ) : Fragment( parent ), ui( new Ui::PropertyFragment ) {
    // setup ui
    this->ui->setupUi( this );

    // set tip icons
    const QPixmap pixmap( QIcon::fromTheme( "info" ).pixmap( 16, 16 ));
    const QList<QLabel*> tips( QList<QLabel*>() << this->ui->propertyTipIcon << this->ui->valuesTipIcon );
    for ( QLabel *tip : tips )
        tip->setPixmap( pixmap );

    // setup property table
    this->ui->propertyView->verticalHeader()->hide();
    this->ui->propertyView->horizontalHeader()->hide();
    this->ui->propertyView->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
    this->ui->propertyView->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );

    // property addition lambda
    auto addProperties = [ this ]( bool all = false ) {
        if ( this->host()->reagentId() == Id::Invalid )
            return;

        if ( all )
            this->ui->propertyView->selectAll();

        for ( const QModelIndex &index : this->ui->propertyView->selectionModel()->selectedRows()) {
            auto *widget( qobject_cast<PropertyWidget *>( this->ui->propertyView->cellWidget( index.row(), 1 )));
            if ( widget != nullptr ) {
                if ( widget->tagId() != Id::Invalid ) {
                    widget->add( this->host()->reagentId());
                } else {
                    Row row = Row::Invalid;
                    const QPixmap pixmap( widget->pixmap());
                    if ( pixmap.isNull() || this->host()->reagentId() == Id::Invalid )
                        return;

                    for ( int y = 0; y < Tag::instance()->count(); y++ ) {
                        const auto r = static_cast<Row>( y );
                        if ( Tag::instance()->type( r ) == Tag::Formula ) {
                            row = r;
                            break;
                        }
                    }

                    if ( row != Row::Invalid )
                        Property::instance()->add( ExtractionDialog::tr( "Structural formula" ), Tag::instance()->id( row ), PixmapUtils::toData( pixmap ), this->host()->reagentId());
                }
            }
        }

        this->host()->close();
        PropertyDock::instance()->updateView();
    };

    // setup finished connection to the NetworkManager
    NetworkManager::connect( NetworkManager::instance(), &NetworkManager::finished, this, [ this ]( const QString &, NetworkManager::Types type, const QVariant &, const QByteArray &data ) {
        switch ( type ) {
        case NetworkManager::DataRequest:
            qDebug() << "network->data" << this->host()->searchFragment()->identifier();
            if ( !this->parseDataRequest( data )) {
                qDebug() << "  parseDataRequest failed";
                this->host()->setErrorMessage( PropertyFragment::tr( "Could not parse data request" ));
            }
            // TODO: adjust ui controls
            //this->toggleControls( true );
            break;

        case NetworkManager::FormulaRequest:
            qDebug() << "network->formula (data)" << this->host()->searchFragment()->identifier();
            if ( !this->parseFormulaRequest( data )) {
                qDebug() << "  parseFormulaRequest (DATA) failed";
                this->host()->setErrorMessage( PropertyFragment::tr( "Could not parse formula request" ));
                return;
            }
            // TODO: adjust ui controls
            break;

        default:
            ;
        }
    } );

    // setup error connection to the NetworkManager
    NetworkManager::connect( NetworkManager::instance(), &NetworkManager::error, this, [ this ]( const QString &, NetworkManager::Types type, const QVariant &, const QString &errorMessage ) {
        if ( type == NetworkManager::DataRequest || type == NetworkManager::FormulaRequest ) {
            this->host()->setErrorMessage( StructureFragment::tr( "Error: " ) + errorMessage );
            this->host()->adjustSize();
        }
    } );

    // connect addAll, addSelected actions
    QAction::connect( this->ui->actionAddAll, &QAction::triggered, this, std::bind( addProperties, true ));
    QAction::connect( this->ui->actionAddSelected, &QAction::triggered, std::bind( addProperties, false ));
    QAction::connect( this->ui->actionRefresh, &QAction::triggered, this, [ this ]() { this->getDataAndFormula( this->host()->structureFragment()->cid()); } );
    QAction::connect( this->ui->actionSelectTags, &QAction::triggered, this, [ this ]() {
        TagSelectionDialog td( this );
        td.exec();
    } );

    // enable addSelected action only if actions are selected
    QItemSelectionModel::connect( this->ui->propertyView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [ this ]( const QItemSelection &, const QItemSelection & ) {
        const QModelIndexList list( this->ui->propertyView->selectionModel()->selectedRows());
        this->ui->actionAddSelected->setEnabled( !list.isEmpty() && this->host()->reagentId() != Id::Invalid );
    } );

    // setup clear action
    QAction::connect( this->ui->actionClear, &QAction::triggered, this, [ this ]() {
        this->ui->propertyView->setRowCount( 0 );
        this->ui->actionAddAll->setDisabled( true );
        this->ui->actionAddSelected->setDisabled( true );
        this->host()->setCurrentFragment( this->host()->searchFragment());
        this->host()->setFragmentEnabled( this, false );
        this->host()->setFragmentEnabled( this->host()->structureFragment(), false );
    } );
}

/**
 * @brief PropertyFragment::~PropertyFragment
 */
PropertyFragment::~PropertyFragment() {
    NetworkManager::disconnect( NetworkManager::instance(), &NetworkManager::finished, this, nullptr );
    NetworkManager::disconnect( NetworkManager::instance(), &NetworkManager::error, this, nullptr );
    QAction::disconnect( this->ui->actionAddAll, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionAddSelected, &QAction::triggered, this, nullptr );
    QItemSelectionModel::disconnect( this->ui->propertyView->selectionModel(), &QItemSelectionModel::selectionChanged, this, nullptr );
    QAction::disconnect( this->ui->actionClear, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionRefresh, &QAction::triggered, this, nullptr );

    delete this->ui;
}

/**
 * @brief PropertyFragment::sendFormulaRequest
 */
void PropertyFragment::sendFormulaRequest() {
    qDebug() << "  request formula (DATA)" << this->host()->searchFragment()->identifier();
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/%1/PNG" ).arg( this->host()->structureFragment()->cid()), NetworkManager::FormulaRequest );
}

/**
 * @brief PropertyFragment::sendDataRequest
 */
void PropertyFragment::sendDataRequest(){
    qDebug() << "  request data (DATA)" << this->host()->searchFragment()->identifier();
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/%1/JSON" ).arg( this->host()->structureFragment()->cid()), NetworkManager::DataRequest );
}

/**
 * @brief PropertyFragment::parseFormulaRequest
 * @param data
 * @return
 */
bool PropertyFragment::parseFormulaRequest( const QByteArray &data ) {
    if ( !data.isEmpty()) {
        qDebug() << "    network->formula (DATA)" << this->host()->searchFragment()->identifier();
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
        qDebug() << "    network->data (DATA)" << this->host()->searchFragment()->identifier();
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
    qDebug() << "  getDataAndFormula";

    // clear old data
    this->ui->propertyView->setRowCount( 0 );
    this->ui->actionAddAll->setDisabled( true );
    this->ui->actionAddSelected->setDisabled( true );

    // validate id
    if ( id <= 0 ) {
        this->host()->setStatusMessage( PropertyFragment::tr( "Error: could not get data for requested reagent" ));
        return false;
    }

    // update status
    this->host()->setStatusMessage( PropertyFragment::tr( "Fetching data (properties and formula)" ));

    // get formula
    if ( Cache::instance()->contains( Cache::FormulaContext, QString( "%1.png" ).arg( id ))) {
        qDebug() << "    cache->formula (DATA)" << this->host()->searchFragment()->identifier();
        this->readFormula( Cache::instance()->getData( Cache::FormulaContext, QString( "%1.png" ).arg( id )));
    } else {
        // formula not in the cache, fetch it
        this->sendFormulaRequest();
    }

    // get data
    if ( Cache::instance()->contains( Cache::DataContext, QString( "%1.dat" ).arg( id ))) {
        qDebug() << "    cache->data (DATA)" << this->host()->searchFragment()->identifier();
        this->readData( Cache::instance()->getData( Cache::DataContext, QString( "%1.dat" ).arg( id ), true ));
    } else {
        // data not in the cache, fetch it
        this->sendDataRequest();
    }

    return true;
}

/**
 * @brief PropertyFragment::readData
 * @param uncompressed
 */
void PropertyFragment::readData( const QByteArray &uncompressed ) {
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

    // get selected tags
    const QList<Id> selectedTags = ListUtils::toNumericList<Id>( Variable::value<QStringList>( "propertyFragment/selectedTags" ));

    // get properties and fill table widget
    QMap<QString, PropertyWidget*>propList;
    for ( int y = 0; y < Tag::instance()->count(); y++ ) {
        const auto row = static_cast<Row>( y );

        // check for selected tags for extraction
        if ( !selectedTags.isEmpty()) {
            const Id id( Tag::instance()->id( row ));

            // not in the list? - continue
            if ( !selectedTags.contains( id ))
                continue;
        }

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

            auto *group( new PropertyWidget( nullptr, values, Tag::instance()->id( row )));
            //propList[Tag::instance()->name( row )] = group;
            propList[QApplication::translate( "Tag", Tag::instance()->name( row ).toUtf8().constData())] = group;
            continue;
        }

        if ( Tag::instance()->type( row ) == Tag::PubChemId ) {
            const QString cid( QString::number( this->host()->structureFragment()->cid()));
            const QList<QStringList> list = QList<QStringList>() << ( QStringList() << cid << cid );
            auto *group( new PropertyWidget( nullptr, list, Tag::instance()->id( row )));
            propList[QApplication::translate( "Tag", Tag::instance()->name( row ).toUtf8().constData())] = group;
        }
    }

    int row = this->ui->propertyView->rowCount();
    const QStringList propListKeys( propList.keys());
    this->ui->propertyView->setRowCount( this->ui->propertyView->rowCount() + propListKeys.count());

    // add properties and their names
    for ( const QString &name : propListKeys ) {
        this->ui->propertyView->setItem( row, 0, new QTableWidgetItem( name ));
        this->ui->propertyView->setCellWidget( row, 1, propList[name] );
        row++;
    }

    // add PubChemId
   // this->ui->propertyView->setRowCount( this->ui->propertyView->rowCount() + 1 );
   // this->ui->propertyView->setItem( row, 0, new QTableWidgetItem( name ));
   // //this->ui->propertyView->setCellWidget( row, 1, propList[name] );

    // resize content
    this->ui->propertyView->resizeRowsToContents();
    this->ui->propertyView->resizeColumnsToContents();

    // resize to 40% of screen height
    QScreen *screen( this->window()->windowHandle()->screen());
    this->ui->propertyView->setFixedHeight( static_cast<int>( screen->geometry().height() * 0.4 ));
    this->host()->adjustSize();

    // enable action
    if ( this->host()->reagentId() != Id::Invalid )
        this->ui->actionAddAll->setEnabled( propListKeys.count() > 0 );

    // clear message (success!)
    this->host()->clearStatusMessage();
}

/**
 * @brief PropertyFragment::readFormula
 * @param data
 */
void PropertyFragment::readFormula( const QByteArray &data ) {
    // get selected tags
    const QList<Id> selectedTags = ListUtils::toNumericList<Id>( Variable::value<QStringList>( "propertyFragment/selectedTags" ));
    for ( int y = 0; y < Tag::instance()->count(); y++ ) {
        const auto row = static_cast<Row>( y );

        // ignore tags that are not of Formula type
        if ( Tag::instance()->type( row ) != Tag::Formula )
            continue;

        // check if formula tag is enabled
        if ( !selectedTags.isEmpty()) {
            const Id id( Tag::instance()->id( row ));

            // not in the list? - continue
            if ( !selectedTags.contains( id ))
                return;
        }
    }

    QPixmap pixmap;
    if ( !pixmap.loadFromData( data ))
        return;

    if ( pixmap.isNull())
        return;

    const QPixmap cropped( PixmapUtils::cropAndRemoveAlpha( qAsConst( pixmap ), QColor::fromRgb( 245, 245, 245, 255 )));
    const int rows = this->ui->propertyView->rowCount();
    this->ui->propertyView->setRowCount( rows + 1 );
    this->ui->propertyView->setItem( rows, 0, new QTableWidgetItem( ExtractionDialog::tr( "Structural formula" )));
    this->ui->propertyView->setCellWidget( rows, 1, new PropertyWidget( nullptr, cropped ));
    this->ui->propertyView->resizeRowToContents( rows );
}

/**
 * @brief PropertyFragment::keyPressEvent
 * @param event
 */
void PropertyFragment::keyPressEvent( QKeyEvent *event ) {
    if (( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ) && this->host()->fragmentHost()->currentWidget() == this )
        this->ui->actionAddAll->trigger();

    Fragment::keyReleaseEvent( event );
}
