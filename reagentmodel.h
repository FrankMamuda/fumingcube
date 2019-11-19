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

#pragma once

/*
 * includes
 */
#include <QAbstractItemModel>
#include "property.h"
#include "table.h"
#include <QStandardItem>

/**
 * @brief The TreeItem class
 */
class TreeItem {
public:
    explicit TreeItem( const QVariantList &itemData = QVariantList(), TreeItem *parentItem = nullptr ) : m_data( itemData ), m_parent( parentItem ) {}
    ~TreeItem() { qDeleteAll( this->items ); }

    enum Data {
        NoData = -1,
        Name,
        Id,
        ParentId
    };

    /**
     * @brief columnCount
     * @return
     */
    int columnCount() const { return 1; }

    /**
     * @brief data
     * @param position
     * @return
     */
    QVariant data( int position ) const { return ( position < 0 || position >= this->m_data.count()) ? QVariant() : this->m_data.at( position ); }

    /**
     * @brief row
     * @return
     */
    int row() const { return ( this->parent() != nullptr ) ? this->parent()->items.indexOf( const_cast<TreeItem*>( this )) : 0; }

    /**
     * @brief count
     * @return
     */
    int count() const { return this->items.count(); }

    /**
     * @brief parent
     * @return
     */
    TreeItem *parent() { return this->m_parent; }
    TreeItem *parent() const { return this->m_parent; }

    /**
     * @brief index
     * @return
     */
    QModelIndex index() const { return this->m_index; }

    /**
     * @brief append
     * @param child
     */
    void append( TreeItem *child ) { child->setParent( this ); this->items << child; }

    /**
     * @brief remove
     * @param child
     */
    void remove( TreeItem *child ) { this->items.removeAll( child ); }

    /**
     * @brief at
     * @param row
     * @return
     */
    TreeItem *at( const int row ) { return ( row < 0 || row >= this->count()) ? nullptr : this->items.at( row ); }
    TreeItem *at( const int row ) const { return ( row < 0 || row >= this->count()) ? nullptr : this->items.at( row ); }

    /**
     * @brief setParent
     * @param parentItem
     */
    void setParent( TreeItem *parentItem = nullptr ) { this->m_parent = parentItem; }

    /**
     * @brief setIndex
     * @param index
     */
    void setIndex( const QModelIndex &index ) { this->m_index = index; }

private:
    QVariantList m_data;
    TreeItem *m_parent;
    QList<TreeItem*> items;
    QModelIndex m_index;
};

/**
 * @brief The ReagentModel class
 */
class ReagentModel : public QAbstractItemModel {
public:
    /**
     * @brief TreeModel
     * @param parent
     */
    ReagentModel( QObject *parent = nullptr ) : QAbstractItemModel( parent ) { this->setupModelData(); }

    /**
     * @brief ~TreeModel
     */
    ~ReagentModel() override { delete this->m_rootItem; }

    int rowCount( const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief columnCount
     * @param parent
     * @return
     */
    int columnCount( const QModelIndex & = QModelIndex()) const override { return 1; }
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    QList<Id> setupModelData( const QString &filter = QString());

    /**
     * @brief find
     * @param id
     * @param table
     * @return
     */
    QModelIndex find( const Id &id ) const;

    /**
     * @brief rootItem
     * @return
     */
    TreeItem *rootItem() const { return this->m_rootItem; }

private:
    TreeItem *m_rootItem = nullptr;
    mutable QMap<TreeItem*, QModelIndex> itemIndexes;
};