/*
 * Copyright (C) 2017-2018 Factory #12
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

//
// includes
//
#include "imageutils.h"
#include "textedit.h"
#include <QBuffer>
#include <QMimeDatabase>
#include <QMimeData>
#include <QDropEvent>
#include <QDebug>
#ifdef Q_OS_WIN
#include <windows.h>
#include <QtWin>
#include <QWinMime>
#include <QRegularExpression>
#endif

/**
 * @brief TextEdit::insertPixmap
 * @param pixmap
 */
void TextEdit::insertPixmap( const QPixmap &pixmap ) {
    ImageUtils iu( this, pixmap );

    if ( iu.exec() == QDialog::Accepted ) {
        QByteArray bytes;
        QBuffer buffer( &bytes );

        // abort on invalid pixmap
        if ( iu.pixmap.isNull())
            return;

        // convert image to png internally
        buffer.open( QIODevice::WriteOnly );
        iu.pixmap.save( &buffer, "PNG" );

        // insert in textEdit
        this->textCursor().insertHtml( QString( "<img width=\"%1\" height=\"%2\" src=\"data:image/png;base64,%3\">" ).arg( iu.pixmap.width()).arg( iu.pixmap.height()).arg( bytes.toBase64().constData()));
    }
}

/**
 * @brief TextEdit::canInsertFromMimeData
 * @param source
 * @return
 */
bool TextEdit::canInsertFromMimeData( const QMimeData *source ) const {
    QMimeDatabase db;

    //
    // TODO: HERE!!!
    //

    // check if dropped item is an image
    foreach ( const QUrl url, source->urls()) {
        if ( db.mimeTypeForFile( url.toLocalFile(), QMimeDatabase::MatchExtension ).iconName().startsWith( "image" ))
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
            foreach ( QUrl url, source->urls()) {
                if ( !url.isLocalFile())
                    continue;

                image.load( url.toLocalFile());
                if ( !image.isNull())
                    break;
            }
        }

        if ( image.isNull())
            image = qvariant_cast<QImage>( source->imageData());

        this->insertPixmap( QPixmap::fromImage( image ));
        return;
    }

#ifdef Q_OS_WIN
    // open clipBoard to retrieve metaFiles (formulas from ChemDraw, Accelrys Draw, etc.)
    // QWinMime does not work for some reason, so we read metaFiles directly from win32 clipBoard
    if ( OpenClipboard( nullptr )) {
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
                qreal aspect;
                const int MaxPixmapWidth = 1024;

                qDebug() << "header" << header.nSize;

                // get metaFile dimensions
                width = static_cast<int>( qAbs( header.rclFrame.left - header.rclFrame.right ));
                height = static_cast<int>( qAbs( header.rclFrame.top - header.rclFrame.bottom ));
                aspect = static_cast<qreal>( width ) / static_cast<qreal>( height );
                width = qMin( MaxPixmapWidth, width );
                height = static_cast<int>( width / aspect );

                // construct rectangle
                memset( &rect, 0, sizeof( RECT ));
                rect.right = width;
                rect.bottom = height;

                // proceed with valid sizes
                if ( width > 0 && height > 0 ) {
                    QPixmap pixmap;

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
                    if ( !pixmap.isNull()) {
                        this->insertPixmap( pixmap );
                        return;
                    }

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
    }
#endif

    // check droped items for images
    foreach ( QUrl url, source->urls()) {
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
    QString html( input );

    // TODO: also remove references

    //
    // NOTE: this is highly inefficient, but I just can't get QRefExp
    //       to work properly with QString::remove
    QRegularExpression re( "((?:<\\/?(?:table|a|td|tr|tbody|div|span|li|ul|img).*?[>])|(?:<!--\\w+-->))" );
    QRegularExpressionMatchIterator i = re.globalMatch( html );
    QStringList words;

    // capture all unnecessary html tags
    while ( i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString word = match.captured( 1 );

        if ( !words.contains( word ))
            words << word;
    }

    // remove tags one by one
    foreach ( const QString word, words )
        html = html.remove( word );

    // replace newline with space
    html = html.replace( "\n", " " );

    // return stripped down HTML
    return html;
}
