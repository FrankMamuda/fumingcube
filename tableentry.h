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
 * @brief The TableEntry class
 */
class TableEntry final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY( TableEntry )

public:
    /**
     * @brief The Fields enum
     */
    enum Fields {
        NoField = -1,
        ID,
        Name,
        Mode,

        // count (DO NOT REMOVE)
        Count
    };
    Q_ENUM( Fields )

    /**
     * @brief The Modes enum
     */
    enum Modes {
        NoMode = -1,
        Reagents,
        ReagentsAndBatches
    };
    Q_ENUM( Modes )

    /**
     * @brief instance
     * @return
     */
    static TableEntry *instance() { static TableEntry *i( new TableEntry()); return i; }
    ~TableEntry() override = default;
    Row add( const QString &name, const Modes &mode = Reagents );

    // initialize field setters and getters
    INITIALIZE_FIELD( Id,       ID,           id )
    INITIALIZE_FIELD( QString,  Name,         name )
    INITIALIZE_FIELD( Modes,    Mode,         mode )


public slots:
    void removeOrphanedEntries() override;
private:
    explicit TableEntry();
};

// declare enums
Q_DECLARE_METATYPE( TableEntry::Fields )
Q_DECLARE_METATYPE( TableEntry::Modes )
