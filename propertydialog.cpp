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
#include <QMessageBox>
#include <QSqlError>
#include <QTextEdit>
#include "propertydialog.h"
#include "ui_propertydialog.h"
#include "propertydelegate.h"
#include "template.h"
#include "property.h"
#include "database.h"
#include "extractiondialog.h"
#include "nfpawidget.h"
#include "ghswidget.h"
#include "tagdialog.h"

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
    this->ui->propertyView->setModel( Property::instance());
    Property::instance()->sort( Property::Order, Qt::AscendingOrder );

    // hide unwanted columns
    for ( y = 0; y < Property::instance()->columnCount(); y++ ) {
        if ( y != Property::Name && y != Property::HTML )
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

        const Id id = Template::instance()->id( this->templateRow );
        if ( id == Id::Invalid )
            return;

        // TODO: display error is message bar
        if ( name.isEmpty() || html.isEmpty())
            return;

        switch ( mode ) {
        case PropertyEditor::Add:
            if ( Property::instance()->add( name, html, id ) == Row::Invalid )
                return;

            this->resetView();
            break;

        case PropertyEditor::Edit:
        {
            const Row row( this->current());
            if ( row == Row::Invalid )
                return;

            Property::instance()->setName( row, name );
            Property::instance()->setHTML( row, html );

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

        this->editor->open( PropertyEditor::Edit, Property::instance()->name( row ), Property::instance()->html( row ));
    } );

    // connect remove action
    this->connect( this->ui->actionRemove, &QAction::triggered, [ this ]() {
        const Row row( this->current());
        if ( row == Row::Invalid )
            return;

        // remove property from database
        Property::instance()->remove( row );

        // update table
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
    //this->connect( this->ui->propertyView->selectionModel(), &QItemSelectionModel::currentRowChanged, [ upDownCheck ]() { upDownCheck(); } );
    this->connect( this->ui->propertyView, SIGNAL( clicked( QModelIndex )), this, SLOT( buttonTest( QModelIndex )));
    this->buttonTest();

    // setup resizeTimer (delay resetView, to avoid too frequent updates)
    this->resizeTimer.setSingleShot( true );
    this->connect( &this->resizeTimer, &QTimer::timeout, [ this ]() {
        this->ui->stackedWidget->setCurrentIndex( 0 );
        //this->ui->propertyView->show();
        //this->ui->propertyView->setUpdatesEnabled( true );
        this->resetView();
    } );
}

/**
 * @brief PropertyDialog::move
 * @param direction
 */
void PropertyDialog::move( Directions direction ) {
    const bool up = direction == Up;

    // test integrity
    QSet<int> orderSet;
    bool reindex = false;
    int y;
    for ( y = 0; y < Property::instance()->count(); y++ ) {
        const int order = Property::instance()->order( Property::instance()->row( y ));
        if ( orderSet.contains( order )) {
            if ( QMessageBox::question( this, this->tr( "Corrupted order" ),
                                        this->tr( "Tasks have corrupted order. Perform reindexing? This cannot be undone." )) == QMessageBox::Yes ) {
                reindex = true;
            }
            break;
        } else {
            orderSet << order;
        }
    }

    // reindex tasks if requested
    if ( reindex ) {
        QList<Id> idList;

        // get id list
        for ( int y = 0; y < Property::instance()->count(); y++ )
            idList << Property::instance()->id( Property::instance()->row( y ));

        // reorder tasks accordint to id list
        y = 0;
        foreach ( const Id id, idList ) {
            Property::instance()->setOrder( Property::instance()->row( id ), y );
            y++;
        }
    }

    // get container pointer and order indexes
    QTableView *container( this->ui->propertyView );
    const QModelIndex index( container->currentIndex());
    const QModelIndex other( container->model()->index( container->currentIndex().row() + ( up ? -1 : 1 ), 0 ));

    if ( !index.isValid() || !other.isValid())
        return;

    // use ids in lookup (QPersistentModel index should work too?)
    const Id id0 = Property::instance()->id( Property::instance()->row( index ));
    const Id id1 = Property::instance()->id( Property::instance()->row( other ));
    const int order0 = Property::instance()->order( Property::instance()->row( index ));
    const int order1 = Property::instance()->order( Property::instance()->row( other ));

    // swap widgets (so that we don't do useless updates)
#if 0
    // doesn't work
    QWidget *widget( this->ui->propertyView->indexWidget( index ));
    QWidget *otherWidget( this->ui->propertyView->indexWidget( other ));
    this->ui->propertyView->setIndexWidget( index, otherWidget );
    this->ui->propertyView->setIndexWidget( other, widget );
#endif

    // swap order
    Property::instance()->setOrder( Property::instance()->row( id0 ), order1 );
    Property::instance()->setOrder( Property::instance()->row( id1 ), order0 );

    // sort and reset view
    Property::instance()->sort( Property::Order, Qt::AscendingOrder );
    this->resetView();

    // restore index and check up/down buttons
    const QModelIndex current( container->model()->index( static_cast<int>( Property::instance()->row( id0 )), 0 ));
    container->setCurrentIndex( current );
    container->setFocus();
    this->buttonTest( current );
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
    this->disconnect( this->ui->propertyView, SIGNAL( clicked( QModelIndex )));

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
    return Property::instance()->row( index );
}

/**
 * @brief PropertyDialog::resizeEvent
 * @param event
 */
void PropertyDialog::resizeEvent( QResizeEvent *event ) {
    //this->resetView();

    this->resizeTimer.start( 100 );
    //this->ui->propertyView->hide();
    //this->ui->thumbnail->setPixmap( this->ui->propertyView->grab( this->ui->propertyView->rect()));
    this->ui->stackedWidget->setCurrentIndex( 1 );

    QMainWindow::resizeEvent( event );
}

/**
 * @brief PropertyDialog::resetView
 */
void PropertyDialog::resetView() {
    dynamic_cast<PropertyDelegate*>( this->ui->propertyView->itemDelegate())->clearDocumentCache();
    this->setSpecialWidgets();
    this->ui->propertyView->resizeRowsToContents();
    this->ui->propertyView->resizeColumnsToContents();
    this->buttonTest();
}

/**
 * @brief PropertyDialog::on_actionTags_triggered
 */
void PropertyDialog::on_actionTags_triggered() {
    TagDialog *td;

    // display tag dialog
    td = new TagDialog( this );
    td->setAttribute( Qt::WA_DeleteOnClose, true );
    td->show();
}

/**
 * @brief PropertyDialog::setSpecialWidgets
 */
void PropertyDialog::setSpecialWidgets() {
    for ( int y = 0; y < Property::instance()->count(); y++ ) {
        const QModelIndex index( Property::instance()->index( y, Property::HTML ));
        const Row row = Property::instance()->row( y );
        const QString plainName( Property::instance()->name( row ).remove( QRegExp("<[^>]*>" )));
        QWidget *widget( this->ui->propertyView->indexWidget( index ));
        const QString html( Property::instance()->html( row ));
        bool hasWidget = widget != nullptr;

        auto setWidget = [ this, index, hasWidget, row, html ]( PropertyWidget *widget ) {
            if ( hasWidget && widget != nullptr ) {
                widget->update( html );
            } else {
                // set widget and make sure to delete it on close
                this->ui->propertyView->setIndexWidget( index, widget );
                widget->setAttribute( Qt::WA_DeleteOnClose, true );
            }
        };

        if ( plainName.contains( "NFPA 704" )) {
            NFPAWidget *nfpa( hasWidget ? dynamic_cast<NFPAWidget*>( widget ) : new NFPAWidget( html ));
            setWidget( nfpa );
        } else if ( plainName.contains( "GHS Pictograms" )) {
            GHSWidget *ghs( hasWidget ? dynamic_cast<GHSWidget*>( widget ) : new GHSWidget( html ));
            setWidget( ghs );
        }
    }
}

/**
 * @brief PropertyDialog::buttonTest
 */
void PropertyDialog::buttonTest( const QModelIndex &index ) {
    this->ui->actionUp->setEnabled( index.isValid() && index.row() != 0 );
    this->ui->actionDown->setEnabled( index.isValid() && index.row() != Property::instance()->count() - 1 );
    this->ui->actionEdit->setEnabled( index.isValid());
    this->ui->actionRemove->setEnabled( index.isValid());
}
