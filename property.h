/*
 * Copyright (C) 2019 Armands Aleksejevs
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

/*
 * includes
 */
#include "table.h"

/**
 * @brief The Property class
 */
class Property final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY( Property )

public:
    enum StateOfMatter {
        NoState = -1,
        Solid,
        Liquid,
        Gaseous,
        Plasma,

        OtherState = 1024
    };
    Q_ENUM( StateOfMatter )

    /**
     * @brief The Fields enum
     */
    enum Fields {
        NoField = -1,
        ID,
        Name,
        TagID,
        Value,
        ReagentID,
        Order_,

        // count (DO NOT REMOVE)
        Count
    };
    Q_ENUM( Fields )

    /**
     * @brief instance
     * @return
     */
    static Property *instance() { static Property *property( new Property()); return property; }
    ~Property() override = default;
    Row add( const QString &name = QString(), const Id &tagId = Id::Invalid,
             const QVariant &value = QVariant(), const Id &reagentId = Id::Invalid );

    /**
     * @brief id
     * @param row
     * @return
     */
    Id id( const Row &row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }

    /**
     * @brief name
     * @param row
     * @return
     */
    QString name( const Row &row ) const { return this->value( row, Name ).toString(); }

    /**
     * @brief tag
     * @param row
     * @return
     */
    Id tagId( const Row &row ) const { return static_cast<Id>( this->value( row, TagID ).toInt()); }

    /**
     * @brief parentId
     * @param row
     * @return
     */
    Id reagentId( const Row &row ) const { return static_cast<Id>( this->value( row, ReagentID ).toInt()); }

    /**
     * @brief valueData
     * @param row
     * @return
     */
    QVariant valueData( const Row &row ) const { return this->value( row, Value ); }

    /**
     * @brief stringValue
     * @param row
     * @return
     */
    QString stringValue( const Row &row ) const { return this->value( row, Value ).toString(); }

    /**
    * @brief order
    * @param row
    * @return
    */
   int order( const Row &row ) const { return this->value( row, Order_ ).toInt(); }

public slots:
    void removeOrphanedEntries() override;

    /**
     * @brief setName
     * @param row
     * @param name
     */
    void setName( const Row &row, const QString &name ) { this->setValue( row, Name, name ); }

    /**
     * @brief setOrder
     * @param row
     * @param position
     */
    void setOrder( const Row &row, const int &position ) { this->setValue( row, Order_, position ); }

    /**
     * @brief setStringValue
     * @param row
     * @param value
     */
    void setStringValue( const Row &row, const QString &value ) { this->setValue( row, Value, value ); }


private:
    explicit Property();
};

// declare enums
Q_DECLARE_METATYPE( Property::Fields )
