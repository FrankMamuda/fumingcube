/*
 * Copyright (C) 2019 Armands Aleksejevs
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

/*
 * includes
 */
#include "reagentdock.h"
#include "ui_reagentdock.h"
#include <QInputDialog>
#include <QMenu>
#include <QClipboard>
#include <QMoveEvent>
#include <QSqlQuery>
#include <QMessageBox>
#include "reagent.h"
#include "property.h"
#include "tag.h"
#include "variable.h"
#include "propertydock.h"
#include "reagentdialog.h"

// TODO: reapply property filter on new property addition

/**
 * @brief ReagentDock::ReagentDock
 * @param parent
 */
ReagentDock::ReagentDock( QWidget *parent ) : DockWidget( parent ), ui( new Ui::ReagentDock ) {
    // set up ui
    this->ui->setupUi( this );

    // set a model to treeview
    this->ui->reagentView->setModel( model );

    // disable the remove button (and enable it only when a reagent is clicked upon)
    this->ui->removeButton->setEnabled( false );
    this->ui->reagentView->selectionModel()->connect( this->ui->reagentView->selectionModel(), &QItemSelectionModel::currentChanged, [ this ]( const QModelIndex &current, const QModelIndex & ) {
        this->ui->removeButton->setEnabled( current.isValid());
        emit this->currentIndexChanged( current );
    } );

    // hide searchBox
    this->ui->searchEdit->hide();

    // implement search
    this->ui->searchEdit->connect( this->ui->searchEdit, &QLineEdit::textChanged, [ this ]( const QString &filter ) {
        // update the match list
        this->matches = this->model->setupModelData( filter );

        // if list empty, resert currentMatch and clear selection
        if ( this->matches.isEmpty()) {
            this->currentMatch = Id::Invalid;
            this->select( QModelIndex());
            return;
        }

        // expand all nodes in search since we want to see all reagents and batches that match the search criteria
        this->ui->reagentView->expandAll();

        if ( this->currentMatch == Id::Invalid || !this->matches.contains( this->currentMatch )) {
            // do this if:
            //   current match has not been set (first search)
            //   current match differs from the current filter
            this->currentMatch = this->matches.last();
            this->select( this->model->find( this->currentMatch ));
        } else {
            // the previous item is the same, reselect it
            this->select( this->model->find( this->currentMatch ));
        }
    } );

    // implement going through found entries
    this->ui->searchEdit->connect( this->ui->searchEdit, &QLineEdit::returnPressed, [ this ]() {
        // no filter - hide the search box
        if ( this->ui->searchEdit->text().isEmpty()) {
            this->ui->searchEdit->hide();
            return;
        }

        if ( this->matches.count() >= 2 ) {
            // advance or wrap around the match list
            int pos = this->matches.indexOf( this->currentMatch ) + 1;
            if ( pos > this->matches.count() - 1 )
                pos = 0;

            // set the new item
            this->currentMatch = this->matches.at( pos );
            this->select( this->model->find( this->currentMatch ));
        } else {
            // if there is just one item left, hide the search box
            this->ui->searchEdit->hide();
        }
    } );
}

/**
 * @brief ReagentDock::~ReagentDock
 */
ReagentDock::~ReagentDock() {
    delete this->model;
    delete ui;
}

/**
 * @brief ReagentDock::on_reagentView_clicked
 * @param index
 */
void ReagentDock::on_reagentView_clicked( const QModelIndex &index ) {
    // if reagent is invalid, display no properties
    if ( !index.isValid()) {
        Property::instance()->setFilter( "false" );
        Variable::instance()->setValue<QVariantList>( "reagentDock/selection", QVariantList());
        return;
    }

    // retrieve data from model
    const TreeItem *item( static_cast<TreeItem*>( index.internalPointer()));
    const Id reagentId = static_cast<Id>( item->data( TreeItem::Id ).toInt());
    const Id parentId = static_cast<Id>( item->data( TreeItem::ParentId ).toInt());

    // apply sql filter
    Property::instance()->setFilter( QString( "( %1=%2 ) or ( %1=%3 and %4 not in ( select %4 from %5 where ( %1=%2 )))" )
                                     .arg( Property::instance()->fieldName( Property::ReagentID ))   // 1
                                     .arg( static_cast<int>( reagentId ))                            // 2
                                     .arg( static_cast<int>( parentId ))                             // 3
                                     .arg( Property::instance()->fieldName( Property::TagID ))       // 4
                                     .arg( Property::instance()->tableName())                        // 5
                                     );
    Property::instance()->sort( Property::Order_, Qt::AscendingOrder );
    Property::instance()->select();


    // resize the property view to fit contents
    PropertyDock::instance()->resizeViewContents();

    // store last selection in a variabe
    Variable::instance()->setInteger( "reagentDock/selection", static_cast<int>( reagentId ));
}

