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
class ReagentModel : public QStandardItemModel {
    Q_OBJECT

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
    ReagentModel( QObject *parent = nullptr ) : QStandardItemModel( parent ) { this->setupModelData(); }

    /**
     * @brief ~TreeModel
     */
    ~ReagentModel() override {}

    /**
     * @brief columnCount
     * @param parent
     * @return
     */
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    QVariant data( const QModelIndex &index, int role ) const;
    QList<Id> setupModelData( const QString &filter = QString());
    void sort( int column = 0, Qt::SortOrder order = Qt::AscendingOrder ) override;

    /**
     * @brief find
     * @param id
     * @param table
     * @return
     */
    QModelIndex find( const Id &id ) const;
};
