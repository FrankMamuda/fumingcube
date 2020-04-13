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

/*
 * includes
 */
#include "pixmaputils.h"
#include <QBuffer>
#include <QFileDialog>
#include <QImageReader>

/**
 * @brief PixmapUtils::autoCrop
 * @param pixmap
 * @param key
 * @return
 */
QPixmap PixmapUtils::autoCrop( const QPixmap &pixmap, const QColor &key ) {
    const QImage image( pixmap.toImage());

    // check left
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
    for ( int x = 0; x < image.width(); x++ ) {
        bool found = false;

        for ( int y = 0; y < image.height(); y++ ) {
            if ( image.pixelColor( x, y ) != key ) {
                left = x;
                found = true;
                break;
            }
        }
        if ( found )
            break;
    }

    // check right
    for ( int x = image.width() - 1; x >= 0; x-- ) {
        bool found = false;

        for ( int y = 0; y < image.height(); y++ ) {
            if ( image.pixelColor( x, y ) != key ) {
                right = x;
                found = true;
                break;
            }
        }
        if ( found )
            break;
    }

    // find bottom
    for ( int y = image.height() - 1; y >= 0; y-- ) {
        bool found = false;

        for ( int x = 0; x < image.width(); x++ ) {
            if ( image.pixelColor( x, y ) != key ) {
                bottom = y;
                found = true;
                break;
            }
        }
        if ( found )
            break;
    }

    // find bottom
    for ( int y = 0; y < image.height(); y++ ) {
        bool found = false;

        for ( int x = 0; x < image.width(); x++ ) {
            if ( image.pixelColor( x, y ) != key ) {
                top = y;
                found = true;
                break;
            }
        }
        if ( found )
            break;
    }

    QRect copyRect( QRect( left, top, right - left + 1, bottom - top + 1 ));
    QImage out( image.convertToFormat( QImage::Format_ARGB32 ));

    auto alphaFromKey = []( const QColor &colour, const QColor &key ) {
        QVector<qreal> alpha( 3 );
        QVector<qreal> colours = { colour.redF(), colour.greenF(), colour.blueF(), colour.alphaF() };
        const QVector<qreal> keyColours = { key.redF(), key.greenF(), key.blueF(), key.alphaF() };

        for ( int y = 0; y < 3; y++ ) {
            if ( colours[y] > keyColours[y] )
                alpha[y] = ( colours[y] - keyColours[y] ) / ( 255.0 - keyColours[y] );
            else if ( colours[y] < keyColours[y] )
                alpha[y] = ( keyColours[y] - colours[y] ) / ( keyColours[y] );
            else
                alpha[y] = 0.0;
        }

        if ( alpha[0] > alpha[1] )
            colours[3] = ( alpha[0] > alpha[2] ) ? alpha[0] : alpha[2];
        else
            colours[3] = ( alpha[1] > alpha[2] ) ? alpha[1] : alpha[2];

        colours[3] *= 255.0;

        if ( colours[3] > 1.0 ) {
            for ( int y = 0; y < 3; y++ )
                colours[y] = qBound( 0.0, 255.0 * ( colours[y] - keyColours[y] ) / colours[3] + keyColours[y], 1.0 );

            colours[3] = qBound( 0.0, colours[3] * colour.alphaF() / 255.0, 1.0 );
        }

        return QColor::fromRgbF( colours[0], colours[1], colours[2], colours[3] );
    };

    for ( int x = 0; x < out.width(); x++ ) {
        for ( int y = 0; y < out.height(); y++ ) {
            const QColor colour( out.pixelColor( x, y ));
            out.setPixelColor( x, y, alphaFromKey( colour, key ));//.lighter( 500 ).lighter( 500 ));
        }
    }

    return QPixmap::fromImage( out.copy( copyRect ));
}

/**
 * @brief PixmapUtils::brighten
 * @param pixmap
 * @param factor
 * @return
 */
QPixmap PixmapUtils::brighten( const QPixmap &pixmap, const int factor ) {
    QImage out( pixmap.toImage());
    for ( int x = 0; x < out.width(); x++ ) {
        for ( int y = 0; y < out.height(); y++ ) {
            const QColor colour( out.pixelColor( x, y ));
            out.setPixelColor( x, y, colour.lighter( factor ));
        }
    }

    return QPixmap::fromImage( qAsConst( out ));
}

/**
 * @brief PixmapUtils::invert
 * @param pixmap
 * @return
 */
QPixmap PixmapUtils::invert( const QPixmap &pixmap ) {
    QImage out( pixmap.toImage());
    out.invertPixels();
    return QPixmap::fromImage( qAsConst( out ));
}

/**
 * @brief PixmapUtils::convertToData
 * @param pixmap
 * @param key
 * @return
 */
QByteArray PixmapUtils::convertToData( const QPixmap &pixmap, const QString &key ) {
    if ( !key.isEmpty() && PixmapUtils::instance()->cache.contains( key ))
        return PixmapUtils::instance()->cache[key];

    QByteArray data;
    if ( pixmap.isNull())
        return data;

    QBuffer buffer( &data );
    buffer.open( QIODevice::WriteOnly );
    pixmap.save( &buffer, "PNG" );
    buffer.close();

    if ( !key.isEmpty())
        PixmapUtils::instance()->cache[key] = data;

    return data;
}

/**
 * @brief PixmapUtils::getOpenPixmap
 * @param context
 * @return
 */
QPixmap PixmapUtils::getOpenPixmap( QWidget *context ) {
    const QString fileName( QFileDialog::getOpenFileName( context, QWidget::tr( "Open Image" ), "", QWidget::tr( "Images (*.png *.jpg)" )));
    if ( fileName.isEmpty())
        return QPixmap();

    return QPixmap::fromImage( QImageReader( fileName ).read());
}
