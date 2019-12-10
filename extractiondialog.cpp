/*
 * Copyright (C) 2017-2018 Factory #12
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

//
// includes
//
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
#include <QStringListModel>
#include <QToolButton>
#include "tag.h"
#include "propertywidget.h"
#include "tag.h"

/**
 * @brief ExtractionDialog::ExtractionDialog
 * @param parent
 */
ExtractionDialog::ExtractionDialog( QWidget *parent, const Id &reagentId ) : QDialog( parent ), ui( new Ui::ExtractionDialog ), m_reagentId( reagentId ) {
    this->ui->setupUi( this );
    this->ui->propertyView->verticalHeader()->hide();

    // make cache dir
    this->m_path = QDir( QDir::homePath() + "/" + Main::Path + "/cache/" ).absolutePath();
    const QDir dir( this->path());
    if ( !dir.exists()) {
        dir.mkpath( dir.absolutePath());
        if ( !dir.exists())
            return;
    }

    this->ui->nameEdit->setText( Reagent::instance()->name( reagentId ));
    auto checkName = [ this ]() { this->ui->extractButton->setEnabled( !this->ui->nameEdit->text().isEmpty()); };
    this->ui->nameEdit->connect( this->ui->nameEdit, &QLineEdit::textChanged, checkName );
    checkName();

    auto addProperties = [ this ]( bool all = false ) {
        if ( all )
            this->ui->propertyView->selectAll();


        foreach ( const QModelIndex &index, this->ui->propertyView->selectionModel()->selectedRows()) {
            const int row = index.row();
            PropertyWidget *widget( qobject_cast<PropertyWidget *>( this->ui->propertyView->cellWidget( row, 1 )));
            if ( widget != nullptr ) {
                if ( widget->tagId() != Id::Invalid ) {
                    widget->add( this->reagentId());
                } else {
                    Row row = Row::Invalid;
                    const QPixmap pixmap( widget->pixmap());
                    if ( pixmap.isNull() || this->reagentId() == Id::Invalid )
                        return;

                    for ( int y = 0; y < Tag::instance()->count(); y++ ) {
                        const Row r = static_cast<Row>( y );
                        if ( Tag::instance()->type( r ) == Tag::Formula ) {
                            row = r;
                            break;
                        }
                    }

                    if ( row != Row::Invalid ) {
                        QByteArray bytes;
                        QBuffer buffer( &bytes );
                        buffer.open( QIODevice::WriteOnly );
                        pixmap.save( &buffer, "PNG" );
                        Property::instance()->add( this->tr( "Structural formula" ), Tag::instance()->id( row ), bytes, this->reagentId());
                    }
                }
            }
        }

        this->accept();
    };

    this->ui->addAllButton->connect( this->ui->addAllButton, &QPushButton::pressed, std::bind( addProperties, true ));
    this->ui->addSelectedButton->connect( this->ui->addSelectedButton, &QPushButton::pressed, addProperties );

    this->manager->connect( this->manager, &QNetworkAccessManager::finished, [ this ]( QNetworkReply *reply ) {
        if ( reply->error()) {
            this->ui->cidEdit->setText( this->tr( "Error: could not get a valid CID" ));
            return;
        }

        switch ( reply->request().attribute( QNetworkRequest::User ).toInt()) {
        case CIDRequest:
        {
            this->cidList << QString( reply->readAll()).split( "\n" );
            if ( this->cidList.isEmpty())
                return;

            const QString cid( this->cidList.first());
            this->ui->cidEdit->setText( this->tr( "Success: CID - %1" ).arg( cid ));
            this->request.setUrl( QUrl( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/%1/JSON" ).arg( cid )));
            this->request.setAttribute( QNetworkRequest::User, DataRequest );
            this->manager->get( this->request );

            this->getFormula( cid );
        }
            break;

        case DataRequest:
        {
            const QByteArray data( reply->readAll());
            const QByteArray compressedData( qCompress( data ));
            QFile file( this->cache());
            if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate )) {
                file.write( compressedData.constData(), compressedData.length());
                file.close();
            }

            this->readData( data );
        }
            break;

        case FormulaRequest:
        {
            const QByteArray data( reply->readAll());
            QFile file( this->cache() + ".png" );
            if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate )) {
                file.write( data.constData(), data.length());
                file.close();
            }

            this->readFormula( data );
        }
            break;
        }
    }
    );
}

/**
 * @brief ExtractionDialog::~ExtractionDialog
 */
