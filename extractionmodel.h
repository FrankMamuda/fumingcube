/*
 * Copyright (C) 2017-2018 Factory #12
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

#pragma once

/*
 * includes
 */
#include <QAbstractListModel>

/**
 * @brief The ExtractionModel class
 */
class ExtractionModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit ExtractionModel( QObject *parent = nullptr, const QStringList &entries = QStringList()) : QAbstractListModel( parent ), list( entries ) {}
    int rowCount( const QModelIndex & = QModelIndex()) const override { return this->list.count(); }
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override {
        if ( index.row() < 0 || index.row() >= this->list.count())
            return QVariant();

        if ( role == Qt::DisplayRole )
            return this->list.at( index.row());

        return QVariant();
    }
    void reset( const QStringList &entries = QStringList()) { this->beginResetModel(); this->list = entries; this->endResetModel(); }

private:
    QStringList list;
};
