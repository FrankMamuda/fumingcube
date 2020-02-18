/*
 * Copyright (C) 2020 Armands Aleksejevs
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
#include "reagentmodel.h"

#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStyledItemDelegate>
#include <QTextDocument>

/**
 * @brief The ReagentDelegate class
 */
class ReagentDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    QSortFilterProxyModel *model() const { return this->m_model; }
    ReagentModel *sourceModel() const { return qobject_cast<ReagentModel *>( this->m_model->sourceModel()); }

public slots:
    void setModel( QSortFilterProxyModel *model ) { this->m_model = model; }
    void clearCache() { this->cache.clear(); }

protected:
    void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const;

private:
    QSortFilterProxyModel *m_model = nullptr;
    mutable QMap<QString, QTextDocument*> cache;
};