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
#include <QMouseEvent>
#include <QDebug>
#include "cropwidget.h"
#include "imagewidget.h"

/**
 * @brief CropWidget::CropWidget
 * @param parent
 */
CropWidget::CropWidget( QWidget *parent ) : QWidget( parent ) {
    this->boxLayout->setContentsMargins( 0, 0, 0, 0 );
    this->boxLayout->addWidget( this->topLeftSizeGrip, 0, Qt::AlignLeft | Qt::AlignTop );
    this->boxLayout->addWidget( this->bottomRightSizeGrip, 0, Qt::AlignRight | Qt::AlignBottom );
    this->setLayout( this->boxLayout );

    this->setWindowFlags( Qt::SubWindow );
    this->setMouseTracking( true );
    this->hide();
}

/**
 * @brief CropWidget::~CropWidget
 */
CropWidget::~CropWidget() {
    delete this->topLeftSizeGrip;
    delete this->bottomRightSizeGrip;
    delete this->boxLayout;
}

/**
 * @brief CropWidget::validateGeometry
 * @param geometry
 * @return
 */
bool CropWidget::validateGeometry( QRect &geometry ) {
    const ImageWidget *widget( qobject_cast<ImageWidget*>( this->parent()));
    if ( widget != nullptr ) {
        const QRect imageGeometry( widget->imageGeometry());

        if ( !imageGeometry.contains( geometry )) {
           geometry.setLeft( qMax( imageGeometry.left(), geometry.left()));
           geometry.setRight( qMin( imageGeometry.right(), geometry.right()));
           geometry.setTop( qMax( imageGeometry.top(), geometry.top()));
           geometry.setBottom( qMin( imageGeometry.bottom(), geometry.bottom()));

           return false;
        }
    }

    return true;
}

/**
 * @brief CropWidget::moveEvent
 * @param event
 */
void CropWidget::moveEvent( QMoveEvent *event ) {
    QWidget::moveEvent( event );

    QRect geometry( this->geometry());
    if ( !this->validateGeometry( geometry ))
        this->setGeometry( geometry );
}

/**
 * @brief CropWidget::mousePressEvent
 * @param event
 */
void CropWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent( event );
    this->m_drag = true;
    this->m_initial = event->pos();
}

/**
 * @brief CropWidget::mouseReleaseEvent
 * @param event
 */
void CropWidget::mouseReleaseEvent( QMouseEvent *event ) {
    QWidget::mouseReleaseEvent( event );
    this->m_drag = false;
}

/**
 * @brief CropWidget::mouseMoveEvent
 * @param event
 */
void CropWidget::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent( event );

    if ( this->isDragged()) {
        const QPoint delta( this->intialPosition() - event->pos());
        QRect geometry( this->geometry());
        geometry.moveTopLeft( this->pos() - delta );

        if ( !this->validateGeometry( geometry )) {
            this->m_initial = event->pos();
            return;
        }

        this->setGeometry( geometry );
    }
}
