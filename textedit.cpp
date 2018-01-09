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
    bool found = false;

    // insert as plain text if required
    if ( this->pastePlainText()) {
        this->insertPlainText( source->text());
        return;
    }

    // check clipboard for image
    if ( source->hasImage()) {
        this->insertPixmap( QPixmap::fromImage( qvariant_cast<QImage>( source->imageData())));
        return;
    }

    // check droped items for images
    foreach ( QUrl url, source->urls()) {
        if ( db.mimeTypeForFile( url.toLocalFile(), QMimeDatabase::MatchContent ).iconName().startsWith( "image" )) {
            QPixmap pixmap;

            pixmap.load( url.toLocalFile());

            if ( !pixmap.isNull())
                this->insertPixmap( pixmap );

            found = true;
        }
    }

    // nothing valid found
    if ( !found )
        QTextEdit::insertFromMimeData( source );
}
