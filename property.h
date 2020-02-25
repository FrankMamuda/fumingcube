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

/**
 * @brief The Property class
 */
class Property final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY( Property )

public:
    // disable move
    Property( Property&& ) = delete;
    Property& operator=( Property&& ) = delete;

    /**
     * @brief The StateOfMatter enum
     */
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
        TagId,
        PropertyData,
        ReagentId,
        TableOrder,

        // count (DO NOT REMOVE)
        Count
    };
    Q_ENUM( Fields )

    /**
     * @brief instance
     * @return
     */
    static Property *instance() {
        static auto *property( new Property());
        return property;
    }
    ~Property() override = default;
    Row add( const QString &name = QString(), const Id &tagId = Id::Invalid,
             const QVariant &value = QVariant(), const Id &reagentId = Id::Invalid );

    // initialize field setters and getters
    INITIALIZE_FIELD( Id, ID, id )
    INITIALIZE_FIELD( QString, Name, name )
    INITIALIZE_FIELD( Id, TagId, tagId )
    INITIALIZE_FIELD( QVariant, PropertyData, propertyData )
    INITIALIZE_FIELD( Id, ReagentId, reagentId )
    INITIALIZE_FIELD( int, TableOrder, tableOrder )

protected:
    [[nodiscard]] QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;

public slots:
    void removeOrphanedEntries() override;

private:
    explicit Property();
};

// declare enums
Q_DECLARE_METATYPE( Property::Fields )
