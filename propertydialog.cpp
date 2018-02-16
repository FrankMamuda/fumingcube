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
#include "propertydialog.h"
#include "ui_propertydialog.h"
#include "propertydelegate.h"
#include "propertymodel.h"
#include "template.h"
#include "property.h"
#include "database.h"
#include "extractiondialog.h"
#include <QDebug>
#include <QSqlError>

/**
 * @brief PropertyDialog::PropertyDialog
 * @param parent
 */
PropertyDialog::PropertyDialog( QWidget *parent, Template *t ) :
    QMainWindow( parent ),
    ui( new Ui::PropertyDialog ),
    templ( t ),
    editor( new PropertyEditor( this )) {

    // set up ui
    this->ui->setupUi( this );

    // set up property table
    PropertyModel *model = new PropertyModel( this, this->templ );
    model->setView( this->ui->propertyView );
    this->ui->propertyView->setModel( model );
    this->ui->propertyView->setItemDelegate( new PropertyDelegate( this ));
    this->ui->propertyView->resizeColumnsToContents();
    this->ui->propertyView->resizeRowsToContents();
    this->ui->propertyView->setTextElideMode( Qt::ElideRight );

    // set up close button lambda
    this->connect( this->ui->closeButton, &QPushButton::clicked, [ this ]() {
        this->close();
    } );

    // connect property editor
    this->connect( this->editor, &PropertyEditor::accepted, [ this ]( PropertyEditor::Modes mode, const QString &title, const QString &value ) {
        Property *property;

        // return if invalid template
        if ( this->templ == nullptr )
            return;

        // TODO: display error is message bar
        if ( title.isEmpty() || value.isEmpty())
            return;

        switch ( mode ) {
        case PropertyEditor::Add:
            // TODO: check of duplicates (is it really necessary at all?)
            property = Property::add( title, value, this->templ->id());
            if ( property == nullptr )
                return;

            this->resetView();
            break;

        case PropertyEditor::Edit:
            property = this->current();

            // TODO: nothing selected warning
            if ( this->current() == nullptr )
                return;

            property->setName( title );
            property->setHtml( value );
            this->resetView();
            break;

        case PropertyEditor::NoMode:
            break;
        }
    } );

    // connect add action
    this->connect( this->ui->actionAdd, &QAction::triggered, [ this ]() {
        this->editor->open( PropertyEditor::Add );
    } );

    // connect edit action
    this->connect( this->ui->actionEdit, &QAction::triggered, [ this ]() {
        Property *property;

        property = this->current();
        if ( property == nullptr )
            return;

        this->editor->open( PropertyEditor::Edit, property->title(), property->html());
    } );

    // connect remove action
    this->connect( this->ui->actionRemove, &QAction::triggered, [ this ]() {
        Property *property;
        QSqlQuery query;
        Template *templ;

        property = this->current();
        if ( property == nullptr )
            return;

        // remove property from database
        Database::instance()->propertyMap.remove( property->id());
        if ( !query.exec( QString( "delete from properties where id=%1" ).arg( property->id())))
            qCritical() << this->tr( "could not delete property, reason: '%1'" ).arg( query.lastError().text());

        // retrieve template from templateId
        templ = Template::fromId( property->templateId());
        if ( templ == nullptr )
            return;

        // remove property from template's propertyMap
        templ->propertyMap.remove( property->id());

        // update table
        this->resetView();
    } );

    // connect up action
    this->connect( this->ui->actionUp, &QAction::triggered, [ this ]() {
        //qDebug() << "move up not implemented yet";
        this->move( Up );
    } );

    // connect down action
    this->connect( this->ui->actionDown, &QAction::triggered, [ this ]() {
        this->move( Down );
    } );


    // connect wiki extraction action
    this->connect( this->ui->actionWiki, &QAction::triggered, [ this ]() {
        ExtractionDialog ed;
        ed.setTemplateId( this->templ->id());
        ed.exec();

        this->resetView();
    } );

    // up/down button enabler/disabler
    auto upDownCheck = [ this ] {
        const QModelIndex currentIndex( this->ui->propertyView->currentIndex());
        this->ui->actionUp->setEnabled( currentIndex.row() != -1 && currentIndex.row() != 0 );
        this->ui->actionDown->setEnabled( currentIndex.row() != -1 && currentIndex.row() != this->ui->propertyView->model()->rowCount() - 1 );
    };
    this->connect( this->ui->propertyView->selectionModel(), &QItemSelectionModel::currentRowChanged, [ upDownCheck ]() { upDownCheck(); } );
    upDownCheck();
}

/**
 * @brief PropertyDialog::move
 * @param direction
 */
void PropertyDialog::move( Directions direction ) {
    Property *p0, *p1;
    PropertyModel *model;
    const QModelIndex currentIndex( this->ui->propertyView->currentIndex());
    QModelIndex swap;

    model = qobject_cast<PropertyModel*>( this->ui->propertyView->model());
    if ( model == nullptr )
        return;

    p0 = Property::fromId( model->data( currentIndex, Qt::UserRole ).toInt());
    if ( p0 == nullptr )
        return;

    if ( direction == Up )
        swap = model->index( currentIndex.row() - 1, currentIndex.column());
    else
        swap = model->index( currentIndex.row() + 1, currentIndex.column());

    p1 = Property::fromId( model->data( swap, Qt::UserRole ).toInt());
    if ( p1 == nullptr )
        return;

    // do the actual reordering
    const int o0 = p0->order();
    const int o1 = p1->order();
    p0->setOrder( o1 );
    p1->setOrder( o0 );

    // reset model
    this->resetView();

    // reselect value
    this->ui->propertyView->setCurrentIndex( swap );
}

/**
 * @brief PropertyDialog::~PropertyDialog
 */
PropertyDialog::~PropertyDialog() {
    this->disconnect( this->ui->closeButton, &QPushButton::clicked, this, nullptr );
    this->disconnect( this->editor, &PropertyEditor::accepted, this, nullptr );
    this->disconnect( this->ui->actionAdd, &QAction::triggered, this, nullptr );
    this->disconnect( this->ui->actionEdit, &QAction::triggered, this, nullptr );
    this->disconnect( this->ui->actionRemove, &QAction::triggered, this, nullptr );
    this->disconnect( this->ui->actionUp, &QAction::triggered, this, nullptr );
    this->disconnect( this->ui->actionDown, &QAction::triggered, this, nullptr );
    this->disconnect( this->ui->actionWiki, &QAction::triggered, this, nullptr );

    delete this->ui;
    delete this->editor;
}

/**
 * @brief PropertyDialog::current
 * @return
 */
Property *PropertyDialog::current() {
    QModelIndex index( this->ui->propertyView->currentIndex());

    // make sure template entry is valid
    if ( this->templ == nullptr )
        return nullptr;

    // check bounds
    if ( index.row() < 0 || index.row() >= templ->propertyMap.count())
        return nullptr;

    // get property id from model and return the corresponding property
    return Property::fromId( qobject_cast<PropertyModel*>( this->ui->propertyView->model())->data( index, PropertyModel::PropertyIdRole ).toInt());
}

/**
 * @brief PropertyDialog::resizeEvent
 * @param event
 */
void PropertyDialog::resizeEvent( QResizeEvent *event ) {
    this->resetView();
    QMainWindow::resizeEvent( event );
}

/**
 * @brief PropertyDialog::resetView
 */
void PropertyDialog::resetView() {
    qobject_cast<PropertyModel*>( this->ui->propertyView->model())->reset();
    this->ui->propertyView->resizeRowsToContents();
    this->ui->propertyView->resizeColumnsToContents();
}
