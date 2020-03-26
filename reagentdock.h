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
#include "reagentview.h"
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
    // disable move
    ReagentDock( ReagentDock&& ) = delete;
    ReagentDock& operator=( ReagentDock&& ) = delete;

    /**
     * @brief instance
     * @return
     */
    static ReagentDock *instance() {
        static auto *reagentDock( new ReagentDock());
        return reagentDock;
    }
    ~ReagentDock() override;
    [[nodiscard]] bool checkForDuplicates( const QString &name, const QString &reference, Id reagentId = Id::Invalid ) const;
    [[nodiscard]] bool checkBatchForDuplicates( const QString &name, Id parentId ) const;
    [[nodiscard]] ReagentView *view() const;
    QMenu *buildMenu( bool context = true );

    Id addReagent( const Id &parentId, const QString &reagentName = QString(), const int cid = 0 );

signals:
    void currentIndexChanged( const QModelIndex &index );

private slots:
    void on_reagentView_customContextMenuRequested( const QPoint &pos );
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_buttonFind_clicked();
    void on_editButton_clicked();

private:
    explicit ReagentDock( QWidget *parent = nullptr );
    Ui::ReagentDock *ui;
    QShortcut *shortcut;
};
