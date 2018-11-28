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
#include "charactermap.h"
#include <QPainter>
#include <QtMath>
#include <QApplication>

/**
 * @brief CharacterMap::CharacterMap
 * @param parent
 */
CharacterMap::CharacterMap( QWidget *parent ) : QWidget( parent ), blockId( QPair<int,int>( -1, -1 )) {
    // this->size
    this->setWindowFlags( this->windowFlags() | Qt::Tool );
    this->setWindowTitle( this->tr( "Character selector" ));

    // get character list
    this->characters = CharacterNamespace::SpecialChars.split( ";" );
}

/**
 * @brief CharacterMap::paintEvent
 * @param event
 */
void CharacterMap::paintEvent( QPaintEvent *event ) {
    if ( !this->characters.isEmpty()) {
        int y;
        QPainter painter( this );
        QFont font( this->font());
        font.setPixelSize( static_cast<int>( CharacterNamespace::GridSize * 0.8 ));

        // get minimum block size to display all characters
        const int blocks = static_cast<int>( qCeil( qSqrt( this->characters.count())));
        this->setFixedSize( CharacterNamespace::GridSize * blocks + 1, CharacterNamespace::GridSize * blocks + 1 );

        // paint gray lines
        painter.setPen( qApp->palette().color( QPalette::Mid ));
        for ( y = 0; y <= blocks; y++ ) {
            painter.drawLine( 0, y * CharacterNamespace::GridSize, CharacterNamespace::GridSize * blocks, y * CharacterNamespace::GridSize );
            painter.drawLine( y * CharacterNamespace::GridSize, 0, y * CharacterNamespace::GridSize, CharacterNamespace::GridSize * blocks );
        }

        // draw characters
        painter.setFont( font );
        painter.setPen( qApp->palette().color( QPalette::Text ));
        for ( y = 0; y < this->characters.count(); y++ ) {
            // get block coordinates
            const int hBlock = y % blocks;
            const int vBlock = qFloor( y / blocks );

            // draw hilight rect
            const QRect rect( hBlock * CharacterNamespace::GridSize, vBlock * CharacterNamespace::GridSize, CharacterNamespace::GridSize, CharacterNamespace::GridSize );
            if ( this->blockId == QPair<int,int>( hBlock, vBlock ))
                painter.fillRect( rect, QColor::fromRgb( 255, 0, 0, 128 ));

            // draw character
            painter.drawText( rect, Qt::AlignCenter, QString::fromUtf8( this->characters.at( y ).toUtf8().data()));
        }
    }

    QWidget::paintEvent( event );
}

/**
 * @brief CharacterMap::mouseReleaseEvent
 * @param event
 */
void CharacterMap::mouseReleaseEvent( QMouseEvent *event ) {
    int y;

    // reset block id
    this->blockId = QPair<int,int>( -1, -1 );

    // insert char on mouse press
    if ( event->button() == Qt::LeftButton ) {
        // get block size
        const int blocks = static_cast<int>( qCeil( qSqrt( this->characters.count())));

        // find block under mouse cursor
        for ( y = 0; y < this->characters.count(); y++ ) {
            // get block coordinates
            const int hBlock = y % blocks;
            const int vBlock = qFloor( y / blocks );

            if ( QRect( hBlock * CharacterNamespace::GridSize, vBlock * CharacterNamespace::GridSize, CharacterNamespace::GridSize, CharacterNamespace::GridSize ).contains( event->pos())) {

                // set blockId, repaint the widget and insert character in the parent widget
                this->blockId = QPair<int,int>( hBlock, vBlock );
                this->repaint();
                emit this->characterSelected( QString::fromUtf8( this->characters.at( y ).toUtf8().data()));
            }
        }
    }

    QWidget::mouseReleaseEvent( event );
}
