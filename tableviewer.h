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

/*
 * includes
 */
#include <QDialog>
#include <QSqlQueryModel>
#include "propertydelegate.h"
#include "reagentdelegate.h"
#include "table.h"

/**
 * @brief The Ui namespace
 */
namespace Ui {
class TableViewer;
}

/**
 * @brief The FilterModel class
 */
class FilterModel : public QSortFilterProxyModel {
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;
};

/**
 * @brief The TableViewer class
 */
class TableViewer : public QDialog {
    Q_OBJECT

public:
    explicit TableViewer( QWidget *parent = nullptr, const Id &tableId = Id::Invalid );
    ~TableViewer() override;

    /**
     * @brief filter
     * @return
     */
    [[nodiscard]] QString filter() const { return this->m_filter; }

protected:
    void showEvent( QShowEvent *event ) override;

public slots:
    void populateTable( const QList<Id> tagIds, const QString &filter );
    void setFilter( const Id tabId, const QList<Id> tagList );

private:
    Ui::TableViewer *ui;
    QSqlQueryModel *model = new QSqlQueryModel();
    QString m_filter;
    FilterModel *filterModel = new FilterModel();
    ReagentDelegate *reagentDelegate = nullptr;
    PropertyDelegate *propertyDelegate = nullptr;
    Id tableId = Id::Invalid;
};
