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
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QMap>

/**
 * @brief The PropertyDelegate class
 */
class PropertyDelegate : public QStyledItemDelegate {
public:
    PropertyDelegate( QObject *parent = nullptr ) : QStyledItemDelegate( parent ) {}
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    mutable QMap<QModelIndex, QTextDocument*> documentMap;

public slots:
    void clearDocumentCache() { qDeleteAll( this->documentMap ); this->documentMap.clear(); }

private slots:
    void setupDocument( const QModelIndex &index, const QFont &font ) const;

private:
};
