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

/*
 * includes
 */
#include "tableentry.h"
#include "database.h"

/**
 * @brief TableEntry::TableEntry
 */
TableEntry::TableEntry() : Table( "table_" ) {
    this->addField( PRIMARY_FIELD( ID ) );
    this->addField( UNIQUE_FIELD( Name, QString ));
    this->addField( FIELD( Mode, Int ));
}

/**
 * @brief TableEntry::add
 * @param name
 * @return
 */
Row TableEntry::add( const QString &name, const Modes &mode ) {
    return Table::add( QVariantList() << Database_::null << name << static_cast<int>( mode ));
}

/**
 * @brief TableEntry::removeOrphanedEntries
 */
void TableEntry::removeOrphanedEntries() {
    // NOTE: property tables do not have foreign ids, therefore they cannot be orphaned
}
