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
        this->ui->densitySpin->setDisabled( true );
        this->ui->amountSpin->setSuffix( " g" );
    } else {
        this->ui->densitySpin->setEnabled( true );
        this->ui->amountSpin->setSuffix( " ml" );
    }
}
