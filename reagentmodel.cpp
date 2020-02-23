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
#include "reagentdock.h"
#include <QDebug>
#include <QTextEdit>

/**
 * @brief ReagentModel::headerData
 * @param section
 * @param orientation
 * @param role
 * @return
 */
QVariant ReagentModel::headerData( int section, Qt::Orientation orientation, int role ) const {
    if ( section == 0 && role == Qt::DisplayRole && orientation == Qt::Horizontal )
        return ReagentModel::tr( "Reagents" );

    return QVariant();
}

/**
 * @brief ReagentModel::data
 * @param index
 * @param role
 * @return
 */
QVariant ReagentModel::data( const QModelIndex &index, int role ) const {
    if ( index.isValid() && role == ReagentModel::Pixmap ) {
        if ( !LabelDock::instance()->isVisible())
            return QVariant();

        QStandardItem *item( this->itemFromIndex( index ));
        if ( item->data( ReagentModel::ParentId ).value<Id>() != Id::Invalid )
            return QVariant();

        const auto pixmap( item->data( ReagentModel::Pixmap ).value<QPixmap>());
        if ( !pixmap.isNull())
            return pixmap;

        const auto reagentId = item->data( ID ).value<Id>();
        if ( reagentId == Id::Invalid )
            return QVariant();

        const QList<Id> labelIds( Reagent::instance()->labelIds( Reagent::instance()->row( reagentId )));
        if ( labelIds.isEmpty())
            return QVariant();

        QList<QColor> colours;
        for ( const Id &id : labelIds )
            colours << Label::instance()->colour( id );

        item->setData( Label::instance()->pixmap( qAsConst( colours )), ReagentModel::Pixmap );
        return item->data( ReagentModel::Pixmap ).value<QPixmap>();
    }

    // for search
    /*if ( index.isValid() && role == Qt::DisplayRole ) {
        QStandardItem *item( this->itemFromIndex( index ));
        return QTextEdit( item->text()).toPlainText();
    }*/

    return QStandardItemModel::data( index, role );
}

/**
 * @brief ReagentModel::setupModelData
 * @return
 */
void ReagentModel::setupModelData() {
    // clear index cache
    this->beginResetModel();
    this->clear();

    // go through reagents and their batches
    for ( int y = 0; y < Reagent::instance()->count(); y++ ) {
        const Row row = Reagent::instance()->row( y );

        // add top level reagents first (skip batches)
        if ( Reagent::instance()->parentId( row ) != Id::Invalid )
            continue;

        // make a new reagent treeItem
        const Id reagentId = Reagent::instance()->id( row );
        const QString generatedName(
                ReagentModel::generateName( Reagent::instance()->name( row ), Reagent::instance()->reference( row )));
        auto *reagent( new QStandardItem( QTextEdit( generatedName ).toPlainText()));
        reagent->setData( static_cast<int>( reagentId ), ID );
        reagent->setData( static_cast<int>( Id::Invalid ), ParentId );
        reagent->setData( generatedName, HTML );

        // go through batches (children of the reagent)
        for ( const Row &child : Reagent::instance()->children( row ))
            ReagentModel::addItem( Reagent::instance()->id( child ), reagentId, reagent );

        // add reagent to treeView
        this->invisibleRootItem()->appendRow( reagent );
    }

    this->endResetModel();
}

/**
 * @brief ReagentModel::find
 * @param id
 * @param table
 * @return
 */
QModelIndex ReagentModel::indexFromId( const Id &id ) const {
    for ( int y = 0; y < this->invisibleRootItem()->rowCount(); y++ ) {
        const QStandardItem *reagent( this->invisibleRootItem()->child( y ));
        if ( reagent->data( ID ).value<Id>() == id )
            return reagent->index();

        for ( int k = 0; k < reagent->rowCount(); k++ ) {
            const QStandardItem *batch( reagent->child( k ));
            {
                if ( batch->data( ID ).value<Id>() == id )
                    return batch->index();
            }
        }
    }

    return QModelIndex();
}

/**
 * @brief ReagentModel::idFromIndex
 * @param index
 * @return
 */
Id ReagentModel::idFromIndex( const QModelIndex &index ) const {
    if ( !index.isValid())
        return Id::Invalid;

    const QStandardItem *item( this->itemFromIndex( index ));
    return item->data( ReagentModel::ID ).value<Id>();
}

/**
 * @brief ReagentModel::generateName
 * @param name
 * @param reference
 * @return
 */
QString ReagentModel::generateName( const QString &name, const QString &reference ) {
    if ( reference.isEmpty())
        return name;

    return ( !QString::compare( name, reference ) ? name : QString( "%1 (%2)" ).arg( name, reference ));
}

/**
 * @brief ReagentModel::add
 * @param id
 */
void ReagentModel::add( const Id &id ) {
    const Id parentId = Reagent::instance()->parentId( id );
    if ( parentId != Id::Invalid ) {
        const QModelIndex index( this->indexFromId( parentId ));
        if ( !index.isValid())
            return;

        ReagentModel::addItem( id, parentId, this->itemFromIndex( index ));
    } else {
        ReagentModel::addItem( id, Id::Invalid, this->invisibleRootItem());
    }

    ReagentDock::instance()->view()->filterModel()->sort( 0, Qt::AscendingOrder );
}

/**
 * @brief ReagentModel::addItem
 * @param id
 * @param parentId
 * @param parentItem
 */
void ReagentModel::addItem( const Id &id, const Id &parentId, QStandardItem *parentItem ) {
    const QString generatedName( parentId == Id::Invalid ? ReagentModel::generateName( Reagent::instance()->name( id ),
                                                                                       Reagent::instance()->reference(
                                                                                               id ))
                                                         : Reagent::instance()->name( id ));
    auto *item( new QStandardItem( QTextEdit( generatedName ).toPlainText()));
    item->setData( static_cast<int>( id ), ID );
    item->setData( static_cast<int>( parentId ), ParentId );
    item->setData( generatedName, HTML );
    parentItem->appendRow( item );
}

/**
 * @brief ReagentModel::remove
 * @param list
 */
void ReagentModel::remove( const QModelIndexList &list ) {
    // first build a list of QPersistentModelIndexes
    QList<QPersistentModelIndex> pList;
    for ( const QModelIndex &index : list ) {
        if ( !index.isValid())
            continue;

        pList << QPersistentModelIndex( index );
    }

    // then delete items one by one
    for ( const QPersistentModelIndex &index : qAsConst( pList ))
        this->removeRow( index.row(), index.parent());
}
