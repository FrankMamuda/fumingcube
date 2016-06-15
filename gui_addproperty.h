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

#ifndef GUI_ADDPROPERTY_H
#define GUI_ADDPROPERTY_H

//
// includes
//
#include <QDialog>

//
// namespace: Ui
//
namespace Ui {
class Gui_AddProperty;
}

//
// class: AddProperty (ui)
//
class Gui_AddProperty : public QDialog {
    Q_OBJECT

public:
    explicit Gui_AddProperty( const int reagentId, QWidget *parent = 0 );
    ~Gui_AddProperty();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::Gui_AddProperty *ui;
    int m_reagentId;
};

#endif // GUI_ADDPROPERTY_H
