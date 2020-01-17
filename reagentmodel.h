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

#pragma once

/*
 * includes
 */
#include <QAbstractItemModel>
#include "property.h"
#include "table.h"
#include <QStandardItem>

/**
 * @brief The ReagentModel class
 */
class ReagentModel : public QAbstractItemModel {
public:
    enum Data {
        NoData = Qt::UserRole,
        ID,
        ParentId
    };

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
    int columnCount( const QModelIndex & = QModelIndex()) const override;
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
    QStandardItem *rootItem() const { return this->m_rootItem; }

private:
    QStandardItem *m_rootItem = nullptr;
    mutable QMap<QStandardItem*, QModelIndex> itemIndexes;
};
