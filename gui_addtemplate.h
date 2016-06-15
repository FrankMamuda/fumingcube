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

#ifndef GUI_ADDTEMPLATE_H
#define GUI_ADDTEMPLATE_H

//
// includes
//
#include <QDialog>

//
// namespace: Ui
//
namespace Ui {
class Gui_AddTemplate;
}

//
// class: AddTemplate (ui)
//
class Gui_AddTemplate : public QDialog {
    Q_OBJECT

public:
    explicit Gui_AddTemplate( QWidget *parent = 0 );
    ~Gui_AddTemplate();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected() {}

private:
    Ui::Gui_AddTemplate *ui;
};

#endif // GUI_ADDTEMPLATE_H
