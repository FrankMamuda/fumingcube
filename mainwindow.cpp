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
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "reagentdialog.h"
#include "database.h"

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ), signalMapper( new QSignalMapper( this )) {
    // set up ui
    this->ui->setupUi( this );

    // refill reagents and templates on database changes
    this->connect( Database::instance(), &Database::changed, [ this ]() {
        qDebug() << "fill";

        // clear reagent list
        this->ui->reagentCombo->clear();

        // add reagents
        this->ui->reagentCombo->addItem( "<None>", -1 );
        foreach ( Reagent *reagent, Database::instance()->reagentMap )
            this->ui->reagentCombo->addItem( reagent->name(), reagent->id());

        // add templates
        this->fillTemplates();
    } );

    // fill templates on reagent change
    this->connect<void( QComboBox::* )( int )>( this->ui->reagentCombo, &QComboBox::currentIndexChanged, [ this ]() {        
        bool enable;
        this->ui->reagentCombo->currentData( Qt::UserRole ).toInt() == -1 ? enable = false : enable = true;
        this->ui->templateCombo->setEnabled( enable );
        this->ui->molarMassEdit->setReadOnly( enable );
        this->ui->densityEdit->setReadOnly( enable );
        this->ui->assayEdit->setReadOnly( enable );

        // refill templates
        this->fillTemplates();
    } );

    // fill data on template change
    this->connect<void( QComboBox::* )( int )>( this->ui->templateCombo, &QComboBox::currentIndexChanged, [ this ]() {
        Template *entry = Template::fromId( this->ui->templateCombo->currentData( Qt::UserRole ).toInt());
        if ( entry == nullptr )
            return;

        this->ui->molarMassEdit->setScaledValue( entry->molarMass());
        this->ui->densityEdit->setScaledValue( entry->density());
        this->ui->assayEdit->setScaledValue( entry->assay() / 100.0 );
    } );

    // set up mass display
    this->ui->massEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(mg|g|kg)?[\\s|\\n]*$" );
    this->ui->massEdit->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );
    this->ui->massEdit->setMode( LineEdit::Mass );
    this->inputList << this->ui->massEdit;

    // set up molar mass display
    this->ui->molarMassEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(?:(mg|g|kg)\\s*[\\/|·]\\s*(mmol|mol|kmol|mol\\−1))?[\\s|\n]*$" );
    this->ui->molarMassEdit->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );
    this->ui->molarMassEdit->setUnits( QString( "mol,mmol,kmol" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000, LineEdit::Secondary );
    this->ui->molarMassEdit->setMode( LineEdit::MolarMass );
    this->inputList << this->ui->molarMassEdit;

    // set up mol display
    this->ui->molEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(mol|mmol|kmol)?[\\s|\\n]*$" );
    this->ui->molEdit->setUnits( QString( "mol,mmol,kmol" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );
    this->ui->molEdit->setMode( LineEdit::Mol );
    this->inputList << this->ui->molEdit;

    // set up density display
    this->ui->densityEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(?:(mg|g|kg)\\s*\\/\\s*(ul|ml|l|cm3|m3|cm³|m³))?[\\s|\\n]*$" );
    this->ui->densityEdit->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );
    this->ui->densityEdit->setUnits( QString( "ml,ul,l,cm3,m3,cm³,m³" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 << 1 << 1000000 << 1 << 1000000, LineEdit::Secondary );
    this->ui->densityEdit->setMode( LineEdit::Density );
    this->inputList << this->ui->densityEdit;

    // set up assay display
    this->ui->assayEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(%)?[\\s|\\n]*$" );
    this->ui->assayEdit->setUnits( QString( "%," ).split( "," ), QList<qreal>() << 0.01 << 1 );
    this->ui->assayEdit->setMode( LineEdit::Assay );
    this->inputList << this->ui->assayEdit;

    // set up 100% display
    this->ui->pureEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(mg|g|kg)?[\\s|\\n]*$" );
    this->ui->pureEdit->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );
    this->ui->pureEdit->setMode( LineEdit::Pure );
    this->inputList << this->ui->pureEdit;

    // set up volume display
    this->ui->volumeEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(ul|ml|l|cm3|m3|cm³|m³)?[\\s|\\n]*$" );
    this->ui->volumeEdit->setUnits( QString( "ml,ul,l,cm3,m3,cm³,m³" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 << 1 << 1000000 << 1 << 1000000 );
    this->ui->volumeEdit->setMode( LineEdit::Volume );
    this->inputList << this->ui->volumeEdit;

    // set some defaults
    if ( this->ui->molarMassEdit->text().isEmpty())
        this->ui->molarMassEdit->setScaledValue( 18.0 );
    if ( this->ui->densityEdit->text().isEmpty())
        this->ui->densityEdit->setScaledValue( 1.0 );
    if ( this->ui->assayEdit->text().isEmpty())
        this->ui->assayEdit->setScaledValue( 1.0 );

    // connect for updates
    this->connect( this->signalMapper, SIGNAL( mapped( int )), this, SLOT( calculate( int )));
    foreach ( LineEdit *lineEdit, this->inputList ) {
        this->signalMapper->setMapping( lineEdit, static_cast<int>( lineEdit->mode()) );
        this->connect( lineEdit, SIGNAL( valueChanged()), this->signalMapper, SLOT( map()));
    }

    // fill mass field to trigger updates
    if ( this->ui->massEdit->text().isEmpty())
        this->ui->massEdit->setScaledValue( 1.0 );

    // focus on mass input by default
    this->ui->massEdit->setFocus();
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    this->disconnect( this->signalMapper, SIGNAL( mapped( int )));
    delete this->signalMapper;

    // connect for updates
    foreach ( LineEdit *lineEdit, this->inputList )
        this->disconnect( lineEdit, SIGNAL( valueChanged()));

    delete this->ui;
}

/**
 * @brief MainWindow::fillTemplates
 * @param reagentId
 */
void MainWindow::fillTemplates() {
    Reagent *reagent;

    // clear template list
    this->ui->templateCombo->clear();

    // get current reagent
    reagent = Reagent::fromId( this->ui->reagentCombo->currentData( Qt::UserRole ).toInt());
    if ( reagent == nullptr )
        return;

    // add templates
    foreach ( Template *entry, reagent->templateList )
        this->ui->templateCombo->addItem( entry->name(), entry->id());
}

/**
 * @brief MainWindow::calculate
 */
void MainWindow::calculate( int mode ) {
    // connect for updates
    foreach ( LineEdit *lineEdit, this->inputList )
        lineEdit->blockSignals( true );

    // NOTE: ugly code, but works fine
    switch ( static_cast<LineEdit::Modes>( mode )) {
    case LineEdit::Mass:
        this->ui->pureEdit->setScaledValue( this->ui->massEdit->scaledValue() * this->ui->assayEdit->scaledValue());
        this->ui->molEdit->setScaledValue( this->ui->pureEdit->scaledValue() / this->ui->molarMassEdit->scaledValue());
        this->ui->volumeEdit->setScaledValue( this->ui->massEdit->scaledValue() * this->ui->densityEdit->scaledValue());
        break;

    case LineEdit::MolarMass:
        this->ui->molEdit->setScaledValue( this->ui->pureEdit->scaledValue() / this->ui->molarMassEdit->scaledValue());
        break;

    case LineEdit::Mol:
        this->ui->massEdit->setScaledValue( this->ui->molEdit->scaledValue() * this->ui->molarMassEdit->scaledValue());
        this->ui->pureEdit->setScaledValue( this->ui->massEdit->scaledValue() * this->ui->assayEdit->scaledValue());
        this->ui->volumeEdit->setScaledValue( this->ui->massEdit->scaledValue() * this->ui->densityEdit->scaledValue());
        break;

    case LineEdit::Density:
        this->ui->massEdit->setScaledValue( this->ui->volumeEdit->scaledValue() * this->ui->densityEdit->scaledValue());
        this->ui->pureEdit->setScaledValue( this->ui->massEdit->scaledValue() * this->ui->assayEdit->scaledValue());
        this->ui->molEdit->setScaledValue( this->ui->pureEdit->scaledValue() / this->ui->molarMassEdit->scaledValue());
        break;

    case LineEdit::Assay:
        this->ui->pureEdit->setScaledValue( this->ui->massEdit->scaledValue() * this->ui->assayEdit->scaledValue());
        this->ui->molEdit->setScaledValue( this->ui->pureEdit->scaledValue() / this->ui->molarMassEdit->scaledValue());
        break;

    case LineEdit::Pure:
        this->ui->massEdit->setScaledValue( this->ui->pureEdit->scaledValue() / this->ui->assayEdit->scaledValue());
        this->ui->volumeEdit->setScaledValue( this->ui->massEdit->scaledValue() * this->ui->densityEdit->scaledValue());
        this->ui->molEdit->setScaledValue( this->ui->pureEdit->scaledValue() / this->ui->molarMassEdit->scaledValue());
        break;

    case LineEdit::Volume:
        this->ui->massEdit->setScaledValue( this->ui->volumeEdit->scaledValue() * this->ui->densityEdit->scaledValue());
        this->ui->pureEdit->setScaledValue( this->ui->massEdit->scaledValue() * this->ui->assayEdit->scaledValue());
        this->ui->molEdit->setScaledValue( this->ui->pureEdit->scaledValue() / this->ui->molarMassEdit->scaledValue());
        break;

    case LineEdit::NoMode:
        return;
    }

    // connect for updates
    foreach ( LineEdit *lineEdit, this->inputList )
       lineEdit->blockSignals( false );
}

/**
 * @brief MainWindow::on_actionAdd_triggered
 */
void MainWindow::on_actionAdd_triggered() {
    ReagentDialog dialog( this );
    switch ( dialog.exec()) {
    case QDialog::Accepted:
        dialog.add();
        break;

    case QDialog::Rejected:
        break;
    }
}

/**
 * @brief MainWindow::on_actionEdit_triggered
 */
void MainWindow::on_actionEdit_triggered() {
    ReagentDialog dialog( this );
    Reagent *reagent = Reagent::fromId( this->ui->reagentCombo->currentData( Qt::UserRole ).toInt());
    if ( reagent == nullptr ) {
        QMessageBox::warning( this, this->tr( "Cannot open edit dialog" ), this->tr( "Reagent not selected" ));
        return;
    }

    //dialog.setReagent()

    //dialog.setReagent( )
    /*switch ( dialog.exec()) {
    case QDialog::Accepted:
        dialog.add();
        break;

    case QDialog::Rejected:
        break;
    }*/
}
