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
#include <QButtonGroup>
#include <QCryptographicHash>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringListModel>
#include <QToolButton>
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
            PropertyValueWidget *widget( qobject_cast<PropertyValueWidget *>( this->ui->propertyView->cellWidget( row, 1 )));
            if ( widget != nullptr )
                widget->add( this->reagentId());
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

            const QString cid = this->cidList.first();
            this->ui->cidEdit->setText( this->tr( "Success: CID - %1" ).arg( cid ));
            this->request.setUrl( QUrl( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/%1/JSON" ).arg( cid )));
            this->request.setAttribute( QNetworkRequest::User, DataRequest );
            this->manager->get( this->request );
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
        }
    }
    );
}

/**
 * @brief ExtractionDialog::~ExtractionDialog
 */
ExtractionDialog::~ExtractionDialog() {
    this->manager->disconnect( this->manager, &QNetworkAccessManager::finished, this, nullptr );

    delete this->manager;
    delete this->ui;
}

/**
 * @brief ExtractionDialog::readData
 * @param uncompressed
 */
void ExtractionDialog::readData( const QByteArray &uncompressed ) {
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
            out << values;
        }
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

    int rows = 0;
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

            this->ui->propertyView->setRowCount( rows + 1 );

            PropertyValueWidget *group( new PropertyValueWidget( nullptr, values, Tag::instance()->id( row )));
            this->ui->propertyView->setItem( rows, 0, new QTableWidgetItem( Tag::instance()->name( row )));
            this->ui->propertyView->setCellWidget( rows, 1, group );

            rows++;
        }
    }

    this->ui->propertyView->resizeRowsToContents();
    //this->ui->propertyView->resizeColumnsToContents();
}

/**
 * @brief ExtractionDialog::on_extractButton_clicked
 */
void ExtractionDialog::on_extractButton_clicked() {
    this->m_cache = this->path() + "/" + QString( QCryptographicHash( QCryptographicHash::Md5 ).hash( this->ui->nameEdit->text().toUtf8().constData(), QCryptographicHash::Md5 ).toHex());
    if ( this->cache().isEmpty())
        return;

    if ( QFileInfo( this->cache()).exists()) {
        this->ui->cidEdit->setText( this->tr( "Reading from cache" ));

        QFile file( this->cache());
        if ( file.open( QIODevice::ReadOnly )) {
            const QByteArray uncompressedData( qUncompress( file.readAll()));
            this->readData( uncompressedData );

            file.close();
        }

        return;
    }


    this->request.setUrl( QUrl( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/%1/cids/TXT?name_type=word" ).arg( this->ui->nameEdit->text().replace( " ", "-" ))));
    //this->request.setUrl( QUrl( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/substance/name/%1/cids/TXT" ).arg( this->ui->nameEdit->text().replace( " ", "-" ))));
    this->request.setAttribute( QNetworkRequest::User, CIDRequest );
    this->manager->get( this->request );
}

/**
 * @brief PropertyValueWidget::PropertyValueWidget
 * @param parent
 * @param values
 * @param type
 */
PropertyValueWidget::PropertyValueWidget( QWidget *parent, const QList<QStringList> &values, const Id &tagId ) : QWidget( parent ) {
    this->m_tagId = tagId;
    if ( this->tagId() == Id::Invalid )
        return;

    const Tag::Types &type = Tag::instance()->type( tagId );

    this->left->setIcon( QIcon( ":/icons/left" ));
    this->right->setIcon( QIcon( ":/icons/right" ));
    this->left->setIconSize( QSize( 8, 16 ));
    this->right->setIconSize( QSize( 8, 16 ));
    this->layout->setContentsMargins( 0, 0, 0, 0 );
    this->layout->setSpacing( 0 );


    if ( values.isEmpty())
        return;

    this->ghs->setLinear();

    int index = 0;
    for ( int y = 0; y < values.count(); y++ ) {
        QStringList valueList( values.at( y ));
        if ( valueList.count() < 2 )
            continue;

        this->displayValues[index] = valueList.takeFirst();
        this->propertyValues[index] = valueList;
        index++;
    }

    if ( displayValues.isEmpty() || propertyValues.isEmpty())
        return;

    switch ( type ) {
    case Tag::Text:
    case Tag::Integer:
    case Tag::Real:
    case Tag::CAS:
        this->label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
        this->label->setWordWrap( true );
        this->label->setText( this->displayValues.first());
        this->layout->addWidget( this->label );
        break;

    case Tag::NFPA:
        this->layout->addWidget( this->nfpa );
        this->nfpa->setScale( 16 );
        this->nfpa->update( propertyValues.first());
        break;

    case Tag::GHS:
        this->layout->addWidget( this->ghs );
        this->ghs->update( PropertyValueWidget::parseGHS( this->propertyValues.first()));
        break;

    case Tag::State:
    case Tag::NoType:
        return;
    }

    this->m_position = 0;

    if ( this->displayValues.count() >= 2 ) {
        this->layout->addWidget( this->left );
        this->layout->addWidget( this->right );
        this->left->connect( this->left, &QToolButton::pressed, [ this, type ]() {
            if ( this->position() == -1 )
                return;

            if ( this->position() - 1 >= 0 )
                this->m_position--;
            else
                this->m_position = this->displayValues.count() - 1;

            switch ( type ) {
            case Tag::Text:
            case Tag::Integer:
            case Tag::Real:
            case Tag::CAS:
                this->label->setText( this->displayValues[this->position()] );
                break;

            case Tag::NFPA:
                this->nfpa->update( this->propertyValues[this->position()] );
                break;

            case Tag::GHS:
                this->ghs->update( PropertyValueWidget::parseGHS( this->propertyValues[this->position()] ));
                break;

            case Tag::State:
            case Tag::NoType:
                return;
            }
        } );
        this->right->connect( this->right, &QToolButton::pressed, [ this, type ]() {
            if ( this->position() == -1 )
                return;

            if ( this->position() + 1 < this->displayValues.count())
                this->m_position++;
            else
                this->m_position = 0;

            switch ( type ) {
            case Tag::Text:
            case Tag::Integer:
            case Tag::Real:
            case Tag::CAS:
                this->label->setText( this->displayValues[this->position()] );
                break;

            case Tag::NFPA:
                this->nfpa->update( this->propertyValues[this->position()] );
                break;

            case Tag::GHS:
                this->ghs->update( PropertyValueWidget::parseGHS( this->propertyValues[this->position()] ));
                break;

            case Tag::State:
            case Tag::NoType:
                return;
            }
        } );
    }

    this->setLayout( this->layout );
}

/**
 * @brief PropertyValueWidget::add
 */
void PropertyValueWidget::add( const Id &id ) {
    if ( id == Id::Invalid )
        return;

    switch ( Tag::instance()->type( this->tagId())) {
    case Tag::Text:
    case Tag::Integer:
    case Tag::Real:
    case Tag::CAS:
        Property::instance()->add( QString(), this->tagId(), this->propertyValues[this->position()].first(), id );
        break;

    case Tag::NFPA:
        Property::instance()->add( QString(), this->tagId(), this->propertyValues[this->position()].join( " " ), id );
        break;

    case Tag::GHS:
        Property::instance()->add( QString(), this->tagId(), PropertyValueWidget::parseGHS( this->propertyValues[this->position()] ).join( " " ), id );
        break;

    case Tag::State:
    case Tag::NoType:
        return;
    }
}

