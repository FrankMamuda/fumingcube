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
#include "table.h"

/**
 * @brief The PropertyTable namespace
 */
namespace PropertyTable {
const static QString Name( "properties" );
}

/**
 * @brief The Property class
 */
class Property final : public Table {
    Q_OBJECT
    Q_ENUMS( Fields )
    Q_DISABLE_COPY( Property )

public:
    enum Fields {
        NoField = -1,
        ID,
        Name,
        HTML,
        Template,
      //Tag,
        Order,

        // count
        Count
    };

    enum Roles {
        PropertyIdRole = Qt::UserRole,
        NameWidthRole,
        HTMLWidthRole
    };

    /**
     * @brief instance
     * @return
     */
    static Property *instance() { static Property *instance = new Property(); return instance; }
    Row add( const QString &name, const QString &html, const Id &templateId );
    Id id( const Row &row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }
    QString name( const Row &row ) const { return this->value( row, Name ).toString(); }
    QString html( const Row &row ) const { return this->value( row, HTML ).toString(); }
    Id templateId( const Row &row ) const { return static_cast<Id>( this->value( row, Template ).toInt()); }
    int order( const Row &row ) const { return this->value( row, Order ).toInt(); }
  //Id tagId( const Row &row ) const { return static_cast<Id>( this->value( row, Tag ).toInt()); }

public slots:
    void setName( const Row &row, const QString &name ) { this->setValue( row, Name, name ); }
    void setHTML( const Row &row, const QString &html ) { this->setValue( row, HTML, html ); }
    void setOrder( const Row &row, const int order ) { this->setValue( row, Order, order ); }
    void removeOrphanedEntries() override;

private:
    explicit Property();
};

// declare enums
Q_DECLARE_METATYPE( Property::Fields )
