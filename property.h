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
#include <QSqlQuery>

//
// classes
//
class Template;

/**
 * @brief The Property class
 */
class Property : public Entry {
    Q_OBJECT
    Q_DISABLE_COPY( Property )
    Q_CLASSINFO( "description", "Property SQL Entry" )
    Q_PROPERTY( QString textValue READ textValue WRITE setTextValue )
    Q_PROPERTY( int templateId READ templateId WRITE setTemplateId )

public:
    explicit Property( const QSqlRecord &record ) { this->setRecord( record ); this->setTable( "properties" ); }
    ~Property() {}

    QString textValue() const { return this->record().value( "textValue" ).toString(); }
    int templateId() const { return this->record().value( "templateId" ).toInt(); }

    // static functions
    static Property *fromId( int id );
    static Property *add( const QString &name , const QString &value, int templateId );
    static Property *store( const QSqlQuery &query );
    static void load();
    static bool contains( const QString &name );

public slots:
    void setTextValue( const QString &textValue ) { this->setValue( "textValue", textValue ); }
    void setTemplateId( const int templateId ) { this->setValue( "templateId", templateId ); }
};
