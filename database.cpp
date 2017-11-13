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
// includes
//
#include "database.h"
#include "template.h"
#include <QDebug>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>

/**
 * @brief Database::Database
 * @param parent
 */
Database::Database( QObject *parent ) : QObject( parent ) {
    QDir directory;
    QFile file;
    QString filename;
    QSqlDatabase database( QSqlDatabase::database());

    // set default path
#ifdef QT_DEBUG
    this->setPath( QDir::home().absolutePath() + "/.fumingCubeDebug" );
#else
    this->setPath( QDir::home().absolutePath() + "/.fumingCube" );
#endif
    directory.setPath( this->path());

    // check if cache dir exists
    if ( !directory.exists()) {
        qInfo() << this->tr( "creating non-existant database dir" );
        directory.mkpath( this->path());

        // additional failsafe
        if ( !directory.exists()) {
            qFatal( this->tr( "unable to create database dir" ).toUtf8().constData());
        }
    }

    // failafe
    filename = QString( this->path() + "/database.db" );
    file.setFileName( filename );
    if ( !file.exists()) {
        file.open( QFile::WriteOnly );
        file.close();
        qInfo() << this->tr( "creating non-existant database" );

        if ( !file.exists())
            qFatal( this->tr( "unable to create database file" ).toUtf8().constData());
    }

    // announce
    qInfo() << this->tr( "loading database" );

    // failsafe
    if ( !database.isDriverAvailable( "QSQLITE" ))
        qFatal( this->tr( "sqlite not present on the system" ).toUtf8().constData());

    // set sqlite driver
    database = QSqlDatabase::addDatabase( "QSQLITE" );
    database.setHostName( "localhost" );
    database.setDatabaseName( filename );

    // set path and open
    if ( !database.open())
        qFatal( this->tr( "could not load database" ).toUtf8().constData());

    // create initial table structure (if non-existant)
    if ( !this->createStructure())
        qFatal( this->tr( "could not create internal database structure" ).toUtf8().constData());
}

/**
 * @brief Database::createStructure
 * @return
 */
bool Database::createStructure() {
    unsigned int y, k;
    QSqlQuery query;
    QSqlDatabase database( QSqlDatabase::database());
    QStringList tables( database.tables());
    bool mismatch = false;

    // announce
    if ( !tables.count())
        qInfo() << this->tr( "creating an empty database" );

    // validate schema
    for ( y = 0; y < API::numTables; y++ ) {
        table_t table = API::tables[y];

        foreach ( QString tableName, tables ) {
            if ( !QString::compare( table.name, tableName, Qt::CaseInsensitive )) {
                for ( k = 0; k < table.numFields; k++ ) {
                    if ( !database.record( table.name ).contains( table.fields[k].name ))
                        mismatch = true;
                }
            }
        }
    }

    // failure
    if ( mismatch ) {
        qCritical( this->tr( "database API mismatch" ).toUtf8().constData());
        return false;
    }

    // create initial table structure (if non-existant)
    return Database::createEmptyTable();
}

/**
 * @brief Database::~Database
 */
Database::~Database() {
    QString connectionName;
    bool open = false;

    // announce
    qInfo() << this->tr( "unloading database" );

    // remove reagents and templates
    qDeleteAll( this->templateMap );
    this->templateMap.clear();
    qDeleteAll( this->reagentMap );
    this->reagentMap.clear();

    // close database if open and delete orphaned logs on shutdown
    // according to Qt5 documentation, this must be out of scope
    {
        QSqlDatabase database( QSqlDatabase::database());
        if ( database.isOpen()) {
            open = true;
            this->removeOrphanedEntries();
            connectionName = database.connectionName();
            database.close();
        }
    }

    // only now we can sever the connection completely
    if ( open )
        QSqlDatabase::removeDatabase( connectionName );
}

/**
 * @brief Database::load
 */
void Database::load() {
    // load reagents and templates
    Reagent::load();
    Template::load();

    // update gui
    emit this->changed();
}

/**
 * @brief Database::generateSchemas
 * @return
 */
QStringList Database::generateSchemas() {
    unsigned int y, k;
    QStringList schemas;

    for ( y = 0; y < API::numTables; y++ ) {
        table_t api = API::tables[y];
        QString schema = QString( "create table if not exists %1 ( " ).arg( api.name );

        for ( k = 0; k < api.numFields; k++ ) {
            tableField_t apif = api.fields[k];
            schema.append( QString( "%1 %2" ).arg( apif.name ).arg( apif.type ));

            if ( k == api.numFields - 1 )
                schema.append( " )" );
            else
                schema.append( " ," );
        }
        schemas << schema;
    }
    return schemas;
}

/**
 * @brief Database::createEmptyTable
 * @return
 */
bool Database::createEmptyTable() {
    QSqlQuery query;
    QStringList schemas;

    // get schemas
    schemas = this->generateSchemas();
    foreach ( QString schema, schemas ) {
        if ( !query.exec( schema ))
            qFatal( this->tr( "could not create internal database structure, reason - '%1'" ).arg( query.lastError().text()).toUtf8().constData());
    }
    return true;
}

/**
 * @brief Database::removeOrphanedEntries
 */
void Database::removeOrphanedEntries() {
    QSqlQuery query;

    // announce
    qInfo() << this->tr( "removing orphaned logs" );

    // remove orphaned logs (hard coded for now)
    if ( !query.exec( "delete from templates where reagentId not in ( select id from reagents )" ) ||
         !query.exec( "delete from properties where templateId not in ( select id from templates )" ))
        qCritical() << this->tr( "could not delete orphaned logs, reason: '%1'" ).arg( query.lastError().text());
}
