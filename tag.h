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

constexpr const static Id PixmapTag = static_cast<Id>( -2 );

/**
 * @brief The Tag class
 */
class Tag final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY( Tag )

public:
    /**
     * @brief The Fields enum
     */
    enum Fields {
        NoField = -1,
        ID,
        Name,
        Type,
        Units,
        Min,
        Max,
        Value,
        Precision,
        Function,
        Scale,

        // count (DO NOT REMOVE)
        Count
    };
    Q_ENUM( Fields )

    enum Types {
        NoType = -1,
        Text,
        Integer,
        Real,
        GHS,
        NFPA,
        CAS,
        State
    };
    Q_ENUM( Types )

    /**
     * @brief instance
     * @return
     */
    static Tag *instance() { static Tag *i( new Tag()); return i; }
    ~Tag() override = default;
    Row add( const QString &name, const Types &type = Text, const QString &units = QString(), const QVariant &min = QVariant(), const QVariant &max = QVariant(), const QVariant &value = QVariant(), const int precision = 0, const QString &function = QString(), const qreal scale = 1.0 );

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
     * @brief units
     * @param row
     * @return
     */
    QString units( const Row &row ) const { return this->value( row, Units ).toString(); }

    /**
     * @brief type
     * @param row
     * @return
     */
    Types type( const Row &row ) const { return static_cast<Types>( this->value( row, Type ).toInt()); }

    /**
     * @brief precison
     * @param row
     * @return
     */
    int precison( const Row &row ) const { return this->value( row, Precision ).toInt(); }

    /**
     * @brief min
     * @param row
     * @return
     */
    QVariant min( const Row &row ) const { return this->value( row, Min ); }

    /**
     * @brief max
     * @param row
     * @return
     */
    QVariant max( const Row &row ) const { return this->value( row, Max ); }

    /**
     * @brief defaultValue
     * @param row
     * @return
     */
    QVariant defaultValue( const Row &row ) const { return this->value( row, Value ); }

    /**
     * @brief function
     * @param row
     * @return
     */
    QString function( const Row &row ) const { return this->value( row, Function ).toString(); }

    /**
     * @brief scale
     * @param row
     * @return
     */
    qreal scale( const Row &row ) const { return this->value( row, Scale ).toDouble(); }

public slots:
    void removeOrphanedEntries() override;
    void populate();

    /**
     * @brief setName
     * @param row
     * @param name
     */
    void setName( const Row &row, const QString &name ) { this->setValue( row, Name, name ); }

private:
    explicit Tag();
};

// declare enums
Q_DECLARE_METATYPE( Tag::Fields )
Q_DECLARE_METATYPE( Tag::Types )
