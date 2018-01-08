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
// TODO:
//   properties
//      check for duplicates
//      image scaling
//      common properties
//      order
//   icons for actions
//   restore last id on open, etc.
//   calculator widget
//   fix templateWidget init lag
//   built-in database
//   disable toolTips and volume/density values for Solid states
//   copy data from default template, upon opening new tab
//   remember last input values on open
//   fix messageBar timeOut

//
// includes
//
#include "mainwindow.h"
#include "database.h"
#include <QApplication>
#include <QDebug>

// default message handler
static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler( 0 );

/**
 * @brief messageFilter
 * @param type
 * @param context
 * @param msg
 */
void messageFilter( QtMsgType type, const QMessageLogContext &context, const QString &msg ) {
    (*QT_DEFAULT_MESSAGE_HANDLER)( type, context, msg );

    if ( type == QtFatalMsg ) {
        QApplication::quit();
        exit( 0 );
    }
}

/**
 * @brief qMain
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char *argv[] ) {
    // set console output pattern
    qSetMessagePattern( "%{if-category}%{category}: %{endif}%{function}: %{message}" );

    // log to file in non-qtcreator environment
    qInstallMessageHandler( messageFilter );

    // show main window
    QApplication a( argc, argv );
    MainWindow w;
    w.show();

    // load database on separate thread
    QThread *thread = new QThread;
    Database::instance()->moveToThread( thread );
    thread->start();
    thread->connect( thread, &QThread::finished, thread, &QThread::deleteLater );
    Database::instance()->load();

    return a.exec();
}
