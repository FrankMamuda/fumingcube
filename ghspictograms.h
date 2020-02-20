/*
 * Copyright (C) 2017-2018 Factory #12
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
#include <QIcon>
#include <QMap>
#include <QPixmapCache>

/**
 * @brief The Pictograms namespace
 */
namespace GHSHazards {
    const QMap<QString, QString> Hazards {
            { "GHS01", QT_TR_NOOP( "Explosive" ) },
            { "GHS02", QT_TR_NOOP( "Flammable" ) },
            { "GHS03", QT_TR_NOOP( "Oxidizing" ) },
            { "GHS04", QT_TR_NOOP( "Compressed gas" ) },
            { "GHS05", QT_TR_NOOP( "Corrosive" ) },
            { "GHS06", QT_TR_NOOP( "Toxic" ) },
            { "GHS07", QT_TR_NOOP( "Harmful" ) },
            { "GHS08", QT_TR_NOOP( "Health hazard" ) },
            { "GHS09", QT_TR_NOOP( "Environment hazard" ) }
    };
}

/**
 * @brief The GHSPictograms class
 */
class GHSPictograms {
public:
    /**
     * @brief icon
     * @param index
     * @return
     */
    static QIcon icon( const QString &key ) {
        if ( !GHSHazards::Hazards.contains( key ))
            return QIcon();

        return QIcon( QString( ":/pictograms/%1" ).arg( key ));
    }

    /**
     * @brief pixmap
     * @param index
     * @param scale
     * @return
     */
    static QPixmap pixmap( const QString &key, const int &scale ) {
        if ( scale <= 0 || scale > 512 )
            return QPixmap();

        QPixmap pixmap;
        const QString cacheKey( QString( ":/pictograms/%1_%2" ).arg( key ).arg( scale ));
        if ( !QPixmapCache::find( cacheKey, &pixmap )) {
            pixmap = GHSPictograms::icon( key ).pixmap( scale, scale );
            QPixmapCache::insert( cacheKey, qAsConst( pixmap ));
        }

        return qAsConst( pixmap );
    }

    /**
     * @brief name
     * @param index
     * @return
     */
    static QString name( const QString &key ) {
        if ( !GHSHazards::Hazards.contains( key ))
            return QString();

        return GHSHazards::Hazards[key];
    }
};
