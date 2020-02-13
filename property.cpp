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
    this->addField( FIELD( TagId, Int ));        // special tag
    this->addField( FIELD( PropertyData, ByteArray ));  // value (can be anything)
    this->addField( FIELD( ReagentId, Int ));    // Id in parent table
    this->addField( FIELD( TableOrder, Int ));        // order
    this->setSort( TableOrder, Qt::AscendingOrder );
}

/**
 * @brief Property::add
 * @param name
 * @param tagId
 * @param value
 * @param reagentId
 * @return
 */
Row Property::add( const QString &name, const Id &tagId, const QVariant &value, const Id &reagentId ) {
    // advance position in propertyView
    int highestOrder = 0;
    for ( int y = 0; y < Property::instance()->count(); y++ )
        highestOrder = qMax( highestOrder, Property::instance()->tableOrder( static_cast<Row>( y )));
    highestOrder++;

    bool pixmap = false;
    if ( tagId != Id::Invalid )
        pixmap = Tag::instance()->type( tagId ) == Tag::Formula || tagId == PixmapTag;

    // add the property
    return Table::add( QVariantList() << Database_::null <<
                       (( tagId == Id::Invalid || pixmap ) ? name : QString()) <<
                       static_cast<int>( tagId ) <<
                       value.toByteArray() <<
                       static_cast<int>( reagentId ) <<
                       qAsConst( highestOrder )
                       );
}

/**
 * @brief Property::headerData
 * @param section
 * @param orientation
 * @param role
 * @return
 */
QVariant Property::headerData( int section, Qt::Orientation orientation, int role ) const {
    if ( role == Qt::DisplayRole && orientation == Qt::Horizontal ) {
        switch ( section ) {
        case Property::Name:
            return this->tr( "Property" );

        case Property::PropertyData:
            return this->tr( "Value" );

        default:
            break;
        }
    }

    return Table::headerData( section, orientation, role );
}

/**
 * @brief Property::removeOrphanedEntries
 */
void Property::removeOrphanedEntries() {
    QSqlQuery().exec( QString( "delete from %1 where %2 not in ( select %3 from %4 )" )
                      .arg( this->tableName())
                      .arg( this->fieldName( Property::ReagentId ))
                      .arg( Reagent::instance()->fieldName( Reagent::ID ))
                      .arg( Reagent::instance()->tableName()));

    // NOTE: must not delete -1 and -2 tags (NoTag and PixmapTag)
    QSqlQuery().exec( QString( "delete from %1 where %2!=-1 and %2!=-2 and %2 not in ( select %3 from %4 )" )
                      .arg( this->tableName())
                      .arg( this->fieldName( Property::TagId ))
                      .arg( Tag::instance()->fieldName( Tag::ID ))
                      .arg( Tag::instance()->tableName()));
}
