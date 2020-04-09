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

/*
 * includes
 */
#include "imageutils.h"
#include "textedit.h"
#include <QBuffer>
#include <QMimeDatabase>
#include <QMimeData>
#include <QDropEvent>
#include <QRegularExpression>
#ifdef Q_OS_WIN
#include "emfmime.h"
#endif
#include "htmlutils.h"
#include "property.h"
#include "propertydock.h"
#include <QAbstractItemView>
#include "pixmaputils.h"
#include "networkmanager.h"

/**
 * @brief TextEdit::TextEdit
 * @param parent
 */
TextEdit::TextEdit( QWidget *parent ) : QTextEdit( parent ) {
   this->setTabChangesFocus( true );
   this->installEventFilter( this );
   this->m_id = QRandomGenerator::global()->generate();

    // setup finished connection to the NetworkManager
    NetworkManager::connect( NetworkManager::instance(), &NetworkManager::finished, this, [ this ]( const QString &url, NetworkManager::Types type, const QVariant &userData, const QByteArray &data ) {
        qDebug() << this->id();

        switch ( type ) {
        case NetworkManager::TextEditImage:
        {
            const quint32 idn = userData.value<quint32>();
            if ( idn != this->id())
                return;

            // TODO: svg not implemented yet
            if ( url.endsWith( ".svg" )) {
                qDebug() << "network->image; svg not implemented yet" << idn << this->id();
                return;
            }

            qDebug() << "network->image" << idn << this->id();
            QPixmap pixmap;
            pixmap.loadFromData( data );
            if ( pixmap.isNull())
                return;

            const int sectionSize = PropertyDock::instance()->sectionSize( Property::PropertyData );
            this->insertPixmap( pixmap, qMin( sectionSize, pixmap.width()));
            return;
        }

        default:
            ;
        }
    } );
}

/**
 * @brief TextEdit::~TextEdit
 */
TextEdit::~TextEdit() {
    NetworkManager::disconnect( NetworkManager::instance(), &NetworkManager::finished, this, nullptr );
}

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
        // abort on invalid pixmap
        if ( out.isNull())
            return;

        // insert in textEdit
        this->textCursor().insertHtml( QString( R"(<img width="%1" height="%2" src="data:image/png;base64,%3">)" )
                                       .arg( out.width()).arg( out.height()).arg( PixmapUtils::convertToData( out ).toBase64().constData()));
    }
}

/**
 * @brief TextEdit::canInsertFromMimeData
 * @param source
 * @return
 */
