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

#pragma once

//
// includes
//
#include "entry.h"

//
// classes
//
class QSqlQuery;

/**
 * @brief The Template class
 */
class Template : public Entry {
    Q_OBJECT
    Q_DISABLE_COPY( Template )
    Q_CLASSINFO( "description", "Template SQL Entry" )
    Q_PROPERTY( double amount READ amount WRITE setAmount )
    Q_PROPERTY( double density READ density WRITE setDensity )
    Q_PROPERTY( double assay READ assay WRITE setAssay )
    Q_PROPERTY( double molarMass READ molarMass WRITE setMolarMass )
    Q_PROPERTY( State state READ state WRITE setState )
    Q_PROPERTY( int reagentId READ reagentId WRITE setReagentId )
    Q_ENUMS( State )

public:
    enum State {
        Solid = 0,
        Liquid
    };
    explicit Template( const QSqlRecord &record ) { this->setRecord( record ); this->setTable( "templates" ); }
    ~Template() {}
    double amount() const { return this->record().value( "amount" ).toDouble(); }
    double density() const { return this->record().value( "density" ).toDouble(); }
    double assay() const { return this->record().value( "assay" ).toDouble(); }
    double molarMass() const { return this->record().value( "molarMass" ).toDouble(); }
    State state() const { return static_cast<State>( this->record().value( "state" ).toInt()); }
    int reagentId() const { return this->record().value( "reagentId" ).toInt(); }

    // static functions
    static Template *fromId( int id );
    static void add( const QString &name, const double amount, const double density, const double assay, const double molarMass, const State state, const int reagentId );
    static void load();
    static void store( const QSqlQuery &query );

public slots:
    void setAmount( const double amount ) { this->setValue( "amount", amount ); }
    void setDensity( const double density ) { this->setValue( "density", density ); }
    void setAssay( const double assay ) { this->setValue( "assay", assay ); }
    void setMolarMass( const double molarMass ) { this->setValue( "molarMass", molarMass ); }
    void setState( const State state ) { this->setValue( "state", static_cast<int>( state )); }
    void setReagentId( const int reagentId ) { this->setValue( "reagentId", reagentId ); }
};
