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
#include "propertymodel.h"
#include "template.h"
#include "property.h"
#include <QDebug>

/**
 * @brief PropertyModel::rowCount
 * @param parent
 * @return
 */
int PropertyModel::rowCount( const QModelIndex & ) const {
    // failsafe
    if ( this->entry == nullptr )
        return 0;

    return this->entry->propertyMap.count();
}

/**
 * @brief PropertyModel::columnCount
 * @param parent
 * @return
 */
int PropertyModel::columnCount( const QModelIndex & ) const {
    return Value + 1;
}

/**
 * @brief PropertyModel::data
 * @param index
 * @param role
 * @return
 */
QVariant PropertyModel::data( const QModelIndex &index, int role ) const {
    Property *property;
    int width;

    // failsafe
    if ( this->entry == nullptr || !index.isValid() || this->view == nullptr ) {
        qDebug() << this->tr( "invlaid model data" );
        return QVariant();
    }

    property = this->entry->propertyMap[this->entry->propertyMap.keys().at( index.row())];
    if ( property == nullptr )
        return QVariant();

    width = this->view->viewport()->width();

    if ( role == Qt::DisplayRole ) {
        switch ( static_cast<Columns>( index.column())) {
        case Title:
            return property->title();

        case Value:
            return property->html();

        case NoColumn:
            break;
        }
    } else if ( role == PropertyIdRole ) {
        return property->id();
    } else if ( role == ColumnWidthRole ) {
        switch ( static_cast<Columns>( index.column())) {
        case Title:
            return static_cast<int>( width * 0.33 );

        case Value:
            return static_cast<int>( width * 0.66 );

        case NoColumn:
            return 0;
        }
    }

    return QVariant();
}
