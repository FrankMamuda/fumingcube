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
 * @brief The LabelSet class
 */
class LabelSet final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY( LabelSet )

public:
    /**
     * @brief The Fields enum
     */
    enum Fields {
        NoField = -1,
        ID,
        LabelId,
        ReagentId,

        // count (DO NOT REMOVE)
        Count
    };
    Q_ENUM( Fields )

    /**
     * @brief instance
     * @return
     */
    static LabelSet *instance() { static LabelSet *labelSet( new LabelSet()); return labelSet; }
    ~LabelSet() override = default;
    Row add( const Id &labelId, const Id &reagentId );
    void remove( const Id &labelId, const Id &reagentId );

    // initialize field setters and getters
    INITIALIZE_FIELD( Id, ID,        id )
    INITIALIZE_FIELD( Id,LabelId,   labelId )
    INITIALIZE_FIELD( Id, ReagentId, reagentId )

public slots:
    void removeOrphanedEntries() override;

private:
    explicit LabelSet();
};

// declare enums
Q_DECLARE_METATYPE( LabelSet::Fields )
