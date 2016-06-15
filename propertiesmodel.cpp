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

//
// includes
//
#include "propertiesmodel.h"

/*
==========
headerData
==========
*/
QVariant PropertiesModel::headerData( int section, Qt::Orientation orientation, int role ) const {
    if ( role == Qt::DisplayRole ) {
        if ( orientation == Qt::Horizontal ) {
            switch ( section ) {
            case 0:
                return QString( "Parameter" );

            case 1:
                return QString( "Value" );
            }
        }
    }

    return QVariant();
}

/*
==========
data
==========
*/
QVariant PropertiesModel::data( const QModelIndex &index, int role ) const {
    if ( this->m_reagentId == -1 )
        return QVariant();

    Template *templatePtr = Template::fromId( this->m_reagentId );
    if ( templatePtr == NULL )
        return QVariant();

    Property *propPtr = templatePtr->propertyList.at( index.row());
    if ( propPtr == NULL )
        return QVariant();

    if ( role == Qt::DisplayRole ) {
        if ( index.column() == 0 )
            return propPtr->propertyName();
        else if ( index.column() == 1 )
            return propPtr->propertyValue();

    } else if ( role == Qt::UserRole ) {
        return propPtr->id();
    }

    return QVariant();
}

/*
==========
rowCount
==========
*/
int PropertiesModel::rowCount( const QModelIndex & ) const {
    if ( this->m_reagentId == -1 )
        return 0;

    Template *templatePtr = Template::fromId( this->m_reagentId );
    if ( templatePtr != NULL )
        return templatePtr->propertyList.count();

    return 0;
}
