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

#ifndef ENTRY_H
#define ENTRY_H

//
// includes
//
#include <QObject>
#include <QVariant>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include "main.h"
#include <QDebug>

//
// class: Entry
//
class Entry : public QObject {
    Q_OBJECT
    Q_PROPERTY( int id READ id )
    Q_PROPERTY( QString table READ table WRITE setTable )
    Q_PROPERTY( QSqlRecord record READ record WRITE setRecord )
    Q_CLASSINFO( "description", "SQL Entry" )

public:
    Entry() {}
    ~Entry() { this->m_table.clear(); this->m_record.clear(); }
    int id () const { return this->record().value( "id" ).toInt(); }
    QSqlRecord record() const { return this->m_record; }
    QString table() const { return this->m_table; }

public slots:
    void setTable( const QString &name ) { this->m_table = name; }
    void setRecord( const QSqlRecord &record ) { this->m_record = record; }
    void setValue( const QString &name, const QVariant &value ) {
        QSqlQuery query;
        QVariant update;
        QString table = this->table();

        // copy local value
        update = value;

        // make sure we don't perform useless updates
        if ( this->record().value( name ) == update )
            return;

        // update counters
        emit this->changed();

        // store local value
        this->m_record.setValue( name, update );

        // check for strings (should be wrapped in quotes)
        if ( value.type() == QVariant::String )
            update.setValue( QString( "%1" ).arg( value.toString()));

        // update database value
        if ( !this->table().isNull()) {
            query.prepare( QString( "update %1 set %2 = :value where id = :id" ).arg( table ).arg( name ));

            qDebug() << QString( "update %1 set %2 = %3 where id = %4" ).arg( table ).arg( name ).arg( update.toString()).arg( this->record().value( "id" ).toInt());

            query.bindValue( ":value", update.toString());
            query.bindValue( ":id", this->record().value( "id" ).toInt());

            if ( !query.exec())
                m.error( Main::SoftError, QString( "could not store value, reason - '%1'\n" ).arg( query.lastError().text()));
        }
    }
    void store() {
        QSqlQuery query;
        query.exec( QString( "insert into %1 select * from merge.%1 where id = %2" ).arg( this->table()).arg( this->id()));
    }

private:
    QString m_table;
    QSqlRecord m_record;

signals:
    void changed();
};

#endif // ENTRY_H
