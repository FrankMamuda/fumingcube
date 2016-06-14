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
// include
//
#include "gui_properties.h"
#include "ui_gui_properties.h"
#include "gui_main.h"
#include <QDebug>

/*
==========
constructor
==========
*/
Gui_Properties::Gui_Properties( QWidget *parent, const int templateId ) : QMainWindow( parent ), ui( new Ui::Gui_Properties ) {
    Q_UNUSED( templateId )

    this->ui->setupUi( this );
    this->m_model = new PropertiesModel( this->ui->propertiesView );
    this->ui->propertiesView->setModel( this->m_model );
    this->ui->propertiesView->setAlternatingRowColors( true );

    //
    // FIXME/TODO: just bind reagentId to gui_main combobox, if it changes, emit signal
    //


    qDebug() << "show";
}

/*
==========
destructor
==========
*/
Gui_Properties::~Gui_Properties() {
    delete ui;
    delete this->m_model;
}

/*
==========
setReagentId
==========
*/
void Gui_Properties::setReagentId( const int reagentId ) {
    this->m_model->setReagentId( reagentId );
    //this->ui->propertiesView->reset();
}

/*
==========
addPropertyAction->triggered
==========
*/
void Gui_Properties::on_addPropertyAction_triggered() {
    Gui_Main *gui = qobject_cast<Gui_Main*>( this->parent());

    if ( gui != NULL ) {
        if ( gui->curentTemplate != NULL ) {
            Property::add( gui->curentTemplate->id(), "Boiling point", "110 degC" );

            // TODO: refresh model data
            this->m_model->reset();
            //this->ui->propertiesView->model()->beginResetModel();
            //this->ui->propertiesView->setModel( this->m_model );
            //this->ui->propertiesView->in
            //this->ui->propertiesView->update()
           // this->ui->propertiesView->model()->up
            //this->ui->propertiesView->setModel( this->m_model );
        }
    }
}
