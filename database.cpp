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

//
// includes
//
#include "database.h"
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <QSqlQuery>
#include <QSqlError>

//
// singleton: Database
//
Database &db = Database::instance();

/*
==========
makePath
==========
*/
void Database::makePath() {
    QString path;

    // default path?
#ifdef Q_OS_UNIX
    path = QString( QDir::homePath() + "/.chemtoolbox/" );
#else
    path = QString( QDir::currentPath() + "/" );
#endif
    path.append( "/" );
    path.append( "data.db" );

    // make path id nonexistant
    QFileInfo db( path );
    QDir dir;
    dir.setPath( db.absolutePath());

    if ( !dir.exists()) {
        dir.mkpath( db.absolutePath());
        if ( !dir.exists())
            m.error( Main::FatalError, QString( "could not create database path - '%1'\n" ).arg( path ));
    }

    // store path
    this->path = path;
    this->path.replace( "//", "/" );
}

/*
==========
touch
==========
*/
void Database::create() {
    // create query
    QSqlQuery query;

    // failafe
    QFile database( this->path );
    if ( !database.exists())
        m.error( Main::FatalError, QString( "unable to create database file\n" ));

    // create initial table structure (if non-existant)
    if ( !query.exec( QString( "create table if not exists templates ( id integer primary key, name varchar( 128 ), amount float, density float, assay float, molarMass float, state integer )" )))
        m.error( Main::FatalError, QString( "could not create internal database structure, reason - '%1'\n" ).arg( query.lastError().text()));
}

/*
==========
load
==========
*/
void Database::load() {
    // make path
    this->makePath();

    // create database
    QFile databaseFile( this->path );
    QFileInfo databaseInfo( databaseFile );
    QSqlDatabase database = QSqlDatabase::database();

    // announce
    m.print( QString( "loading database '%1'\n" ).arg( this->path ));

    // failsafe
    if ( !database.isDriverAvailable( "QSQLITE" ))
        m.error( Main::FatalError, QString( "sqlite not present on the system\n" ));

    // set sqlite driver
    database = QSqlDatabase::addDatabase( "QSQLITE" );
    database.setHostName( "localhost" );
    database.setDatabaseName( this->path );

    // touch file if empty
    if ( !databaseFile.exists()) {
        databaseFile.open( QFile::WriteOnly );
        databaseFile.close();
        m.print( QString( "creating non-existant database - '%1'\n" ).arg( databaseInfo.fileName()));
    }

    // set path and open
    if ( !database.open())
        m.error( Main::FatalError, QString( "could not load database - '%1'\n" ).arg( databaseInfo.fileName()));

    // create database
    this->create();

    /* delete orphaned entries */
    /* load templates */
    this->loadTemplates();
}

/*
==========
unload
==========
*/
void Database::unload() {
    QString connectionName;
    bool open = false;

    // announce
    m.print( QString( "unloading database\n" ));

    // close database if open and delete orphaned logs on shutdown
    // according to Qt5 documentation, this must be out of scope
    {
        QSqlDatabase database = QSqlDatabase::database();
        if ( database.isOpen()) {
            open = true;
            /* delete orphaned entries */
            connectionName = database.connectionName();;
            database.close();

        }
    }

    // only now we can remove the connection completely
    if ( open )
        QSqlDatabase::removeDatabase( connectionName );
}

/*
==========
loadTemplates
==========
*/
void Database::loadTemplates() {
    QSqlQuery query;

    // announce
    m.print( "loading templates from database\n" );

    // read all template entries
    query.exec( "select * from templates order by name asc;" );

    // store entries in memory
    while ( query.next())
        this->templateList << new Template( query.record());
}
