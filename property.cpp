/*
 * Copyright (C) 2017-2018 Factory #12
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
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include "property.h"
#include "database.h"

/**
 * @brief Property::fromId
 * @param id
 * @return
 */
Property *Property::fromId( int id ) {
    if ( Database::instance()->propertyMap.contains( id ))
        return Database::instance()->propertyMap[id];

    return nullptr;
}

/**
 * @brief Property::add
 * @param name
 */
Property *Property::add( const QString &title, const QString &value, int templateId ) {
    QSqlQuery query;
    Property *property = nullptr;
    Template *entry;

    // prepare statement
    query.prepare( QString( "insert into properties values ( null, :name, :value, :templateId, :parent )" ));
    query.bindValue( ":name", title );
    query.bindValue( ":value", value );
    query.bindValue( ":templateId", templateId );
    query.bindValue( ":parent", -1 );

    // excecute statement
    if ( !query.exec()) {
        qCritical() << QObject::tr( "could not add property, reason - '%1'" ).arg( query.lastError().text());
        return property;
    }

    // select the newly created entry and store in memory
    query.exec( QString( "select * from properties where id=%1" ).arg( query.lastInsertId().toInt()));
    if ( query.next()) {
        property = new Property( query.record());
        Database::instance()->propertyMap[property->id()] = property;

        // retrieve template from templateId
        entry = Template::fromId( property->templateId());
        if ( entry == nullptr )
            return property;

        // add property to template's propertyMap
        entry->propertyMap[property->id()] = property;
    }

    return property;
}

/**
 * @brief Property::load
 */
void Property::load() {
    QSqlQuery query;

    // announce
    qInfo() << QObject::tr( "loading properties from database" );

    // read all property entries
    if ( !query.exec( "select * from properties order by name asc;" ))
        qCritical() << query.lastError().text();

    // store entries in memory
    while ( query.next())
        Property::store( query );
}

/**
 * @brief Template::store
 * @param query
 */
Property *Property::store( const QSqlQuery &query ) {
    Template *entry;
    Property *property = nullptr;

    // construct a property entry from sql recort and store in memory
    property = new Property( query.record());
    Database::instance()->propertyMap[property->id()] = property;

    // retrieve template from templateId
    entry = Template::fromId( property->templateId());
    if ( entry == nullptr )
        return property;

    // add property to templates
    entry->propertyMap[property->id()] = property;
    return property;
}

/**
 * @brief Property::contains
 * @param name
 */
bool Property::contains( const QString &title ) {
    foreach ( Property *property, Database::instance()->propertyMap ) {
        if ( !QString::compare( property->title(), title ))
            return true;
    }

    return false;
}
