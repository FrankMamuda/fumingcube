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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "reagentdialog.h"
#include "database.h"

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ) {
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

        this->ui->molarMassEdit->setText( QString::number( entry->molarMass()));
        this->ui->densityEdit->setText( QString::number( entry->density()));
        this->ui->assayEdit->setText( QString::number( entry->assay()));
    } );

    // set up mass display
    this->ui->massEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(mg|g|kg)?[\\s|\\n]*$" );
    this->ui->massEdit->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );

    // set up molar mass display
    this->ui->molarMassEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(?:(mg|g|kg)\\s*[\\/|·]\\s*(mmol|mol|kmol|mol\\−1))?[\\s|\n]*$" );
    this->ui->molarMassEdit->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );
    this->ui->molarMassEdit->setUnits( QString( "mol,mmol,kmol" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000, LineEdit::Secondary );

    // set up mol display
    this->ui->molEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(mol|mmol|kmol)?[\\s|\\n]*$" );
    this->ui->molEdit->setUnits( QString( "mol,mmol,kmol" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );

    // set up density display
    this->ui->densityEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(?:(mg|g|kg)\\s*\\/\\s*(ul|ml|l|cm3|m3|cm³|m³))?[\\s|\\n]*$" );
    this->ui->densityEdit->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );
    this->ui->densityEdit->setUnits( QString( "ml,ul,l,cm3,m3,cm³,m³" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 << 1 << 1000000 << 1 << 1000000, LineEdit::Secondary );

    // set up assay display
    this->ui->assayEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(%)?[\\s|\\n]*$" );
    this->ui->assayEdit->setUnits( QString( "%," ).split( "," ), QList<qreal>() << 1 << 0.001 );

    // set up 100% display
    this->ui->pureEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(mg|g|kg)?[\\s|\\n]*$" );
    this->ui->pureEdit->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );

    // set up volume display
    this->ui->volumeEdit->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(ul|ml|l|cm3|m3|cm³|m³)?[\\s|\\n]*$" );
    this->ui->volumeEdit->setUnits( QString( "ml,ul,l,cm3,m3,cm³,m³" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 << 1 << 1000000 << 1 << 1000000 );

    // connect for updates
    this->connect( this->ui->massEdit, SIGNAL( textChanged( const QString & )), this, SLOT( calculate()));
    this->connect( this->ui->molarMassEdit, SIGNAL( textChanged( const QString & )), this, SLOT( calculate()));
    this->connect( this->ui->molEdit, SIGNAL( textChanged( const QString & )), this, SLOT( calculate()));
    this->connect( this->ui->densityEdit, SIGNAL( textChanged( const QString & )), this, SLOT( calculate()));
    this->connect( this->ui->assayEdit, SIGNAL( textChanged( const QString & )), this, SLOT( calculate()));
    this->connect( this->ui->pureEdit, SIGNAL( textChanged( const QString & )), this, SLOT( calculate()));
    this->connect( this->ui->volumeEdit, SIGNAL( textChanged( const QString & )), this, SLOT( calculate()));
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
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
void MainWindow::calculate() {
    qDebug() << "calc";// << qApp->focusWidget()->

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


// molar mass (g/mol kg/kmol g/kmol or none) \s*(\d+[\.|,]?\s*\d*)\s*(?:(mg|g|kg)\s*\/\s*(mmol|mol|kmol))?[\s|\n]*$
// mass (mg g kg) \s*(\d+[\.|,]?\s*\d*)\s*(mg|g|kg)?[\s|\n]*$
// density (mg/g/kg on ul/ml/L/cm3/m3)  \s*(\d+[\.|,]?\s*\d*)\s*(?:(mg|g|kg)\s*\/\s*(ul|ml|L|cm3|m3))?[\s|\n]*$
// assay (%) \s*(\d+[\.|,]?\s*\d*)\s*(%)?[\s|\n]*$


