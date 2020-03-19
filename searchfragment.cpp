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
#include "searchfragment.h"
#include "ui_searchfragment.h"

/**
 * @brief SearchFragment::SearchFragment
 * @param parent
 */
SearchFragment::SearchFragment( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::SearchFragment ) {
    // setup ui
    this->ui->setupUi( this );

    // set tip icons
    const QPixmap pixmap( QIcon::fromTheme( "info" ).pixmap( 16, 16 ));
    const QList<QLabel*> tips( QList<QLabel*>() << this->ui->cacheTipIcon << this->ui->searchTipIcon );
    for ( QLabel *tip : tips )
        tip->setPixmap( pixmap );
}

/**
 * @brief SearchFragment::~SearchFragment
 */
SearchFragment::~SearchFragment() {
    delete this->ui;
}

/**
 * @brief SearchFragment::identifier
 * @return
 */
QString SearchFragment::identifier() const {
    return this->ui->identifierEdit->text();
}

/**
 * @brief SearchFragment::setIdentifier
 * @param string
 */
void SearchFragment::setIdentifier( const QString &string ) {
    this->ui->identifierEdit->setText( string );
}
