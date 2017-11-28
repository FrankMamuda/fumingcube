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
#include "propertydialog.h"

/**
 * @brief TemplateWidget::TemplateWidget
 * @param parent
 */
TemplateWidget::TemplateWidget( QWidget *parent, Template *templateEntry ) : QWidget( parent ), ui( new Ui::TemplateWidget ), entry( templateEntry ) {
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

    // properties button
    this->connect( this->ui->propsButton, &QPushButton::clicked, [ this ]() {
        qDebug() << "open props dialog";
        PropertyDialog *props = new PropertyDialog( this );
        props->show();
    } );

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
    delete this->ui;
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
    } else {
        this->ui->densityEdit->setEnabled( true );
        this->ui->amountEdit->setCurrentUnits( "ml" );
        this->ui->amountEdit->displayValue();
    }
}
