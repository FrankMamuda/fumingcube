/*
 * Copyright (C) 2020 Armands Aleksejevs
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
#include "variable.h"
#include "propertydock.h"
#include "reagentview.h"
#include "reagent.h"

/**
 * @brief ReagentView::ReagentView
 * @param parent
 */
ReagentView::ReagentView( QWidget *parent) : QTreeView( parent ) {
    // set a model to treeview
    this->setModel( new QSortFilterProxyModel());
    this->filterModel()->setSourceModel( new ReagentModel());
    this->filterModel()->setRecursiveFilteringEnabled( true );
    this->filterModel()->setSortCaseSensitivity( Qt::CaseInsensitive );
    this->filterModel()->setFilterCaseSensitivity( Qt::CaseInsensitive );
    this->setRootIndex( this->model()->invisibleRootItem()->index());

    this->m_nodeHistory = new NodeHistory( this );
    this->connect( this, SIGNAL( clicked( const QModelIndex & )), this, SLOT( selectReagent( const QModelIndex & )));
}

/**
 * @brief ReagentView::updateView
 */
void ReagentView::updateView() {
    this->nodeHistory()->setEnabled( false );
    this->model()->setupModelData();

    // clear selection
    this->filterModel()->sort( 0, Qt::AscendingOrder );
    this->nodeHistory()->restoreNodeState();
    this->restoreIndex();
}

/**
 * @brief ReagentView::selectReagent
 * @param index
 */
void ReagentView::selectReagent( const QModelIndex &filterIndex ) {
    const QModelIndex &index( this->filterModel()->mapToSource( filterIndex ));
    this->setCurrentIndex( filterIndex );
    this->selectionModel()->select( filterIndex, QItemSelectionModel::ClearAndSelect );

    // if reagent is invalid, display no properties
    if ( !index.isValid()) {
        Property::instance()->setFilter( "false" );
        Variable::instance()->setInteger( "reagentDock/selection", -1 );
        return;
    }

    // retrieve data from model
    const QStandardItem *item( this->itemFromIndex( index ));
    const int reagentId = item->data( ReagentModel::ID ).toInt();

    // store last selection in a variabe
    Variable::instance()->setInteger( "reagentDock/selection", reagentId );

    // apply sql filter
    Property::instance()->setFilter( QString( "( %1=%2 and %1>-1 ) or ( %1=%3 and %1>-1 and %4 not in ( select %4 from %5 where ( %1=%2 )))" )
                                     .arg( Property::instance()->fieldName( Property::ReagentId ))   // 1
                                     .arg( reagentId )                                               // 2
                                     .arg( item->data( ReagentModel::ParentId ).toInt())             // 3
                                     .arg( Property::instance()->fieldName( Property::TagId ))       // 4
                                     .arg( Property::instance()->tableName())                        // 5
                                     );
    Property::instance()->sort( Property::TableOrder, Qt::AscendingOrder );
    Property::instance()->select();

    // resize the property view to fit contents
    PropertyDock::instance()->updateView();
}

/**
 * @brief ReagentView::restoreIndex
 */
void ReagentView::restoreIndex() {
    // get id list from variable
    const Id id( Variable::instance()->value<Id>( "reagentDock/selection" ));
    if ( id != Id::Invalid ) {
        const Id parentId( Reagent::instance()->parentId( id ));
        if ( parentId != Id::Invalid )
            this->expand( this->filterModel()->mapFromSource( this->indexFromId( parentId )));

        const QModelIndex index( this->filterModel()->mapFromSource( this->indexFromId( id )));
        this->selectReagent( index );
        this->scrollTo( index );

        return;
    }

    this->selectReagent();
}