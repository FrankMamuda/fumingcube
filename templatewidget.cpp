/*
 * Copyright (C) 2017 Factory #12
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
#include "templatewidget.h"
#include "ui_templatewidget.h"
#include "template.h"

// TODO: replace spinboxes with special QLineEdits

/**
 * @brief TemplateWidget::TemplateWidget
 * @param parent
 */
TemplateWidget::TemplateWidget( QWidget *parent ) : QWidget( parent ), ui( new Ui::TemplateWidget ) {
    // set up ui
    this->ui->setupUi( this );

    // update tabWidget on name change
    this->connect( this->ui->nameEdit, &QLineEdit::textChanged, [ this ]( const QString &name ) {
        emit this->nameChanged( name );
    } );

    // toggle density input
    this->connect<void( QComboBox::* )( int )>( this->ui->stateCombo, &QComboBox::currentIndexChanged, [ this ]( int index ) {
        this->changeState( index );
    } );
    this->changeState( this->ui->stateCombo->currentIndex());

    // properties button
    this->connect( this->ui->propsButton, &QPushButton::clicked, [ this ]() {
        qDebug() << "open props dialog";
    } );


    //
    // TODO: move these to LineEdit::setMode
    //
    this->ui->amountEdit->setMode( LineEdit::Amount );
    this->ui->amountEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(g|mg|kg|ul|ml|l|cm3|m3|cm³|m³)?[\\s|\\n]*$" );
    this->ui->amountEdit->setUnits( QString( "g,mg,kg,ml,ul,l,cm3,m3,cm³,m³" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 << 1 << 0.001 << 1000 << 1 << 1000000 << 1 << 1000000 );
    this->ui->amountEdit->setScaledValue( 1.0 );

    this->ui->densityEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(?:(mg|g|kg)\\s*\\/\\s*(ul|ml|l|cm3|m3|cm³|m³))?[\\s|\\n]*$" );
    this->ui->densityEdit->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );
    this->ui->densityEdit->setUnits( QString( "ml,ul,l,cm3,m3,cm³,m³" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 << 1 << 1000000 << 1 << 1000000, LineEdit::Secondary );
    this->ui->densityEdit->setMode( LineEdit::Density );
    this->ui->densityEdit->setScaledValue( 1.0 );

    this->ui->molarMassEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(?:(mg|g|kg)\\s*[\\/|·]\\s*(mmol|mol|kmol|mol\\?1))?[\\s|\n]*$" );
    this->ui->molarMassEdit->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );
    this->ui->molarMassEdit->setUnits( QString( "mol,mmol,kmol" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000, LineEdit::Secondary );
    this->ui->molarMassEdit->setMode( LineEdit::MolarMass );
    this->ui->molarMassEdit->setScaledValue( 18.0 );

    this->ui->assayEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(%)?[\\s|\\n]*$" );
    this->ui->assayEdit->setUnits( QString( "%," ).split( "," ), QList<qreal>() << 0.01 << 1 );
    this->ui->assayEdit->setMode( LineEdit::Assay );
    this->ui->assayEdit->setScaledValue( 1.0 );
}

/**
 * @brief TemplateWidget::~TemplateWidget
 */
TemplateWidget::~TemplateWidget() {
    delete this->ui;
}

/**
 * @brief TemplateWidget::changeState
 * @param state
 */
void TemplateWidget::changeState( int state ) {
    if ( state == static_cast<int>( Template::Solid )) {
        this->ui->densityEdit->setDisabled( true );
        this->ui->amountEdit->setCurrentUnits( "g" );
        this->ui->amountEdit->displayValue();
    } else {
        this->ui->densityEdit->setEnabled( true );
        this->ui->amountEdit->setCurrentUnits( "ml" );
        this->ui->amountEdit->displayValue();
    }
}