/**
 * @brief ReagentDock::on_reagentView_customContextMenuRequested
 * @param pos
 */
void ReagentDock::on_reagentView_customContextMenuRequested( const QPoint &pos ) {
    QMenu menu;

    auto addReagent = [ this ]( const Id &parentId ) {
        QString name, alias;
        bool ok;

        //
        // NOTE:
        //  reagents can only have unique names and aliases
        //  batches can be named anything and do not have aliases at all
        //
        if ( parentId != Id::Invalid ) {
            name = QInputDialog::getText( this, this->tr( "Add batch" ), this->tr( "Name:" ), QLineEdit::Normal, QString(), &ok );
        } else {
            ReagentDialog rd( this );
            ok = ( rd.exec() == QDialog::Accepted );
            name = rd.name();
            alias = rd.alias();

            QSqlQuery query;

            // check alias for duplicates
            query.exec( QString( "select %1 from %2 where %1='%3'" )
                             .arg( Reagent::instance()->fieldName( Reagent::Alias ))
                             .arg( Reagent::instance()->tableName())
                             .arg( alias ));
            if ( query.next()) {
                QMessageBox::warning( this,  this->tr( "Cannot add reagent" ), this->tr( "Alias '%1' already exists" ).arg( alias ));
                return;
            }

            // check name for duplicates
            query.exec( QString( "select %1 from %2 where %1='%3'" )
                             .arg( Reagent::instance()->fieldName( Reagent::Name ))
                             .arg( Reagent::instance()->tableName())
                             .arg( name ));
            if ( query.next()) {
                QMessageBox::warning( this,  this->tr( "Cannot add reagent" ), this->tr( "Reagent '%1' already exists" ).arg( name ));
                return;
            }
        }

        if ( ok ) {
            if ( !name.isEmpty()) {
                const Row row = Reagent::instance()->add( qAsConst( name ), qAsConst( alias ), parentId );
                if ( row == Row::Invalid )
                    return;

                // reexpand parent reagent
                // unfortunately we have to repaint to restore index cache
                this->reset();

                if ( parentId != Id::Invalid )
                    this->expand( this->model->find( parentId ));

                this->select( this->model->find( Reagent::instance()->id( row )));
            } else {
                QMessageBox::warning( this,  this->tr( "Cannot add reagent" ), ( parentId != Id::Invalid ?  this->tr( "Batch" ) : this->tr( "Reagent" )) + this->tr( " name is empty" ));
                return;
            }
        }
    };

    menu.addAction( this->tr( "Add new reagent" ), std::bind( addReagent, Id::Invalid ));

    const QModelIndex index( this->ui->reagentView->currentIndex());
    if ( index.isValid()) {
        const TreeItem *item( static_cast<TreeItem*>( index.internalPointer()));
        const Id parentId = static_cast<Id>( item->data( TreeItem::ParentId ).toInt());
        const QString name(( parentId == Id::Invalid ) ? item->data( TreeItem::Name ).toString() : item->parent()->data( TreeItem::Name ).toString());

        menu.addAction( this->tr( "Add new batch to reagent \"%1\"" ).arg( name ), std::bind( addReagent, ( parentId == Id::Invalid ) ? static_cast<Id>( item->data( TreeItem::Id ).toInt()) : parentId  ));
        menu.addAction( this->tr( "Copy name" ), [ item ]() { QGuiApplication::clipboard()->setText( item->data( TreeItem::Name ).toString()); } );
    }

    menu.exec( this->mapToGlobal( pos ));
}

