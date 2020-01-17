/*
 * Copyright (C) 2019-2020 Armands Aleksejevs
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
#include "label.h"
#include "labeldock.h"
#include <QDebug>

/**
 * @brief ReagentModel::rowCount
 * @param parent
 * @return
 */
int ReagentModel::rowCount( const QModelIndex &parent ) const {
    if ( parent.column() > 0 || this->rootItem() == nullptr )
        return 0;

    return (( !parent.isValid()) ? this->rootItem() : static_cast<QStandardItem*>( parent.internalPointer()))->rowCount();
}

/**
 * @brief ReagentModel::columnCount
 * @return
 */
int ReagentModel::columnCount( const QModelIndex & ) const {
    return 1;
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
    QStandardItem *childItem((( !parent.isValid()) ? this->rootItem() : static_cast<QStandardItem*>( parent.internalPointer()))->child( row ));
    if ( childItem != nullptr ) {
        if ( this->itemIndexes.contains( childItem ))
            return this->itemIndexes[childItem];

        const QModelIndex index( this->createIndex( row, column, childItem ));
        //childItem->setIndex( index );
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

    QStandardItem *parentItem( static_cast<QStandardItem*>( child.internalPointer())->parent());
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
    if ( !index.isValid() || this->rootItem() == nullptr )
        return QVariant();

    QStandardItem *item( static_cast<QStandardItem*>( index.internalPointer()));
    if ( role == Qt::DisplayRole )
        return item->text();

    if ( role == Qt::DecorationRole ) {
        if ( !LabelDock::instance()->isVisible())
            return QVariant();

        if ( !item->icon().isNull())
            return item->icon();

        const Id reagentId = item->data( ID ).value<Id>();
        if ( reagentId == Id::Invalid )
            return QVariant();

        const QList<Id>labelIds( Reagent::instance()->labelIds( Reagent::instance()->row( reagentId )));
        if ( labelIds.isEmpty())
            return QVariant();

        QList<QColor> colours;
        foreach ( const Id &id, labelIds )
            colours << Label::instance()->colour( id );

        item->setIcon( QIcon( Label::instance()->pixmap( qAsConst( colours ))));
        return item->icon();
    }

    return QVariant();
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
    this->m_rootItem = new QStandardItem();

    // make a list of matches (for search)
    QList<Id> total;

    // go through reagents and their batches
    for ( int y = 0; y < Reagent::instance()->count(); y++ ) {
        const Row row = Reagent::instance()->row( y );
        const Id parentId = Reagent::instance()->parentId( row );

        // add top level reagents first
        if ( parentId != Id::Invalid )
            continue;

        const QString reagentName( Reagent::instance()->name( row ));
        const QString alias( Reagent::instance()->alias( row ));
        const Id reagentId = Reagent::instance()->id( row );
        bool found = false;

        // make a new reagent treeItem
        QStandardItem *reagent( new QStandardItem( !QString::compare( reagentName, alias ) ? reagentName : QString( "%1 (%2)" ).arg( reagentName ).arg( alias )));
        reagent->setData( static_cast<int>( reagentId ), ID );
        reagent->setData( static_cast<int>( Id::Invalid ), ParentId );

        // go through batches (children of the reagent)
        const QList<Row>children( Reagent::instance()->children( row ));
        foreach ( const Row &child, children ) {
            const QString childName( Reagent::instance()->name( child ));
            const Id childId = Reagent::instance()->id( child );

            // apply search filter if any
            if ( !filter.isEmpty() && !childName.contains( filter, Qt::CaseInsensitive ))
                continue;

            // add batch to both treeView and search list
            QStandardItem *batch( new QStandardItem( Reagent::instance()->name( child )));
            batch->setData( static_cast<int>( childId ), ID );
            batch->setData( static_cast<int>( reagentId ), ParentId );
            reagent->appendRow( batch );
            found = true;
            total << childId;
        }

        if ( found || filter.isEmpty()) {
            // add reagent to treeView
            this->rootItem()->appendRow( reagent );

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
                this->rootItem()->appendRow( reagent );
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
 * @brief ReagentModel::sort
 * @param column
 * @param order
 */
void ReagentModel::sort( int, Qt::SortOrder order ) {
    if ( this->rootItem() == nullptr )
        return;

    emit this->layoutAboutToBeChanged();
    this->rootItem()->sortChildren( 0, order );
}

/**
 * @brief ReagentModel::find
 * @param id
 * @param table
 * @return
 */
QModelIndex ReagentModel::find( const Id &id ) const {
    foreach ( const QStandardItem *item, this->itemIndexes.keys()) {
        if ( item->data( ID ).value<Id>() == id )
            return this->itemIndexes[const_cast<QStandardItem*>( item )];
    }

    return QModelIndex();
}

/**
 * @brief ReagentModel::indexFromItem
 * @param item
 * @return
 */
QModelIndex ReagentModel::indexFromItem( QStandardItem *item ) const {
    if ( this->itemIndexes.contains( item ))
        return this->itemIndexes[item];

    return QModelIndex();
}
