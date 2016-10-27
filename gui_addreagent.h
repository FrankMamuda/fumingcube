/*
===========================================================================
Copyright (C) 2016 Avotu Briezhaudzetava

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

#ifndef GUI_ADDREAGENT_H
#define GUI_ADDREAGENT_H

//
// includes
//
#include <QDialog>
#include "ui_gui_addreagent.h"

//
// namespace: Ui
//
namespace Ui {
class Gui_AddReagent;
}

/**
 * @brief The Gui_AddReagent class
 */
class Gui_AddReagent : public QDialog {
    Q_OBJECT

public:
    explicit Gui_AddReagent( QWidget *parent = 0 ) : QDialog( parent ), ui( new Ui::Gui_AddReagent ) { this->ui->setupUi( this ); }
    ~Gui_AddReagent() { delete this->ui; }

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected() {}

private:
    Ui::Gui_AddReagent *ui;
};

#endif // GUI_ADDREAGENT_H
