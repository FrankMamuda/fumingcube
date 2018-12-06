/*
 * Copyright (C) 2017-2018 Factory #12
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

//
// includes
//
#include "propertyeditor.h"
#include <QMainWindow>
#include "table.h"

//
// classes
//
class Property;
class Template;

/**
 * @brief The Ui namespace
 */
namespace Ui {
class PropertyDialog;
}

/**
 * @brief The PropertyDialog class
 */
class PropertyDialog : public QMainWindow {
    Q_OBJECT
    Q_ENUMS( Directions )

public:
    enum Directions {
        NoDirection = -1,
        Up,
        Down
    };

    explicit PropertyDialog( QWidget *parent = 0, const Row &id = Row::Invalid );
    ~PropertyDialog();
    Row current();

protected:
    void resizeEvent( QResizeEvent *event ) override;

private slots:
    void resetView();
    void move( Directions direction );

    void on_actionTags_triggered();

private:
    Ui::PropertyDialog *ui;
    Row templateRow;
    PropertyEditor *editor;
};
