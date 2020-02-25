/*
 * Copyright (C) 2020 Armands Aleksejevs
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
#include <QColor>
#include <QPixmap>

/**
 * @brief The Label class
 */
class Label final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY( Label )

public:
    // disable move
    Label( Label&& ) = delete;
    Label& operator=( Label&& ) = delete;

    /**
     * @brief The Fields enum
     */
    enum Fields {
        NoField = -1,
        ID,
        Name,
        Colour,

        // count (DO NOT REMOVE)
                Count
    };
    Q_ENUM( Fields )

    /**
     * @brief instance
     * @return
     */
    static Label *instance() {
        static auto *label( new Label());
        return label;
    }
    ~Label() override = default;
    Row add( const QString &name, const QColor &colour = Qt::transparent );

    // initialize field setters and getters
    INITIALIZE_FIELD( Id, ID, id )
    INITIALIZE_FIELD( QString, Name, name )
    INITIALIZE_FIELD( QColor, Colour, colour )

    [[nodiscard]] QVariant data( const QModelIndex &index, int role ) const override;
    [[nodiscard]] QPixmap pixmap( const QColor &colour ) const;
    [[nodiscard]] QPixmap pixmap( const QList<QColor> &colourList ) const;

public slots:
    void removeOrphanedEntries() override;
    void populate();

private:
    explicit Label();
    mutable QMap<QString, QPixmap> cache;
};

// declare enums
Q_DECLARE_METATYPE( Label::Fields )
