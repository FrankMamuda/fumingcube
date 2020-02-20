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
#include "table.h"

/**
 * @brief The Reagent class
 */
class Reagent final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY( Reagent )

public:
    /**
     * @brief The Fields enum
     */
    enum Fields {
        NoField = -1,
        ID,
        Name,
        Alias,
        ParentId,

        // count (DO NOT REMOVE)
                Count
    };
    Q_ENUM( Fields )

    /**
     * @brief instance
     * @return
     */
    static Reagent *instance() {
        static auto *reagent( new Reagent());
        return reagent;
    }
    ~Reagent() override = default;
    Row add( const QString &name, const QString &reference, const Id &parentId = Id::Invalid );
    [[nodiscard]] QList<Row> children( const Row &row ) const;
    [[nodiscard]] QList<Id> labelIds( const Row &row ) const;

    // initialize field setters and getters
    INITIALIZE_FIELD( Id, ID, id )
    INITIALIZE_FIELD( QString, Name, name )
    INITIALIZE_FIELD( QString, Alias, alias )
    INITIALIZE_FIELD( Id, ParentId, parentId )

    // FIXME: temporary
    [[nodiscard]] QString reference( const Row &row ) const { return this->alias( row ); }
    [[nodiscard]] QString reference( const Id &id ) const { return this->alias( id ); }

public slots:
    void removeOrphanedEntries() override;

private:
    explicit Reagent();
};

// declare enums
Q_DECLARE_METATYPE( Reagent::Fields )
