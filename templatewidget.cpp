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
#include "propertydialog.h"
#include "reagentdialog.h"
#include "networkmanager.h"
#include "textedit.h"
#include "template.h"
#include "reagent.h"

/**
 * @brief TemplateWidget::TemplateWidget
 * @param parent
 */
TemplateWidget::TemplateWidget( QWidget *parent, const Row &row ) : QWidget( parent ), ui( new Ui::TemplateWidget ), templateRow( row ) {
    const QWidget *dialogParent( parent );

    // set up ui
    this->ui->setupUi( this );

    // update tabWidget on name change
    this->connect( this->ui->nameEdit, &QLineEdit::textChanged, [ this ]( const QString &name ) {
        emit this->nameChanged( name );
    } );

    // toggle density input  
    this->connect( this->ui->stateCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), [ this ]( int index ) {
        this->setState( static_cast<Template_N::State_N>( index ));
    } );
    this->setState( static_cast<Template_N::State_N>( this->ui->stateCombo->currentIndex()));

    // connect density and molar mass extraction buttons
    auto extractProperty = [ this, dialogParent ]( Properties property ) {
        bool ok;
        const ReagentDialog *dialog( qobject_cast<const ReagentDialog*>( dialogParent ));
        if ( dialog == nullptr )
            return;

        const QString url( QInputDialog::getText( this, this->tr( "Extract from Wikipedia" ), this->tr( "URL:" ), QLineEdit::Normal, QString( "https://en.wikipedia.org/wiki/%1" ).arg( dialog->name().replace( " ", "_" )), &ok ));
        if ( ok && !url.isEmpty())
            NetworkManager::instance()->execute( qAsConst( url ), NetworkManager::BasicProperties, property );
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
    this->ui->nameEdit->setText( this->templateRow == Row::Invalid ? "" :  Template_N::instance()->name( row ));
    this->ui->amountEdit->setScaledValue( this->templateRow == Row::Invalid ? 1.0 :  Template_N::instance()->amount( row ));
    this->ui->densityEdit->setScaledValue( this->templateRow == Row::Invalid ? 1.0 :  Template_N::instance()->density( row ));
    this->ui->molarMassEdit->setScaledValue( this->templateRow == Row::Invalid ? 18.0 : Template_N::instance()->molarMass( row ));
    this->ui->assayEdit->setScaledValue( this->templateRow == Row::Invalid ? 1.0 :  Template_N::instance()->assay( row ));
    this->ui->stateCombo->setCurrentIndex( this->templateRow == Row::Invalid ? static_cast<int>( Template_N::Solid ) : static_cast<int>(  Template_N::instance()->state( row )));
}

/**
 * @brief TemplateWidget::~TemplateWidget
 */
TemplateWidget::~TemplateWidget() {
    this->disconnect( this->ui->nameEdit, &QLineEdit::textChanged, this, nullptr );
    this->disconnect( this->ui->stateCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, nullptr );
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
        const QRegularExpressionMatch match( re.match( data ));

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
 * @param row
 * @return
 */
Row TemplateWidget::save( const Row &reagentRow ) {
    // check reagent
    const Id reagentId = Reagent_N::instance()->id( reagentRow );
    if ( reagentId == Id::Invalid ) {
        qCritical() << this->tr( "invalid reagent" );
        return Row::Invalid;
    }

    // check template
    if ( Template_N::instance()->id( this->templateRow ) == Id::Invalid ) {
        this->templateRow = Template_N::instance()->add( this->name(), this->amount(), this->density(), this->assay(), this->molarMass(), this->state(), reagentId );
    } else {
        Template_N::instance()->setName( this->templateRow, this->name());
        Template_N::instance()->setAmount( this->templateRow, this->amount());
        Template_N::instance()->setDensity( this->templateRow, this->density());
        Template_N::instance()->setMolarMass( this->templateRow, this->molarMass());
        Template_N::instance()->setAssay( this->templateRow, this->assay());
        Template_N::instance()->setState( this->templateRow, this->state());
    }

    return this->templateRow;
}

/**
 * @brief TemplateWidget::setState
 * @param state
 */
void TemplateWidget::setState( Template_N::State_N state ) {
    if ( state == static_cast<int>( Template_N::Solid )) {
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
