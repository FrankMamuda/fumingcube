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

#ifndef PROPERTY_H
#define PROPERTY_H

//
// includes
//
#include <QObject>
#include "entry.h"

/**
 * @brief The Property class
 */
class Property : public Entry {
    Q_OBJECT
    Q_CLASSINFO( "description", "Property SQL Entry" )

public:
    Property( const QSqlRecord &record ) { this->setRecord( record ); this->setTable( "properties" ); }
    QString propertyName() const { return this->record().value( "property" ).toString(); }
    QString propertyValue() const { return this->record().value( "value" ).toString(); }
    int reagentId() const { return this->record().value( "reagentId" ).toInt(); }
    static Property *fromId( int id );

public slots:
    void setPropertyName( const QString &name ) { this->setValue( "property", name ); }
    void setPropertyValue( const QString &value ) { this->setValue( "value", value ); }
    void setReagentId( const int id ) { this->setValue( "reagentId", id ); }
    static void add( const int reagentId, const QString &property, const QString &value );
};

#endif // PROPERTY_H
