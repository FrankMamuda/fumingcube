/*
 * Copyright (C) 2018 Factory #12
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
// includes
//
#include <QDebug>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QApplication>
#include "database.h"
#include "table.h"
#include "field.h"
#include "main.h"
#include "variable.h"

/**
 * @brief Database::Database
 * @param parent
 */
Database::Database( QObject *parent ) : QObject( parent ), m_initialised( false ) {
    QDir path( QDir::homePath() + "/" + Main::Path );
    QFile file( path.absolutePath() + "/" + "database.db" );
    QSqlDatabase database( QSqlDatabase::database());

    if ( !path.exists()) {
        qCDebug( Database_::Debug ) << this->tr( "making non-existant database path \"%1\"" ).arg( path.absolutePath());
        path.mkpath( path.absolutePath());

        if ( !path.exists())
            qFatal( QT_TR_NOOP_UTF8( "could not create database path" ));
    }

    // failsafe
    if ( !file.exists()) {
        file.open( QFile::WriteOnly );
        file.close();
        qCDebug( Database_::Debug ) << this->tr( "creating non-existant database" );

        if ( !file.exists())
            qFatal( QT_TR_NOOP_UTF8( "unable to create database file" ));
    }

    // announce
    qCInfo( Database_::Debug ) << this->tr( "loading database" );

    // failsafe
    if ( !database.isDriverAvailable( "QSQLITE" ))
        qFatal( QT_TR_NOOP_UTF8( "sqlite not present on the system" ));

    // set sqlite driver
    database = QSqlDatabase::addDatabase( "QSQLITE" );
    database.setHostName( "localhost" );
    database.setDatabaseName( QFileInfo( file ).absoluteFilePath());

    // set path and open
    if ( !database.open())
        qFatal( QT_TR_NOOP_UTF8( "could not load database" ));

    // done
    this->setInitialised();

    // add to garbage collector
    GarbageMan::instance()->add( this );
}

/**
 * @brief Database::removeOrphanedEntries
 */
void Database::removeOrphanedEntries() {
    foreach ( Table *table, this->tables )
        table->removeOrphanedEntries();
}

/**
 * @brief Database::~Database
 */
Database::~Database() {
    QString connectionName;
    bool open = false;

    // remove orphans
    this->removeOrphanedEntries();

    // announce
    qCInfo( Database_::Debug ) << this->tr( "unloading database" );
    this->setInitialised( false );

    // unbind variables
    //Variable::instance()->unbind( "reagentId" );
    //Variable::instance()->unbind( "templateId" );
    foreach ( Table *table, this->tables )
        table->clear();
    qDeleteAll( this->tables );

    // according to Qt5 documentation, this must be out of scope
    {
        QSqlDatabase database( QSqlDatabase::database());
        if ( database.isOpen()) {
            open = true;
            connectionName = database.connectionName();

            qCInfo( Database_::Debug ) << this->tr( "closing database" );
            database.close();
        }
    }

    // only now we can sever the connection completely
    if ( open )
        QSqlDatabase::removeDatabase( connectionName );
}

/**
 * @brief Database::add
 * @param table
 */
void Database::add( Table *table ) {
    QSqlDatabase database( QSqlDatabase::database());
    const QStringList tables( database.tables());
    QString statement;
    QSqlQuery query;
    bool found = false;

    // store table
    this->tables[table->tableName()] = table;

    // announce
    if ( !tables.count())
        qCInfo( Database_::Debug ) << this->tr( "creating an empty database" );

    // validate schema
    foreach ( const QString &tableName, tables ) {
        if ( !QString::compare( table->tableName(), tableName )) {
            foreach ( const Field &field, qAsConst( table->fields )) {
                if ( !database.record( table->tableName()).contains( field->name())) {
                    qCCritical( Database_::Debug ) << this->tr( "database field mismatch" );
                    return;
                }
            }
            found = true;
        }
    }

    // table has been verified and is marked as valid
    if ( found ) {
        table->setValid();
    } else {
        // announce
        qCInfo( Database_::Debug ) << this->tr( "creating an empty table - \"%1\"" ).arg( table->tableName());

        // prepare statement
        foreach ( const Field &field, qAsConst( table->fields )) {
            statement.append( QString( "%1 %2" ).arg( field->name()).arg( field->format()));

            if ( QString::compare( field->name(), table->fields.last()->name()))
                statement.append( ", " );
        }

        if ( !query.exec( QString( "create table if not exists %1 ( %2 )" ).arg( table->tableName()).arg( statement )))
            qCCritical( Database_::Debug ) << this->tr( "could not create table - \"%1\", reason - \"%2\"" ).arg( table->tableName()).arg( query.lastError().text());
    }

    // create table model
    table->setTable( table->tableName());

    // load data
    if ( !table->select()) {
        qCCritical( Database_::Debug ) << this->tr( "could not initialize model for table - \"%1\"" ).arg( table->tableName());
        table->setValid( false );
    }
}
