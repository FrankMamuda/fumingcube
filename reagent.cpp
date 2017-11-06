/*
 * Copyright (C) 2017 Factory #12
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
#include "reagent.h"
#include "database.h"

/**
 * @brief Reagent::fromId
 * @param id
 * @return
 */
Reagent *Reagent::fromId( int id ) {
    if ( Database::instance()->reagentMap.contains( id ))
        return Database::instance()->reagentMap[id];

    return nullptr;
}

/**
 * @brief Reagent::add
 * @param name
 */
Reagent *Reagent::add( const QString &name ) {
    QSqlQuery query;
    Reagent *reagent = nullptr;

    // prepare statement
    query.prepare( QString( "insert into reagents values ( null, :name )" ));
    query.bindValue( ":name", name );

    // excecute statement
    if ( !query.exec()) {
        qCritical() << QObject::tr( "could not add reagent, reason - '%1'" ).arg( query.lastError().text());
        return reagent;
    }

    // select the newly created entry and store in memory
    query.exec( QString( "select * from reagents where id=%1" ).arg( query.lastInsertId().toInt()));
    if ( query.next()) {
        reagent = new Reagent( query.record());
        Database::instance()->reagentMap[reagent->id()] = reagent;
    }

    return reagent;
}

/**
 * @brief Reagent::load
 */
void Reagent::load() {
    QSqlQuery query;

    // announce
    qInfo() << QObject::tr( "loading reagents from database" );

    // read all reagent entries
    if ( !query.exec( "select * from reagents order by name asc;" ))
        qCritical() << query.lastError().text();

    // store entries in memory
    while ( query.next()) {
        Reagent *reagent;

        reagent = new Reagent( query.record());
        Database::instance()->reagentMap[reagent->id()] = reagent;
    }
}

/**
 * @brief Reagent::contains
 * @param name
 */
bool Reagent::contains( const QString &name ) {
    foreach ( Reagent *reagent, Database::instance()->reagentMap ) {
        if ( !QString::compare( reagent->name(), name ))
            return true;
    }

    return false;
}
