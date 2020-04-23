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
#include "imageutils.h"
#include "pixmaputils.h"
#include "ui_imageutils.h"
#include <QColorDialog>
#include <QImageReader>
#include <QInputDialog>
#include <QScreen>
#include <QTransform>
#include <QDebug>
#include <QPainter>
#include <QResizeEvent>

/**
 * @brief ImageUtils::ImageUtils
 * @param parent
 */
ImageUtils::ImageUtils( QWidget *parent ) : QDialog( parent ), ui( new Ui::ImageUtils ) {
    this->ui->setupUi( this );
    this->ui->contents->setWindowFlags( Qt::Widget );

    this->ui->scrollArea->setBackgroundRole( QPalette::Dark );
    this->imageWidget()->setImageUtilsParent( this );
    this->resize( QApplication::primaryScreen()->availableSize() * 3 / 5 );
    this->m_cropWidget = new CropWidget( this->imageWidget());

    // this->loadImage( "C:/Home/Downloads/CN103819475AD00061.png" );
    this->loadImage( "C:/Home/Downloads/131411.png" );

    // SCALE lambda
    QAction::connect( this->ui->actionScale, &QAction::triggered, this, [ this ] () {
        bool ok;
        const qreal scale = QInputDialog::getInt( this, ImageUtils::tr( "Scale image" ), ImageUtils::tr( "Input scale factor (%)" ), static_cast<int>( this->scale() * 100 ), 5, 200, 1, &ok ) / 100.0;
        if ( ok )
            this->scaleImage( scale );
    } );

    // ZOOM IN lambda
    QAction::connect( this->ui->actionZoomIn, &QAction::triggered, this, [ this ] () {
        this->imageWidget()->setZoomScale( qMin( this->imageWidget()->zoomScale() + 0.25, 4.0 ));
        this->scaleImage( this->scale());
    } );

    // ZOOM OUT lambda
    QAction::connect( this->ui->actionZoomOut, &QAction::triggered, this, [ this ] () {
        this->imageWidget()->setZoomScale( qMax( this->imageWidget()->zoomScale() - 0.25, 0.05 ));
        this->scaleImage( this->scale());
    } );

    // NORMAL SIZE lambda
    QAction::connect( this->ui->actionNormalSize, &QAction::triggered, this, [ this ] () {
        this->imageWidget()->setZoomScale( 1.0 );
        this->scaleImage( this->scale());
    } );

    // MAKE TRANSPARENT lambda
    QAction::connect( this->ui->actionMakeTransparent, &QAction::triggered, this, [ this ] () {
        QColorDialog cd( this );
        cd.move( this->imageWidget()->mapToGlobal( this->imageWidget()->imageGeometry().topRight()));
        cd.setWindowTitle( ImageUtils::tr( "Pick current background colour (colour key)" ));
        if ( cd.exec() == QDialog::Accepted && cd.currentColor().isValid())
            this->setImage( ImageUtils::colourToAlpha( this->image(), cd.currentColor()));
    } );

    // FIT SCREEN lambda
    QAction::connect( this->ui->actionFit, &QAction::triggered, this, [ this ] () {
        if ( this->image().width() < this->image().height())
            this->imageWidget()->setZoomScale(( this->ui->scrollArea->size().width()) / ( this->image().width() * this->scale()));
        else
            this->imageWidget()->setZoomScale(( this->ui->scrollArea->size().height()) / ( this->image().height() * this->scale()));

        this->scaleImage( this->scale());
    } );

    // AUTOCROP lambda
    QAction::connect( this->ui->actionAutocrop, &QAction::triggered, this, [ this ] () {
        this->setImage( ImageUtils::autoCrop( this->image()));
    } );

    // ROTATE lambda
    QAction::connect( this->ui->actionRotate, &QAction::triggered, this, [ this ] () {
        this->setImage( this->image().transformed( QTransform().rotate( 90 )));
    } );

    // INVERT lambda
    QAction::connect( this->ui->actionInvert, &QAction::triggered, this, [ this ] () {
        QImage image( this->image().convertToFormat( QImage::Format_ARGB32 ));
        image.invertPixels();
        this->setImage( qAsConst( image ));
    } );

    // SET BACKGROUND lambda
    QAction::connect( this->ui->actionSetBackground, &QAction::triggered, this, [ this ] () {
        QColorDialog cd( this );
        if ( cd.exec() == QDialog::Accepted && cd.currentColor().isValid()) {
            QImage image( this->image().size(), QImage::Format_ARGB32 );
            image.fill( cd.currentColor());

            QPainter painter( &image );
            painter.drawImage( 0, 0, this->image());
            painter.end();

            this->setImage( qAsConst( image ));
        }
    } );

    // RESTORE lambda
    QAction::connect( this->ui->actionRestoreOriginal, &QAction::triggered, this, [ this ] () {
        this->setImage( this->originalImage );
    } );

    // MANUAL CROP lambda
    QAction::connect( this->ui->actionCrop, &QAction::toggled, this, [ this ] ( bool checked ) {
        if ( checked ) {
            this->cropWidget()->setGeometry( this->imageWidget()->imageGeometry());
            this->cropWidget()->show();
        } else {
            if ( this->cropWidget()->isVisible()) {
                const QPointF delta( QPointF( this->cropWidget()->geometry().topLeft() - this->imageWidget()->imageGeometry().topLeft()) / this->imageWidget()->zoomScale());
                const QSizeF scale( this->cropWidget()->size() / this->imageWidget()->zoomScale());

                const QImage image( this->image().convertToFormat( QImage::Format_ARGB32 ).copy( QRect( delta.toPoint(), scale.toSize())));
                this->setImage( image );
                this->cropWidget()->hide();
                this->imageWidget()->setZoomScale( 1.0 );
            }
        }
    } );

    this->ui->iconLabel->setPixmap( QIcon::fromTheme( "info" ).pixmap( 16, 16 ));
    this->ui->titleEdit->hide();
    this->ui->titleLabel->hide();
}

