/*
 * Copyright (C) 2018 Factory #12
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

//
// includes
//
#include "table.h"

/**
 * @brief The TagTable namespace
 */
namespace TagTable {
const static QString Name( "tags" );
}

/**
 * @brief The Tag class
 */
class Tag final : public Table {
    Q_OBJECT
    Q_ENUMS( Fields )
    Q_DISABLE_COPY( Tag )

public:
    enum Fields {
        NoField = -1,
        ID,
        Name,

        // count
        Count
    };

    /**
     * @brief instance
     * @return
     */
    static Tag *instance() { static Tag *instance = new Tag(); return instance; }
    Row add( const QString &name );
    Id id( const Row &row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }
    QString name( const Row &row ) const { return this->value( row, Name ).toString(); }

public slots:
    void setName( const Row &row, const QString &name ) { this->setValue( row, Name, name ); }
    void removeOrphanedEntries() override;

private:
    explicit Tag();
};

// declare enums
Q_DECLARE_METATYPE( Tag::Fields )
