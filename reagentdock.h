/*
 * Copyright (C) 2019-2020 Armands Aleksejevs
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
#include "dockwidget.h"
#include "nodehistory.h"
#include <QShortcut>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class ReagentDock;
}

/**
 * @brief The ReagentDock class
 */
class ReagentDock final : public DockWidget {
    Q_OBJECT
    Q_DISABLE_COPY( ReagentDock )

public:
    static ReagentDock *instance() { static ReagentDock *reagentDock( new ReagentDock()); return reagentDock; }
    ~ReagentDock() override;
    bool checkForDuplicates( const QString &name, const QString &alias, const Id reagentId = Id::Invalid ) const;
    Id indexFromId( const QModelIndex &index ) const;

signals:
    void currentIndexChanged( const QModelIndex &index );

public slots:
    void restoreIndex();
    void select( const QModelIndex &index );
    void expand( const QModelIndex &index );
    void reset();

private slots:
    void on_reagentView_clicked( const QModelIndex &index );
    void on_reagentView_customContextMenuRequested( const QPoint &pos );
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_buttonFind_clicked();
    void on_editButton_clicked();

private:
    explicit ReagentDock( QWidget *parent = nullptr );
    Ui::ReagentDock *ui;
    ReagentModel *model = new ReagentModel();
    QMainWindow *m_windowParent = nullptr;
    QList<Id> matches;
    Id currentMatch;
    QShortcut *shortcut;
    NodeHistory *nodeHistory;
};
