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
ReagentDialog::ReagentDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::ReagentDialog ), newTab( new QToolButton( this )) {
    // set up ui
    this->ui->setupUi(this);

    // set up newTab button
    this->ui->tabWidget->setCornerWidget( this->newTab, Qt::TopLeftCorner );
    this->newTab->setCursor( Qt::ArrowCursor );
    this->newTab->setAutoRaise( true );
    this->newTab->setToolTip( this->tr( "Add additional template" ));
    // this->newTab->setIcon( QIcon( "..." ));
    QObject::connect( this->newTab, &QToolButton::clicked, [this]() {
        this->addNewTab();
    } );

    this->addNewTab();
}

/**
 * @brief ReagentDialog::addNewTab
 */
void ReagentDialog::addNewTab() {
    TemplateWidget *widget;
    int tabIndex;

    widget = new TemplateWidget( this );
    tabIndex = this->ui->tabWidget->addTab( widget, "" );
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

    reagent = Reagent::add( this->ui->nameEdit->text());
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
