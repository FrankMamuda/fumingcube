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
#include "template.h"
#include "textedit.h"
#include "ui_extractiondialog.h"

//
// TODO: validate URL!!!!
//

/**
 * @brief ExtractionDialog::ExtractionDialog
 * @param parent
 */
ExtractionDialog::ExtractionDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::ExtractionDialog ),
    model( new ExtractionModel( this )), m_templateId( -1 ) {
    this->ui->setupUi( this );

    // connect network manager
    this->connect( NetworkManager::instance(), SIGNAL( finished( QString, NetworkManager::Type, QVariant, QByteArray )), this, SLOT( requestFinished( QString, NetworkManager::Type, QVariant, QByteArray )));

    // connect wiki extraction action
    this->connect( this->ui->extractButton, &QPushButton::clicked, [ this ]() {        
        //
        // FIXME: finished requests ARE NOT REMOVED PROPERLY
        //
        if ( this->ui->urlEdit->text().isEmpty()) {
            qCritical() << this->tr( "no url specified" );
            return;
        }

        NetworkManager::instance()->execute( this->ui->urlEdit->text(), NetworkManager::Properties );
    } );

    // connect buttonBox
    this->connect( this->ui->buttonBox, &QDialogButtonBox::accepted, [ this ]() {
        if ( this->templateId() == -1 ) {
            qCritical() << this->tr( "invalid template id" );
            return;
        }

        foreach ( const QModelIndex &index, this->ui->propertyView->selectionModel()->selectedIndexes()) {
            int row;
            QString title, value;

            row = index.row();
            if ( row < 0 || row >= this->properties.count())
                continue;

            // NOTE: ugly, but works
            const QString header( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head></head><body style=\"font-family:'Times New Roman'; font-size:11pt; font-weight:400; font-style:normal;\">\n<p>" );
            const QString footer( "</p></body></html>" );
            title = TextEdit::stripHTML( QString( "%1%2%3" ).arg( header ).arg( this->properties.at( row )).arg( footer ));
            value = TextEdit::stripHTML( QString( "%1%2%3" ).arg( header ).arg( this->values.at( row )).arg( footer ));

            Property::add( title, value, this->templateId());
        }
    } );

    // set model
    this->ui->propertyView->setModel( this->model );
}

/**
 * @brief ExtractionDialog::~ExtractionDialog
 */
ExtractionDialog::~ExtractionDialog() {
    this->disconnect( this->ui->extractButton, &QPushButton::clicked, this, nullptr );
    this->disconnect( this->ui->buttonBox, &QDialogButtonBox::accepted, this, nullptr );
    this->disconnect( NetworkManager::instance(), SIGNAL( finished( QString, NetworkManager::Type, QVariant, QByteArray )), this, SLOT( requestFinished( QString, NetworkManager::Type, QVariant, QByteArray )));
    delete this->ui;
    delete this->model;
}

/**
 * @brief ExtractionDialog::setTemplateId
 * @param id
 */
void ExtractionDialog::setTemplateId( int id ) {
    Reagent *reagent;
    Template *entry;
    QString url;

    // set id
    this->m_templateId = id;

    // set reagent url
    if ( this->templateId() >= 0 ) {
        entry = Template::fromId( this->templateId());
        if ( entry != nullptr ) {
            reagent = Reagent::fromId( entry->reagentId());
            if ( reagent != nullptr )
                 this->ui->urlEdit->setText( QString( "https://en.wikipedia.org/wiki/%1" ).arg( reagent->name()).replace( " ", "_" ));
        }
    }
}

/**
 * @brief ExtractionDialog::requestFinished
 * @param url
 * @param type
 * @param userData
 * @param data
 * @param error
 */
void ExtractionDialog::requestFinished( const QString &url, NetworkManager::Type type, const QVariant &userData, const QByteArray &data ) {
    Q_UNUSED( url )
    Q_UNUSED( userData )

    /*if ( error ) {
        qCritical() << this->tr( "error processing network request" );
        return;
    }*/

    switch ( type ) {
    case NetworkManager::Properties:
    {
        // we currently support only wikipedia
        QRegularExpression re( Ui::PatternWiki );
        re.setPatternOptions( QRegularExpression::DotMatchesEverythingOption );
        QRegularExpressionMatchIterator i = re.globalMatch( data );
        QStringList words;
        QStringList plainList;

        // clear previous entries
        this->properties.clear();
        this->values.clear();

        // capture all unnecessary html tags
        while ( i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString property, value, plain, plainValue;

            property = TextEdit::stripHTML( match.captured( 1 )).simplified();
            value = TextEdit::stripHTML( match.captured( 2 )).simplified();
            plain = property.remove( QRegExp( "<[^>]*>" ));
            plainValue = property.remove( QRegExp( "<[^>]*>" ));

            if ( plain.isEmpty() || !QString::compare( plain, "*" ) || !QString::compare( plain, "**" ) || plainValue.isEmpty())
                continue;

            this->properties << property;
            this->values << value;
            plainList << plain;
        }
        this->model->reset( plainList );
    }
        break;

    case NetworkManager::BasicProperties:
        break;

    case NetworkManager::NoType:
        qCritical() << this->tr( "unknown network request type" );
        return;
    }
}
