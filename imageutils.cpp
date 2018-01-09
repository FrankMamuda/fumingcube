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
#include "ui_imageutils.h"
#include <QDebug>

/**
 * @brief ImageUtils::ImageUtils
 * @param parent
 * @param px
 */
ImageUtils::ImageUtils( QWidget *parent, const QPixmap &pixmap ) : QDialog( parent ), ui( new Ui::ImageUtils ) {
    QPixmap scaledPixmap( pixmap );

    // limit width
    if ( scaledPixmap.width() > Ui::MaxImageSize )
        scaledPixmap = scaledPixmap.scaledToWidth( Ui::MaxImageSize, Qt::SmoothTransformation );

    // limit height
    if ( scaledPixmap.height() > Ui::MaxImageSize )
        scaledPixmap = scaledPixmap.scaledToHeight( Ui::MaxImageSize, Qt::SmoothTransformation );

    // set up ui
    this->ui->setupUi( this );

    if ( scaledPixmap.isNull()) {
        qDebug() << "bad pixmap";
        return;
    }

    // set pixmap to label
    this->ui->pixmapLabel->setPixmap( scaledPixmap );
    this->ui->pixmapLabel->setFixedSize( scaledPixmap.width(), scaledPixmap.height());
    this->layout()->setSizeConstraint( QLayout::SetFixedSize );

    // setup scaling slider as lambda
    auto scalePixmap = [ this, scaledPixmap ]( int value ) {
        qreal scale = static_cast<qreal>( value ) / 100.0;
        qreal aspect = static_cast<qreal>( scaledPixmap.width()) / static_cast<qreal>( scaledPixmap.height());
        qreal width = scaledPixmap.width() * scale;
        qreal height = scaledPixmap.height() * scale;

        // limit width
        if ( width > Ui::MaxImageSize && width > height ) {
            width = Ui::MaxImageSize;
            height = width / aspect;
            this->ui->sizeSlider->setMaximum( 100.0 );
        }

        // limit height
        if ( height > Ui::MaxImageSize && height > width ) {
            height = Ui::MaxImageSize;
            width = height * aspect;
            this->ui->sizeSlider->setMaximum( 100.0 );
        }

        // set fixed label size
        this->size = QSize( static_cast<int>( width ), static_cast<int>( height ));
        scale = width / scaledPixmap.width();
        this->ui->percentLabel->setText( QString( "%1%" ).arg( static_cast<int>( scale * 100.0 )));
        this->ui->pixmapLabel->setFixedSize( this->size );
        this->ui->sizeSlider->setSliderPosition( static_cast<int>( scale * 100.0 ));
    };

    // set maximum scaled size
    scalePixmap( 100.0 );
    this->ui->sizeSlider->repaint();
    this->connect( this->ui->sizeSlider, &QSlider::valueChanged, scalePixmap );

    // connect ok button
    this->connect( this->ui->buttonBox, &QDialogButtonBox::accepted, [ this, scaledPixmap ]() {
        this->pixmap = scaledPixmap.scaled( this->size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
    } );

    // limit frame size
    this->ui->frame->setFixedSize( this->ui->pixmapLabel->size().width() + 8, this->ui->pixmapLabel->size().height() + 8 );
}

/**
 * @brief ImageUtils::~ImageUtils
 */
ImageUtils::~ImageUtils() {
    delete this->ui;
}
