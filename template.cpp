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
#include "template.h"
#include "field.h"
#include "database.h"
#include "reagent.h"

/**
 * @brief Template::Template
 * @param parent
 */
Template::Template() : Table( TemplateTable::Name ) {
    this->addField( ID,        "id",         QVariant::UInt,   "integer primary key", true, true );
    this->addField( Name,      "name",       QVariant::String, "text" );
    this->addField( Amount,    "amount",     QVariant::Double, "float" );
    this->addField( Density,   "density",    QVariant::Double, "float" );
    this->addField( Assay,     "assay",      QVariant::Double, "float" );
    this->addField( MolarMass, "molarMass",  QVariant::Double, "float" );
    this->addField( ChemState, "state",      QVariant::Int,    "integer" );
    this->addField( Reagent,   "reagentId",  QVariant::Int,    "integer" );
}

/**
 * @brief Template::add
 * @param name
 */
Row Template::add( const QString &name, const double amount, const double density, const double assay, const double molarMass, const State &state, const Id &reagentId ) {
    qDebug() << "add template" << name;

    return Table::add( QVariantList() << Database_::null <<
                       name << amount << density << assay << molarMass << static_cast<int>( state ) << static_cast<int>( reagentId ));
}

/**
 * @brief Template::removeOrphanedEntries
 */
void Template::removeOrphanedEntries() {
    // remove orphaned templates
    QSqlQuery().exec( QString( "delete from %1 where %2 not in (select %3 from %4)" )
                .arg( this->tableName())
                .arg( this->fieldName( Reagent ))
                .arg( Reagent::instance()->fieldName( Reagent::ID ))
                .arg( Reagent::instance()->tableName()));
    this->select();
}
