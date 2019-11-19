/*
 * Copyright (C) 2019 Armands Aleksejevs
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

/*
 * includes
 */
#include "reagentdialog.h"
#include "ui_reagentdialog.h"

/**
 * @brief ReagentDialog::ReagentDialog
 * @param parent
 */
ReagentDialog::ReagentDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::ReagentDialog ) {
    this->ui->setupUi( this );
}

/**
 * @brief ReagentDialog::~ReagentDialog
 */
ReagentDialog::~ReagentDialog() {
    delete this->ui;
}

/**
 * @brief ReagentDialog::name
 * @return
 */
QString ReagentDialog::name() const {
    return this->ui->nameEdit->text();
}

/**
 * @brief ReagentDialog::alias
 * @return
 */
QString ReagentDialog::alias() const {
    return this->ui->aliasEdit->text();
}

/**
 * @brief ReagentDialog::on_nameEdit_textChanged
 * @param text
 */
void ReagentDialog::on_nameEdit_textChanged( const QString &text ) {
    this->ui->aliasEdit->setText( QString( text ).remove( ' ' ));
}
