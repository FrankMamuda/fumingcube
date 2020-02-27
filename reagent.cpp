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
#include "labelset.h"
#include <QSqlQuery>

/**
 * @brief Reagent::Reagent
 */
Reagent::Reagent() : Table( "reagent" ) {
    this->addField( PRIMARY_FIELD( ID ) );
    this->addField( FIELD( Name, String ) );
    this->addField( FIELD( Reference, String ) );
    this->addField( FIELD( ParentId, Int ) );
}

/**
 * @brief Reagent::add
 * @param name
 * @return
 */
Row Reagent::add( const QString &name, const QString &reference, const Id &parentId ) {
    return Table::add( QVariantList() << Database_::null << name << reference << static_cast<int>( parentId ));
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
                        .arg( Reagent::instance()->fieldName( ID ),
                              Reagent::instance()->tableName(),
                              Reagent::instance()->fieldName( ParentId ),
                              QString::number( static_cast<int>( id ))));
    while ( query.next()) {
        list << this->row( query.value( 0 ).value<Id>());
    }

    return list;
}

/**
 * @brief Reagent::labelIds
 * @param row
 * @return
 */
QList<Id> Reagent::labelIds( const Row &row ) const {
    QList<Id> list;

    Id reagentId = this->id( row );
    const Id parentId = this->parentId( row );
    if ( parentId != Id::Invalid )
        reagentId = parentId;

    QSqlQuery query;
    query.exec( QString( "select %1 from %2 where %3=%4" )
                        .arg( LabelSet::instance()->fieldName( LabelSet::LabelId ),
                              LabelSet::instance()->tableName(),
                              LabelSet::instance()->fieldName( LabelSet::ReagentId ),
                              QString::number( static_cast<int>( qAsConst( reagentId )))));
    while ( query.next()) {
        const auto id = query.value( 0 ).value<Id>();
        list << id;
    }

    return list;
}

/**
 * @brief Reagent::removeOrphanedEntries
 */
void Reagent::removeOrphanedEntries() {
    // NOTE: labels do not have foreign ids, therefore they cannot be orphaned
    //       batch deletion is handled elsewhere, so that there should not be
    //       any orphaned batches
}
