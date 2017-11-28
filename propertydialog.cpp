/*
 * Copyright (C) 2017 Factory #12
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

//
// includes
//
#include "propertydialog.h"
#include "ui_propertydialog.h"
#include "propertyeditor.h"

/**
 * @brief PropertyDialog::PropertyDialog
 * @param parent
 */
PropertyDialog::PropertyDialog( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::PropertyDialog ) {
    this->ui->setupUi( this );
}

/**
 * @brief PropertyDialog::~PropertyDialog
 */
PropertyDialog::~PropertyDialog() {
    delete this->ui;
}

/**
 * @brief PropertyDialog::on_actionAdd_triggered
 */
void PropertyDialog::on_actionAdd_triggered() {
    PropertyEditor *pe = new PropertyEditor( this );
    pe->show();
}