/**
 * @brief ImageUtils::~ImageUtils
 */
ImageUtils::~ImageUtils() {
    // disconnect all actions
    QAction::disconnect( this->ui->actionScale, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionZoomIn, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionZoomOut, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionNormalSize, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionMakeTransparent, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionFit, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionAutocrop, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionRotate, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionInvert, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionSetBackground, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionRestoreOriginal, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionCrop, &QAction::toggled, this, nullptr );

    // delete ui
    delete this->ui;
}

/**
 * @brief ImageUtils::loadFile
 * @param fileName
 * @return
 */
bool ImageUtils::loadImage( const QString &fileName ) {
    if ( fileName.isEmpty())
        return false;

    const QImage image( QImageReader( fileName ).read());
    if ( image.isNull())
        return false;

    // set image to widget and store original
    this->setImage( image, true );

    return true;
}

/**
 * @brief ImageUtils::setImage
 * @param image
 * @param reset
 */
void ImageUtils::setImage( const QImage &image, bool reset ) {
    if ( image.isNull())
        return;

    // store original image
    if ( reset )
        this->originalImage = image;

    // set image to widget
    this->imageWidget()->setImage( image );

    // store last geometry for crop widget
    this->lastImageGeometry = this->imageWidget()->imageGeometry();
}

/**
 * @brief ImageUtils::setViewMode
 */
void ImageUtils::setViewMode() {
    // add a simple menubar for read-only image viewing
    this->ui->menuBar->clear();
    this->ui->menuBar->addAction( this->ui->actionZoomIn );
    this->ui->menuBar->addAction( this->ui->actionZoomOut );
    this->ui->menuBar->addAction( this->ui->actionFit );
    this->ui->menuBar->addAction( this->ui->actionNormalSize );
    this->ui->buttonBox->setStandardButtons( QDialogButtonBox::Close );
    this->ui->infoLabel->hide();
    this->ui->iconLabel->hide();
    this->setWindowTitle( ImageUtils::tr( "Image viewer" ));
}

/**
 * @brief ImageUtils::setAddMode
 */
