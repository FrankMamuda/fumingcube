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
#include "label.h"
#include "labeldialog.h"
#include "ui_labeldialog.h"

#include <QColorDialog>

/**
 * @brief LabelDialog::LabelDialog
 * @param parent
 */
LabelDialog::LabelDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::LabelDialog ) {
    this->ui->setupUi( this );

    this->setColour( QColor::fromRgb( 128, 128, 128, 32 ));
    this->ui->colourButton->connect( this->ui->colourButton, &QToolButton::clicked, [ this ]() {
        QColorDialog cd( this );
        if ( cd.exec() == QDialog::Accepted ) {
            const QColor c( cd.selectedColor());
            //this->setColour( QColor::fromRgb( c.red(), c.green(), c.blue(), 32 ));
            this->setColour( c );
        }
    } );
}

/**
 * @brief LabelDialog::~LabelDialog
 */
LabelDialog::~LabelDialog() {
    delete this->ui;
}

/**
 * @brief LabelDialog::name
 * @return
 */
QString LabelDialog::name() const {
    return this->ui->nameEdit->text();
}

/**
 * @brief LabelDialog::setName
 * @param name
 */
void LabelDialog::setName( const QString &name ) {
    this->ui->nameEdit->setText( name );
}

/**
 * @brief LabelDialog::setColour
 * @param colour
 */
void LabelDialog::setColour( const QColor &colour ) {
    this->m_colour = colour;
    this->ui->colourButton->setIcon( QIcon( Label::instance()->pixmap( this->colour())));
}
