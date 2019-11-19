/*
 * Copyright (C) 2018-2019 Armands Aleksejevs
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
#include <QDebug>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QApplication>
#include <QTime>
#include "database.h"
#include "table.h"
#include "field.h"
#include "main.h"
#include "variable.h"
#include "mainwindow.h"

/**
 * @brief Database::testPath checks if provided database path is valid and creates non-existant sub-directories
 * @param path database path
 * @return success
 */
bool Database::testPath( const QString &path ) {
    const QDir dir( QFileInfo( path ).absoluteDir());

    // reject empty paths
    if ( path.isEmpty()) {
        qCDebug( Database_::Debug ) << this->tr( "empty database path" );
        return false;
    }

    // only accept absolute paths
    if ( !dir.isAbsolute()) {
        qCDebug( Database_::Debug ) << this->tr( "relative or invalid database path \"%1\"" ).arg( path );
        return false;
    }

    if ( !dir.exists()) {
        qCDebug( Database_::Debug ) << this->tr( "making non-existant database path \"%1\"" ).arg( dir.absolutePath());
        dir.mkpath( dir.absolutePath());

        if ( !dir.exists())
            return false;
    }

    return true;
}

/**
 * @brief Database::Database initializes database
 * @param parent any qobject parent
 */
Database::Database( QObject *parent ) : QObject( parent ) {
    QSqlDatabase database( QSqlDatabase::database());

    // validate path
    if ( !testPath( Variable::instance()->string( "databasePath" ))) {
        Variable::instance()->setString( "databasePath", QDir( QDir::homePath() + "/" + Main::Path ).absolutePath() + "/" + "database.db" );

        if ( !this->testPath( Variable::instance()->string( "databasePath" )))
            qFatal( QT_TR_NOOP_UTF8( "could not create database path" ));
    }

    // failsafe
    QFile file( Variable::instance()->string( "databasePath" ));
    if ( !file.exists()) {
        file.open( QFile::WriteOnly );
        file.close();
        qCDebug( Database_::Debug ) << this->tr( "creating non-existant database" ) << '"' << Variable::instance()->string( "databasePath" ) << '"';

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
}

/**
 * @brief Database::removeOrphanedEntries removes orphaned entries in database tables
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
    //Variable::instance()->unbind( "report" );
    qCInfo( Database_::Debug ) << this->tr( "clearing tables" );
    foreach ( Table *table, this->tables )
        table->clear();

    // delete all tables
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
 * @brief Database::add adds and validates Table instance to database
 * @param table Table instance (QSqlTableModel)
 */
bool Database::add( Table *table ) {
    QSqlDatabase database( QSqlDatabase::database());
    const QStringList tables( database.tables());

    // store table
    this->tables[table->tableName()] = table;

    // announce
    if ( !tables.count())
        qCInfo( Database_::Debug ) << this->tr( "creating an empty database" );

    // validate schema
    bool found = false;
    foreach ( const QString &tableName, tables ) {
        if ( !QString::compare( table->tableName(), tableName )) {
            foreach ( const Field &field, qAsConst( table->fields )) {

                if ( !database.record( table->tableName()).contains( field->name())) {
                    qCCritical( Database_::Debug ) << this->tr( "database field mismatch in table \"%1\", field - \"%2\"" ).arg( tableName ).arg( field->name());
                    return false;
                } else {
                    // ignore unsigned ints for now
                    const QVariant::Type internalType = field->type() == QVariant::UInt ? QVariant::Int : field->type();
                    const QVariant::Type databaseType = database.record( table->tableName()).field( field->id()).type();

                    if ( internalType != databaseType ) {
                        qCCritical( Database_::Debug ) << this->tr( "database type mismatch in table \"%1\", field - \"%2\"" ).arg( tableName ).arg( field->name());
                        return false;
                    }
                }
            }
            found = true;
        }
    }

    QString statement;
    QSqlQuery query;

    if ( !found ) {
        // announce
        qCInfo( Database_::Debug ) << this->tr( "creating an empty table - \"%1\"" ).arg( table->tableName());

        // prepare statement
        foreach ( const Field &field, qAsConst( table->fields )) {
            statement.append( QString( "%1 %2" ).arg( field->name()).arg( field->format()));

            if ( field->isUnique())
                statement.append( " unique" );

            if ( QString::compare( field->name(), table->fields.last()->name()))
                statement.append( ", " );
        }

        // check for constraints
        QString constraints;
        const int tc = table->constraints.count();

        if ( tc > 0 ) {
            for ( int y = 0; y < table->constraints.count(); y++ ) {
                constraints.append( "unique( " );

                const int cc = table->constraints.at( y ).count();
                for ( int k = 0; k < cc; k++ ) {
                    const QSharedPointer<Field_> field( table->constraints.at( y ).at( k ));
                    constraints.append( field->name());
                    constraints.append( k == cc - 1 ? " )" : ", " );

                }

                if ( y < tc - 1 )
                    constraints.append( ", " );
            }

            statement.append( ", " );
            statement.append( constraints );
        }

        if ( !query.exec( QString( "create table if not exists %1 ( %2 )" ).arg( table->tableName()).arg( statement )))
            qCCritical( Database_::Debug ) << this->tr( "could not create table - \"%1\", reason - \"%2\"" ).arg( table->tableName()).arg( query.lastError().text());
    }

    // table has been verified and is marked as valid
    table->setValid();

    // create table model
    table->setTable( table->tableName());

    // load data
    if ( !table->select()) {
        qCCritical( Database_::Debug ) << this->tr( "could not initialize model for table - \"%1\"" ).arg( table->tableName());
        table->setValid( false );
    }

    return true;
}

