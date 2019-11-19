/*
 * Copyright (C) 2017-2018 Factory #12
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

/*
 * includes
 */
#include "imageutils.h"
#include "textedit.h"
#include <QBuffer>
#include <QMimeDatabase>
#include <QMimeData>
#include <QDropEvent>
#include <QDebug>
#include <QRegularExpression>
#ifdef Q_OS_WIN
#include <windows.h>
#include <QtWin>
#include <QWinMime>
#endif

/**
 * @brief TextEdit::insertPixmap
 * @param pixmap
 * @param preferredWidth
 */
void TextEdit::insertPixmap( const QPixmap &pixmap, const int preferredWidth ) {
    bool accepted = true;
    QPixmap out( pixmap );

    if ( preferredWidth > 0 ) {
        ImageUtils iu( this, pixmap, preferredWidth );
        accepted = iu.exec();

        if ( accepted )
            out = iu.pixmap;
    }

    if ( accepted ) {
        QByteArray bytes;
        QBuffer buffer( &bytes );

        // abort on invalid pixmap
        if ( out.isNull())
            return;

        // convert image to png internally
        buffer.open( QIODevice::WriteOnly );
        out.save( &buffer, "PNG" );

        // insert in textEdit
        this->textCursor().insertHtml( QString( "<img width=\"%1\" height=\"%2\" src=\"data:image/png;base64,%3\">" ).arg( out.width()).arg( out.height()).arg( bytes.toBase64().constData()));
    }
}

/**
 * @brief TextEdit::canInsertFromMimeData
 * @param source
 * @return
 */
bool TextEdit::canInsertFromMimeData( const QMimeData *source ) const {
    if ( this->isSimpleEditor())
        return false;

    // check if dropped item is an image
    foreach ( const QUrl &url, source->urls()) {
        if ( QMimeDatabase().mimeTypeForFile( url.toLocalFile(), QMimeDatabase::MatchExtension ).iconName().startsWith( "image" ))
            return true;
    }

    // check if clipboard contains an image
    return source->hasImage() ? true : QTextEdit::canInsertFromMimeData( source );
}

/**
 * @brief TextEdit::dropEvent
 * @param event
 */
void TextEdit::dropEvent( QDropEvent *event ) {
    if ( this->isSimpleEditor())
        return;

    // move cursot to drop position
    this->setTextCursor( this->cursorForPosition( event->pos()));

    // insert image
    this->insertFromMimeData( event->mimeData());

    // HACK: fixes the disappearing cursor bug
    this->setReadOnly( true );
    QTextEdit::dropEvent( event );
    this->setReadOnly( false );
}

/**
 * @brief TextEdit::insertFromMimeData
 * @param source
 */
