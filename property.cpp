/*
 * Copyright (C) 2019 Armands Aleksejevs
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
#include "property.h"
#include "field.h"
#include "database.h"
#include "tag.h"
#include "reagent.h"
#include <QSqlQuery>

/**
 * @brief Property::Property
 */
Property::Property() : Table( "property" ) {
    this->addField( PRIMARY_FIELD( ID )); // Id
    this->addField( FIELD( Name, String ));      // rich text
    this->addField( FIELD( TagID, Int ));        // special tag
    this->addField( FIELD( Value, ByteArray ));  // value (can be anything)
    this->addField( FIELD( ReagentID, Int ));    // Id in parent table
    this->addField( FIELD( Index, Int ));        // order
    this->setSort( Index, Qt::AscendingOrder );
}

/**
 * @brief Property::add
 * @param name
 * @param tagId
 * @param value
 * @param tableId
 * @param parentId
 * @return
 */
Row Property::add( const QString &name, const Id &tagId,
                   const QByteArray &value, const Id &parentId ) {

    return Table::add( QVariantList() << Database_::null <<
                       (( tagId == Id::Invalid || tagId == PixmapTag ) ? name : QByteArray()) <<
                       static_cast<int>( tagId ) <<
                       value <<
                       static_cast<int>( parentId )
                       );
}

/**
 * @brief Property::data
 * @param index
 * @param role
 * @return
 */
QVariant Property::data( const QModelIndex &index, int role ) const {
    const QVariant value( Table::data( index, role ));

    if ( role == Qt::DisplayRole && ( index.column() == Name )) {
        const Row row = this->row( index );
        const Id tagId = this->tagId( row );

        // FIXME:
        if ( tagId == Id::Invalid )
            return value;

        const Row tagRow = Tag::instance()->row( tagId );
        if ( tagRow == Row::Invalid )
            return QVariant();

        // FIXME: proper find
        if ( index.column() == Name )
            return Tag::instance()->name( tagRow );
    }

    return value;
}

/**
 * @brief Property::removeOrphanedEntries
 */
void Property::removeOrphanedEntries() {
    QSqlQuery().exec( QString( "delete from %1 where %2 not in ( select %3 from %4 )" )
                      .arg( this->tableName())
                      .arg( this->fieldName( Property::ReagentID ))
                      .arg( Reagent::instance()->fieldName( Reagent::ID ))
                      .arg( Reagent::instance()->tableName()));
}
