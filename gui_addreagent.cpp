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

//
// includes
//
#include "gui_addreagent.h"
#include "ui_gui_addreagent.h"
#include "reagent.h"
#include <QMessageBox>
#include "database.h"
#include "gui_main.h"

/**
 * @brief Gui_AddReagent::on_buttonBox_accepted
 */
void Gui_AddReagent::on_buttonBox_accepted() {
    QString name;
    Gui_Main *parentWindow;
    QMessageBox msgBox;

    name = this->ui->nameEdit->text();
    parentWindow = qobject_cast<Gui_Main*>( this->parent());

    if ( parentWindow == NULL )
        this->close();

    msgBox.setWindowFlags( parentWindow->windowFlags());

    if ( name.isEmpty()) {
        msgBox.setIcon( QMessageBox::Warning );
        msgBox.setText( this->tr( "Empty reagent name" ));
        msgBox.exec();
        return;
    }
    foreach ( Reagent *reagentPtr, db.reagentList ) {
        if ( !QString::compare( name, reagentPtr->name())) {
            msgBox.setIcon( QMessageBox::Critical );
            msgBox.setText( this->tr( "Reagent already exists" ));
            msgBox.exec();
            return;
        }
    }
    parentWindow->addReagent( name );
}
