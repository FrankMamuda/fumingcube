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
#include <QDebug>
#include <QInputDialog>
#include <ui_templatewidget.h>
#include "templatewidget.h"
#include "ui_templatewidget.h"
#include "template.h"
#include "propertydialog.h"
#include "reagentdialog.h"
#include "networkmanager.h"
#include "textedit.h"

/**
 * @brief TemplateWidget::TemplateWidget
 * @param parent
 */
TemplateWidget::TemplateWidget( QWidget *parent, Template *templateEntry ) : QWidget( parent ), ui( new Ui::TemplateWidget ), entry( templateEntry ) {
    QWidget *dialogParent = parent;

    // set up ui
    this->ui->setupUi( this );

    // update tabWidget on name change
    this->connect( this->ui->nameEdit, &QLineEdit::textChanged, [ this ]( const QString &name ) {
        emit this->nameChanged( name );
    } );

    // toggle density input
    this->connect<void( QComboBox::* )( int )>( this->ui->stateCombo, &QComboBox::currentIndexChanged, [ this ]( int index ) {
        this->setState( static_cast<Template::State>( index ));
    } );
    this->setState( static_cast<Template::State>( this->ui->stateCombo->currentIndex()));

    // connect density and molar mass extraction buttons
    auto extractProperty = [ this, dialogParent ]( Properties property ) {
        bool ok;
        QString url;
        ReagentDialog *dialog;

        dialog = qobject_cast<ReagentDialog*>( dialogParent );
        if ( dialog == nullptr )
            return;

        url = QInputDialog::getText( this, this->tr( "Extract from Wikipedia" ), this->tr( "URL:" ), QLineEdit::Normal, QString( "https://en.wikipedia.org/wiki/%1" ).arg( dialog->name().replace( " ", "_" )), &ok );
        if ( ok && !url.isEmpty()) {
            NetworkManager::instance()->execute( url, NetworkManager::BasicProperties, property );
        }        
    };
    this->connect( this->ui->densityButton, &QToolButton::clicked, [ extractProperty ]() { extractProperty( Density ); } );
    this->connect( this->ui->molarMassButton, &QToolButton::clicked, [ extractProperty ]() { extractProperty( MolarMass ); } );

    // connect network manager
    this->connect( NetworkManager::instance(), SIGNAL( finished( QString, NetworkManager::Type, QVariant, QByteArray )), this, SLOT( requestFinished( QString, NetworkManager::Type, QVariant, QByteArray )));

    // set up inputs
    this->ui->amountEdit->setMode( LineEdit::Amount );
    this->ui->densityEdit->setMode( LineEdit::Density );
    this->ui->molarMassEdit->setMode( LineEdit::MolarMass );
    this->ui->assayEdit->setMode( LineEdit::Assay );
    this->ui->nameEdit->setText( entry == nullptr ? "" : entry->name());
    this->ui->amountEdit->setScaledValue( entry == nullptr ? 1.0 : entry->amount());
    this->ui->densityEdit->setScaledValue( entry == nullptr ? 1.0 : entry->density());
    this->ui->molarMassEdit->setScaledValue( entry == nullptr ? 18.0 : entry->molarMass());
    this->ui->assayEdit->setScaledValue( entry == nullptr ? 1.0 : entry->assay());
    this->ui->stateCombo->setCurrentIndex( entry == nullptr ? static_cast<int>( Template::Solid ) : static_cast<int>( entry->state()));
}

/**
 * @brief TemplateWidget::~TemplateWidget
 */
TemplateWidget::~TemplateWidget() {
    this->disconnect( this->ui->nameEdit, &QLineEdit::textChanged, this, nullptr );
    this->disconnect<void( QComboBox::* )( int )>( this->ui->stateCombo, &QComboBox::currentIndexChanged, this, nullptr );
    this->disconnect( this->ui->densityButton, &QToolButton::clicked, this, nullptr );
    this->disconnect( this->ui->molarMassButton, &QToolButton::clicked, this, nullptr );
    this->disconnect( NetworkManager::instance(), SIGNAL( finished( QString, NetworkManager::Type, QVariant, QByteArray )), this, SLOT( requestFinished( QString, NetworkManager::Type, QVariant, QByteArray )));
    delete this->ui;
}

/**
 * @brief TemplateWidget::requestFinished
 * @param url
 * @param type
 * @param userData
 * @param data
 * @param error
 */
void TemplateWidget::requestFinished( const QString &url, NetworkManager::Type type, const QVariant &userData, const QByteArray &data ) {
    Q_UNUSED( url )
    Q_UNUSED( userData )

    /*if ( error ) {
        qCritical() << this->tr( "error processing network request" );
        return;
    }*/

    switch ( type ) {
    case NetworkManager::BasicProperties:
    {
        QRegularExpression re;

        switch ( static_cast<Properties>( userData.toInt())) {
        case Density:
            re.setPattern( "<td.*?(?=Density).*?(?=<td>)<td>(\\d+(?:.\\d+)?).*?(?=<\\/td>)" );
            break;

        case MolarMass:
            re.setPattern( "<td.*?(?=Molar mass).*?(?=<td>)<td>(\\d+(?:.\\d+)?).*?(?=<\\/td>)" );
            break;

        case NoProperty:
            return;
        }

        // we currently support only wikipedia
        re.setPatternOptions( QRegularExpression::DotMatchesEverythingOption );
        QRegularExpressionMatch match = re.match( data );

        // capture all unnecessary html tags
        if ( match.hasMatch()) {
            qreal value;
            bool ok;

            value = TextEdit::stripHTML( match.captured( 1 )).simplified().remove( QRegExp( "<[^>]*>" )).toDouble( &ok );
            if ( !ok )
                return;

            switch ( static_cast<Properties>( userData.toInt())) {
            case Density:
                this->ui->densityEdit->setScaledValue( value );
                break;

            case MolarMass:
                this->ui->molarMassEdit->setScaledValue( value );
                break;

            case NoProperty:
                return;
            }
        }
    }
        break;

    case NetworkManager::Properties:
        break;

    case NetworkManager::NoType:
        qCritical() << this->tr( "unknown network request type" );
        return;
    }
}

/**
 * @brief TemplateWidget::save
 */
int TemplateWidget::save( int id ) {
    if ( this->entry == nullptr ) {
        this->entry = Template::add( this->name(), this->amount(), this->density(), this->assay(), this->molarMass(), this->state(), id );
    } else {
        entry->setName( this->name());
        entry->setAmount( this->amount());
        entry->setDensity( this->density());
        entry->setMolarMass( this->molarMass());
        entry->setAssay( this->assay());
        entry->setState( this->state());
    }

    return ( entry == nullptr ? -1 : entry->id());
}

/**
 * @brief TemplateWidget::setState
 * @param state
 */
void TemplateWidget::setState( Template::State state ) {
    if ( state == static_cast<int>( Template::Solid )) {
        this->ui->densityEdit->setDisabled( true );
        this->ui->amountEdit->setCurrentUnits( "g" );
        this->ui->amountEdit->displayValue();
        this->ui->densityButton->setDisabled( true );
    } else {
        this->ui->densityEdit->setEnabled( true );
        this->ui->amountEdit->setCurrentUnits( "ml" );
        this->ui->amountEdit->displayValue();
        this->ui->densityButton->setEnabled( true );
    }
}
