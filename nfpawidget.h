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

#pragma once

/*
 * includes
 */
#include <QPainter>
#include <QRegularExpression>
#include <QWidget>
#include <QtMath>
#include <QDebug>
#include "propertywidget.h"

/**
 * @brief The NFPAWidget class
 */
class NFPAWidget final : public PropertyWidget {
public:
    static constexpr const qreal scale = 32;
    explicit NFPAWidget( QWidget *parent = nullptr, const QStringList &parms = QStringList()) : PropertyWidget( parent, parms ) {
        this->update( parms );
    }
    QSize size() const { return this->sizeHint(); }

public slots:
    void update( const QStringList &parms ) override {
        if ( this->parameters() == parms )
             return;

        this->m_parameters = parms;
        this->repaint();
    }

protected:
    void paintEvent( QPaintEvent * ) override {
        const qreal vScale = qSqrt( 2 * ( this->scale * this->scale ));

        // translate painter and rotate it by 45 degrees
        QPainter painter( this );
        painter.translate( vScale, this->height() * 0.5 );
        painter.rotate( 45 );

        // draw rects
        painter.setPen( QPen( Qt::black, 1 ));
        painter.setBrush( QColor::fromRgb( 255, 102, 102 ));
        painter.drawRect( QRectF( -this->scale, -this->scale, this->scale, this->scale ));
        painter.setBrush( Qt::white );
        painter.drawRect( QRectF( 0,      0,      this->scale, this->scale ));
        painter.setBrush( QColor::fromRgb( 102, 145, 255 ));
        painter.drawRect( QRectF( -this->scale, 0,      this->scale, this->scale ));
        painter.setBrush( QColor::fromRgb( 252, 255, 102 ));
        painter.drawRect( QRectF( 0,      -this->scale, this->scale, this->scale ));

        // draw outer grid
        painter.setBrush( Qt::transparent );
        painter.setPen( QPen( Qt::black, 1.2 ));
        painter.drawRect( QRectF( -this->scale, -this->scale, this->scale * 2, this->scale * 2 ));

        // reset transformations
        painter.resetTransform();
        painter.translate( vScale, this->height() * 0.5 );

        // draw numbers
        const QMap<int,int> scales { { 0, 0 }, { 1, this->scale * 0.5 }, { 2, this->scale * 0.42 }, { 3, this->scale * 0.30 }, { 4, this->scale * 0.22 } };
        const QList<QRectF> rects( QList<QRectF>()<<
                               QRectF( -vScale * 0.5, -vScale, vScale, vScale ) <<
                                   QRectF( -vScale, -vScale * 0.5, vScale, vScale ) <<
                                   QRectF( 0, -vScale * 0.5, vScale, vScale ) <<

                                   QRectF( -vScale * 0.5, 0, vScale, vScale )

                               );

        for ( int y = 0; y < qMin( this->parameters().count(), 4 ); y++ ) {
            painter.save();
            QFont font( painter.font());

            const QString parm( this->parameters().at( y ));
            if ( !parm.isEmpty()) {
                if ( !QString::compare( this->parameters().at( y ), "W" ))
                    font.setStrikeOut( true );

                font.setPointSize(( y == 3 ) ? scales[parm.length()] : static_cast<int>( this->scale * 0.5 ));
                painter.setFont( font );
            }

            painter.drawText( rects.at( y ), parm, { Qt::AlignCenter });
            painter.restore();
        }
    }

    /**
     * @brief sizeHint
     * @return
     */
    QSize sizeHint() const override {
        const qreal vScale = sqrt( 2 * ( this->scale * this->scale ));
        return QSizeF( vScale * 2, vScale * 2 ).toSize();
    }
};
