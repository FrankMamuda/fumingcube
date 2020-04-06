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
    Q_DISABLE_COPY( ReagentModel )

public:
    /**
     * @brief The Data enum
     */
    enum Data {
        NoData = Qt::UserRole,
        ID,
        ParentId,
        HTML,
        Pixmap,
        DateTime
    };

    /**
     * @brief TreeModel
     * @param parent
     */
    explicit ReagentModel( QObject *parent = nullptr ) : QStandardItemModel( parent ) { this->setupModelData(); }

    // disable move
    ReagentModel( ReagentModel&& ) = delete;
    ReagentModel& operator=( ReagentModel&& ) = delete;

    /**
     * @brief ~TreeModel
     */
    ~ReagentModel() override = default;

    /**
     * @brief columnCount
     * @param parent
     * @return
     */
    [[nodiscard]] QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    [[nodiscard]] QVariant data( const QModelIndex &index, int role ) const override;
    [[nodiscard]] QModelIndex indexFromId( const Id &id ) const;
    [[nodiscard]] Id idFromIndex( const QModelIndex &index ) const;
    static QString generateName( const QString &name, const QString &reference = QString());

public slots:
    void add( const Id &id );
    static void addItem( const Id &id, const Id &parentId, QStandardItem *parentItem );
    void setupModelData();

    /**
     * @brief remove
     * @param index
     */
    void remove( const QModelIndex &index ) { this->remove( QModelIndexList() << index ); }
    void remove( const QModelIndexList &list );
};
