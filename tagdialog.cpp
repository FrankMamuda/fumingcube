/*
 * Copyright (C) 2018 Factory #12
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
#include "tagdialog.h"
#include "ui_tagdialog.h"
#include "tag.h"

//
// TODO: on tag removal, update all affected properties and set tag as Id::Invalid
//

/**
 * @brief TagDialog::TagDialog
 * @param parent
 */
TagDialog::TagDialog( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::TagDialog ) {
    this->ui->setupUi( this );
    this->ui->listView->setModel( Tag::instance());
}

/**
 * @brief TagDialog::~TagDialog
 */
TagDialog::~TagDialog() {
    delete this->ui;
}
