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
#include <QStringListModel>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "reagentdialog.h"
#include "database.h"
#include "messagedock.h"
#include "propertyeditor.h"
#include "propertydialog.h"
#include "xmltools.h"
#include "variable.h"
#include "reagent.h"
#include "template.h"
#include "property.h"

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ),
    messageDock( new MessageDock( this ))
{
    // set up ui
    this->ui->setupUi( this );

    // set up models
    this->ui->reagentCombo->setModel( Reagent::instance());
    this->ui->reagentCombo->setModelColumn( Reagent::Name );
    this->ui->templateCombo->setModel( Template::instance());
    this->ui->templateCombo->setModelColumn( Template::Name );

    // set up completer
    this->reagentCompleter = new QCompleter( Reagent::instance());
    this->reagentCompleter->setCompletionColumn( Reagent::Name );
    this->reagentCompleter->setCaseSensitivity( Qt::CaseInsensitive );
    this->reagentCompleter->setCompletionMode( QCompleter::PopupCompletion );
    this->ui->findEdit->setCompleter( this->reagentCompleter );

    // ui locker lambda
    auto uiLocker = [ this ]( bool enable ) {
        this->ui->templateCombo->setEnabled( enable );
        this->ui->molarMassEdit->setReadOnly( enable );
        this->ui->densityEdit->setReadOnly( enable );
        this->ui->assayEdit->setReadOnly( enable );
        this->ui->volumeEdit->setDisabled( enable );
        this->ui->densityEdit->setDisabled( enable );
        this->ui->actionEdit->setEnabled( enable );
        this->ui->actionRemove->setEnabled( enable );
        this->ui->actionProperties->setEnabled( enable );

        if ( enable ) {
            const QPixmap pixmap( ":/icons/lock" );

            if ( pixmap.isNull()) {
                this->ui->molarMassLock->setPixmap( pixmap );
                this->ui->densityLock->setPixmap( pixmap );
                this->ui->assayLock->setPixmap( pixmap );
            }
        }
    };

    // template change lambda
    auto templateChanged = [ this, uiLocker ]( const int index ) {
        // failsafe
        const Row row = Template::instance()->row( index );
        if ( row == Row::Invalid ) {
            Property::instance()->setFilter( "templateId=-1" );
            uiLocker( true );
            return;
        }

        if ( Template::instance()->state( row ) == Template::Solid ) {
            this->ui->massEdit->setScaledValue( Template::instance()->amount( row ));
            this->ui->densityEdit->setDisabled( true );
            this->ui->volumeEdit->setDisabled( true );
        } else if ( Template::instance()->state( row ) == Template::Liquid ) {
            this->ui->volumeEdit->setScaledValue( Template::instance()->amount( row ));
            this->ui->densityEdit->setEnabled( true );
            this->ui->volumeEdit->setEnabled( true );
        } else {
            return;
        }

        // filter properties
        Property::instance()->setFilter( QString( "templateId=%1" ).arg( static_cast<int>( Template::instance()->id( row ))));

        this->ui->molarMassEdit->setScaledValue( Template::instance()->molarMass( row ));
        this->ui->densityEdit->setScaledValue( Template::instance()->density( row ));
        this->ui->assayEdit->setScaledValue( Template::instance()->assay( row ));
    };
    this->connect( this->ui->templateCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), templateChanged );

    // reagent change lambda
    auto reagentChanged = [ this, uiLocker ]( const int index ) {
        // failsafe
        const Row row = Reagent::instance()->row( index );
        if ( row == Row::Invalid ) {
            Template::instance()->setFilter( "reagentId=-1" );
            uiLocker( true );
            return;
        }

        // filter templates
        Template::instance()->setFilter( QString( "reagentId=%1" ).arg( static_cast<int>( Reagent::instance()->id( row ))));

        if ( Template::instance()->count())
            this->ui->templateCombo->setCurrentIndex( 0 );
    };
    this->connect( this->ui->reagentCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), reagentChanged );

    // set up reagent completer
    auto setReagent = [ this ]( const QString &name ) {
        /*const QModelIndexList list( Reagent_N::instance()->match( Reagent_N::instance()->index( 0, 0 ), Reagent_N::Name, name, 1, Qt::MatchExactly ));*/
        int y;

        for ( y = 0; y < Reagent::instance()->count(); y++ ) {
            if ( !QString::compare( name, Reagent::instance()->name( Reagent::instance()->row( y )), Qt::CaseInsensitive )) {
                this->ui->reagentCombo->setCurrentIndex( y );
                break;
            }
        }

        this->ui->stackedWidget->setCurrentIndex( 0 );
    };
    this->connect( this->reagentCompleter, QOverload<const QString &>::of( &QCompleter::activated ), [ setReagent ]( const QString &name ) { setReagent( name ); } );
    this->connect( this->ui->findEdit, &QLineEdit::returnPressed, [ this, setReagent ] { setReagent( this->ui->findEdit->text()); } );

    // set up reagent find button
    this->connect( this->ui->findButton, &QPushButton::clicked, [ this ]  {
        this->ui->stackedWidget->setCurrentIndex( 1 );

        const Row row = Reagent::instance()->row( this->ui->reagentCombo->currentIndex());
        if ( row == Row::Invalid ) {
            this->ui->stackedWidget->setCurrentIndex( 0 );
            return;
        }

        this->ui->findEdit->setText( Reagent::instance()->name( row ));
        this->ui->findEdit->selectAll();
        this->ui->findEdit->setFocus();
    } );

    // set up clear button
    // FIXME: is this the intended behaviour?
    this->connect( this->ui->clearButton, &QPushButton::clicked, [ this ]  {
        this->ui->reagentCombo->setCurrentIndex( this->ui->reagentCombo->count() - 1 );
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
    foreach ( LineEdit *lineEdit, this->inputList ) {
        this->connect( lineEdit, &LineEdit::valueChanged, [ this, lineEdit ]() {
            this->calculate( static_cast<int>( lineEdit->mode()));
        } );
    }

    // fill mass field to trigger updates
    if ( this->ui->massEdit->text().isEmpty())
        this->ui->massEdit->setScaledValue( 1.0 );

    // focus on mass input by default
    this->ui->massEdit->setFocus();

    // trigger reagent & template change
    reagentChanged( this->ui->reagentCombo->currentIndex());

    // hide message dock
    this->messageDock->hide();
}

/**
 * @brief restoreIndexes
 */
void MainWindow::restoreIndexes() {
    // load previous indexes
    const int reagentIndex = Variable::instance()->integer( "ui_lastReagentIndex" );
    if ( reagentIndex >= 0 && reagentIndex < this->ui->reagentCombo->count())
        this->ui->reagentCombo->setCurrentIndex( reagentIndex );

    const int templateIndex = Variable::instance()->integer( "ui_lastTemplateIndex" );
    if ( templateIndex >= 0 && templateIndex < this->ui->templateCombo->count())
        this->ui->templateCombo->setCurrentIndex( templateIndex );

    // get template
    const Row row = Template::instance()->row( this->ui->templateCombo->currentIndex());
    if ( row == Row::Invalid )
        return;

    // restore previous value to either volume or mass field
    const qreal lastValue = Variable::instance()->decimalValue( "ui_lastValue" );
    if ( Template::instance()->state( row ) == Template::Liquid ) {
        this->ui->volumeEdit->setScaledValue( lastValue );
    } else if ( Template::instance()->state( row ) == Template::Solid ) {
        this->ui->massEdit->setScaledValue( lastValue );
    }
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    this->disconnect( this->ui->reagentCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, nullptr );
    this->disconnect( this->ui->templateCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, nullptr );

    foreach ( LineEdit *lineEdit, this->inputList )
        this->disconnect( lineEdit, SIGNAL( valueChanged()));

    this->disconnect( this->reagentCompleter, SLOT( activated( QString )));
    this->disconnect( this->ui->findEdit, SLOT( returnPressed()));
    this->disconnect( this->ui->findButton, SLOT( clicked( bool )));
    this->disconnect( this->ui->clearButton, SLOT( clicked( bool )));

    delete this->reagentCompleter;



    // connect for updates
    foreach ( LineEdit *lineEdit, this->inputList )
        this->disconnect( lineEdit, SIGNAL( valueChanged()));

    delete this->ui;
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
        if ( dialog.reagentRow() != Row::Invalid )
            this->ui->reagentCombo->setCurrentIndex( static_cast<int>( dialog.reagentRow()));
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

    // get reagent
    const Row row = Reagent::instance()->row( this->ui->reagentCombo->currentIndex());
    if ( row == Row::Invalid ) {
        this->messageDock->displayMessage( this->tr( "Cannot open edit dialog: reagent not selected" ), MessageDock::Warning, 3000 );
        return;
    }

    dialog.setReagentRow( Reagent::instance()->row( this->ui->reagentCombo->currentIndex()));
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

    // get template
    const Row row = Template::instance()->row( this->ui->templateCombo->currentIndex());
    if ( row == Row::Invalid )
        return;

    // store current value (volume or mass) as variable
    if ( Template::instance()->state( row ) == Template::Liquid ) {
        Variable::instance()->setDecimalValue( "ui_lastValue", this->ui->volumeEdit->scaledValue());
    } else if ( Template::instance()->state( row ) == Template::Solid ) {
        Variable::instance()->setDecimalValue( "ui_lastValue", this->ui->massEdit->scaledValue());
    }

    // write ot configuration
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
    // get reagent
    const Row row = Reagent::instance()->row( this->ui->reagentCombo->currentIndex());
    if ( row == Row::Invalid ) {
        this->messageDock->displayMessage( this->tr( "Cannot remove: reagent not selected" ), MessageDock::Warning, 3000 );
        return;
    }

    // request confirmation
    if ( QMessageBox::question( this, this->tr( "Confirm removal" ), this->tr( "Remove reagent '%1'" ).arg( Reagent::instance()->name( row )), QMessageBox::Yes | QMessageBox::No, QMessageBox::NoButton ) != QMessageBox::Yes )
        return;

    // remove reagent and its templates
    Reagent::instance()->remove( row );
    Template::instance()->removeOrphanedEntries();
}

/**
 * @brief MainWindow::on_actionProperties_triggered
 */
void MainWindow::on_actionProperties_triggered() {
    PropertyDialog *pd;

    // get template
    const Row row = Template::instance()->row( this->ui->templateCombo->currentIndex());
    if ( row == Row::Invalid ) {
        this->messageDock->displayMessage( this->tr( "Cannot display properties: template not selected" ), MessageDock::Warning, 3000 );
        return;
    }

    // display property dialog
    pd = new PropertyDialog( this, row );
    pd->setAttribute( Qt::WA_DeleteOnClose, true );
    pd->show();

    // HACK: this works for now (dirty rescaling html document)
    pd->resize( 400, 400 );
}
