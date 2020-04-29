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
#include "main.h"
#include <QPixmap>
#include <QByteArray>
#include <QMap>

/**
 * @brief The PixmapInfo class
 */
class PixmapInfo {
public:
    int width = 0;
    int height = 0;
    quint32 crc = 0;
    bool isValid() { return this->width > 0 && this->height > 0 && this->crc != 0; }
};

/**
 * @brief The PixmapUtils class
 */
class PixmapUtils final : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY( PixmapUtils )

public:
    // disable move
    PixmapUtils( PixmapUtils&& ) = delete;
    PixmapUtils& operator=( PixmapUtils&& ) = delete;

    QMap<QString, QByteArray> cache;

    /**
     * @brief instance
     * @return
     */
    static PixmapUtils *instance() {
        static auto *pu( new PixmapUtils());
        return pu;
    }

    [[nodiscard]] static QPixmap cropAndRemoveAlpha( const QPixmap &pixmap, const QColor &key = Qt::white );
    [[maybe_unused]][[nodiscard]] static QPixmap brighten( const QPixmap &pixmap, int factor = 150 );
    [[nodiscard]] static QPixmap invert( const QPixmap &pixmap );
    [[nodiscard]] static QByteArray toData( const QPixmap &pixmap );
    [[nodiscard]] static QPixmap getOpenPixmap( QWidget *context );
    [[nodiscard]] static bool readHeader( const QByteArray &array, PixmapInfo *info = nullptr );
    [[nodiscard]] static QPixmap fromUrl( const QUrl &url );

private:
    explicit PixmapUtils() {
        // add to garbage collector
        GarbageMan::instance()->add( this );
    }
};
