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
#include "property.h"
#include "field.h"
#include "database.h"

/**
 * @brief Property::Property
 * @param parent
 */
Property::Property() : Table( PropertyTable::Name ) {
    this->addField( ID,       "id",         QVariant::UInt,   "integer primary key", true, true );
    this->addField( Name,     "name",       QVariant::String, "text" );
    this->addField( HTML,     "html",       QVariant::String, "text" );
    this->addField( Template, "templateId", QVariant::Int,    "integer" );
    this->addField( Order,    "parent",     QVariant::Int,    "integer" );
}

/**
 * @brief Property::add
 * @param name
 */
Row Property::add( const QString &name, const QString &html, const Id &templateId ) {
    return Table::add( QVariantList() << Database_::null << name << html << static_cast<int>( templateId ) << 0 );
}
