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
#include <QSqlError>
#include "propertydialog.h"
#include "ui_propertydialog.h"
#include "propertydelegate.h"
#include "template.h"
#include "property.h"
#include "database.h"
#include "extractiondialog.h"

/**
 * @brief PropertyDialog::PropertyDialog
 * @param parent
 */
PropertyDialog::PropertyDialog( QWidget *parent, const Row &id ) :
    QMainWindow( parent ),
    ui( new Ui::PropertyDialog ),
    templateRow( id ),
    editor( new PropertyEditor( this )) {
    int y;

    // set up ui
    this->ui->setupUi( this );

    // set up property table
    this->ui->propertyView->setModel( Property_N::instance());

    // hide unwanted columns
    for ( y = 0; y < Property_N::instance()->columnCount(); y++ ) {
        if ( y != Property_N::Name && y != Property_N::HTML )
            this->ui->propertyView->hideColumn( y );
    }

    // set view delegate
    this->ui->propertyView->setItemDelegate( new PropertyDelegate( this->ui->propertyView ));
    this->ui->propertyView->resizeColumnsToContents();
    this->ui->propertyView->resizeRowsToContents();
    this->ui->propertyView->setTextElideMode( Qt::ElideRight );

    // set up close button lambda
    this->connect( this->ui->closeButton, &QPushButton::clicked, [ this ]() {
        this->close();
    } );


    // connect property editor
    this->connect( this->editor, &PropertyEditor::accepted, [ this ]( PropertyEditor::Modes mode, const QString &name, const QString &html ) {
        // return if invalid template
        if ( this->templateRow == Row::Invalid )
            return;

        const Id id = Template_N::instance()->id( this->templateRow );
        if ( id == Id::Invalid )
            return;

        // TODO: display error is message bar
        if ( name.isEmpty() || html.isEmpty())
            return;

        switch ( mode ) {
        case PropertyEditor::Add:
        {
            // TODO: check of duplicates (is it really necessary at all?)
            if ( Property_N::instance()->add( name, html, id ) == Row::Invalid )
                return;

            this->resetView();
        }
            break;

        case PropertyEditor::Edit:
        {
            const Row row( this->current());
            if ( row == Row::Invalid )
                return;

            Property_N::instance()->setName( row, name );
            Property_N::instance()->setHTML( row, html );

            this->resetView();
        }
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
        const Row row( this->current());
        if ( row == Row::Invalid )
            return;

        this->editor->open( PropertyEditor::Edit, Property_N::instance()->name( row ), Property_N::instance()->html( row ));
    } );

    // connect remove action
    this->connect( this->ui->actionRemove, &QAction::triggered, [ this ]() {
        const Row row( this->current());
        if ( row == Row::Invalid )
            return;

        // remove property from database
        Property_N::instance()->remove( row );

        // update table
        // TODO/FIXME: do we need this?
        this->resetView();
    } );

    // connect up action
    this->connect( this->ui->actionUp, &QAction::triggered, [ this ]() {
        this->move( Up );
    } );

    // connect down action
    this->connect( this->ui->actionDown, &QAction::triggered, [ this ]() {
        this->move( Down );
    } );

    // connect wiki extraction action
    this->connect( this->ui->actionWiki, &QAction::triggered, [ this ]() {
        ExtractionDialog ed;

        ed.setTemplateRow( this->templateRow );
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
    // TODO: implement actual sorting in model
#if 0
    Property *p0, *p1;

    const QModelIndex currentIndex( this->ui->propertyView->currentIndex());

    const PropertyModel *model( qobject_cast<PropertyModel*>( this->ui->propertyView->model()));
    if ( model == nullptr )
        return;

    p0 = Property::fromId( model->data( currentIndex, Qt::UserRole ).toInt());
    if ( p0 == nullptr )
        return;

    const QModelIndex swap( direction == Up ?
                                model->index( currentIndex.row() - 1, currentIndex.column()) :
                                model->index( currentIndex.row() + 1, currentIndex.column()));

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
#endif
    Q_UNUSED( direction )
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
Row PropertyDialog::current() {
    const QModelIndex index( this->ui->propertyView->currentIndex());

    // make sure template entry is valid
    if ( this->templateRow == Row::Invalid )
        return Row::Invalid;

    // return property
    return Property_N::instance()->row( index );
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
    this->ui->propertyView->resizeRowsToContents();
    this->ui->propertyView->resizeColumnsToContents();
}
