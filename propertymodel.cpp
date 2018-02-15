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
 * @brief PropertyModel::PropertyModel
 * @param parent
 * @param t
 */
PropertyModel::PropertyModel( QObject *parent, Template *t ) : QAbstractTableModel( parent ), templ( t ), view( nullptr ) {
    this->reset();
}

/**
 * @brief PropertyModel::rowCount
 * @param parent
 * @return
 */
int PropertyModel::rowCount( const QModelIndex & ) const {
    return this->list.count();
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
    if ( this->templ == nullptr || !index.isValid() || this->view == nullptr ) {
        qDebug() << this->tr( "invalid model data" );
        return QVariant();
    }

    property = list.at( index.row());
    if ( property == nullptr )
        return QVariant();

    width = this->view->viewport()->width();

    if ( role == Qt::DisplayRole ) {
        switch ( static_cast<Columns>( index.column())) {
        case Title:
            //qDebug() << property->title() << property->order();
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

/**
 * @brief PropertyModel::reset
 */
void PropertyModel::reset() {
    // begin reset
    this->beginResetModel();

    if ( this->templ == nullptr )
        return;

    // checks whether reindexing of property list is required (lambda)
    auto requiresReindexing = []( const QList<Property*> &propertyList ) {
        QList<int> indices;
        bool reindex = false;

        foreach ( const Property *property, propertyList ) {
            int order = property->order();

            if ( indices.contains( order ) || order == -1 || order >= propertyList.count())
                reindex = true;

            indices << order;
        }

        if ( reindex )
            qDebug() << "reindexing required" << indices;
        else
            qDebug() << "reindexing NOT required" << indices;

        return reindex;
    };

    // reindex list if necessary
    QList<Property*> propertyList( this->templ->propertyMap.values());
    std::sort( propertyList.begin(), propertyList.end(), []( const Property *p0, const Property *p1 ) { return p0->order() < p1->order(); } );

    if ( requiresReindexing( propertyList )) {
        int y = 0;

        foreach ( Property *property, propertyList )
            property->setOrder( y++ );

        /*QList<int> indices;
        foreach ( const Property *property, propertyList )
            indices << property->order();
        qDebug() << "reindexed list" << indices;*/
    }

    // store list
    this->list = propertyList;

    // end reset
    this->endResetModel();
}

