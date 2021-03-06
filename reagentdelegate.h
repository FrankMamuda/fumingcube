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
#include <QListWidget>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QTreeView>
#include <QApplication>

/**
 * @brief The ReagentDelegate class
 */
class ReagentDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit ReagentDelegate( QObject *parent = nullptr ) : QStyledItemDelegate( parent ) {
        const QListWidget w;
        this->internalFont = QFont( QApplication::font( &w ));
    }

    /**
     * @brief model
     * @return
     */
    [[nodiscard]] QSortFilterProxyModel *model() const { return this->m_model; }

    /**
     * @brief sourceModel
     * @return
     */
    [[nodiscard]] ReagentModel *sourceModel() const { return qobject_cast<ReagentModel *>( this->m_model->sourceModel()); }

    /**
     * @brief parentView
     * @return
     */
    [[nodiscard]] QTreeView *parentView() const { return this->m_view; }

    /**
     * @brief viewMode
     * @return
     */
    [[nodiscard]] bool viewMode() const { return this->m_viewMode; }

public slots:
    /**
     * @brief setModel
     * @param model
     */
    void setModel( QSortFilterProxyModel *model ) { this->m_model = model; }

    /**
     * @brief clearCache
     */
    void clearCache() {
        if ( this->cache.isEmpty())
            return;

        qDeleteAll( this->cache );
        this->cache.clear();
    }

    /**
     * @brief setParentView
     * @param view
     */
    void setParentView( QTreeView *view ) { this->m_view = view; }

    /**
     * @brief setViewMode
     * @param viewMode
     */
    void setViewMode( bool viewMode = true ) {
        this->m_viewMode = viewMode;
    }

protected:
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    [[nodiscard]] QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

private:
    QSortFilterProxyModel *m_model = nullptr;
    mutable QMap<QModelIndex, QTextDocument *> cache;
    QTreeView *m_view = nullptr;
    bool m_viewMode = false;
    QFont internalFont;
};