void ImageUtils::setAddMode() {
    this->ui->titleEdit->show();
    this->ui->titleLabel->show();
}

/**
 * @brief ImageUtils::resizeEvent
 * @param event
 */
void ImageUtils::resizeEvent( QResizeEvent *event ) {
    QDialog::resizeEvent( event );

    // move crop rectangle along with the image
    if ( this->cropWidget()->isVisible()) {
        const QPoint delta( this->imageWidget()->imageGeometry().topLeft() + QPoint( this->cropWidget()->geometry().topLeft() - this->lastImageGeometry.topLeft()));
        this->cropWidget()->move( delta );
    }

    // store last geometry for crop widget
    this->lastImageGeometry = this->imageWidget()->imageGeometry();
}

/**
 * @brief ImageUtils::showEvent
 * @param event
 */
void ImageUtils::showEvent( QShowEvent *event ) {
    QDialog::showEvent( event );
    this->lastImageGeometry = this->imageWidget()->imageGeometry();
}

/**
 * @brief ImageUtils::scaleImage
 * @param factor
 */
void ImageUtils::scaleImage( double scale ) {
    if ( this->cropWidget()->isVisible())
        this->cropWidget()->hide();

    if ( this->imageWidget()->image().isNull() || scale < 0.05 || scale > 2.0 )
        return;

    this->m_scale = scale;
    this->setImage( this->image().scaledToWidth( static_cast<int>( this->image().width() * this->scale()), Qt::SmoothTransformation ));
}

/**
 * @brief ImageUtils::autoCrop
 * @param image
 * @return
 */
QImage ImageUtils::autoCrop( const QImage &image, bool preserveAspectRatio ) {
    // TODO: rework this using scanlines

    // find key
    const QList<QColor> colours { image.pixelColor( 0, 0 ), image.pixelColor( image.width() - 1, 0 ), image.pixelColor( 0, image.height() - 1 ), image.pixelColor( image.width() - 1, image.height() - 1 ) };
    int max = 0;
    QColor key;
    for ( const QColor &colour : colours ) {
        const int count = colours.count( colour );
        if ( count > max ) {
            max = count;
            key = colour;
        }
    }

    // check left
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
    for ( int x = 0; x < image.width(); x++ ) {
        bool found = false;

        for ( int y = 0; y < image.height(); y++ ) {
            if ( image.pixelColor( x, y ) != qAsConst( key )) {
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
            if ( image.pixelColor( x, y ) != qAsConst( key )) {
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
            if ( image.pixelColor( x, y ) != qAsConst( key )) {
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
            if ( image.pixelColor( x, y ) != qAsConst( key )) {
                top = y;
                found = true;
                break;
            }
        }
        if ( found )
            break;
    }

    QRect copyRect( left, top, right - left + 1, bottom - top + 1 );
    if ( preserveAspectRatio ) {
        const qreal originalAspect = image.width() / image.height();
        const int delta = static_cast<int>((( copyRect.width() * originalAspect ) - copyRect.width()) / 2.0 );
        copyRect.setX( copyRect.x() - delta );
        copyRect.setWidth( copyRect.width() + delta * 2 );
    }

    return image.copy( qAsConst( copyRect ));
}

/**
 * @brief ImageUtils::colourToAlpha
 * @param image
 * @param key
 * @return
 */
QImage ImageUtils::colourToAlpha( const QImage &image, const QColor &key ) {
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
        for ( int y = 0; y < out.height(); y++ )
            out.setPixelColor( x, y, alphaFromKey( image.pixelColor( x, y ), key ));
    }

    return out;
}

/**
 * @brief ImageUtils::imageWidget
 * @return
 */
ImageWidget *ImageUtils::imageWidget() const {
    return this->ui->imageWidget;
}

/**
 * @brief ImageUtils::image
 * @return
 */
QImage ImageUtils::image() const {
    return this->imageWidget()->image();
}

/**
 * @brief ImageUtils::title
 * @return
 */
QString ImageUtils::title() const {
    return this->ui->titleEdit->text();
}
