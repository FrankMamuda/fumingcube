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

#pragma once

//
// includes
//
#include <QLoggingCategory>
#include <QSharedPointer>
#include <QSqlTableModel>

//
// classes
//
class Table;

/**
 * @brief The Dabanase_ class
 */
namespace Database_ {
const static QLoggingCategory Debug( "database" );
#ifdef Q_CC_MSVC
const static unsigned int null = 0;
#else
const static  __attribute__((unused)) unsigned int null = 0;
#endif
};


/**
 * @brief The Database class
 */
class Database_N final : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY( Database_N )

public:
    /**
     * @brief instance
     * @return
     */
    static Database_N *instance() { static Database_N *instance( new Database_N()); return instance; }
    ~Database_N();
    void add( Table *table );
    bool hasInitialised() const { return this->m_initialised; }

public slots:
    void removeOrphanedEntries();

private:
    explicit Database_N( QObject *parent = nullptr );
    void setInitialised( bool initialised = true ) { this->m_initialised = initialised; }

    /**
     * @brief createInstance
     * @return
     */
    QMap<QString, Table*> tables;
    bool m_initialised;
};
