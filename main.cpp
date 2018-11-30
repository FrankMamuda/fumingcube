/*
 * Copyright (C) 2017-2018 Factory #12
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
//      common properties
//      order
//      search
//   proper warnings
//   property sorting combo (custom, alphabetically, etc.)
//   default fonts
//   icons for actions
//   calculator widget
//   fix templateWidget init lag
//   built-in database
//   disable toolTips and volume/density values for Solid states
//   copy data from default template, upon opening new tab
//   fix messageBar timeOut
//   after reagent addition, select the newly added
//
// OTHER:
//   fix constants, etc.
//

//
// includes
//
#include "main.h"
#include "mainwindow.h"
#include "database.h"
#include "xmltools.h"
#include "variable.h"
#include "reagent.h"
#include "template.h"
#include "property.h"
#include <QApplication>
#include <QDebug>
#include <QThread>

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

    // load settings
    Variable::instance()->add( "ui_lastReagentIndex", -1 );
    Variable::instance()->add( "ui_lastTemplateIndex", -1 );
    Variable::instance()->add( "ui_lastValue", 1.0 );
    XMLTools::instance()->read();

    // show main window
    QApplication a( argc, argv );

    // load database on separate thread
    Database::instance();
    Database::instance()->add( Reagent::instance());
    Database::instance()->add( Template::instance());
    Database::instance()->add( Property::instance());

    MainWindow w;
    w.show();

    // clean up on exit
    qApp->connect( qApp, &QApplication::aboutToQuit, []() {
        GarbageMan::instance()->clear();
        delete GarbageMan::instance();
        delete Database::instance();

        // fixes segfault on newer qt versions
        Variable::instance()->deleteLater();
    } );

    return a.exec();
}
