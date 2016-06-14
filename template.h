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

#ifndef TEMPLATE_H
#define TEMPLATE_H

//
// includes
//
#include <QObject>
#include "entry.h"
#include "property.h"

//
// class: Template
//
class Template : public Entry {
    Q_OBJECT
    Q_CLASSINFO( "description", "Template SQL Entry" )

public:
    enum State {
        Liquid = 0,
        Solid
    };
    Template( const QSqlRecord &record ) { this->setRecord( record ); this->setTable( "templates" ); }
    QString name() const { return this->record().value( "name" ).toString(); }
    double amount() const { return this->record().value( "amount" ).toDouble(); }
    double density() const { return this->record().value( "density" ).toDouble(); }
    double assay() const { return this->record().value( "assay" ).toDouble(); }
    double molarMass() const { return this->record().value( "molarMass" ).toDouble(); }
    State state() const { return static_cast<State>( this->record().value( "state" ).toInt()); }
    static Template *fromId( int id );

    QList<Property*> propertyList;

public slots:
    void setName( const QString &name ) { this->setValue( "name", name ); }
    void setAmount( const double amount ) { this->setValue( "amount", amount ); }
    void setDensity( const double density ) { this->setValue( "density", density ); }
    void setAssay( const double assay ) { this->setValue( "assay", assay ); }
    void setMolarMass( const double molarMass ) { this->setValue( "molarMass", molarMass ); }
    void setState( const State state ) { this->setValue( "state", static_cast<int>( state )); }
    static void add( const QString &name, const double amount, const double density, const double assay, const double molarMass, const State state );
};

#endif // TEMPLATE_H
