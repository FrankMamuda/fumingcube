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

#pragma once

/*
 * includes
 */
#include "table.h"

/**
 * @brief The TableProperty class
 */
class TableProperty final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY( TableProperty )

public:
    /**
     * @brief The Fields enum
     */
    enum Fields {
        NoField = -1,
        ID,
        TableId,
        TagId,
        Tab,
        TableOrder,

        // count (DO NOT REMOVE)
        Count
    };
    Q_ENUM( Fields )

    /**
     * @brief instance
     * @return
     */
    static TableProperty *instance() { static TableProperty *i( new TableProperty()); return i; }
    ~TableProperty() override = default;
    Row add( const Id &tableId = Id::Invalid, const Id &tagId = Id::Invalid, bool tabbed = false, int order = -1 );

    // initialize field setters and getters
    INITIALIZE_FIELD( Id,     ID,         id )
    INITIALIZE_FIELD( Id,     TableId,    tableId )
    INITIALIZE_FIELD( Id,     TagId,      tagId )
    INITIALIZE_FIELD( int,    Tab,        tab )
    INITIALIZE_FIELD( int,    TableOrder, tableOrder )

public slots:
    void removeOrphanedEntries() override;

private:
    explicit TableProperty();
};

// declare enums
Q_DECLARE_METATYPE( TableProperty::Fields )
