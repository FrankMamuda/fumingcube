/*
 * Copyright (C) 2020 Armands Aleksejevs
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
#include "labelset.h"
#include "field.h"
#include "database.h"
#include "label.h"
#include "reagent.h"

#include <QSqlQuery>

/**
 * @brief LabelSet::LabelSet
 */
LabelSet::LabelSet() : Table( "labelset" ) {
    this->addField( PRIMARY_FIELD( ID ) );
    this->addField( FIELD( LabelId, Int ) );
    this->addField( FIELD( ReagentId, Int ) );
}

/**
 * @brief LabelSet::add
 * @param labelId
 * @param reagentId
 * @return
 */
Row LabelSet::add( const Id &labelId, const Id &reagentId ) {
    return Table::add(
            QVariantList() << Database_::null << static_cast<int>( labelId ) << static_cast<int>( reagentId ));
}

/**
 * @brief LabelSet::remove
 * @param labelId
 * @param reagentId
 */
void LabelSet::remove( const Id &labelId, const Id &reagentId ) {
    QSqlQuery().exec( QString( "delete from %1 where %2=%3 and %4=%5" )
                              .arg( this->tableName())
                              .arg( this->fieldName( LabelId ))
                              .arg( static_cast<int>( labelId ))
                              .arg( this->fieldName( ReagentId ))
                              .arg( static_cast<int>( reagentId )));
}

/**
 * @brief LabelSet::removeOrphanedEntries
 */
void LabelSet::removeOrphanedEntries() {
    // remove if label has been deleted
    QSqlQuery().exec( QString( "delete from %1 where %2 not in ( select %3 from %4 )" )
                              .arg( this->tableName())
                              .arg( this->fieldName( LabelId ))
                              .arg( Label::instance()->fieldName( Label::ID ))
                              .arg( Label::instance()->tableName()));

    // remove if reagent has been deleted
    QSqlQuery().exec( QString( "delete from %1 where %2 not in ( select %3 from %4 )" )
                              .arg( this->tableName())
                              .arg( this->fieldName( ReagentId ))
                              .arg( Reagent::instance()->fieldName( Reagent::ID ))
                              .arg( Reagent::instance()->tableName()));
}
