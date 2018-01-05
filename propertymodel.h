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

#pragma once

//
// includes
//
#include <QAbstractTableModel>

//
// classes
//
class Template;

/**
 * @brief The PropertyModel class
 */
class PropertyModel : public QAbstractTableModel {
    Q_OBJECT
    Q_ENUMS( Columns )

public:
    enum Columns {
        NoColumn = -1,
        Title,
        Value
    };

    explicit PropertyModel( QObject *parent = nullptr, Template *t = nullptr ) : QAbstractTableModel( parent ), entry( t ) {}
    int rowCount( const QModelIndex &parent = QModelIndex()) const override;
    int columnCount( const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole ) const override;

private:
    Template *entry;
};
