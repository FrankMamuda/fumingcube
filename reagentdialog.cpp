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
ReagentDialog::ReagentDialog( QWidget *parent, Modes mode ) : QDialog( parent ), ui( new Ui::ReagentDialog ), newTab( new QToolButton( this )), m_reagentRow( Row::Invalid ), messageDock( new MessageDock( this )) {
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
void ReagentDialog::addNewTab( const Row &templateRow )  {
    TemplateWidget *widget( new TemplateWidget( this, templateRow ));

    // check template
    // const Id templateId = Template_N::instance()->id( templateRow );
    const int tabIndex = this->ui->tabWidget->addTab( widget, templateRow == Row::Invalid ? "" : Template::instance()->name( templateRow ));
    this->ui->tabWidget->setCurrentWidget( widget );
    this->widgetList << widget;

    if ( this->widgetList.indexOf( widget ) == 0 ) {
        this->ui->tabWidget->setTabText( tabIndex, this->tr( "Default template" ));
        widget->setDefault();
    } else {
        this->connect( widget, &TemplateWidget::nameChanged, [ this, tabIndex ]( const QString &name ) {
            this->ui->tabWidget->setTabText( tabIndex, name );
        } );
    }

    if ( templateRow != Row::Invalid )
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
 * @brief ReagentDialog::add
 */
bool ReagentDialog::add() {
    int y;
    const QString name( this->ui->nameEdit->text());

    if ( Reagent::instance()->contains( Reagent::Name, name )) {
        this->messageDock->displayMessage( this->tr( "Reagent already exists in database" ), MessageDock::Error, 3000 );
        return false;
    }

    Row row = Reagent::instance()->add( name );
    if ( row == Row::Invalid )
        return false;

    Id id = Reagent::instance()->id( row );
    if ( id == Id::Invalid )
        return false;

    for ( y = 0; y < this->ui->tabWidget->count(); y++ ) {
        const TemplateWidget *widget( qobject_cast<TemplateWidget *>( this->ui->tabWidget->widget( y )));
        Template::instance()->add( widget->name(), widget->amount(), widget->density(), widget->assay(), widget->molarMass(), widget->state(), id );
    }

    this->m_reagentRow = row;
    return true;
}

/**
 * @brief ReagentDialog::edit
 */
bool ReagentDialog::edit() {
    QSqlQuery query;

    // failsafe
    if ( this->mode() != Edit || this->reagentRow() == Row::Invalid )
        return false;

    // get modified and newly added template ids from template widget
    for ( int y = 0; y < this->ui->tabWidget->count(); y++ )
        qobject_cast<TemplateWidget *>( this->ui->tabWidget->widget( y ))->save( this->reagentRow());

    // clean up - remove deleted template entries
    foreach ( const Id &id, idList ) {
        if ( id != Id::Invalid )
            Template::instance()->remove( Template::instance()->row( id ));
    }

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
 * @brief ReagentDialog::setReagentRow
 * @param row
 */
void ReagentDialog::setReagentRow( const Row &row ) {
    this->m_reagentRow = row;

    if ( row == Row::Invalid ) {
        qCritical() << this->tr( "editing invalid reagent" );
        return;
    }

    this->setMode( Edit );
    this->ui->nameEdit->setText( Reagent::instance()->name( this->reagentRow()));
    for ( int y = 0; y < Template::instance()->count(); y++ )
        this->addNewTab( Template::instance()->row( y ));
}

/**
 * @brief ReagentDialog::accept
 */
void ReagentDialog::accept() {
    bool success = false;
    const QString name( this->ui->nameEdit->text());

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
    if ( index == 0 )
        return;

    if ( this->reagentRow() != Row::Invalid ) {
        const QString tabName( this->ui->tabWidget->currentWidget()->windowTitle());
        const Row row = Template::instance()->row( index );

        if ( row == Row::Invalid )
            return;

        const QString templateName( Template::instance()->name( row ));

        if ( QMessageBox::question( this, this->tr( "Confirm removal" ), this->tr( "Remove template '%1'" ).arg( templateName ), QMessageBox::Yes | QMessageBox::No, QMessageBox::NoButton ) != QMessageBox::Yes )
            return;

        qDebug() << "beep bop, removing";
        TemplateWidget *widget( qobject_cast<TemplateWidget *>( this->ui->tabWidget->widget( index )));
        this->widgetList.removeOne( widget );
        this->ui->tabWidget->removeTab( index );
        idList << Template::instance()->id( row );
    }
}

/**
 * @brief ReagentDialog::resizeEvent
 * @param event
 */
void ReagentDialog::resizeEvent( QResizeEvent *event ) {
    QDialog::resizeEvent( event );
    this->messageDock->resize( this->width(), this->messageDock->height());
}

