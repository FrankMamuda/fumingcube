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
#include "template.h"
#include "database.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

/**
 * @brief Template::fromId
 * @param id
 * @return
 */
Template *Template::fromId( int id ) {
    if ( Database::instance()->templateMap.contains( id ))
        return Database::instance()->templateMap[id];

    return nullptr;
}

/**
 * @brief Template::add
 * @param name
 * @param amount
 * @param density
 * @param assay
 * @param molarMass
 * @param state
 */
Template *Template::add( const QString &name, const double amount, const double density, const double assay, const double molarMass, const State state, const int reagentId ) {
    QSqlQuery query;
    Template *templ = nullptr;

    // prepare statement
    query.prepare( QString( "insert into templates values ( null, :name, :amount, :density, :assay, :molarMass, :state, :reagentId )" ));
    query.bindValue( ":name", name );
    query.bindValue( ":amount", amount );
    query.bindValue( ":density", density );
    query.bindValue( ":assay", assay );
    query.bindValue( ":molarMass", molarMass );
    query.bindValue( ":state", state );
    query.bindValue( ":reagentId", reagentId );

    // excecute statement
    if ( !query.exec()) {
        qCritical() << QObject::tr( "could not add template, reason - '%1'" ).arg( query.lastError().text());
        return templ;
    }

    // select the newly created entry and store in memory
    query.exec( QString( "select * from templates where id=%1" ).arg( query.lastInsertId().toInt()));
    if ( query.next())
        templ = Template::store( query );

    return templ;
}

/**
 * @brief Template::load
 */
void Template::load() {
    QSqlQuery query;

    // announce
    qInfo() << QObject::tr( "loading templates from database" );

    // read all template entries
    if ( !query.exec( "select * from templates order by name asc;" ))
        qCritical() << query.lastError().text();

    // store entries in memory
    while ( query.next())
        Template::store( query );
}

/**
 * @brief Template::store
 * @param query
 */
Template *Template::store( const QSqlQuery &query ) {
    Reagent *reagent;
    Template *templ = nullptr;

    // construct a template entry from sql recort and store in memory
    templ = new Template( query.record());
    Database::instance()->templateMap[templ->id()] = templ;

    // retrieve reagent from reagentId
    reagent = Reagent::fromId( templ->reagentId());
    if ( reagent == nullptr )
        return templ;

    // add template to reagent's templateMap
    reagent->templateMap[templ->id()] = templ;
    return templ;
}
