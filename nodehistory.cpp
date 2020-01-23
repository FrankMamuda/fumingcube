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
#include <QStandardItem>

/**
 * @brief NodeHistory::NodeHistory
 * @param parent
 */
NodeHistory::NodeHistory( QTreeView *parent ) : m_treeParent( parent ) {
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


        // FIXME: save on exit
        this->saveHistory();
    };

    this->treeParent()->connect( this->treeParent(), &QTreeView::expanded, [ saveNodeState ]( const QModelIndex &index ) {
        saveNodeState( index, true );
    } );

    this->treeParent()->connect( this->treeParent(), &QTreeView::collapsed, [ saveNodeState ]( const QModelIndex &index ) {
        saveNodeState( index, false );
    } );

    this->loadHistory();
}

/**
 * @brief NodeHistory::~NodeHistory
 */
NodeHistory::~NodeHistory() {
    if ( this->treeParent() != nullptr ) {
        this->disconnect( this->treeParent(), &QTreeView::expanded, this, nullptr );
        this->disconnect( this->treeParent(), &QTreeView::collapsed, this, nullptr );
    }
}

/**
 * @brief NodeHistory::restoreNodeState
 */
void NodeHistory::restoreNodeState() {
    if ( this->treeParent() == nullptr )
        return;

    this->setEnabled( false );

    ReagentView *view( qobject_cast<ReagentView*>( this->treeParent()));
    if ( view == nullptr )
        return;

    this->treeParent()->collapseAll();
    for ( int y = 0; y < view->model()->invisibleRootItem()->rowCount(); y++ ) {
        const QStandardItem *item( view->model()->invisibleRootItem()->child( y ));
        const QModelIndex index( item->index());
        if ( !index.isValid())
            continue;

        if ( view == nullptr )
            return;

        if ( this->openNodes.contains( item->data( ReagentModel::ID ).value<Id>()))
            view->expand( view->filterModel()->mapFromSource( index ));
    }

    this->setEnabled();
}

/**
 * @brief NodeHistory::saveHistory
 */
void NodeHistory::saveHistory() {
    QStringList list;
    foreach ( const Id &id, this->openNodes )
        list << QString::number( static_cast<int>( id ));
    Variable::instance()->setValue( "reagentDock/openNodes", list );
}

/**
 * @brief NodeHistory::loadHistory
 */
void NodeHistory::loadHistory() {
    // TODO: add global static QStringList->QList<Id>
    const QStringList list( Variable::instance()->value<QStringList>( "reagentDock/openNodes" ));
    foreach ( const QString &n, list ) {
        bool ok;
        const Id id = static_cast<Id>( n.toInt( &ok ));
        if ( ok )
            this->openNodes << id;
    }

    this->restoreNodeState();
}
