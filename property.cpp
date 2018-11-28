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
 * @brief Property::fromTitle
 * @param title
 * @param templateId
 * @return
 */
Property *Property::fromTitle( const QString &title, int templateId ) {
    const Template *templ( Template::fromId( templateId ));
    if ( templ == nullptr )
        return nullptr;

    foreach ( Property *property, templ->propertyMap ) {
        if ( !QString::compare( property->title(), title ))
            return property;
    }

    return nullptr;
}

/**
 * @brief Property::add
 * @param name
 */
Property *Property::add( const QString &title, const QString &value, int templateId ) {
    QSqlQuery query;

    // check for duplicates
    Property *property( Property::fromTitle( title, templateId ));
    if ( property != nullptr ) {
        qWarning() << QObject::tr( "duplicate property \"%1\" ignored" ).arg( title );
        return property;
    }

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
        Template *templ( Template::fromId( property->templateId()));
        if ( templ == nullptr )
            return property;

        // add property to template's propertyMap
        templ->propertyMap[property->id()] = property;
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
    // construct a property entry from sql recort and store in memory
    Property *property( new Property( query.record()));
    Database::instance()->propertyMap[property->id()] = property;

    // retrieve template from templateId
    Template *templ( Template::fromId( property->templateId()));
    if ( templ == nullptr )
        return property;

    // add property to templates
    templ->propertyMap[property->id()] = property;
    return property;
}

/**
 * @brief Property::contains
 * @param name
 */
bool Property::contains( const QString &title ) {
    foreach ( const Property *property, Database::instance()->propertyMap ) {
        if ( !QString::compare( property->title(), title ))
            return true;
    }

    return false;
}
