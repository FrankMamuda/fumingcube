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
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "reagentdialog.h"
#include "database.h"
#include "messagedock.h"
#include "propertyeditor.h"
#include "propertydialog.h"
#include "xmltools.h"
#include "variable.h"

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ), signalMapper( new QSignalMapper( this )), messageDock( new MessageDock( this )) {
    // set up ui
    this->ui->setupUi( this );

    // refill reagents and templates on database changes
    this->connect( Database::instance(), &Database::changed, [ this ]() {
        // clear reagent list
        this->ui->reagentCombo->clear();

        // add reagents
        this->ui->reagentCombo->addItem( "<None>", -1 );
        foreach ( Reagent *reagent, Database::instance()->reagentMap )
            this->ui->reagentCombo->addItem( reagent->name(), reagent->id());

        // add templates
        this->fillTemplates();

        // load indexes
        this->restoreIndexes();
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

        if ( entry->state() == Template::Solid ) {
            this->ui->massEdit->setScaledValue( entry->amount());
            this->ui->densityEdit->setDisabled( true );
            this->ui->volumeEdit->setDisabled( true );
        } else if ( entry->state() == Template::Liquid ) {
            this->ui->volumeEdit->setScaledValue( entry->amount());
            this->ui->densityEdit->setEnabled( true );
            this->ui->volumeEdit->setEnabled( true );
        } else {
            return;
        }

        this->ui->molarMassEdit->setScaledValue( entry->molarMass());
        this->ui->densityEdit->setScaledValue( entry->density());
        this->ui->assayEdit->setScaledValue( entry->assay());
    } );

    // set up mass display
    this->ui->massEdit->setMode( LineEdit::Mass );
    this->inputList << this->ui->massEdit;

    // set up molar mass display
    this->ui->molarMassEdit->setMode( LineEdit::MolarMass );
    this->inputList << this->ui->molarMassEdit;

    // set up mol display
    this->ui->molEdit->setMode( LineEdit::Mol );
    this->inputList << this->ui->molEdit;

    // set up density display
    this->ui->densityEdit->setMode( LineEdit::Density );
    this->inputList << this->ui->densityEdit;

    // set up assay display
    this->ui->assayEdit->setMode( LineEdit::Assay );
    this->inputList << this->ui->assayEdit;

    // set up 100% display
    this->ui->pureEdit->setMode( LineEdit::Pure );
    this->inputList << this->ui->pureEdit;

    // set up volume display
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
 * @brief restoreIndexes
 */
void MainWindow::restoreIndexes() {
    int reagentIndex, templateIndex;

    // load previous indexes
    reagentIndex = Variable::instance()->integer( "ui_lastReagentIndex" );
    if ( reagentIndex >= 0 && reagentIndex < this->ui->reagentCombo->count())
        this->ui->reagentCombo->setCurrentIndex( reagentIndex );

    templateIndex = Variable::instance()->integer( "ui_lastTemplateIndex" );
    if ( templateIndex >= 0 && templateIndex < this->ui->templateCombo->count())
        this->ui->templateCombo->setCurrentIndex( templateIndex );
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    this->disconnect( Database::instance(), &Database::changed, this, nullptr );
    this->disconnect<void( QComboBox::* )( int )>( this->ui->reagentCombo, &QComboBox::currentIndexChanged, this, nullptr );
    this->disconnect<void( QComboBox::* )( int )>( this->ui->templateCombo, &QComboBox::currentIndexChanged, this, nullptr );
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
    foreach ( Template *entry, reagent->templateMap )
        this->ui->templateCombo->addItem( entry->name(), entry->id());
}

/**
 * @brief MainWindow::calculate
 */
void MainWindow::calculate( int mode ) {
    // connect for updates
    foreach ( LineEdit *lineEdit, this->inputList )
        lineEdit->blockSignals( true );

    // rather ugly code, but works fine
    auto pureFromMassAndAssay = [ this ]() { this->ui->pureEdit->setScaledValue( this->ui->massEdit->scaledValue() * this->ui->assayEdit->scaledValue()); };
    auto molFromPureAndMolarMass = [ this ]() { this->ui->molEdit->setScaledValue( this->ui->pureEdit->scaledValue() / this->ui->molarMassEdit->scaledValue()); };
    auto volumeFromMassAndDensity = [ this ]() { this->ui->volumeEdit->setScaledValue( this->ui->massEdit->scaledValue() / this->ui->densityEdit->scaledValue()); };
    auto massFromMolAndMolarMassAndAssay = [ this ]() { this->ui->massEdit->setScaledValue( this->ui->molEdit->scaledValue() * this->ui->molarMassEdit->scaledValue() / this->ui->assayEdit->scaledValue()); };
    auto massFromVolumeAndDensity = [ this ]() { this->ui->massEdit->setScaledValue( this->ui->volumeEdit->scaledValue() * this->ui->densityEdit->scaledValue()); };
    auto massFromPureAndAssay = [ this ]() { this->ui->massEdit->setScaledValue( this->ui->pureEdit->scaledValue() / this->ui->assayEdit->scaledValue()); };

    switch ( static_cast<LineEdit::Modes>( mode )) {
    case LineEdit::Mass:
        pureFromMassAndAssay();
        molFromPureAndMolarMass();
        volumeFromMassAndDensity();
        break;

    case LineEdit::MolarMass:
        molFromPureAndMolarMass();
        break;

    case LineEdit::Mol:
        massFromMolAndMolarMassAndAssay();
        pureFromMassAndAssay();
        volumeFromMassAndDensity();
        break;

    case LineEdit::Density:
        massFromVolumeAndDensity();
        pureFromMassAndAssay();
        molFromPureAndMolarMass();
        break;

    case LineEdit::Assay:
        pureFromMassAndAssay();
        molFromPureAndMolarMass();
        break;

    case LineEdit::Pure:
        massFromPureAndAssay();
        volumeFromMassAndDensity();
        molFromPureAndMolarMass();
        break;

    case LineEdit::Volume:
        massFromVolumeAndDensity();
        pureFromMassAndAssay();
        molFromPureAndMolarMass();
        break;

    case LineEdit::Amount:
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

    // save state
    Variable::instance()->setInteger( "ui_lastReagentIndex", this->ui->reagentCombo->currentIndex());
    Variable::instance()->setInteger( "ui_lastTemplateIndex", this->ui->templateCombo->currentIndex());

    if ( dialog.exec() == QDialog::Accepted ) {
        if ( dialog.reagentId() != -1 )
            this->ui->reagentCombo->setCurrentIndex( dialog.reagentId());
    }
}

/**
 * @brief MainWindow::on_actionEdit_triggered
 */
void MainWindow::on_actionEdit_triggered() {
    ReagentDialog dialog( this, ReagentDialog::Edit );

    // save state
    Variable::instance()->setInteger( "ui_lastReagentIndex", this->ui->reagentCombo->currentIndex());
    Variable::instance()->setInteger( "ui_lastTemplateIndex", this->ui->templateCombo->currentIndex());

    Reagent *reagent = Reagent::fromId( this->ui->reagentCombo->currentData( Qt::UserRole ).toInt());
    if ( reagent == nullptr ) {
        this->messageDock->displayMessage( this->tr( "Cannot open edit dialog: reagent not selected" ), MessageDock::Warning, 3000 );
        return;
    }

    dialog.setReagent( reagent );
    dialog.exec();
}

/**
 * @brief MainWindow::resizeEvent
 * @param event
 */
void MainWindow::resizeEvent( QResizeEvent *event ) {
    QMainWindow::resizeEvent( event );
    this->messageDock->resize( this->width(), this->messageDock->height());
}

/**
 * @brief MainWindow::closeEvent
 * @param event
 */
void MainWindow::closeEvent( QCloseEvent *event ) {
    // save settings
    Variable::instance()->setInteger( "ui_lastReagentIndex", this->ui->reagentCombo->currentIndex());
    Variable::instance()->setInteger( "ui_lastTemplateIndex", this->ui->templateCombo->currentIndex());
    XMLTools::instance()->write();

    // proceed
    QMainWindow::closeEvent( event );
}

/**
 * @brief MainWindow::on_actionRemove_triggered
 */
void MainWindow::on_actionRemove_triggered() {
    QSqlQuery query;

    // fetch current reagent
    Reagent *reagent = Reagent::fromId( this->ui->reagentCombo->currentData( Qt::UserRole ).toInt());
    if ( reagent == nullptr ) {
        this->messageDock->displayMessage( this->tr( "Cannot remove: reagent not selected" ), MessageDock::Warning, 3000 );
        return;
    }

    // request confirmation
    if ( QMessageBox::question( this, this->tr( "Confirm removal" ), this->tr( "Remove reagent '%1'" ).arg( reagent->name()), QMessageBox::Yes | QMessageBox::No, QMessageBox::NoButton ) != QMessageBox::Yes )
        return;

    // remove reagent and its templates
    Database::instance()->reagentMap.remove( reagent->id());
    if ( !query.exec( QString( "delete from reagents where id=%1" ).arg( reagent->id())) || !query.exec( QString( "delete from templates where reagentId=%1" ).arg( reagent->id())))
        qCritical() << this->tr( "could not delete reagent, reason: '%1'" ).arg( query.lastError().text());

    delete reagent;

    // update view
    Database::instance()->update();
}

/**
 * @brief MainWindow::on_actionProperties_triggered
 */
void MainWindow::on_actionProperties_triggered() {
    PropertyDialog *pd;

    // get current template
    Template *entry = Template::fromId( this->ui->templateCombo->currentData( Qt::UserRole ).toInt());
    if ( entry == nullptr ) {
        this->messageDock->displayMessage( this->tr( "Cannot display properties: template not selected" ), MessageDock::Warning, 3000 );
        return;
    }

    // display property dialog
    pd = new PropertyDialog( this, entry );
    pd->setAttribute( Qt::WA_DeleteOnClose, true );
    pd->show();

    // HACK: this works for now (dirty rescaling html document)
    pd->resize( 400, 400 );
}
