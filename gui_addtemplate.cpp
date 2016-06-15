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
#include "gui_addtemplate.h"
#include "ui_gui_addtemplate.h"
#include "template.h"
#include <QMessageBox>
#include "database.h"
#include "gui_main.h"

/*
==========
constructor
==========
*/
Gui_AddTemplate::Gui_AddTemplate( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_AddTemplate ) {
    this->ui->setupUi( this );
}

/*
==========
destructor
==========
*/
Gui_AddTemplate::~Gui_AddTemplate() {
    delete this->ui;
}

/*
==========
buttonBox->accepted
==========
*/
void Gui_AddTemplate::on_buttonBox_accepted() {
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
        msgBox.setText( QString( "Empty template name" ));
        msgBox.exec();
        return;
    }
    foreach ( Template *templatePtr, db.templateList ) {
        if ( !QString::compare( name, templatePtr->name())) {
            msgBox.setIcon( QMessageBox::Critical );
            msgBox.setText( QString( "Template already exists" ));
            msgBox.exec();
            return;
        }
    }
    parentWindow->addTemplate( name );
}

