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
#include "imagewidget.h"
#include "pixmaputils.h"
#include "networkmanager.h"
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QRandomGenerator>
#endif
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>
#include <QPaintEvent>
#include <QMimeData>
#include <QRegularExpression>

/**
 * @brief ImageWidget::ImageWidget
 * @param parent
 */
ImageWidget::ImageWidget( QWidget *parent ) : QWidget( parent ) {
    this->setAcceptDrops( true );
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    this->m_id = QRandomGenerator::global()->generate();
#else
    srand( time( nullptr ));
    this->m_id = static_cast<quint32>( std::rand());
#endif

     // setup finished connection to the NetworkManager
     NetworkManager::connect( NetworkManager::instance(), &NetworkManager::finished, this, [ this ]( const QString &url, NetworkManager::Types type, const QVariant &userData, const QByteArray &data ) {
         switch ( type ) {
         // FIXME: dup code
         case NetworkManager::DropImageRequest:
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
             if ( pixmap.loadFromData( data )) {
                 this->imageUtilsParent()->paste( pixmap.toImage());
                 return;
             }

             return;
         }

         default:
             ;
         }
     } );
}

/**
 * @brief ImageWidget::~ImageWidget
 */
ImageWidget::~ImageWidget() {
    NetworkManager::disconnect( NetworkManager::instance(), &NetworkManager::finished, this, nullptr );
}

/**
 * @brief ImageWidget::imageGeometry
 * @return
 */
QRect ImageWidget::imageGeometry() const {
    const QSizeF size( QSizeF( this->image().size()) * this->zoomScale());
    return QRectF(
                this->geometry().center().x() - size.width() / 2,
                this->geometry().center().y() - size.height() / 2,
                size.width(),
                size.height()
                ).toRect();
}

/**
 * @brief ImageWidget::paintEvent
 */
void ImageWidget::paintEvent( QPaintEvent *event ) {
    if ( this->imageUtilsParent() == nullptr )
        return;

    QPainter painter( this );
    painter.setRenderHint( QPainter::SmoothPixmapTransform, true );

    const QRect imageGeometry( this->imageGeometry());

    painter.save();
    painter.setPen( QApplication::palette().color( QPalette::Text ));
    if ( !this->image().isNull()) {
        painter.drawImage( imageGeometry, this->image());
        painter.drawRect( imageGeometry.adjusted( -1, -1, 0, 0 ));
    } else {
        painter.drawText( event->rect(), ImageWidget::tr( "No image has been set.\nOpen from file or paste from clipboard.\n\nClick anywhere on this window to open an image." ), QTextOption(  Qt::AlignCenter ));
        painter.restore();
        return;
    }

    if ( this->imageUtilsParent()->cropWidget()->isVisible()) {
        painter.save();

        const QRegion region( QRegion( imageGeometry ) - this->imageUtilsParent()->cropWidget()->geometry());
        painter.setClipRegion( region );

        painter.fillRect( imageGeometry, QColor::fromRgb( 0, 0, 0, 64 ));
        painter.restore();
    } else {
        painter.setClipping( false );
    }

    const QFont font( this->font().family(), this->font().pointSize(), QFont::Bold );
    const QFontMetrics fm( font );
    const QString text( QString( "Zoom: %1%" ).arg( static_cast<int>( this->zoomScale() * 100 )));
    const int margin = 8;

    painter.setFont( font );
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    painter.drawText( this->width() - fm.horizontalAdvance( text ) - margin, margin + fm.height(), text );
#else
    painter.drawText( this->width() - fm.width( text ) - margin, margin + fm.height(), text );
#endif
    painter.restore();
}

/**
 * @brief ImageWidget::dropEvent
 * @param event
 */
void ImageWidget::dropEvent( QDropEvent *event ) {
    for ( const QUrl &url : event->mimeData()->urls()) {
        QPixmap pixmap( PixmapUtils::fromUrl( url ));
        if ( !pixmap.isNull()) {
            this->imageUtilsParent()->paste( pixmap.toImage());
            return;
        }
    }

    if ( event->mimeData()->hasHtml()) {
        // handle img urls
        QRegularExpression re( "<img[^>]*src=\"([^\"]+?(?=\"))\"" );
        QRegularExpressionMatch match( re.match( event->mimeData()->html()));
        if ( match.hasMatch()) {
            const QString imgSource( match.captured( 1 ));
            NetworkManager::instance()->execute( imgSource, NetworkManager::DropImageRequest, this->id());
            return;
        }
        return;
    }
}

/**
 * @brief ImageWidget::dragEnterEvent
 * @param event
 */
void ImageWidget::dragEnterEvent( QDragEnterEvent *event ) {
    if ( !event->mimeData()->urls().isEmpty())
        event->acceptProposedAction();
}
