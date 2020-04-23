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
#include "cache.h"
#include "imageutils.h"

/**
 * @brief PixmapUtils::autoCrop
 * @param pixmap
 * @param key
 * @return
 */
QPixmap PixmapUtils::cropAndRemoveAlpha( const QPixmap &pixmap, const QColor &key ) {
    return QPixmap::fromImage( ImageUtils::colourToAlpha( ImageUtils::autoCrop( pixmap.toImage(), true ), key ));
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
QByteArray PixmapUtils::toData( const QPixmap &pixmap ) {
    // abort on invalid pixmaps
    if ( pixmap.isNull())
        return QByteArray();

    // first get png data
    QByteArray data;
    QBuffer buffer( &data );
    buffer.open( QIODevice::WriteOnly );
    pixmap.save( &buffer, "PNG" );
    buffer.close();

    // abort on invalid data
    if ( data.isEmpty())
        return QByteArray();

    // return data
    return data;
}

/**
 * @brief PixmapUtils::readHeader
 * @param array
 */
bool PixmapUtils::readHeader( const QByteArray &array, PixmapInfo *info ) {
    // setup stream
    QDataStream stream( array );
    stream.setByteOrder( QDataStream::BigEndian );

    // validate magic
    quint64 magic;
    stream >> magic;
    if ( magic != 0x89504e470d0a1a0a )
        return false;

    // read IHDR length
    quint32 ihdrLength;
    stream >> ihdrLength;
    if ( ihdrLength != 13 )
        return false;

    // read IHDR magic
    quint32 ihdrMagic;
    stream >> ihdrMagic;
    if ( ihdrMagic != 0x49484452 )
        return false;

    // get width and height from IHDR
    quint32 width;
    quint32 height;
    stream >> width;
    stream >> height;
    stream.skipRawData( 5 + 4 );

    // look for IDAT chunk
    while ( !stream.atEnd()) {
        quint32 chunkLength;
        stream >> chunkLength;

        // skip chunks until we find IDAT
        quint32 chunkMagic;
        stream >> chunkMagic;
        if ( chunkMagic != 0x49444154 ) {
            stream.skipRawData( static_cast<int>( chunkLength ) + 4 );
            continue;
        }

        // skip until CRC and read it
        stream.skipRawData( static_cast<int>( chunkLength ));
        quint32 crc;
        stream >> crc;
        if ( info != nullptr ) {
            info->width = static_cast<int>( width );
            info->height = static_cast<int>( height );
            info->crc = crc;

        }

        return true;
    }

    return false;
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



