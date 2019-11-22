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
#include "reagentmodel.h"
#include "reagent.h"
#include <QDebug>

/**
 * @brief ReagentModel::rowCount
 * @param parent
 * @return
 */
int ReagentModel::rowCount( const QModelIndex &parent ) const {
    if ( parent.column() > 0 || this->rootItem() == nullptr )
        return 0;

    return (( !parent.isValid()) ? this->rootItem() : static_cast<TreeItem*>( parent.internalPointer()))->count();
}

/**
 * @brief ReagentModel::index
 * @param row
 * @param column
 * @param parent
 * @return
 */
QModelIndex ReagentModel::index( int row, int column, const QModelIndex &parent ) const {
    if ( !this->hasIndex( row, column, parent ) || this->rootItem() == nullptr )
        return QModelIndex();

    // assign indexes to items (and store them in cache)
    TreeItem *childItem((( !parent.isValid()) ? this->rootItem() : static_cast<TreeItem*>( parent.internalPointer()))->at( row ));
    if ( childItem != nullptr ) {
        if ( this->itemIndexes.contains( childItem ))
            return this->itemIndexes[childItem];

        const QModelIndex index( this->createIndex( row, column, childItem ));
        childItem->setIndex( index );
        this->itemIndexes[childItem] = index;
        return index;
    }

    return QModelIndex();
}

/**
 * @brief ReagentModel::parent
 * @param child
 * @return
 */
QModelIndex ReagentModel::parent( const QModelIndex &child ) const {
    if ( !child.isValid() || this->rootItem() == nullptr )
        return QModelIndex();

    TreeItem *parentItem( static_cast<TreeItem*>( child.internalPointer())->parent());
    if ( parentItem == this->rootItem())
        return QModelIndex();

    return this->createIndex( parentItem->row(), 0, parentItem );
}

/**
 * @brief ReagentModel::data
 * @param index
 * @param role
 * @return
 */
QVariant ReagentModel::data( const QModelIndex &index, int role ) const {
    if ( !index.isValid() || role != Qt::DisplayRole || this->rootItem() == nullptr )
        return QVariant();

    return static_cast<TreeItem*>( index.internalPointer())->data( index.column());
}

/**
 * @brief ReagentModel::headerData
 * @param section
 * @param orientation
 * @param role
 * @return
 */
QVariant ReagentModel::headerData( int section, Qt::Orientation orientation, int role ) const {
    if ( section == 0 && role == Qt::DisplayRole && orientation == Qt::Horizontal )
        return this->tr( "Reagents" );

    return QVariant();
}

/**
 * @brief ReagentModel::setupModelData
 * @param filter
 * @return
 */
QList<Id> ReagentModel::setupModelData( const QString &filter ) {
    // clear index cache
    this->itemIndexes.clear();

    this->beginResetModel();

    // delete rootItem if any
    if ( this->rootItem() != nullptr )
        delete this->m_rootItem;

    // make a new rootItem
    this->m_rootItem = new TreeItem();

    // make a list of matches (for search)
    QList<Id> total;

    // go through reagents and their batches
    for ( int y = 0; y < Reagent::instance()->count(); y++ ) {
        const Row row = Reagent::instance()->row( y );
        const Id parentId = static_cast<Id>( Reagent::instance()->parentId( row ));

        // add top level reagents first
        if ( parentId != Id::Invalid )
            continue;

        const QString reagentName( Reagent::instance()->name( row ));
        const QString alias( Reagent::instance()->alias( row ));
        const Id reagentId = static_cast<Id>( Reagent::instance()->id( row ));
        bool found = false;

        // make a new reagent treeItem
        TreeItem *reagent( new TreeItem( QVariantList() << ( !QString::compare( reagentName, alias ) ? reagentName : QString( "%1 (%2)" ).arg( reagentName ).arg( alias )) << static_cast<int>( reagentId ) << static_cast<int>( Id::Invalid )));

        // go through batches (children of the reagent)
        const QList<Row>children( Reagent::instance()->children( row ));
        foreach ( const Row &child, children ) {
            const QString childName( Reagent::instance()->name( child ));
            const Id childId = static_cast<Id>( Reagent::instance()->id( child ));

            // apply search filter if any
            if ( !filter.isEmpty() && !childName.contains( filter, Qt::CaseInsensitive ))
                continue;

            // add batch to both treeView and search list
            TreeItem *batch( new TreeItem( QVariantList() << Reagent::instance()->name( child ) << static_cast<int>( childId ) << static_cast<int>( reagentId )));
            reagent->append( batch );
            found = true;
            total << childId;
        }

        if ( found || filter.isEmpty()) {
            // add reagent to treeView
            this->rootItem()->append( reagent );

            // add reagent to search list
            if ( filter.isEmpty())
                total << reagentId;
            else if ( !filter.isEmpty() && reagentName.contains( filter, Qt::CaseInsensitive ))
                total << reagentId;
        } else {
            // if no batches are found that match the filter
            // apply filter to reagent
            if ( !filter.isEmpty() && reagentName.contains( filter, Qt::CaseInsensitive )) {
                // add reagent to both treeView and search list
                this->rootItem()->append( reagent );
                total.prepend( reagentId );
            }
        }
    }
    this->endResetModel();

    // reverse and return search results
    std::reverse( total.begin(), total.end());
    return total;
}

/**
 * @brief ReagentModel::find
 * @param id
 * @param table
 * @return
 */
QModelIndex ReagentModel::find( const Id &id ) const {
    foreach ( const TreeItem *item, this->itemIndexes.keys()) {
        if ( static_cast<Id>( item->data( TreeItem::Id ).toInt()) == id )
            return this->itemIndexes[const_cast<TreeItem*>( item )];
    }

    return QModelIndex();
}
