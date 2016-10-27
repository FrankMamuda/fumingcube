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

#ifndef MAIN_H
#define MAIN_H

//
// include
//
#include <QString>

// message macro
#ifdef Q_CC_MSVC
#define ClassFunc QString( "%1::%2: " ).arg( this->metaObject()->className()).arg( __FUNCTION__ )
#else
#define ClassFunc QString( "%1::%2: " ).arg( this->metaObject()->className()).arg( __func__ )
#endif


/**
 * @brief The Main class
 */
class Main {
public:
    static Main &instance() { static Main *instance = new Main(); return *instance; }

    // error types
    enum ErrorTypes {
        SoftError = 0,
        FatalError
    };

private:
    Main() { }

public:
    void initialise();
    static void print( const QString & );
    static void error( ErrorTypes , const QString & );
    static void shutdown();
};

//
// externals
//
extern Main &m;

#endif // MAIN_H
