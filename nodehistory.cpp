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
#include "nodehistory.h"
#include "reagentmodel.h"
#include "reagentview.h"
#include "variable.h"
#include "listutils.h"
#include "main.h"
#include <QStandardItem>

/**
 * @brief NodeHistory::NodeHistory
 * @param parent
 */
NodeHistory::NodeHistory() {
    this->loadHistory();

    // add to garbage collector
    GarbageMan::instance()->add( this );
}

/**
 * @brief NodeHistory::~NodeHistory
 */
NodeHistory::~NodeHistory() {
    if ( this->treeParent() != nullptr ) {
        NodeHistory::disconnect( this->treeParent(), &QTreeView::expanded, this, nullptr );
        NodeHistory::disconnect( this->treeParent(), &QTreeView::collapsed, this, nullptr );
    }
}

/**
 * @brief NodeHistory::restoreNodeState
 */
void NodeHistory::restoreNodeState() {
    if ( this->treeParent() == nullptr )
        return;

    this->setEnabled( false );

    auto *view( qobject_cast<ReagentView*>( this->treeParent()));
    if ( view == nullptr )
        return;

    this->treeParent()->collapseAll();
    for ( int y = 0; y < view->sourceModel()->invisibleRootItem()->rowCount(); y++ ) {
        const QStandardItem *item( view->sourceModel()->invisibleRootItem()->child( y ));
        const QModelIndex index( item->index());
        if ( !index.isValid())
            continue;

        if ( this->openNodes.contains( item->data( ReagentModel::ID ).value<Id>()))
            view->expand( view->filterModel()->mapFromSource( index ));
    }

    this->setEnabled();
}

/**
 * @brief NodeHistory::saveHistory
 */
void NodeHistory::saveHistory() {
    Variable::setValue( "reagentDock/openNodes", ListUtils::toStringList<Id>( qAsConst( this->openNodes )));
    Variable::setValue( "reagentDock/hiddenNodes", ListUtils::toStringList<Id>( qAsConst( this->hiddenNodes )));
}

/**
 * @brief NodeHistory::loadHistory
 */
void NodeHistory::loadHistory() {
    this->openNodes = ListUtils::toNumericList<Id>( Variable::value<QStringList>( "reagentDock/openNodes" ));
    this->hiddenNodes = ListUtils::toNumericList<Id>( Variable::value<QStringList>( "reagentDock/hiddenNodes" ));
}

/**
 * @brief NodeHistory::setTreeParent
 * @param parent
 */
void NodeHistory::setTreeParent( QTreeView *parent ) {
    this->m_treeParent = parent;

    if ( parent == nullptr )
        return;

    auto saveNodeState = [ this ]( const QModelIndex &filtered, bool expanded ) {
        if ( !this->isEnabled())
            return;

        const ReagentView *view( qobject_cast<ReagentView*>( this->treeParent()));
        if ( view == nullptr )
            return;

        const Id id = view->idFromIndex( view->filterModel()->mapToSource( filtered ));
        if ( id == Id::Invalid )
            return;

        if ( expanded && !this->openNodes.contains( id ))
            this->openNodes << id;
        else
            this->openNodes.removeAll( id );
    };

    QTreeView::connect( this->treeParent(), &QTreeView::expanded, [ saveNodeState ]( const QModelIndex &index ) {
        saveNodeState( index, true );
    } );

    QTreeView::connect( this->treeParent(), &QTreeView::collapsed, [ saveNodeState ]( const QModelIndex &index ) {
        saveNodeState( index, false );
    } );
}
