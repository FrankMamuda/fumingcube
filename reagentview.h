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
#include "reagentdelegate.h"
#include "reagentmodel.h"
#include <QSortFilterProxyModel>
#include <QTreeView>

/**
 * @brief The SortFilterProxyModel class
 */
class SortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit SortFilterProxyModel( QObject *parent = nullptr ) : QSortFilterProxyModel( parent ) {}
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;
};

/**
 * @brief The ReagentView class
 */
class ReagentView : public QTreeView {
    Q_OBJECT
    Q_DISABLE_COPY( ReagentView )

public:
    explicit ReagentView( QWidget *parent = nullptr );

    // disable move
    ReagentView( ReagentView&& ) = delete;
    ReagentView& operator=( ReagentView&& ) = delete;

    /**
     * @brief ~ReagentView::~ReagentView
     */
    ~ReagentView() override {
        delete this->delegate;
        delete this->reagentModel;
    }

    /**
     * @brief model
     * @return
     */
    [[nodiscard]] ReagentModel *sourceModel() const { return qobject_cast<ReagentModel *>( this->filterModel()->sourceModel()); }

    /**
     * @brief filterModel
     * @return
     */
    [[nodiscard]] SortFilterProxyModel *filterModel() const { return qobject_cast<SortFilterProxyModel *>( QTreeView::model()); }

    /**
     * @brief idFromIndex
     * @param index
     * @return
     */
    [[nodiscard]] Id idFromIndex( const QModelIndex &index ) const { return this->sourceModel()->idFromIndex( index ); }

    /**
     * @brief indexFromId
     * @param id
     * @return
     */
    [[nodiscard]] QModelIndex indexFromId( const Id &id ) const { return this->sourceModel()->indexFromId( id ); }

    /**
     * @brief itemFromIndex
     * @param index
     * @return
     */
    [[nodiscard]] QStandardItem *itemFromIndex( const QModelIndex &index ) const { return this->sourceModel()->itemFromIndex( index ); }

public slots:
    void updateView();
    void selectReagent( const QModelIndex &filterIndex = QModelIndex());
    void restoreIndex();

protected:
    void keyReleaseEvent( QKeyEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;

private:
    ReagentModel *reagentModel = new ReagentModel();
    ReagentDelegate *delegate = new ReagentDelegate();
};
