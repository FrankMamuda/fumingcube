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

#include "template.h"
#include <QSqlQuery>
#include <QSqlError>
#include "database.h"

/**
 * @brief Template::add
 * @param name
 * @param amount
 * @param density
 * @param assay
 * @param molarMass
 * @param state
 */
void Template::add( const QString &name, const double amount, const double density, const double assay, const double molarMass, const State state ) {
    QSqlQuery query;

    query.prepare( QString( "insert into templates values ( null, :name, :amount, :density, :assay, :molarMass, :state )" ));
    query.bindValue( ":name", name );
    query.bindValue( ":amount", amount );
    query.bindValue( ":density", density );
    query.bindValue( ":assay", assay );
    query.bindValue( ":molarMass", molarMass );
    query.bindValue( ":state", state );

    if ( !query.exec()) {
        Main::error( Main::SoftError, QString( "Template::add: could not add template, reason - '%1'\n" ).arg( query.lastError().text()));
        return;
    }

    query.exec( QString( "select * from templates where id=%1" ).arg( query.lastInsertId().toInt()));
    while ( query.next()) {
        db.templateList << new Template( query.record());
        break;
    }
}

/**
 * @brief Template::fromId
 * @param id
 * @return
 */
Template *Template::fromId( int id ) {
    foreach ( Template *templatePtr, db.templateList ) {
        if ( templatePtr->id() == id )
            return templatePtr;
    }
    return NULL;
}
