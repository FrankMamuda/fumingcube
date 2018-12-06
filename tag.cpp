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

//
// includes
//
#include <QSqlQuery>
#include "tag.h"
#include "field.h"
#include "database.h"

//
// Tags are special property identifiers for built-in customization, such as
//   - tables (sorting, finding, displaying common information amongst reagents)
//   - special widgets (NFPA 704, GHS hazard pictograms)
// Tags ensure that select properties, regardless of their name, can be grouped
//   and properly identified
//
// for example:
//   acetone has a property "GHS Pictograms"
//   ethanol has a property "Hazard Pictograms"
//     if we label both of these properties with a tag "GHS" and bind a special
//     widget to this tag (GHSWidget), both of these reagents will display the
//     correct widget for their properties regardless of their names
//
//  Right now tags are in early alpha
//    eventually reagent properties, such as molarMass, assay, etc. will also
//    be mapped to tags
//

/**
 * @brief Tag::Tag
 * @param parent
 */
Tag::Tag() : Table( TagTable::Name ) {
    this->addField( ID,       "id",         QVariant::UInt,   "integer primary key", true, true );
    this->addField( Name,     "name",       QVariant::String, "text" );
}

/**
 * @brief Tag::add
 * @param name
 */
Row Tag::add( const QString &name ) {
    qDebug() << "add tag" << name;

    return Table::add( QVariantList() << Database_::null << name );
}

/**
 * @brief Tag::removeOrphanedEntries
 */
void Tag::removeOrphanedEntries() {
    // remove orphaned properties
    ;
    this->select();
}
