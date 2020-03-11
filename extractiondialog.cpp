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
#include <QCryptographicHash>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "tag.h"
#include "propertywidget.h"
#include "imageutils.h"
#include "structurebrowser.h"
#include "pixmaputils.h"
#include "htmlutils.h"
#include "listutils.h"

/**
 * @brief ExtractionDialog::ExtractionDialog
 * @param parent
 */
ExtractionDialog::ExtractionDialog( QWidget *parent, const Id &reagentId, const int cid ) : QDialog( parent ), ui( new Ui::ExtractionDialog ), m_reagentId( reagentId ) {
    this->ui->setupUi( this );
    this->ui->propertyView->verticalHeader()->hide();

    NetworkManager::connect( NetworkManager::instance(), SIGNAL( finished( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )), this, SLOT( replyReceived( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )));
    NetworkManager::connect( NetworkManager::instance(), SIGNAL( error( const QString &, NetworkManager::Types, const QString & )), this, SLOT( error( const QString &, NetworkManager::Types, const QString & )));

    // make cache dir
    this->m_path = QDir( QDir::homePath() + "/" + Main::Path + "/cache/" ).absolutePath();
    const QDir dir( this->path());
    if ( !dir.exists()) {
        dir.mkpath( dir.absolutePath());
        if ( !dir.exists())
            return;
    }

    this->ui->nameEdit->setText( HTMLUtils::convertToPlainText( Reagent::instance()->name( reagentId )));

    // NOTE: this also re-enables extract button after unsuccessful query
    auto checkName = [ this ]() { this->ui->extractButton->setEnabled( !this->ui->nameEdit->text().isEmpty()); };
    QLineEdit::connect( this->ui->nameEdit, &QLineEdit::textChanged, checkName );
    checkName();

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
        this->generateCacheName();

        // begin data request immediately
        this->getFormula( QString::number( cid ));
        NetworkManager::instance()->execute(  QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/%1/JSON" ).arg( cid ), NetworkManager::DataRequest );
    }
}

/**
 * @brief ExtractionDialog::~ExtractionDialog
 */
ExtractionDialog::~ExtractionDialog() {
    NetworkManager::disconnect( NetworkManager::instance(), SIGNAL( finished( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )), this, SLOT( replyReceived( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )));
    NetworkManager::disconnect( NetworkManager::instance(), SIGNAL( error( const QString &, NetworkManager::Types, const QString & )), this, SLOT( error( const QString &, NetworkManager::Types, const QString & )));
    delete this->ui;
}

/**
 * @brief ExtractionDialog::readData
 * @param uncompressed
 * @return
 */
int ExtractionDialog::readData( const QByteArray &uncompressed ) const {
    QMutexLocker lock( &this->mutex );

    // get cid
    int cid = -1;
    const QRegularExpression re( R"("RecordNumber":\s(\d+))" );
    const QRegularExpressionMatch match( re.match( QString( uncompressed.constData())));
    if ( match.hasMatch())
        cid = match.captured( 1 ).toInt();

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
    return qAsConst( cid );
}

/**
 * @brief ExtractionDialog::readFormula
 * @param data
 */
void ExtractionDialog::readFormula( const QByteArray &data ) {    
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
}

/**
 * @brief ExtractionDialog::getFormula
 * @param cid
 */
void ExtractionDialog::getFormula( const QString &cid ) {
    // get formula
    if ( QFileInfo::exists( this->cache() + ".png" )) {
        QFile file( this->cache() + ".png" );
        if ( file.open( QIODevice::ReadOnly )) {
            this->readFormula( file.readAll());
            file.close();
            return;
        }
    }

    if ( !cid.isEmpty())
        NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/%1/PNG" ).arg( cid ), NetworkManager::FormulaRequest );
}

/**
 * @brief ExtractionDialog::getSimilar
 * @param cidListInt
 */
