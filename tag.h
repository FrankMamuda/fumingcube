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
        MinValue,
        MaxValue,
        DefaultValue,
        Precision,
        Function,
        Scale,
        Script,

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
        State,
        Formula
    };
    Q_ENUM( Types )

    /**
     * @brief instance
     * @return
     */
    static Tag *instance() { static Tag *i( new Tag()); return i; }
    ~Tag() override = default;
    Row add( const QString &name, const Types &type = Text, const QString &units = QString(),
             const QVariant &min = QVariant(), const QVariant &max = QVariant(),
             const QVariant &value = QVariant(), const int precision = 0,
             const QString &function = QString(), const qreal scale = 1.0,
             const QVariant &script = QVariant());

    // initialize field setters and getters
    INITIALIZE_FIELD( Id,       ID,           id )
    INITIALIZE_FIELD( QString,  Name,         name )
    INITIALIZE_FIELD( Types,    Type,         type )
    INITIALIZE_FIELD( QString,  Units,        units )
    INITIALIZE_FIELD( int,      Precision,    precision )
    INITIALIZE_FIELD( QVariant, MinValue,     minValue )
    INITIALIZE_FIELD( QVariant, MaxValue,     maxValue )
    INITIALIZE_FIELD( QVariant, DefaultValue, defaultValue )
    INITIALIZE_FIELD( QString,  Function,     function )
    INITIALIZE_FIELD( qreal,    Scale,        scale )
    INITIALIZE_FIELD( QVariant, Script,       script )

public slots:
    void removeOrphanedEntries() override;
    void populate();

private:
    explicit Tag();
};

// declare enums
Q_DECLARE_METATYPE( Tag::Fields )
Q_DECLARE_METATYPE( Tag::Types )
