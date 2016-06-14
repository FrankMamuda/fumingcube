/*
===========================================================================
Copyright (C) 2016 Avotu Briezhaudzetava

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

//
// includes
//
#include "main.h"
#include "gui_main.h"
#include <QApplication>
#include "database.h"
#include <QDebug>

//
// singleton: Main
//
Main &m = Main::instance();

/*
==========
initialise
==========
*/
void Main::initialise() {
    db.load();
}

/*
==========
print
==========
*/
void Main::print( const QString &msg ) {
    QString out = msg;

    if ( msg.endsWith( "\n" ))
        out = msg.left( msg.length()-1 );

    qDebug() << out;
}

/*
==========
error
==========
*/
void Main::error( ErrorTypes type, const QString &msg ) {
    if ( type == FatalError ) {
        Main::print( QString( "FATAL ERROR: %1" ).arg( msg ));
        Main::shutdown();
    } else {
        Main::print( QString( "ERROR: %1" ).arg( msg ));
    }
}

/*
==========
shutdown
==========
*/
void Main::shutdown() {
    db.unload();
    QApplication::quit();
}

/*
==========
entry point
==========
*/
int main( int argc, char *argv[] ) {
    QApplication a( argc, argv );
    Gui_Main w;
    w.show();

    return a.exec();
}
