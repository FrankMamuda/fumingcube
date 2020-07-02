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
#include "system.h"
#include "mainwindow.h"
#include "calcview.h"
#include <QDebug>
#include <QFile>
#include <QRegularExpression>

/**
 * @brief System::System
 */
System::System() {}

/**
 * @brief System::print
 * @param message
 */
void System::print( const QString &message ) {
    qDebug() << message;
}

/**
 * @brief System::replaceGreeting
 */
void System::replaceGreeting() {
    // TODO: reuse code

    // read initial history
    QFile file( ":/initial/calculator_history" );
    QString history;
    if ( file.open( QIODevice::ReadOnly )) {
        const QString html( file.readAll());
        MainWindow::instance()->calcView()->setHtml( html );
        MainWindow::instance()->calcView()->adjustFonts();
        file.close();
    }
}
