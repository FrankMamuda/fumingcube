/*
===========================================================================
Copyright (C) 2013 Avotu Briezhaudzetava

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

//
// singleton class: Main
//
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
    void print( const QString & );
    void error( ErrorTypes , const QString & );
    void shutdown();
};

//
// externals
//
extern Main &m;

#endif // MAIN_H
