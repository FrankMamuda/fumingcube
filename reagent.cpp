/*
 * Copyright (C) 2019-2020 Armands Aleksejevs
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

/*
 * includes
 */
#include "reagent.h"
#include "field.h"
#include "database.h"
#include <QSqlQuery>

/**
 * @brief Reagent::Reagent
 */
Reagent::Reagent() : Table( "reagent" ) {
    this->addField( PRIMARY_FIELD( ID ));
    this->addField( FIELD( Name, String ));
    this->addField( FIELD( Alias, String ));
    this->addField( FIELD( ParentId, Int ));
//    this->setSort( Name, Qt::AscendingOrder ); this has no effect
}

/**
 * @brief Reagent::add
 * @param name
 * @return
 */
Row Reagent::add( const QString &name, const QString &alias, const Id &parentId ) {
    return Table::add( QVariantList() << Database_::null << name << alias << static_cast<int>( parentId ));
}

/**
 * @brief Reagent::children
 * @return
 */
QList<Row> Reagent::children( const Row &row ) const {
    QList<Row> list;

    const Id id = this->id( row );
    QSqlQuery query;
    query.exec( QString( "select %1 from %2 where %3=%4" )
                .arg( Reagent::instance()->fieldName( ID ))
                .arg( Reagent::instance()->tableName())
                .arg( Reagent::instance()->fieldName( ParentId ))
                .arg( static_cast<int>( id )));
    while ( query.next()) {
        const Id id = query.value( 0 ).value<Id>();
        list << this->row( id );
    }

    return list;
}

/**
 * @brief Reagent::removeOrphanedEntries
 */
void Reagent::removeOrphanedEntries() {
    // NOTE: STUB
}
