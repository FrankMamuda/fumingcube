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

#ifndef DATABASE_H
#define DATABASE_H

//
// includes
//
#include <QString>
#include <QList>
#include "reagent.h"
#include "property.h"
#include "template.h"

/**
 * @brief The Database class
 */
class Database : public QObject {
    Q_OBJECT

public:
    static Database &instance() { static Database *instance = new Database(); return *instance; }
    void load();
    void unload();
    QString encrypt( const QString &input );
    QList<Reagent*> reagentList;
    QList<Property*> propertyList;
    QList<Template*> templateList;

private:
    Database() { }
    void create();
    void makePath();
    void loadReagents();
    void loadProperties();
    QString path;
};

//
// externals
//
extern Database &db;

#endif // DATABASE_H
