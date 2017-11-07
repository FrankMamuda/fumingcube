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
#include "reagentdialog.h"
#include "ui_reagentdialog.h"
#include "templatewidget.h"
#include "reagent.h"
#include "template.h"
#include "database.h"

/**
 * @brief ReagentDialog::ReagentDialog
 * @param parent
 */
ReagentDialog::ReagentDialog( QWidget *parent, Modes mode ) : QDialog( parent ), ui( new Ui::ReagentDialog ), newTab( new QToolButton( this )), reagent( nullptr ) {
    // set up ui
    this->ui->setupUi( this );

    // set up newTab button
    this->ui->tabWidget->setCornerWidget( this->newTab, Qt::TopLeftCorner );
    this->newTab->setCursor( Qt::ArrowCursor );
    this->newTab->setAutoRaise( true );
    this->newTab->setToolTip( this->tr( "Add additional template" ));
    this->newTab->setText( "+" );
    // this->newTab->setIcon( QIcon( "..." ));
    QObject::connect( this->newTab, &QToolButton::clicked, [this]() {
        this->addNewTab();
    } );

    this->setMode( mode );
}

/**
 * @brief ReagentDialog::addNewTab
 */
void ReagentDialog::addNewTab( Template *entry )  {
    TemplateWidget *widget;
    int tabIndex;

    widget = new TemplateWidget( this, entry );
    tabIndex = this->ui->tabWidget->addTab( widget, entry == nullptr ? "" : entry->name());
    this->ui->tabWidget->setCurrentWidget( widget );
    this->widgetList << widget;

    if ( this->widgetList.indexOf( widget ) == 0 ) {
        this->ui->tabWidget->setTabText( tabIndex, "Default template" );
        widget->setDefault();
    } else {
        this->connect( widget, &TemplateWidget::nameChanged, [ this, tabIndex ]( const QString &name ) {
            this->ui->tabWidget->setTabText( tabIndex, name );
        } );
    }

    if ( entry != nullptr )
        this->ui->tabWidget->setCurrentIndex( 0 );
}

/**
 * @brief ReagentDialog::~ReagentDialog
 */
ReagentDialog::~ReagentDialog() {
    qDeleteAll( this->widgetList );
    delete this->ui;
}

/**
 * @brief ReagentDialog::add
 */
void ReagentDialog::add() {
    int y;
    Reagent *reagent;
    QString name( this->ui->nameEdit->text());

    // TODO: catch these before close??
    if ( name.isEmpty()) {
        QMessageBox::warning( this, this->tr( "Cannot add reagent" ), this->tr( "No reagent name specified" ));
        return;
    }

    if ( Reagent::contains( name )) {
        QMessageBox::warning( this, this->tr( "Cannot add reagent" ), this->tr( "Reagent already exists in database" ));
        return;
    }

    reagent = Reagent::add( name );
    if ( reagent == nullptr )
        return;

    for ( y = 0; y < this->ui->tabWidget->count(); y++ ) {
        TemplateWidget *widget;

        widget = qobject_cast<TemplateWidget *>( this->ui->tabWidget->widget( y ));
        Template::add( widget->name(), widget->amount(), widget->density(), widget->assay(), widget->molarMass(), widget->state(), reagent->id());
    }

    Database::instance()->update();
}

/**
 * @brief ReagentDialog::edit
 */
void ReagentDialog::edit() {
    int y;
    QString name( this->ui->nameEdit->text());

    // failsafe
    if ( this->mode() != Edit || this->reagent == nullptr )
        return;

    // TODO: catch these before close??
    if ( name.isEmpty()) {
        QMessageBox::warning( this, this->tr( "Cannot add reagent" ), this->tr( "No reagent name specified" ));
        return;
    }

    /*
    NOTE: this is a little tricky since we have to both add new templates and edit existing ones
    one approach would be to delete all existing templates and add everythinh anew*/
    for ( y = 0; y < this->ui->tabWidget->count(); y++ ) {
        TemplateWidget *widget;
        widget = qobject_cast<TemplateWidget *>( this->ui->tabWidget->widget( y ));
        widget->save( reagent->id());
    }

    Database::instance()->update();
}

/**
 * @brief ReagentDialog::setMode
 * @param mode
 */
void ReagentDialog::setMode(ReagentDialog::Modes mode) {
    this->m_mode = mode;

    switch ( this->mode()) {
    case Add:
        this->addNewTab();
        break;

    case Edit:
    case NoMode:
        break;
    }
}

/**
 * @brief ReagentDialog::setReagent
 * @param reagent
 */
void ReagentDialog::setReagent(Reagent *reagent) {
    if ( reagent == nullptr )
        return;

    this->reagent = reagent;
    this->setMode( Edit );

    this->ui->nameEdit->setText( reagent->name());
    foreach ( Template *entry, reagent->templateList ) {
        this->addNewTab( entry );
    }
}

/**
 * @brief ReagentDialog::on_tabWidget_tabCloseRequested
 * @param index
 */
void ReagentDialog::on_tabWidget_tabCloseRequested( int index ) {
    TemplateWidget *widget;

    qDebug() << index;
    if ( index == 0 )
        return;

    widget = qobject_cast<TemplateWidget *>( this->ui->tabWidget->widget( index ));
    this->widgetList.removeOne( widget );
    this->ui->tabWidget->removeTab( index );
}
