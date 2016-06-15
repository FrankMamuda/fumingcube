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
#include "gui_addproperty.h"
#include "ui_gui_addproperty.h"
#include "property.h"
#include "template.h"
#include <QMessageBox>

/*
==========
constructor
==========
*/
Gui_AddProperty::Gui_AddProperty( const int reagentId, QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_AddProperty ), m_reagentId( -1 ) {
    this->ui->setupUi( this );
    this->m_reagentId = reagentId;
}

/*
==========
destructor
==========
*/
Gui_AddProperty::~Gui_AddProperty() {
    delete this->ui;
}

/*
==========
buttonBox->accepted
==========
*/
void Gui_AddProperty::on_buttonBox_accepted() {
    if ( this->m_reagentId == -1 )
        return;
        //this->reject();

    if ( this->ui->propertyEdit->text().isEmpty() || this->ui->valueEdit->text().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowFlags( this->windowFlags());
        msgBox.setIcon( QMessageBox::Critical );
        msgBox.setText( QString( "Empty property and/or value" ));
        msgBox.exec();
        return;
        //this->reject();
    }

    if ( Template::fromId( this->m_reagentId ) != NULL )
        Property::add( this->m_reagentId, this->ui->propertyEdit->text(), this->ui->valueEdit->text());
}
