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

#ifndef PROPERTIESMODEL
#define PROPERTIESMODEL

//
// includes
//
#include <QAbstractTableModel>
#include "template.h"

//
// class: PropertiesModel
//
class PropertiesModel : public QAbstractTableModel {
    Q_OBJECT

public:
    PropertiesModel( QObject *parent ) : QAbstractTableModel( parent ), m_reagentId( -1 ) {}
    int rowCount( const QModelIndex & = QModelIndex()) const {
        if ( this->m_reagentId == -1 )
            return 0;

        Template *templatePtr = Template::fromId( this->m_reagentId );
        if ( templatePtr != NULL )
            return templatePtr->propertyList.count();

        return 0;
    }
    int columnCount( const QModelIndex & = QModelIndex()) const { return 2; }
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const {
        if ( this->m_reagentId == -1 )
            return QVariant();

        if ( role == Qt::DisplayRole ) {
            Template *templatePtr = Template::fromId( this->m_reagentId );
            if ( templatePtr != NULL ) {
                Property *propPtr = templatePtr->propertyList.at( index.row());
                if ( propPtr != NULL ) {
                    if ( index.column() == 0 )
                        return propPtr->propertyName();
                    else if ( index.column() == 1 )
                        return propPtr->propertyValue();
                }
            }
        }
        return QVariant();
    }
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const {
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

public slots:
    void setReagentId( const int reagentId = -1 ) {
        this->m_reagentId = reagentId;
        this->reset();
    }
    void reset() {
        this->beginResetModel();
        /*QModelIndex topLeft = this->index(0, 0);
        QModelIndex bottomRight = this->index(this->rowCount()-1, columnCount()-1);
        emit this->dataChanged( topLeft, bottomRight );*/
        this->endResetModel();
    }

private:
    int m_reagentId;
};

#endif // PROPERTIESMODEL

