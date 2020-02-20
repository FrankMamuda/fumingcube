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
#include "nfpawidget.h"
#include <QPainter>

/**
 * @brief NFPAWidget::NFPAWidget
 * @param parent
 * @param parms
 */
NFPAWidget::NFPAWidget( QWidget *parent, const QStringList &parms ) : PropertyViewWidget( parent, parms ) {
    this->setScale( 24 );
    this->update( parms );
}

/**
 * @brief NFPAWidget::setScale
 * @param scale
 */
void NFPAWidget::setScale( const int &scale ) {
    this->m_scale = scale;
    this->m_vscale = qSqrt( 2 * ( this->scale() * this->scale()));
    this->rects = QList<QRectF>() <<
                                  QRectF( -this->vscale(), -this->vscale() * 0.5, this->vscale(), this->vscale()) <<
                                  QRectF( -this->vscale() * 0.5, -this->vscale(), this->vscale(), this->vscale()) <<
                                  QRectF( 0, -this->vscale() * 0.5, this->vscale(), this->vscale()) <<
                                  QRectF( -this->vscale() * 0.5, 0, this->vscale(), this->vscale());
}

/**
 * @brief NFPAWidget::paintEvent
 */
void NFPAWidget::paintEvent( QPaintEvent * ) {
    if ( this->rects.isEmpty())
        return;

    // translate painter and rotate it by 45 degrees
    QPainter painter( this );
    painter.translate( this->vscale(), this->height() * 0.5 );
    painter.rotate( 45 );

    // draw rects
    painter.setPen( QPen( Qt::black, 1 ));
    painter.setBrush( QColor::fromRgb( 255, 102, 102 ));
    painter.drawRect( QRectF( -this->scale(), -this->scale(), this->scale(), this->scale()));
    painter.setBrush( Qt::white );
    painter.drawRect( QRectF( 0, 0, this->scale(), this->scale()));
    painter.setBrush( QColor::fromRgb( 102, 145, 255 ));
    painter.drawRect( QRectF( -this->scale(), 0, this->scale(), this->scale()));
    painter.setBrush( QColor::fromRgb( 252, 255, 102 ));
    painter.drawRect( QRectF( 0, -this->scale(), this->scale(), this->scale()));

    // draw outer grid
    painter.setBrush( Qt::transparent );
    painter.setPen( QPen( Qt::black, 1.2 ));
    painter.drawRect( QRectF( -this->scale(), -this->scale(), this->scale() * 2, this->scale() * 2 ));

    // reset transformations
    painter.resetTransform();
    painter.translate( this->vscale(), this->height() * 0.5 );

    // draw numbers
    for ( int y = 0; y < qMin( this->parameters().count(), 4 ); y++ ) {
        painter.save();
        QFont font( painter.font());

        const QString parm( this->parameters().at( y ));
        if ( !parm.isEmpty()) {
            if ( !QString::compare( this->parameters().at( y ), "W" ))
                font.setStrikeOut( true );

            font.setPointSize(
                    ( y == 3 ) ? static_cast<int>( this->scales[parm.length()] ) : static_cast<int>( this->scale() *
                                                                                                     0.5 ));
            painter.setFont( font );
        }

        painter.drawText( this->rects.at( y ), parm, { Qt::AlignCenter } );
        painter.restore();
    }
}
