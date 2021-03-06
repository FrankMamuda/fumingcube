/*
 * Copyright (C) 2019-2020 Armands Aleksejevs
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
#include <QDate>

/**
 * @brief The Reagent class
 */
class Reagent final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY( Reagent )

public:
    // disable move
    Reagent( Reagent&& ) = delete;
    Reagent& operator=( Reagent&& ) = delete;

    /**
     * @brief The Fields enum
     */
    enum Fields {
        NoField = -1,
        ID,
        Name,
        Reference,
        ParentId,
        DateTime,

        // count (DO NOT REMOVE)
                Count
    };
    Q_ENUM( Fields )

    /**
     * @brief instance
     * @return
     */
    static Reagent *instance() {
        static auto *reagent( new Reagent());
        return reagent;
    }
    ~Reagent() override = default;
    Row add(const QString &name, const QString &reference, const Id &parentId = Id::Invalid, const QDateTime &dateTime = QDateTime());
    [[nodiscard]] QList<Row> children( const Row &row ) const;
    [[nodiscard]] QList<Id> labelIds( const Row &row ) const;

    // initialize field setters and getters
    INITIALIZE_FIELD( Id, ID, id )
    INITIALIZE_FIELD( QString, Name, name )
    INITIALIZE_FIELD( QString, Reference, reference )
    INITIALIZE_FIELD( Id, ParentId, parentId )

    /**
     * @brief dateTime
     * @param row
     * @return
     */
    [[nodiscard]] QDateTime dateTime( const Row &row ) {
        return QDateTime::fromSecsSinceEpoch( this->value( row, DateTime ).toInt());
    }

    /**
     * @brief dateTime
     * @param id
     * @return
     */
    [[nodiscard]] QDateTime dateTime( const Id &id ) {
        const int dateTime = this->value( id, DateTime ).toInt();
        if ( dateTime <= 0 )
            return QDateTime();

        return QDateTime::fromSecsSinceEpoch( this->value( id, DateTime ).toInt());
    }

public slots:
    void removeOrphanedEntries() override;
    void remove( const Row &row ) override;

    /**
     * @brief setDateTime
     * @param row
     * @param dateTime
     */
    void setDateTime( const Row &row, const QDateTime &dateTime ) {
        this->setValue( row, DateTime, dateTime.toSecsSinceEpoch());
    }

private:
    explicit Reagent();
};

// declare enums
Q_DECLARE_METATYPE( Reagent::Fields )
