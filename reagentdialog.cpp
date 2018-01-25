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
#include "reagentdialog.h"
#include "ui_reagentdialog.h"
#include "templatewidget.h"
#include "reagent.h"
#include "template.h"
#include "database.h"
#include "messagedock.h"

/**
 * @brief ReagentDialog::ReagentDialog
 * @param parent
 */
ReagentDialog::ReagentDialog( QWidget *parent, Modes mode ) : QDialog( parent ), ui( new Ui::ReagentDialog ), newTab( new QToolButton( this )), reagent( nullptr ), messageDock( new MessageDock( this )) {
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
        this->ui->tabWidget->setTabText( tabIndex, this->tr( "Default template" ));
        widget->setDefault();
    } else {
        QObject *object( new QObject( this ));
        this->connect( widget, &TemplateWidget::nameChanged, object, [ this, tabIndex, object ]( const QString &name ) {
            this->ui->tabWidget->setTabText( tabIndex, name );
            object->deleteLater();
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
 * @brief ReagentDialog::name
 * @return
 */
QString ReagentDialog::name() const {
    return this->ui->nameEdit->text();
}

/**
 * @brief ReagentDialog::reagentId
 * @return
 */
int ReagentDialog::reagentId() const {
    if ( this->reagent == nullptr )
        return -1;

    return this->reagent->id();
}

/**
 * @brief ReagentDialog::add
 */
bool ReagentDialog::add() {
    int y;
    Reagent *reagent;
    QString name( this->ui->nameEdit->text());

    if ( Reagent::contains( name )) {
        this->messageDock->displayMessage( this->tr( "Reagent already exists in database" ), MessageDock::Error, 3000 );
        return false;
    }

    reagent = Reagent::add( name );
    if ( reagent == nullptr )
        return false;

    for ( y = 0; y < this->ui->tabWidget->count(); y++ ) {
        TemplateWidget *widget;

        widget = qobject_cast<TemplateWidget *>( this->ui->tabWidget->widget( y ));
        Template::add( widget->name(), widget->amount(), widget->density(), widget->assay(), widget->molarMass(), widget->state(), reagent->id());
    }

    this->reagent = reagent;
    Database::instance()->update();
    return true;
}

/**
 * @brief ReagentDialog::edit
 */
bool ReagentDialog::edit() {
    int y;
    QString name( this->ui->nameEdit->text());
    QSqlQuery query;
    QList<int> idList;

    // failsafe
    if ( this->mode() != Edit || this->reagent == nullptr )
        return false;

    // get modified and newly added template ids from template widget
    for ( y = 0; y < this->ui->tabWidget->count(); y++ ) {
        TemplateWidget *widget;
        widget = qobject_cast<TemplateWidget *>( this->ui->tabWidget->widget( y ));
        idList << widget->save( reagent->id());
    }

    // clean up - remove deleted template entries
    foreach ( const int key, reagent->templateMap.keys()) {
        if ( idList.indexOf( key ) != -1 )
            continue;

        if ( !query.exec( QString( "delete from templates where id=%1" ).arg( key )))
             qCritical() << this->tr( "could not delete removed templates, reason: '%1'" ).arg( query.lastError().text());

        this->reagent->templateMap.remove( key );
    }

    Database::instance()->update();
    return true;
}

/**
 * @brief ReagentDialog::setMode
 * @param mode
 */
void ReagentDialog::setMode( ReagentDialog::Modes mode ) {
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
void ReagentDialog::setReagent( Reagent *reagent ) {
    if ( reagent == nullptr )
        return;

    this->reagent = reagent;
    this->setMode( Edit );

    this->ui->nameEdit->setText( reagent->name());
    foreach ( Template *entry, reagent->templateMap )
        this->addNewTab( entry );
}

/**
 * @brief ReagentDialog::accept
 */
void ReagentDialog::accept() {
    bool success = false;
    QString name( this->ui->nameEdit->text());

    // disallow empty names
    if ( name.isEmpty()) {
        this->messageDock->displayMessage( this->tr( "Reagent name not specified" ), MessageDock::Warning, 3000 );
        return;
    }

    switch ( this->mode()) {
    case Add:
        success = this->add();
        break;

    case Edit:
        success = this->edit();
        break;

    case NoMode:
        break;
    }

    if ( success )
        QDialog::accept();
}

/**
 * @brief ReagentDialog::on_tabWidget_tabCloseRequested
 * @param index
 */
void ReagentDialog::on_tabWidget_tabCloseRequested( int index ) {
    TemplateWidget *widget;

    if ( index == 0 )
        return;

    widget = qobject_cast<TemplateWidget *>( this->ui->tabWidget->widget( index ));
    this->widgetList.removeOne( widget );
    this->ui->tabWidget->removeTab( index );
}

/**
 * @brief ReagentDialog::resizeEvent
 * @param event
 */
void ReagentDialog::resizeEvent( QResizeEvent *event ) {
    QDialog::resizeEvent( event );
    this->messageDock->resize( this->width(), this->messageDock->height());
}

