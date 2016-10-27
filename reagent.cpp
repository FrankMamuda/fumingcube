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

#include "reagent.h"
#include <QSqlQuery>
#include <QSqlError>
#include "database.h"

/**
 * @brief Reagent::add
 * @param name
 * @param amount
 * @param density
 * @param assay
 * @param molarMass
 * @param state
 */
void Reagent::add( const QString &name, const double amount, const double density, const double assay, const double molarMass, const State state ) {
    QSqlQuery query;

    query.prepare( QString( "insert into reagents values ( null, :name, :amount, :density, :assay, :molarMass, :state )" ));
    query.bindValue( ":name", name );
    query.bindValue( ":amount", amount );
    query.bindValue( ":density", density );
    query.bindValue( ":assay", assay );
    query.bindValue( ":molarMass", molarMass );
    query.bindValue( ":state", state );

    // TODO: eventually drop all properties in favour of templates
    //
    // reagent (NaOH)
    //     -template (solid) - 40 g/mol, etc., etc. (named <default> on addition)
    //     -template (20% solution)
    //     -template (40% solution)
    //

    if ( !query.exec()) {
        Main::error( Main::SoftError, QString( "Reagent::add: could not add reagent, reason - '%1'\n" ).arg( query.lastError().text()));
        return;
    }

    query.exec( QString( "select * from reagents where id=%1" ).arg( query.lastInsertId().toInt()));
    while ( query.next()) {
        db.reagentList << new Reagent( query.record());
        break;
    }
}

/**
 * @brief Reagent::fromId
 * @param id
 * @return
 */
Reagent *Reagent::fromId( int id ) {
    foreach ( Reagent *reagentPtr, db.reagentList ) {
        if ( reagentPtr->id() == id )
            return reagentPtr;
    }
    return NULL;
}
