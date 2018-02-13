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
#include <QAbstractListModel>
#include "database.h"

/**
 * @brief The ReagentModel class
 */
class ReagentModel : public QAbstractListModel {
    Q_OBJECT

public:
    /**
     * @brief ReagentModel
     * @param parent
     * @param t
     */
    explicit ReagentModel( QObject *parent = nullptr ) : QAbstractListModel( parent ) {
        //this->connect( Database::instance(), SIGNAL( changed()), this, SLOT( reset()));
    }

    /**
     * @brief ~ReagentModel
     */
    ~ReagentModel() { /*this->disconnect( Database::instance(), SIGNAL( changed()), this, SLOT( reset()));*/ }

    /**
     * @brief rowCount
     * @return
     */
    int rowCount( const QModelIndex & = QModelIndex()) const override { return Database::instance()->reagentMap.count() + 1; }

    /**
     * @brief data
     * @param index
     * @param role
     * @return
     */
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override {
        Reagent *reagent;

        if ( !index.isValid())
            return QVariant();

        if ( index.row() < Database::instance()->reagentMap.count()) {
            reagent = Database::instance()->reagentMap.values().at( index.row());
            if ( reagent != nullptr ) {
                if ( role == Qt::DisplayRole || role == Qt::EditRole )
                    return reagent->name();

                if ( role == Qt::UserRole )
                    return reagent->id();
            }
        } else {
            if ( role == Qt::DisplayRole || role == Qt::EditRole )
                return this->tr( "<none>" );

            if ( role == Qt::UserRole )
                return -1;
        }

        return QVariant();
    }

public slots:
    /**
     * @brief reset
     */
    void reset() { this->beginResetModel(); this->endResetModel(); }
};