/**
 * @brief ReagentDock::on_addButton_clicked
 */
void ReagentDock::on_addButton_clicked() {
    this->on_reagentView_customContextMenuRequested( this->ui->addButton->pos());
}

/**
 * @brief ReagentDock::on_removeButton_clicked
 */
void ReagentDock::on_removeButton_clicked() {
    // get current index
    const QModelIndex index( this->ui->reagentView->currentIndex());
    if ( !index.isValid())
        return;

    // get current item
    const TreeItem *item( static_cast<TreeItem*>( index.internalPointer()));

    // construct a menu
    QMenu menu;
    if ( static_cast<Id>( item->data( TreeItem::ParentId ).toInt()) == Id::Invalid ) {
        // remove reagent and batches
        menu.addAction( this->tr( "Remove reagent '%1' and its batches" ).arg( item->data( TreeItem::Name ).toString()), [ this, item ]() {
            const Id reagentId = static_cast<Id>( item->data( TreeItem::Id ).toInt());
            const Row reagentRow = Reagent::instance()->row( reagentId );
            if ( reagentRow == Row::Invalid )
                return;

            // remove batches
            const QList<Row>children( Reagent::instance()->children( reagentRow ));
            foreach ( const Row &batchRow, children )
                Reagent::instance()->remove( batchRow );

            // remove reagent
            Reagent::instance()->remove( reagentRow );

            // remove orphans just in case
            Reagent::instance()->removeOrphanedEntries();
            Property::instance()->removeOrphanedEntries();

            // reset model
            this->reset();
        } );
    } else {
        // remove batch
        menu.addAction( this->tr( "Remove batch '%1'" ).arg( item->data( TreeItem::Name ).toString()), [ this, item ]() {
            const Id reagentId = static_cast<Id>( item->data( TreeItem::Id ).toInt());
            const Id parentId = static_cast<Id>( item->data( TreeItem::ParentId ).toInt());

            const Row row = Reagent::instance()->row( reagentId );
            if ( row != Row::Invalid ) {
                Reagent::instance()->remove( row );

                // remove orphans just in case
                Reagent::instance()->removeOrphanedEntries();
                Property::instance()->removeOrphanedEntries();

                // reexpand parent reagent
                // unfortunately we have to repaint to restore index cache
                this->reset();
                this->expand( this->model->find( parentId ));
            }
        } );
    }

    // display menu
    menu.exec( this->mapToGlobal( this->ui->removeButton->pos()));
}

/**
 * @brief ReagentDock::restoreIndex
 */
void ReagentDock::restoreIndex() {
    // get id list from variable
    const Id id( Variable::instance()->value<Id>( "reagentDock/selection" ));
    if ( id != Id::Invalid ) {
        const Row row = Reagent::instance()->row( id );
        if ( row != Row::Invalid ) {
            const Id parentId( Reagent::instance()->parentId( row ));
            if ( parentId != Id::Invalid )
                this->expand( this->model->find( parentId ));

            this->select( this->model->find( Reagent::instance()->id( row )));
            return;
        }
    }

    this->select( QModelIndex());
}

/**
 * @brief ReagentDock::select
 * @param index
 */
void ReagentDock::select( const QModelIndex &index ) {
    this->on_reagentView_clicked( index );
    this->ui->reagentView->setCurrentIndex( index );
}

/**
 * @brief ReagentDock::expand
 * @param index
 */
void ReagentDock::expand( const QModelIndex &index ) {
    this->ui->reagentView->expand( index );
    this->ui->reagentView->setCurrentIndex( index );
}

/**
 * @brief ReagentDock::reset
 */
void ReagentDock::reset() {
    this->model->setupModelData();
    this->ui->reagentView->repaint();
}

/**
 * @brief ReagentDock::on_buttonFind_clicked
 */
void ReagentDock::on_buttonFind_clicked() {
    const bool visible = this->ui->searchEdit->isVisible();

    this->ui->searchEdit->setVisible( !visible );
    if ( !visible )
        this->ui->searchEdit->setFocus();
}
