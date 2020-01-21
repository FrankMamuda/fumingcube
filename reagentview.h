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
#include "nodehistory.h"
#include "reagentmodel.h"
#include <QSortFilterProxyModel>
#include <QTreeView>

/**
 * @brief The ReagentView class
 */
class ReagentView : public QTreeView {
    Q_OBJECT

public:
    explicit ReagentView( QWidget *parent = nullptr );

    /**
     * @brief ~ReagentView::~ReagentView
     */
    ~ReagentView() { delete this->m_nodeHistory; }

    /**
     * @brief model
     * @return
     */
    ReagentModel *model() const { return qobject_cast<ReagentModel*>( this->filterModel()->sourceModel()); }

    /**
     * @brief filterModel
     * @return
     */
    QSortFilterProxyModel *filterModel() const { return qobject_cast<QSortFilterProxyModel*>( QTreeView::model()); }

    /**
     * @brief nodeHistory
     * @return
     */
    NodeHistory *nodeHistory() { return this->m_nodeHistory; }

    /**
     * @brief idFromIndex
     * @param index
     * @return
     */
    Id idFromIndex( const QModelIndex &index ) const { return this->model()->idFromIndex( index ); }

    /**
     * @brief indexFromId
     * @param id
     * @return
     */
    QModelIndex indexFromId( const Id &id ) const { return this->model()->indexFromId( id ); }

    /**
     * @brief itemFromIndex
     * @param index
     * @return
     */
    QStandardItem *itemFromIndex( const QModelIndex &index ) const { return this->model()->itemFromIndex( index ); }

public slots:
    void updateView();
    void selectReagent( const QModelIndex &filterIndex = QModelIndex());
    //void openContextMenu( const QPoint &pos );
    void restoreIndex();

private:
    NodeHistory *m_nodeHistory;
};
