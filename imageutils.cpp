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
#include <QClipboard>
#include <QMessageBox>
#include <QFileDialog>

/**
 * @brief ImageUtils::ImageUtils
 * @param parent
 */
ImageUtils::ImageUtils( QWidget *parent, const Modes &mode, const QImage &image ) : QDialog( parent ), ui( new Ui::ImageUtils ) {
    this->ui->setupUi( this );
    this->ui->contents->setWindowFlags( Qt::Widget );

    this->ui->scrollArea->setBackgroundRole( QPalette::Dark );
    this->imageWidget()->setImageUtilsParent( this );
    this->resize( QApplication::primaryScreen()->availableSize() * 3 / 5 );
    this->m_cropWidget = new CropWidget( this->imageWidget());

    // adjustCropWidget lambda
    auto adjustCropWidget = [ this ]() {
        // move crop rectangle along with the image
        if ( this->cropWidget()->isVisible()) {
            const QPoint delta( this->imageWidget()->imageGeometry().topLeft() + QPoint( this->cropWidget()->geometry().topLeft() - this->lastImageGeometry.topLeft()));
            this->cropWidget()->move( delta );
            this->cropWidget()->resize( this->cropWidget()->size() * ( this->imageWidget()->imageGeometry().size().width() / static_cast<qreal>( this->lastImageGeometry.size().width())));
        }

        // store last geometry for crop widget
        this->lastImageGeometry = this->imageWidget()->imageGeometry();
    };

    // SCALE lambda
    QAction::connect( this->ui->actionScale, &QAction::triggered, this, [ this ] () {
        this->hideCropWidget();

        bool ok;
        const qreal scale = QInputDialog::getInt( this, ImageUtils::tr( "Scale image" ), ImageUtils::tr( "Input scale factor (%)" ), 100, 5, 200, 1, &ok ) / 100.0;
        if ( ok )
            this->scaleImage( scale );
    } );


    // ZOOM IN lambda
    QAction::connect( this->ui->actionZoomIn, &QAction::triggered, this, [ this, adjustCropWidget ] () {
        this->imageWidget()->setZoomScale( qMin( this->imageWidget()->zoomScale() + 0.25, 4.0 ));
        adjustCropWidget();
    } );

    // ZOOM OUT lambda
    QAction::connect( this->ui->actionZoomOut, &QAction::triggered, this, [ this, adjustCropWidget ] () {
        this->imageWidget()->setZoomScale( qMax( this->imageWidget()->zoomScale() - 0.25, 0.05 ));
        adjustCropWidget();
    } );

    // NORMAL SIZE lambda
    QAction::connect( this->ui->actionNormalSize, &QAction::triggered, this, [ this, adjustCropWidget ] () {
        this->imageWidget()->setZoomScale( 1.0 );
        adjustCropWidget();
    } );

    // MAKE TRANSPARENT lambda
    QAction::connect( this->ui->actionMakeTransparent, &QAction::triggered, this, [ this ] () {
        QColorDialog cd( this );
        cd.move( this->ui->scrollArea->mapToGlobal( this->ui->scrollArea->geometry().center()));
        cd.setWindowTitle( ImageUtils::tr( "Pick current background colour (colour key)" ));
        if ( cd.exec() == QDialog::Accepted && cd.currentColor().isValid())
            this->setImage( ImageUtils::colourToAlpha( this->image(), cd.currentColor()));
    } );

    // FIT SCREEN lambda
    QAction::connect( this->ui->actionFit, &QAction::triggered, this, [ this, adjustCropWidget ] () {
        const qreal widthIndex = this->ui->scrollArea->width() / static_cast<qreal>( this->image().width());
        const qreal heightIndex = this->ui->scrollArea->height() / static_cast<qreal>( this->image().height());
        this->imageWidget()->setZoomScale( qMin( 1.0, heightIndex < widthIndex ? heightIndex : widthIndex ));

        adjustCropWidget();
    } );

    // AUTOCROP lambda
    QAction::connect( this->ui->actionAutocrop, &QAction::triggered, this, [ this ] () {
        this->hideCropWidget();
        this->setImage( ImageUtils::autoCrop( this->image()));
    } );

    // ROTATE lambda
    QAction::connect( this->ui->actionRotate, &QAction::triggered, this, [ this ] () {
        this->hideCropWidget();
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
        this->hideCropWidget();
        this->setImage( this->originalImage );
    } );

    // MANUAL CROP lambda
    QAction::connect( this->ui->actionCrop, &QAction::triggered, this, [ this ] () {
        if ( !this->cropWidget()->isVisible()) {
            if ( !this->ui->scrollArea->geometry().contains( this->imageWidget()->imageGeometry()))
                this->ui->actionFit->trigger();

            this->cropWidget()->setGeometry( this->imageWidget()->imageGeometry());
            this->cropWidget()->show();
            this->ui->stackedWidget->setCurrentIndex( 1 );
        } else {
            this->hideCropWidget();
        }
    } );

    // DONE button lambda
    QPushButton::connect( this->ui->doneButton, &QPushButton::clicked, this, [ this ] () {
        if ( this->cropWidget()->isVisible()) {
            const QPointF delta( QPointF( this->cropWidget()->geometry().topLeft() - this->imageWidget()->imageGeometry().topLeft()) / this->imageWidget()->zoomScale());
            const QSizeF scale( this->cropWidget()->size() / this->imageWidget()->zoomScale());

            const QImage image( this->image().convertToFormat( QImage::Format_ARGB32 ).copy( QRect( delta.toPoint(), scale.toSize())));
            this->setImage( image );
            this->hideCropWidget();
        }
    } );

    this->ui->iconLabel->setPixmap( QIcon::fromTheme( "info" ).pixmap( 16, 16 ));
    this->ui->titleEdit->hide();
    this->ui->titleLabel->hide();

    // PASTE lambda
    QAction::connect( this->ui->actionPaste, &QAction::triggered, this, [ this ] () {
        this->paste( QApplication::clipboard()->image());
    } );

    // CLEAR lambda
    QAction::connect( this->ui->actionClear, &QAction::triggered, this, [ this ] () {
        this->hideCropWidget();
        this->originalImage = QImage();
        this->imageWidget()->setImage( QImage());
    } );

    // OPEN/REPLACE lambda
    QAction::connect( this->ui->actionReplace, &QAction::triggered, this, [ this ] () {
        const QString fileName( QFileDialog::getOpenFileName( this, QWidget::tr( "Open Image" ), "", QWidget::tr( "Images (*.png *.jpg)" )));
        if ( fileName.isEmpty())
            return;

        this->hideCropWidget();
        this->loadImage( fileName );
        this->ui->actionFit->trigger();
    } );

    // SAVE lambda
    QAction::connect( this->ui->actionSave, &QAction::triggered, this, [ this ] () {
        const QString fileName( QFileDialog::getSaveFileName( this, QWidget::tr( "Save Image" ), "", QWidget::tr( "Image (*.png)" )));
        if ( fileName.isEmpty())
            return;

        this->imageWidget()->image().save( fileName );
    } );

    this->ui->actionSave->setShortcut( QKeySequence::Save );
    this->ui->actionPaste->setShortcut( QKeySequence::Paste );
    this->ui->actionZoomIn->setShortcut( QKeySequence::ZoomIn );
    this->ui->actionZoomOut->setShortcut( QKeySequence::ZoomOut );
    this->ui->actionReplace->setShortcut( QKeySequence::Open );
    this->ui->actionFit->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_F ));
    this->ui->actionNormalSize->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ));
    this->ui->actionAutocrop->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_C ));
    this->ui->actionCrop->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_C ));
    this->ui->actionMakeTransparent->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_T ));
    this->ui->actionSetBackground->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_B ));
    this->ui->actionInvert->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_I ));
    this->ui->actionRotate->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_R ));
    this->ui->actionRestoreOriginal->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_O ));

    // set image if any
    if ( !image.isNull() && mode != OpenMode )
        this->setImage( image, true );

    switch ( mode ) {
    case OpenMode:
        this->ui->actionReplace->trigger();
        break;

    case ViewMode:
        this->setViewMode();
        break;

    case PropertyMode:
        this->setAddMode();
        break;

    case EditMode:
        break;
    }
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
    QAction::disconnect( this->ui->actionCrop, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionPaste, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionClear, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionReplace, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionSave, &QAction::triggered, this, nullptr );
    QPushButton::disconnect( this->ui->doneButton, &QPushButton::clicked, this, nullptr );

    // delete ui
    delete this->m_cropWidget;
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
 * @brief ImageUtils::setTitle
 * @param title
 */