ExtractionDialog::~ExtractionDialog() {
    this->manager->disconnect( this->manager, &QNetworkAccessManager::finished, this, nullptr );

    //qDeleteAll( this->widgetList );
    delete this->manager;
    delete this->ui;
}

/**
 * @brief ExtractionDialog::readData
 * @param uncompressed
 * @return
 */
int ExtractionDialog::readData( const QByteArray &uncompressed ) {
    QMutexLocker lock( &this->mutex );

    // get cid
    int cid = -1;
    const QRegularExpression re( "\"RecordNumber\":\\s(\\d+)" );
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

            foreach ( const QString &key, object.keys()) {
                const QJsonValue keyValue( object.value( key ));

                if ( !QString::compare( key, "TOCHeading" )) {
                    if ( !keyValue.isArray() && !keyValue.isObject()) {
                        if ( !QString::compare( keyValue.toVariant().toString(), heading )) {
                            if ( object.keys().contains( "Information" )) {
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

            foreach ( const QJsonValue &arrayValue, array )
                findTag( arrayValue, heading, matches );
        }
    };

    /**
     * @brief extractValues gets string or numberic values from json
     */
    auto extractValues = []( const QList<QJsonArray> &matches, const QString &name, const QString &pattern, QList<QStringList> &out, bool global ) {
        QStringList values;

        foreach ( const QJsonArray &array, matches ) {
            foreach ( const QJsonValue &info, array ) {
                if ( info.isObject()) {
                    const QJsonObject infoObject( info.toObject());

                    if ( !name.isEmpty() && infoObject.keys().contains( "Name" )) {
                        const QJsonValue nameValue( infoObject["Name"] );
                        if ( !nameValue.isArray() && !nameValue.isObject()) {
                            if ( QString::compare( nameValue.toString(), name ))
                                continue;
                        }
                    }

                    if ( infoObject.keys().contains( "Value" )) {
                        const QJsonValue value( infoObject["Value"] );

                        if ( value.isObject()) {
                            const QJsonObject valueObject( value.toObject());
                            QString units;
                            if ( valueObject.keys().contains( "Unit" ))
                                units = valueObject["Unit"].toVariant().toString();

                            if ( valueObject.keys().contains( "Number" )) {
                                const QJsonValue numberTag( valueObject["Number"] );
                                if ( numberTag.isArray()) {
                                    const QJsonArray numberArray( numberTag.toArray());
                                    foreach ( const QJsonValue &number, numberArray )
                                        values << QString( "%1 %2" ).arg( number.toDouble()).arg( qAsConst( units ));
                                }
                            } else if ( valueObject.keys().contains( "StringWithMarkup" )) {
                                const QJsonValue stringTag( valueObject["StringWithMarkup"] );
                                if ( stringTag.isArray()) {

                                    const QJsonArray stringArray( stringTag.toArray());
                                    foreach ( const QJsonValue &stringValue, stringArray ) {
                                        if ( stringValue.isObject()) {
                                            const QJsonObject stringObject( stringValue.toObject());
                                            QStringList extra;
                                            if ( stringObject.keys().contains( "Markup" )) {
                                                const QJsonValue markupTag( stringObject["Markup"] );
                                                if ( markupTag.isArray()) {
                                                    const QJsonArray markupArray( markupTag.toArray());
                                                    if ( markupArray.count()) {
                                                        foreach ( const QJsonValue markupValue, markupArray ) {
                                                            if ( markupValue.isObject()) {
                                                                const QJsonObject markupObject( markupValue.toObject());

                                                                if ( markupObject.keys().contains( "Type" )) {
                                                                    const QJsonValue typeValue( markupObject["Type"] );
                                                                    if ( !typeValue.isArray() && !typeValue.isObject()) {
                                                                        if ( !QString::compare( typeValue.toString(), "PubChem Internal Link" )) {
                                                                            continue;
                                                                        }
                                                                    }
                                                                }

                                                                if ( markupObject.keys().contains( "Extra" )) {
                                                                    const QJsonValue extraValue( markupObject["Extra"] );
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

                                            if ( stringObject.keys().contains( "String" ))
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

            foreach ( const QString &value, values ) {
                QStringList captured;

                const QString stripped( QString( value ).remove( QRegularExpression( "\\(\\w+, \\d{4}\\)" )));
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
            foreach ( const QString &value, values )
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
        const Row row = static_cast<Row>( y );

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

            /* if ( Tag::instance()->type( row ) == Tag::GHS ) {
                qDebug() << values << global;

            }*/


            PropertyWidget *group( new PropertyWidget( nullptr, values, Tag::instance()->id( row )));
            propList[Tag::instance()->name( row )] = group;
            //this->widgetList << group;
        }
    }


    // TODO: delete items
    int row = this->ui->propertyView->rowCount();
    this->ui->propertyView->setRowCount( this->ui->propertyView->rowCount() + propList.keys().count());
    foreach ( const QString &name, propList.keys()) {
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

    // FIXME: ugly code
    auto autoCropPixmap = []( const QPixmap &pixmap ) {
        const QImage image( pixmap.toImage());
        const QColor key( QColor::fromRgb( 245, 245, 245, 255 ));

        // check left
        int left = 0, right = 0, top = 0, bottom = 0;
        for ( int x = 0; x < image.width(); x++ ) {
            bool found = false;

            for ( int y = 0; y < image.height(); y++ ) {
                if ( image.pixelColor( x, y ) != key ) {
                    left = x;
                    found = true;
                    break;
                }
            }
            if ( found )
                break;
        }

        // check right
        for ( int x = image.width() - 1; x >= 0; x-- ) {
            bool found = false;

            for ( int y = 0; y < image.height(); y++ ) {
                if ( image.pixelColor( x, y ) != key ) {
                    right = x;
                    found = true;
                    break;
                }
            }
            if ( found )
                break;
        }

        // find bottom
        for ( int y = image.height() - 1; y >= 0; y-- ) {
            bool found = false;

            for ( int x = 0; x < image.width(); x++ ) {
                if ( image.pixelColor( x, y ) != key ) {
                    bottom = y;
                    found = true;
                    break;
                }
            }
            if ( found )
                break;
        }

        // find bottom
        for ( int y = 0; y < image.height(); y++ ) {
            bool found = false;

            for ( int x = 0; x < image.width(); x++ ) {
                if ( image.pixelColor( x, y ) != key ) {
                    top = y;
                    found = true;
                    break;
                }
            }
            if ( found )
                break;
        }

        QImage cropped( image.copy( QRect( left, top, right - left + 1, bottom - top + 1 )));
        //cropped = cropped.convertToFormat( QImage::Format_ARGB32 ); does not look good
        for ( int x = 0; x < cropped.width(); x++ ) {
            for ( int y = 0; y < cropped.height(); y++ ) {
                if ( cropped.pixelColor( x, y ) == key ) {
                    cropped.setPixelColor( x, y, QColor::fromRgb( 255, 255, 255, 0 ));
                }
            }
        }

        return QPixmap::fromImage( qAsConst( cropped ));
    };

    const int rows = this->ui->propertyView->rowCount();
    this->ui->propertyView->setRowCount( rows + 1 );
    QPixmap pixmap;
    if ( !pixmap.loadFromData( data ))
        return;

    if ( pixmap.isNull())
        return;

    const QPixmap cropped( autoCropPixmap( qAsConst( pixmap )));
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
    if ( QFileInfo( this->cache() + ".png" ).exists()) {
        QFile file( this->cache() + ".png" );
        if ( file.open( QIODevice::ReadOnly )) {
            this->readFormula( file.readAll());
            file.close();
            return;
        }
    }

    if ( !cid.isEmpty()) {
        this->request.setUrl( QUrl( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/%1/PNG" ).arg( cid )));
        this->request.setAttribute( QNetworkRequest::User, FormulaRequest );
        this->manager->get( this->request );
    }
}

/**
 * @brief ExtractionDialog::on_extractButton_clicked
 */
void ExtractionDialog::on_extractButton_clicked() {
    this->ui->propertyView->clear();
    this->ui->propertyView->setRowCount( 0 );
    this->ui->propertyView->setColumnCount( 2 );

    this->m_cache = this->path() + "/" + QString( QCryptographicHash( QCryptographicHash::Md5 ).hash( this->ui->nameEdit->text().toUtf8().constData(), QCryptographicHash::Md5 ).toHex());
    if ( this->cache().isEmpty())
        return;

    if ( QFileInfo( this->cache()).exists()) {
        this->ui->cidEdit->setText( this->tr( "Reading from cache" ));

        QFile file( this->cache());
        if ( file.open( QIODevice::ReadOnly )) {
            const QByteArray uncompressedData( qUncompress( file.readAll()));
            const int cid = this->readData( uncompressedData );

            if  ( cid != -1 )
                this->getFormula( QString::number( cid ));

            file.close();
        }

        return;
    }

    // get data
    this->request.setUrl( QUrl( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/%1/cids/TXT?name_type=word" ).arg( this->ui->nameEdit->text().replace( " ", "-" ))));
    this->request.setAttribute( QNetworkRequest::User, CIDRequest );
    this->manager->get( this->request );
}
