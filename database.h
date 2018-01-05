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
#include "reagent.h"
#include "singleton.h"
#include "template.h"
#include <QMap>

//
// classes
//
class Template;
class Reagent;
class Property;

/**
 * @brief The tableField struct
 */
typedef struct tableField_s {
    const char *name;
    const char *type;
} tableField_t;

/**
 * @brief The table struct
 */
typedef struct table_s {
    const char *name;
    const tableField_t *fields;
    const unsigned int numFields;
} table_t;

/**
 * @brief The API namespace
 */
namespace API {
// tasks
const static tableField_t templateFields[] = {
    { "id", "integer primary key" },
    { "name", "text" },
    { "amount", "float" },
    { "density", "float" },
    { "assay", "float" },
    { "molarMass", "float" },
    { "state", "integer" },
    { "reagentId", "integer" },
};

// properties
const static tableField_t propertyFields[] = {
    { "id", "integer primary key" },
    { "name", "text" },
    { "textValue", "text" },
    { "templateId", "integer" }
};

// reagents
const static tableField_t reagentFields[] = {
    { "id", "integer primary key" },
    { "name", "text" },
};

// tables
const static table_t tables[] = {
    { "templates",   templateFields, sizeof( templateFields ) / sizeof( tableField_t ) },
    { "properties",  propertyFields, sizeof( propertyFields ) / sizeof( tableField_t ) },
    { "reagents",    reagentFields,  sizeof( reagentFields )  / sizeof( tableField_t ) },
};
const unsigned int numTables = sizeof( tables ) / sizeof( table_t );
}

/**
 * @brief The Database class
 */
class Database : public QObject {
    Q_OBJECT

public:
    static Database *instance() { return Singleton<Database>::instance( Database::createInstance ); }
    ~Database();
    QString path() const { return this->m_path; }
    QMap<int, Reagent*> reagentMap;
    QMap<int, Template*> templateMap;
    QMap<int, Property*> propertyMap;

signals:
    void changed();

public slots:
    void update() { emit this->changed(); }
    void load();

private slots:
    void setPath( const QString &path ) { this->m_path = path; }
    void removeOrphanedEntries();

private:
    explicit Database( QObject *parent = nullptr );
    static Database *createInstance() { return new Database(); }
    QString m_path;
    QStringList generateSchemas();
    bool createEmptyTable();
    bool createStructure();
};
