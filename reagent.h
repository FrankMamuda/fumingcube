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
#include "entry.h"

//
// classes
//
class Template;

/**
 * @brief The Reagent class
 */
class Reagent : public Entry {
    Q_OBJECT
    Q_DISABLE_COPY( Reagent )
    Q_CLASSINFO( "description", "Reagent SQL Entry" )

public:
    explicit Reagent( const QSqlRecord &record ) { this->setRecord( record ); this->setTable( "reagents" ); }
    ~Reagent() {}
    QMap<int, Template*> templateMap;

    // static functions
    static Reagent *fromId( int id );
    static Reagent *add( const QString &name );
    static void load();
    static bool contains( const QString &name );
    static Reagent *fromName( const QString &name );
};