void TextEdit::insertFromMimeData( const QMimeData *source ) {
    if ( this->isSimpleEditor())
        return;

    QMimeDatabase db;

    // insert as plain text if required
    /*if ( this->pastePlainText()) {
        this->insertPlainText( source->text());
        return;
    }*/

    // check clipboard for image
    if ( source->hasImage()) {
        QImage image;

        // NOTE: application/x-qt-image has problems with transparency
        //       therefore prioritize loading image from an actual file
        if ( !source->urls().isEmpty()) {
            foreach ( const QUrl &url, source->urls()) {
                if ( !url.isLocalFile())
                    continue;

                image.load( url.toLocalFile());
                if ( !image.isNull())
                    break;
            }
        }

        if ( image.isNull())
            image = qvariant_cast<QImage>( source->imageData());

        this->insertPixmap( QPixmap::fromImage( qAsConst( image )));
        return;
    }

#ifdef Q_OS_WIN
    // open clipBoard to retrieve metaFiles (formulas from ChemDraw, Accelrys Draw, etc.)
    // QWinMime does not work for some reason, so we read metaFiles directly from win32 clipBoard
    if ( OpenClipboard( nullptr )) {
        QPixmap pixmap;
        int endWidth = 0;

        // check clipBoard for metaFiles
        if ( IsClipboardFormatAvailable( CF_ENHMETAFILE )) {
            HENHMETAFILE metaFile;
            ENHMETAHEADER header;
            HGLOBAL global;

            // get metaFile from clipBoard
            metaFile = reinterpret_cast<HENHMETAFILE>( GetClipboardData( CF_ENHMETAFILE ));
            memset( &header, 0, sizeof( ENHMETAHEADER ));

            // lockMemory
            global = GlobalAlloc( GMEM_MOVEABLE, sizeof( CF_ENHMETAFILE ));
            if ( global == nullptr ) {
                CloseClipboard();
                return;
            }

            // get metaFile header
            if ( GetEnhMetaFileHeader( metaFile, sizeof( ENHMETAHEADER ), &header ) != 0 ) {
                HDC deviceContext, memDC;
                RECT rect;
                HBITMAP bitmap;
                HBRUSH brush;
                int width, height;
                const qreal scaleFactor = 0.125;

                // get metaFile dimensions
                width = static_cast<int>( qAbs( header.rclFrame.left - header.rclFrame.right ) * scaleFactor );
                height = static_cast<int>( qAbs( header.rclFrame.top - header.rclFrame.bottom ) * scaleFactor );
                endWidth = static_cast<int>( width * static_cast<qreal>( header.szlMillimeters.cx ) / static_cast<qreal>( header.szlDevice.cx ));

                // construct rectangle
                memset( &rect, 0, sizeof( RECT ));
                rect.right = width;
                rect.bottom = height;

                // proceed with valid sizes
                if ( width > 0 && height > 0 ) {
                    // get device context
                    deviceContext = GetDC( nullptr );
                    memDC = CreateCompatibleDC( deviceContext );

                    // create bitmap
                    bitmap = CreateCompatibleBitmap( memDC, width, height );
                    SelectObject( memDC, bitmap );

                    // fill white background
                    brush = CreateSolidBrush( static_cast<COLORREF>( 0x00FFFFFF ));
                    FillRect( memDC, &rect, brush );
                    DeleteObject( brush );

                    // render metaFile to bitmap
                    PlayEnhMetaFile( memDC, metaFile, &rect );
                    BitBlt( deviceContext, 0, 0, width, 0, memDC, 0, 0, static_cast<DWORD>( 0x00CC0020 ));

                    // convert to pixmap
                    pixmap = QtWin::fromHBITMAP( bitmap );

                    // clean up
                    DeleteObject( bitmap );
                    DeleteEnhMetaFile( metaFile );
                    DeleteDC( memDC );
                    ReleaseDC( nullptr, deviceContext );
                }
            }

            // unlock memory
            GlobalUnlock( global );
        }

        // close clipBoard
        CloseClipboard();

        if ( !pixmap.isNull()) {
            this->insertPixmap( pixmap, endWidth );
            return;
        }
    }
#endif

    // check droped items for images
    foreach ( const QUrl &url, source->urls()) {
        if ( db.mimeTypeForFile( url.toLocalFile(), QMimeDatabase::MatchContent ).iconName().startsWith( "image" )) {
            QPixmap pixmap;

            pixmap.load( url.toLocalFile());

            if ( !pixmap.isNull()) {
                this->insertPixmap( pixmap );
                return;
            }
        }
    }

    if ( source->hasHtml()) {
        QString html( source->html());

        if ( this->cleanHTML())
            html = TextEdit::stripHTML( html );

        // insert clean html
        this->insertHtml( html );
        return;
    }

    // nothing valid found
    QTextEdit::insertFromMimeData( source );
}

/**
 * @brief TextEdit::stripHTML
 * @return
 */
QString TextEdit::stripHTML( const QString &input ) {
    const QRegularExpression re( "((?:<\\/?(?:table|a|td|tr|tbody|div|span|li|ul|img).*?[>])|(?:<!--\\w+-->))" );
    return QString( input ).replace( re, "" );
}
