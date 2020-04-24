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
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>
#include <QPaintEvent>

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

    if ( !this->image().isNull()) {
        painter.drawImage( imageGeometry, this->image());
        painter.save();
        painter.setPen( QApplication::palette().color( QPalette::Text ));
        painter.drawRect( imageGeometry.adjusted( -1, -1, 0, 0 ));
        painter.restore();
    } else {
        painter.drawText( event->rect(), ImageWidget::tr( "No image has been set.\nOpen from file or paste from clipboard.\n\nClick anywhere on this window to open an image." ), QTextOption(  Qt::AlignCenter ));
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
    painter.drawText( this->width() - fm.width( text ) - margin, margin + fm.height(), text );
}
