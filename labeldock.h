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
#include "dockwidget.h"
#include <QModelIndexList>
#include <QShortcut>
#include "table.h"

/**
 * @brief The Ui namespace
 */
namespace Ui {
    class LabelDock;
}

/**
 * @brief The LabelDock class
 */
class LabelDock final : public DockWidget {
    Q_OBJECT
    Q_DISABLE_COPY( LabelDock )

public:
    // disable move
    LabelDock( LabelDock&& ) = delete;
    LabelDock& operator=( LabelDock&& ) = delete;

    /**
     * @brief instance
     * @return
     */
    static LabelDock *instance() {
        static auto *labelDock( new LabelDock());
        return labelDock;
    }
    ~LabelDock() override;
    [[nodiscard]] Id currentLabel() const;

    /**
     * @brief setFilter
     * @param list
     */
    static void setFilter( const QModelIndexList &list = QModelIndexList());

private slots:
    void on_labelView_customContextMenuRequested( const QPoint &pos );

private:
    explicit LabelDock( QWidget *parent = nullptr );
    Ui::LabelDock *ui;
};
