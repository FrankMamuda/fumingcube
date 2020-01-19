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

QVariant ReagentModel::data( const QModelIndex &index, int role ) const {
    if ( index.isValid() && role == Qt::DecorationRole ) {

        if ( !LabelDock::instance()->isVisible())
            return QVariant();

        QStandardItem *item( this->itemFromIndex( index ));
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

    return QStandardItemModel::data( index, role );
}

/**
 * @brief ReagentModel::setupModelData
 * @param filter
 * @return
 */
QList<Id> ReagentModel::setupModelData( const QString &filter ) {
    // clear index cache
    this->beginResetModel();

    this->clear();

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
            this->invisibleRootItem()->appendRow( reagent );

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
                this->invisibleRootItem()->appendRow( reagent );
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
    emit this->layoutAboutToBeChanged();
    this->invisibleRootItem()->sortChildren( 0, order );
}

/**
 * @brief ReagentModel::find
 * @param id
 * @param table
 * @return
 */
QModelIndex ReagentModel::find( const Id &id ) const {
    for ( int y = 0; y < this->invisibleRootItem()->rowCount(); y++ ) {
        const QStandardItem *reagent( this->invisibleRootItem()->child( y ));
        if ( reagent->data( ID ).value<Id>() == id ) {
            return reagent->index();
        } else {
            for ( int k = 0; k < reagent->rowCount(); k++ ) {
                const QStandardItem *batch( reagent->child( k )); {
                    if ( batch->data( ID ).value<Id>() == id )
                        return batch->index();
                }
            }
        }
    }

    return QModelIndex();
}
