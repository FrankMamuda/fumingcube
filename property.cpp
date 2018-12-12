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
#include "property.h"
#include "field.h"
#include "database.h"
#include "template.h"

/**
 * @brief Property::Property
 * @param parent
 */
Property::Property() : Table( PropertyTable::Name ) {
    this->addField( ID,       "id",         QVariant::UInt,   "integer primary key", true, true );
    this->addField( Name,     "name",       QVariant::String, "text" );
    this->addField( HTML,     "html",       QVariant::String, "text" );
    this->addField( Template, "templateId", QVariant::Int,    "integer" );
  //this->addField( Tag,      "tagId",      QVariant::Int,    "integer" ); // MUST BREAK API EVENTUALLY
    this->addField( Order,    "parent",     QVariant::Int,    "integer" );

    // enable sorting
    this->setSort( Order, Qt::AscendingOrder );
}

/**
 * @brief Property::add
 * @param name
 */
Row Property::add( const QString &name, const QString &html, const Id &templateId ) {
    qDebug() << "add property" << name;

    // find highest order
    int highest = -1;
    for ( int y = 0; y < this->count(); y++ )
        highest = qMax( highest, this->order( this->row( y )));

    return Table::add( QVariantList() << Database_::null << name << html << static_cast<int>( templateId ) << highest );
}

/**
 * @brief Property::removeOrphanedEntries
 */
void Property::removeOrphanedEntries() {
    // remove orphaned properties
    QSqlQuery().exec( QString( "delete from %1 where %2 not in (select %3 from %4)" )
                .arg( this->tableName())
                .arg( this->fieldName( Template ))
                .arg( Template::instance()->fieldName( Template::ID ))
                .arg( Template::instance()->tableName()));
    this->select();
}
