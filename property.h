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
    Q_PROPERTY( QString title READ title WRITE setTitle )
    Q_PROPERTY( QString html READ html WRITE setHtml )
    Q_PROPERTY( int templateId READ templateId WRITE setTemplateId )
    Q_PROPERTY( int order READ order WRITE setOrder )

public:
    explicit Property( const QSqlRecord &record ) { this->setRecord( record ); this->setTable( "properties" ); }
    ~Property() {}

    QString title() const { return this->name(); }
    QString html() const { return this->record().value( "html" ).toString(); }
    int templateId() const { return this->record().value( "templateId" ).toInt(); }
    int order() const { return this->record().value( "parent" ).toInt(); }

    // static functions
    static Property *fromId( int id );
    static Property *add( const QString &title, const QString &html, int templateId );
    static Property *store( const QSqlQuery &query );
    static void load();
    static bool contains( const QString &title );

public slots:
    void setTitle( const QString &title ) { this->setName( title ); }
    void setHtml( const QString &html ) { this->setValue( "html", html ); }
    void setTemplateId( const int templateId ) { this->setValue( "templateId", templateId ); }
    void setOrder( const int order ) { this->setValue( "parent", order ); }
};