bool TextEdit::canInsertFromMimeData( const QMimeData *source ) const {
    if ( this->isSimpleEditor() && source->hasImage())
        return false;

    // check if dropped item is an image
    for ( const QUrl &url : source->urls()) {
        if ( QMimeDatabase().mimeTypeForFile( url.toLocalFile(), QMimeDatabase::MatchExtension ).iconName().startsWith( "image" ) && !this->isSimpleEditor())
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

    // move cursor to drop position
    this->setTextCursor( this->cursorForPosition( event->pos()));

    // insert image
    this->insertFromMimeData( event->mimeData());

    // HACK: fixes the disappearing cursor bug
    this->setReadOnly( true );
    QTextEdit::dropEvent( event );
    this->setReadOnly( false );
}

/**
 * @brief TextEdit::keyPressEvent
 * @param e
 */
void TextEdit::keyPressEvent( QKeyEvent *event ) {
    if ( this->completer() != nullptr ) {
        if ( this->completer()->popup()->isVisible()) {
            switch ( event->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                event->ignore();
                return;
            default:
                break;
            }
        }

        this->completer()->setCompletionPrefix( this->toPlainText());
        this->completer()->complete();
    }

    QTextEdit::keyPressEvent( event );
}

/**
 * @brief TextEdit::eventFilter
 * @param object
 * @param event
 * @return
 */
bool TextEdit::eventFilter(QObject *object, QEvent *event) {
    if ( this->isSimpleEditor()) {
        if ( event->type() == QEvent::KeyPress ) {
            const QKeyEvent *keyEvent( dynamic_cast<QKeyEvent *>( event ));

            if ( keyEvent->key() == Qt::Key_Return )
                return true;
        }
    }

    if ( event->type() == QEvent::FocusIn ) {
        QTextCursor cursor( this->textCursor());
        cursor.movePosition( QTextCursor::End );
        this->setTextCursor( cursor );
    }


    return QTextEdit::eventFilter( object, event );
}

/**
 * @brief TextEdit::insertFromMimeData
 * @param source
 */
void TextEdit::insertFromMimeData( const QMimeData *source ) {
    if ( this->isSimpleEditor() && source->hasImage())
        return;

    if (( source->hasImage() && source->hasHtml())
#ifdef Q_OS_WIN
            || ( source->formats().contains( R"(application/x-qt-windows-mime;value="DragImageBits")" ) && source->hasHtml())
#endif
            ) {
        {
            // handle base64 images
            const QRegularExpression re( "<img[^>]*src=\"data:image\\/\\w+;base64([^\"]+?(?=\"))\"" );
            QRegularExpressionMatch match( re.match( source->html()));
            if ( match.hasMatch()) {
                const QString imgSource( match.captured( 1 ));
                QPixmap pixmap;
                pixmap.loadFromData( QByteArray::fromBase64( imgSource.toLatin1().constData()));
                if ( !pixmap.isNull()) {
                    this->insertPixmap( qAsConst( pixmap ), pixmap.width());
                    return;
                }
            }
        }

        {
            // handle img urls
            QRegularExpression re( "<img[^>]*src=\"([^\"]+?(?=\"))\"" );
            QRegularExpressionMatch match( re.match( source->html()));
            if ( match.hasMatch()) {
                const QString imgSource( match.captured( 1 ));
                NetworkManager::instance()->execute( imgSource, NetworkManager::TextEditImage, this->id());
                return;
            }
        }
    }

    // check clipboard for image
    if ( source->hasImage()) {
        qDebug() << source->formats();

        const QImage image( qvariant_cast<QImage>( source->imageData()));
        if ( !image.isNull()) {
            this->insertPixmap( QPixmap::fromImage( qAsConst( image )), image.width());
            return;
        }
    }

    // check dropped items for images
    for ( const QUrl &url : source->urls()) {
        // TODO: resolve links from the internet

        if ( !url.isLocalFile())
            continue;

        if ( QMimeDatabase().mimeTypeForFile( url.toLocalFile(), QMimeDatabase::MatchContent ).iconName().startsWith( "image" ) && !this->isSimpleEditor()) {
            QPixmap pixmap;

            pixmap.load( url.toLocalFile());

            if ( !pixmap.isNull()) {
                const int sectionSize = PropertyDock::instance()->sectionSize( Property::PropertyData );
                this->insertPixmap( pixmap, qMin( sectionSize, pixmap.width()));
                return;
            }
        }
    }

    if ( source->hasHtml()) {
        QString html( source->html());

        if ( this->cleanHTML())
            html = HTMLUtils::simplify( html );

        // TODO: need special handling of subscript, superscript and italic
        if ( this->isSimpleEditor())
            html = HTMLUtils::convertToPlainText( html ).simplified();

        // insert clean html
        this->insertHtml( html );
        return;
    }

    // nothing valid found
    QTextEdit::insertFromMimeData( source );
}

/**
 * @brief TextEdit::setCompleter
 * @param completer
 */
void TextEdit::setCompleter( QCompleter *completer ) {
    if ( this->completer() != nullptr )
        this->completer()->disconnect(this);

    this->m_completer = completer;
    if ( completer == nullptr )
        return;

    this->completer()->setWidget(this);
    this->completer()->setCompletionMode( QCompleter::PopupCompletion );
    this->completer()->setCaseSensitivity( Qt::CaseInsensitive );
    QCompleter::connect( this->completer(), QOverload<const QString &>::of( &QCompleter::activated ), [ this ]( const QString &completion ) { this->setHtml( completion ); } );
}

