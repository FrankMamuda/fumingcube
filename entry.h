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

#pragma once

//
// includes
//
#include <QObject>
#include <QSqlRecord>
#include <QVariant>

/**
 * @brief The Entry class
 */
class Entry : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY( Entry )
    Q_PROPERTY( int id READ id )
    Q_PROPERTY( QString table READ table WRITE setTable )
    Q_PROPERTY( QSqlRecord record READ record WRITE setRecord )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_CLASSINFO( "description", "SQL Entry" )

public:
    explicit Entry() {}
    ~Entry() { this->m_table.clear(); this->m_record.clear(); }
    virtual QString name() const { return this->record().value( "name" ).toString(); }
    virtual int id () const { return this->record().value( "id" ).toInt(); }
    virtual QSqlRecord record() const { return this->m_record; }
    virtual QString table() const { return this->m_table; }

public slots:
    virtual void setName( const QString &name ) { this->setValue( "name", name ); }

protected slots:
    virtual void setTable( const QString &tableName ) { this->m_table = tableName; }
    virtual void setRecord( const QSqlRecord &record ) { this->m_record = record; }
    virtual void setValue( const QString &valueName, const QVariant &value );

private:
    QString m_table;
    QSqlRecord m_record;

signals:
    void changed();
};
