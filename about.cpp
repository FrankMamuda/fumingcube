/*
 * Copyright (C) 2019 Factory #12
 * Copyright (C) 2019-2020 Armands Aleksejevs
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
#include <QMessageBox>
#include "about.h"
#include "ui_about.h"
#ifdef Q_OS_MAC
#include <QDebug>
#endif


/**
 * @brief About::About
 * @param parent
 */
About::About( QWidget *parent ) : QDialog( parent ), ui( new Ui::About ) {
    this->ui->setupUi( this );
    this->connect( this->ui->closeButton, &QPushButton::clicked, [ this ]() { this->close(); } );
    this->connect( this->ui->qtButton, &QPushButton::clicked, [ this ]() { QMessageBox::aboutQt( this ); } );

#ifndef Q_OS_WIN
    QString text( this->ui->textBrowser->toHtml());
    QTextEdit te;
    text.replace( "MS Shell Dlg 2", te.font().family());
#ifdef Q_OS_MAC
    text.replace( "font-size:10pt", "font-size:12pt" );
    text.replace( "font-size:8.25pt", "font-size:10pt" );
    text.replace( "font-size:13pt", "font-size:15pt" );
    this->ui->textBrowser->setHtml( text );
#endif
#endif
}

/**
 * @brief About::~About
 */
About::~About() {
    this->disconnect( this->ui->closeButton, SIGNAL( clicked()));
    this->disconnect( this->ui->qtButton, SIGNAL( clicked()));
    delete this->ui;
}
