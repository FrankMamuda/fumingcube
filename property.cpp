/*
===========================================================================
Copyright (C) 2016 Avotu Briezhaudzetava

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

//
// includes
//
#include "property.h"
#include "database.h"

/**
 * @brief Property::fromId
 * @param id
 * @return
 */
Property *Property::fromId( int id ) {
    foreach ( Property *propPtr, db.propertyList ) {
        if ( propPtr->id() == id )
            return propPtr;
    }
    return NULL;
}

/**
 * @brief Property::add
 * @param reagentId
 * @param property
 * @param value
 */
void Property::add( const int reagentId, const QString &property, const QString &value ) {
    QSqlQuery query;

    query.prepare( QString( "insert into properties values ( null, :reagentId, :property, :value )" ));
    query.bindValue( ":reagentId", reagentId );
    query.bindValue( ":property", property );
    query.bindValue( ":value", value );

    if ( !query.exec()) {
        Main::error( Main::SoftError, QObject::tr( "Property::add: could not add property, reason - '%1'\n" ).arg( query.lastError().text()));
        return;
    }

    query.exec( QString( "select * from properties where id=%1" ).arg( query.lastInsertId().toInt()));
    while ( query.next()) {
        Property *propPtr = new Property( query.record());
        db.propertyList << propPtr;

        foreach ( Reagent *reagentPtr, db.reagentList ) {
            if ( reagentPtr->id() == reagentId )
                reagentPtr->propertyList << propPtr;
        }

        break;
    }
}
