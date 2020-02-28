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
#include <QPixmap>
#include <QByteArray>
#include <QMap>

/**
 * @brief The PixmapUtils class
 */
class PixmapUtils final {
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

    [[nodiscard]] static QPixmap autoCrop( const QPixmap &pixmap, const QColor &key = Qt::white );
    [[maybe_unused]][[nodiscard]] static QPixmap brighten( const QPixmap &pixmap, int factor = 150 );
    [[nodiscard]] static QPixmap invert( const QPixmap &pixmap );
    [[nodiscard]] static QByteArray convertToData( const QPixmap &pixmap, const QString &key = QString());
    [[nodiscard]] static QPixmap getOpenPixmap( QWidget *context );

private:
    explicit PixmapUtils() {}
};
