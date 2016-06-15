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
    int rowCount( const QModelIndex & = QModelIndex()) const;
    int columnCount( const QModelIndex & = QModelIndex()) const { return 2; }
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const;

public slots:
    void setReagentId( const int reagentId = -1 ) {
        this->m_reagentId = reagentId;
        this->reset();
    }
    void reset() {
        this->beginResetModel();
        this->endResetModel();
    }

private:
    int m_reagentId;
};

#endif // PROPERTIESMODEL