void ExtractionDialog::getSimilar( const QList<int> &cidListInt ) {
    StructureBrowser sb( cidListInt, this );
    if ( sb.exec() != QDialog::Accepted )
        return;

    const int cid = sb.cid();
    if ( cid == -1 )
        return;

    this->ui->cidEdit->setText( ExtractionDialog::tr( "Success: CID - %1" ).arg( cid ));
    NetworkManager::instance()->execute(  QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/%1/JSON" ).arg( cid ), NetworkManager::DataRequest );
    this->getFormula( QString::number( cid ));
}

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

    this->generateCacheName();
    if ( this->cache().isEmpty())
        return;

    if ( QFileInfo::exists( this->cache())) {
        this->ui->cidEdit->setText( ExtractionDialog::tr( "Reading from cache" ));

        QFile file( this->cache());
        if ( file.open( QIODevice::ReadOnly )) {
            const QByteArray uncompressedData( qUncompress( file.readAll()));
            const int cid = this->readData( uncompressedData );

            if  ( cid != -1 )
                this->getFormula( QString::number( cid ));

            file.close();
        }

        this->setStatus( Idle );
        this->buttonTest();

        return;
    }

    // get data
    //NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/%1/cids/TXT?name_type=word" ).arg( this->ui->nameEdit->text().replace( " ", "-" )), NetworkManager::CIDRequestInitial );
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/%1/cids/TXT" ).arg( this->ui->nameEdit->text().replace( " ", "-" )), NetworkManager::CIDRequestInitial );
}

/**
 * @brief ExtractionDialog::on_clearCacheButton_clicked
 */
void ExtractionDialog::on_clearCacheButton_clicked() {
    if ( this->ui->nameEdit->text().isEmpty())
        return;

    this->generateCacheName();
    if ( this->cache().isEmpty())
        return;

    const QFileInfo info( this->cache());
    if ( !info.exists() || !info.isFile())
        return;

    QFile::remove( this->cache());
}

/**
 * @brief ExtractionDialog::generateCacheName
 */
void ExtractionDialog::generateCacheName() {
    this->m_cache = this->path() + "/" + QString( QCryptographicHash::hash( this->ui->nameEdit->text().toUtf8().constData(), QCryptographicHash::Md5 ).toHex());
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
    {
        this->cidList << QString( data ).split( "\n" );
        this->cidList.removeAll( "" );

        if ( this->cidList.isEmpty()) {
            this->setStatus( Idle );
            this->buttonTest();

            return;
        }

        const QString cid( this->cidList.first());
        this->ui->cidEdit->setText( ExtractionDialog::tr( "Success: CID - %1" ).arg( cid ));
        NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/%1/JSON" ).arg( cid ), NetworkManager::DataRequest );
        this->getFormula( cid );
    }
        break;

    case NetworkManager::CIDRequestSimilar:
    {
        this->cidList << QString( data ).split( "\n" );
        this->cidList.removeAll( "" );

        if ( this->cidList.isEmpty()) {
            this->ui->nameEdit->setText( ExtractionDialog::tr( "Could not find the reagent" ));
            this->setStatus( Idle );
            this->buttonTest();

            return;
        }

        QFile file( this->cache() + ".cid" );
        if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate )) {
            file.write( data.constData(), data.length());
            file.close();
        }

        QList<int> cidListInt( ListUtils::toNumericList<int>( qAsConst( this->cidList )));
        cidListInt.removeAll( 0 );
        std::sort( cidListInt.begin(), cidListInt.end());
        cidListInt.erase( std::unique( cidListInt.begin(), cidListInt.end()), cidListInt.end());

        if ( cidListInt.count() == 1 ) {
            this->getFormula( QString::number( cidListInt.first()));
            NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/%1/JSON" ).arg( cidListInt.first()), NetworkManager::DataRequest );
        } else
            this->getSimilar( qAsConst( cidListInt ));
    }
        break;

    case NetworkManager::DataRequest:
    {
        const QByteArray compressedData( qCompress( data ));
        QFile file( this->cache());
        if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate )) {
            file.write( compressedData.constData(), compressedData.length());
            file.close();
        }

        this->readData( data );
        this->setStatus( Idle );
    }
        break;

    case NetworkManager::FormulaRequest:
    {
        QFile file( this->cache() + ".png" );
        if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate )) {
            file.write( data.constData(), data.length());
            file.close();
        }

        this->readFormula( data );
    }
        break;

    case NetworkManager::IUPACName:
    case NetworkManager::NoType:
    case NetworkManager::FormulaRequestBrowser:
    case NetworkManager::FavIcon:
        break;
    }

    this->buttonTest();
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
            if ( QFileInfo::exists( this->cache() + ".cid" )) {
                QFile file( this->cache() + ".cid" );
                if ( file.open( QIODevice::ReadOnly )) {
                    this->cidList = QString( file.readAll()).split( "\n" );
                    file.close();

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
