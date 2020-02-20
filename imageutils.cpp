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
#include "ui_imageutils.h"
#include <QDebug>

/**
 * @brief ImageUtils::ImageUtils
 * @param parent
 * @param px
 */
ImageUtils::ImageUtils( QWidget *parent, const QPixmap &pixmap, const int &preferredWidth, bool view ) : QDialog(
        parent ), ui( new Ui::ImageUtils ) {
    QPixmap scaledPixmap( pixmap );

    if ( preferredWidth > 0 )
        scaledPixmap = scaledPixmap.scaledToWidth( preferredWidth, Qt::SmoothTransformation );

    // limit width
    if ( scaledPixmap.width() > Ui::MaxImageSize )
        scaledPixmap = scaledPixmap.scaledToWidth( Ui::MaxImageSize, Qt::SmoothTransformation );

    // limit height
    if ( scaledPixmap.height() > Ui::MaxImageSize )
        scaledPixmap = scaledPixmap.scaledToHeight( Ui::MaxImageSize, Qt::SmoothTransformation );

    // set up ui
    this->ui->setupUi( this );

    if ( scaledPixmap.isNull())
        return;

    // set pixmap to label
    this->ui->pixmapLabel->setPixmap( scaledPixmap );
    this->ui->pixmapLabel->setFixedSize( scaledPixmap.width(), scaledPixmap.height());
    this->layout()->setSizeConstraint( QLayout::SetFixedSize );

    if ( view ) {
        this->ui->sizeSlider->hide();
        this->ui->labelScale->hide();
        this->ui->percentLabel->hide();
        this->ui->buttonBox->setStandardButtons( QDialogButtonBox::Close );
        this->setWindowTitle( "" );
        return;
    }

    // setup scaling slider as lambda
    auto scalePixmap = [ this, scaledPixmap ]( int value ) {
        qreal scale = static_cast<qreal>( value ) / 100.0;
        const qreal aspect = static_cast<qreal>( scaledPixmap.width()) / static_cast<qreal>( scaledPixmap.height());
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
    ImageUtils::connect( this->ui->sizeSlider, &QSlider::valueChanged, scalePixmap );

    // connect ok button
    ImageUtils::connect( this->ui->buttonBox, &QDialogButtonBox::accepted, [ this, pixmap ]() {
        this->pixmap = pixmap.scaled( this->size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
    } );

    // limit frame size
    this->ui->frame->setFixedSize( qMin( this->ui->pixmapLabel->size().width() * 2 + 8, Ui::MaxImageSize + 8 ),
                                   qMin( this->ui->pixmapLabel->size().height() * 2 + 8, Ui::MaxImageSize + 8 ));
}

/**
 * @brief ImageUtils::~ImageUtils
 */
ImageUtils::~ImageUtils() {
    ImageUtils::disconnect( this->ui->sizeSlider, &QSlider::valueChanged, this, nullptr );
    ImageUtils::disconnect( this->ui->buttonBox, &QDialogButtonBox::accepted, this, nullptr );
    delete this->ui;
}

/**
 * @brief ImageUtils::autoCropPixmap
 * @param pixmap
 * @param key
 * @return
 */
QPixmap ImageUtils::autoCropPixmap( const QPixmap &pixmap, const QColor &key ) {
    const QImage image( pixmap.toImage());

    // check left
    int left = 0, right = 0, top = 0, bottom = 0;
    for ( int x = 0; x < image.width(); x++ ) {
        bool found = false;

        for ( int y = 0; y < image.height(); y++ ) {
            if ( image.pixelColor( x, y ) != key ) {
                left = x;
                found = true;
                break;
            }
        }
        if ( found )
            break;
    }

    // check right
    for ( int x = image.width() - 1; x >= 0; x-- ) {
        bool found = false;

        for ( int y = 0; y < image.height(); y++ ) {
            if ( image.pixelColor( x, y ) != key ) {
                right = x;
                found = true;
                break;
            }
        }
        if ( found )
            break;
    }

    // find bottom
    for ( int y = image.height() - 1; y >= 0; y-- ) {
        bool found = false;

        for ( int x = 0; x < image.width(); x++ ) {
            if ( image.pixelColor( x, y ) != key ) {
                bottom = y;
                found = true;
                break;
            }
        }
        if ( found )
            break;
    }

    // find bottom
    for ( int y = 0; y < image.height(); y++ ) {
        bool found = false;

        for ( int x = 0; x < image.width(); x++ ) {
            if ( image.pixelColor( x, y ) != key ) {
                top = y;
                found = true;
                break;
            }
        }
        if ( found )
            break;
    }

    QRect copyRect( QRect( left, top, right - left + 1, bottom - top + 1 ));
    QImage out( image.convertToFormat( QImage::Format_ARGB32 ));

    auto alphaFromKey = []( const QColor &colour, const QColor &key ) {
        QVector<qreal> alpha( 3 );
        QVector<qreal> colours = { colour.redF(), colour.greenF(), colour.blueF(), colour.alphaF() };
        const QVector<qreal> keyColours = { key.redF(), key.greenF(), key.blueF(), key.alphaF() };

        for ( int y = 0; y < 3; y++ ) {
            if ( colours[y] > keyColours[y] )
                alpha[y] = ( colours[y] - keyColours[y] ) / ( 255.0 - keyColours[y] );
            else if ( colours[y] < keyColours[y] )
                alpha[y] = ( keyColours[y] - colours[y] ) / ( keyColours[y] );
            else
                alpha[y] = 0.0;
        }

        if ( alpha[0] > alpha[1] )
            colours[3] = ( alpha[0] > alpha[2] ) ? alpha[0] : alpha[2];
        else
            colours[3] = ( alpha[1] > alpha[2] ) ? alpha[1] : alpha[2];

        colours[3] *= 255.0;

        if ( colours[3] > 1.0 ) {
            for ( int y = 0; y < 3; y++ )
                colours[y] = qBound( 0.0, 255.0 * ( colours[y] - keyColours[y] ) / colours[3] + keyColours[y], 1.0 );

            colours[3] = qBound( 0.0, colours[3] * colour.alphaF() / 255.0, 1.0 );
        }

        return QColor::fromRgbF( colours[0], colours[1], colours[2], colours[3] );
    };

    for ( int x = 0; x < out.width(); x++ ) {
        for ( int y = 0; y < out.height(); y++ ) {
            const QColor colour( out.pixelColor( x, y ));
            out.setPixelColor( x, y, alphaFromKey( colour, key ));//.lighter( 500 ).lighter( 500 ));
        }
    }

    return QPixmap::fromImage( out.copy( copyRect ));
}

/**
 * @brief ImageUtils::brightenPixmap
 * @param pixmap
 * @param factor
 * @return
 */
QPixmap ImageUtils::brightenPixmap( const QPixmap &pixmap, const int factor ) {
    QImage out( pixmap.toImage());
    for ( int x = 0; x < out.width(); x++ ) {
        for ( int y = 0; y < out.height(); y++ ) {
            const QColor colour( out.pixelColor( x, y ));
            out.setPixelColor( x, y, colour.lighter( factor ));
        }
    }

    return QPixmap::fromImage( qAsConst( out ));
}

/**
 * @brief ImageUtils::invertPixmap
 * @param pixmap
 * @return
 */
QPixmap ImageUtils::invertPixmap( const QPixmap &pixmap ) {
    QImage out( pixmap.toImage());
    out.invertPixels();
    return QPixmap::fromImage( qAsConst( out ));
}
