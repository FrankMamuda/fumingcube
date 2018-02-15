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
 * @brief The TemplateModel class
 */
class TemplateModel : public QAbstractListModel {
    Q_OBJECT

public:
    /**
     * @brief TemplateModel
     * @param parent
     * @param t
     */
    explicit TemplateModel( QObject *parent = nullptr ) : QAbstractListModel( parent ), reagent( nullptr ) {}

    /**
     * @brief ~TemplateModel
     */
    ~TemplateModel() {}

    /**
     * @brief rowCount
     * @return
     */
    int rowCount( const QModelIndex & = QModelIndex()) const override {
        if ( this->reagent == nullptr )
            return 0;

        return this->reagent->templateMap.count();
    }

    /**
     * @brief data
     * @param index
     * @param role
     * @return
     */
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override {
        Template *templ;

        if ( !index.isValid() || reagent == nullptr )
            return QVariant();

        templ = this->reagent->templateMap.values().at( index.row());
        if ( templ != nullptr ) {
            if ( role == Qt::DisplayRole)
                return templ->name();

            if ( role == Qt::UserRole )
                return templ->id();
        }

        return QVariant();
    }

public slots:
    /**
     * @brief reset
     */
    void reset( Reagent *r ) { this->reagent = r; this->beginResetModel(); this->endResetModel(); }

private:
    Reagent *reagent;
};
