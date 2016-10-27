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

#ifndef GUI_PROPERTIES_H
#define GUI_PROPERTIES_H

//
// includes
//
#include <QMainWindow>
#include "propertiesmodel.h"
#include "ui_gui_properties.h"

//
// namespace: Ui
//
namespace Ui {
class Gui_Properties;
}

/**
 * @brief The Gui_Properties class
 */
class Gui_Properties : public QMainWindow {
    Q_OBJECT

public:
    explicit Gui_Properties( QWidget *parent = 0 , const int reagentId = -1 );
    ~Gui_Properties() { delete this->ui; delete this->m_model; }

private slots:
    void on_closeButton_clicked() { this->close(); }
    void on_addPropertyAction_triggered();
    void on_removePropertyAction_triggered();

public slots:
    void setReagentId( const int reagentId = -1 ) { this->m_model->setReagentId( reagentId ); }

private:
    Ui::Gui_Properties *ui;
    PropertiesModel *m_model;
};

#endif // GUI_PROPERTIES_H
