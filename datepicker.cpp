/*
 * Copyright (C) 2020 Armands Aleksejevs
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
#include "datepicker.h"
#include "ui_datepicker.h"

/**
 * @brief DatePicker::DatePicker
 * @param parent
 */
DatePicker::DatePicker( QWidget *parent ) : QDialog( parent ), ui( new Ui::DatePicker ) {
    this->ui->setupUi( this );
}

/**
 * @brief DatePicker::~DatePicker
 */
DatePicker::~DatePicker() {
    delete this->ui;
}

/**
 * @brief DatePicker::date
 * @return
 */
QDate DatePicker::date() const {
    return this->ui->calendarWidget->selectedDate();
}

/**
 * @brief DatePicker::setDate
 * @param date
 */
void DatePicker::setDate( const QDate &date ) {
    this->ui->calendarWidget->setSelectedDate( date );
}
