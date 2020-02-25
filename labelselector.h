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
#include <QMenu>
#include <QDialog>
#include "table.h"

/**
 * @brief The Ui namespace
 */
namespace Ui {
    class LabelSelector;
}

#include <QCloseEvent>

/**
 * @brief The NoCloseMenu class
 */
class NoCloseMenu : public QMenu {
public:
    explicit NoCloseMenu( QWidget *parent = nullptr );

protected:
    /**
     * @brief closeEvent
     * @param event
     */
    void closeEvent( QCloseEvent *event ) override {
        event->ignore();
    }
};

/**
 * @brief The LabelSelector class
 */
class LabelSelector : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY( LabelSelector )

public:
    explicit LabelSelector( QWidget *parent = nullptr, QList<Id> selected = QList<Id>());

    // disable move
    LabelSelector( LabelSelector&& ) = delete;
    LabelSelector& operator=( LabelSelector&& ) = delete;

    ~LabelSelector() override;
    QList<Id> labelIds;

/*protected:
    bool eventFilter( QObject *, QEvent * ) override;*/

private:
    Ui::LabelSelector *ui;
};