void ImageUtils::setTitle( const QString &title ) {
    this->ui->titleEdit->setText( title );
}

/**
 * @brief ImageUtils::hideCropWidget
 */
void ImageUtils::hideCropWidget() {
    this->cropWidget()->hide();
    this->ui->actionFit->trigger();
    this->ui->stackedWidget->setCurrentIndex( 0 );
}

/**
 * @brief ImageUtils::paste
 * @param image
 */
void ImageUtils::paste( const QImage &image ) {
    if ( !image.isNull()) {
        bool ok = true;
        if ( !this->imageWidget()->image().isNull())
            ok = QMessageBox::question( this, ImageUtils::tr( "Confirm replacement" ), ImageUtils::tr( "Replace current image?" )) == QMessageBox::Yes;

        if ( ok ) {
            this->hideCropWidget();
            this->setImage( image, true );
            this->ui->actionFit->trigger();
        }
    } else {
        QMessageBox::information( this, ImageUtils::tr( "Paste error" ), ImageUtils::tr( "Clipboard contains no valid images" ));
    }
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
    this->ui->actionFit->trigger();
    this->lastImageGeometry = this->imageWidget()->imageGeometry();
}

/**
 * @brief ImageUtils::mouseReleaseEvent
 * @param event
 */
void ImageUtils::mouseReleaseEvent( QMouseEvent *event ) {
    QDialog::mouseReleaseEvent( event );

    if ( this->ui->imageWidget->image().isNull())
        this->ui->actionReplace->trigger();
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

    this->setImage( this->image().scaledToWidth( static_cast<int>( this->image().width() * scale ), Qt::SmoothTransformation ));
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

    // find top
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
    QRgb *ptr = reinterpret_cast<QRgb*>( out.scanLine( 0 ));
    const QRgb *end = ptr + out.width() * out.height();
//#define DOUBLE_C2A
#ifndef DOUBLE_C2A
    const int keyColours[3] { key.red(), key.green(), key.blue() };
    for ( ; ptr < end; ptr++ ) {
        int colours[4] { qRed( *ptr ), qGreen( *ptr ), qBlue( *ptr ), qAlpha( *ptr ) };
        int alpha[4] { 0, 0, 0, colours[3] };

        for ( int y = 0; y < 3; y++ ) {
            if ( colours[y] > keyColours[y] )
                alpha[y] = ( colours[y] - keyColours[y] ) / ( 255 - keyColours[y] );
            else if ( colours[y] < keyColours[y] )
                alpha[y] = 255 * ( keyColours[y] - colours[y] ) / keyColours[y];
        }

        colours[3] = ( alpha[0] > alpha[1] ) ? qMax( alpha[0], alpha[2] ) : qMax( alpha[1], alpha[2] );
        if ( colours[3] > 1 ) {
            for ( int y = 0; y < 3; y++ )
                colours[y] = qBound( 0, 255 * ( colours[y] - keyColours[y] ) / colours[3] + keyColours[y], 255 );

            colours[3] = qBound( 0, colours[3] * alpha[3] / 255, 255 );
        }

        *ptr = qRgba( colours[0], colours[1], colours[2], colours[3] );
    }
#else
    const qreal keyColours[3] { key.redF(), key.greenF(), key.blueF() };
    for ( ; ptr < end; ptr++ ) {
        qreal colours[4] { static_cast<qreal>( qRed( *ptr ) / 255.0 ), static_cast<qreal>( qGreen( *ptr ) / 255.0 ), static_cast<qreal>( qBlue( *ptr ) / 255.0 ), static_cast<qreal>( qAlpha( *ptr ) / 255.0 ) };
        qreal alpha[4] { 0.0, 0.0, 0.0, colours[3] };

        for ( int y = 0; y < 3; y++ ) {
            if ( colours[y] > keyColours[y] )
                alpha[y] = ( colours[y] - keyColours[y] ) / ( 255.0 - keyColours[y] );
            else if ( colours[y] < keyColours[y] )
                alpha[y] = ( keyColours[y] - colours[y] ) / ( keyColours[y] );
        }

        colours[3] = ( alpha[0] > alpha[1] ) ? qMax( alpha[0], alpha[2] ) : qMax( alpha[1], alpha[2] );
        if ( colours[3] > 0.004 ) {
            for ( int y = 0; y < 3; y++ )
                colours[y] = qBound( 0.0, 255.0 * ( colours[y] - keyColours[y] ) / ( colours[3] * 255.0 ) + keyColours[y], 1.0 );

            colours[3] = qBound( 0.0, colours[3] * alpha[3], 1.0 );
        }

        *ptr = qRgba( static_cast<int>( colours[0] * 255 ), static_cast<int>( colours[1] * 255 ), static_cast<int>( colours[2] * 255 ), static_cast<int>( colours[3] * 255 ));
    }
#endif

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
