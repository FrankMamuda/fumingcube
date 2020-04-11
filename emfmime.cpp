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
#include "emfmime.h"
#ifdef Q_OS_WIN
#include <QPixmap>
#include <QtWin>
#include <QBitmap>

/**
 * @brief EMFMime::canConvertToMime
 * @param mime
 * @param dataObject
 * @return
 */
bool EMFMime::canConvertToMime( const QString &mime, IDataObject *dataObject ) const {
    if ( !QString::compare( mime, "application/x-qt-image" )) {
        //  NOTE: prioritize CF_DIB
        FORMATETC formatetcEmf { CF_ENHMETAFILE, nullptr, DVASPECT_CONTENT, -1, TYMED_ENHMF };
        FORMATETC formatetcBmp { CF_DIB, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

        const bool canGetEmf = dataObject->QueryGetData( &formatetcEmf ) == static_cast<HRESULT>( 0x00000000 );
        const bool canGetBmp = dataObject->QueryGetData( &formatetcBmp ) == static_cast<HRESULT>( 0x00000000 );

        return canGetEmf && !canGetBmp;
    }

    return false;
}

/**
 * @brief EMFMime::convertToMime
 * @param mime
 * @param dataObject
 * @return
 */
QVariant EMFMime::convertToMime( const QString &mime, IDataObject *dataObject, QVariant::Type ) const {
    QVariant result;

    if ( this->canConvertToMime( mime, dataObject )) {
        FORMATETC formatetc { CF_ENHMETAFILE, nullptr, DVASPECT_CONTENT, -1, TYMED_ENHMF };
        STGMEDIUM s;

        if ( dataObject->GetData( &formatetc, &s ) != static_cast<HRESULT>( 0x00000000 ))
            return QVariant();

        if ( s.tymed != TYMED_ENHMF ) {
            ReleaseStgMedium( &s );
            return QVariant();
        }

        if ( static_cast<int>( GetEnhMetaFileBits( s.hEnhMetaFile, 0, nullptr ) > 0 )) {
            ENHMETAHEADER header;
            memset( &header, 0, sizeof( ENHMETAHEADER ));

            if ( GetEnhMetaFileHeader( s.hEnhMetaFile, sizeof( ENHMETAHEADER ), &header ) != 0 ) {
                constexpr const qreal scaleFactor = 1;
                const RECT rect { 0, 0, static_cast<int>( qAbs( header.rclFrame.left - header.rclFrame.right ) * scaleFactor ), static_cast<int>( qAbs( header.rclFrame.top - header.rclFrame.bottom ) * scaleFactor ) };

                // proceed with valid sizes
                if ( rect.right > 0 && rect.bottom > 0 ) {
                    // get device context
                    const HDC deviceContext = GetDC( nullptr );
                    const HDC memDC = CreateCompatibleDC( deviceContext );

                    // create and select bitmap
                    const BITMAPINFO bi { { sizeof( BITMAPINFOHEADER ), rect.right, rect.bottom, 1, 24, 0, 0, 0, 0, 0, 0 }, { { 0, 0, 0, 0 } } };
                    const HBITMAP bitmap = CreateDIBSection( deviceContext, &bi, DIB_RGB_COLORS, 0,0,0 );
                    SelectObject( memDC, bitmap );

                    // fill magenta background
                    const HBRUSH brush = CreateSolidBrush( RGB( 255, 0, 255 ));
                    FillRect( memDC, &rect, brush );
                    DeleteObject( brush );

                    // render metaFile to bitmap
                    PlayEnhMetaFile( memDC, s.hEnhMetaFile, &rect );

                    // apply mask and convert to image
                    QPixmap pixmap( QtWin::fromHBITMAP( bitmap ));
                    pixmap.setMask( pixmap.createMaskFromColor( { Qt::magenta } ));
                    const QImage image( pixmap.toImage());

                    // clean up
                    DeleteObject( bitmap );
                    DeleteEnhMetaFile( s.hEnhMetaFile );
                    DeleteDC( memDC );
                    ReleaseDC( nullptr, deviceContext );
                    ReleaseStgMedium( &s );

                    // return image
                    return image;
                }
            }
        }

        ReleaseStgMedium( &s );
    }

    return QVariant();
}

#endif
