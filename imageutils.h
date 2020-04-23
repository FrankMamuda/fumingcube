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

#pragma once

/*
 * includes
 */
#include "cropwidget.h"
#include <QDialog>
#include <QRubberBand>

/**
 * @brief The Ui namespace
 */
namespace Ui { class ImageUtils; }

/*
 * classes
 */
class ImageWidget;

/**
 * @brief The ImageUtils class
 */
class ImageUtils : public QDialog {
    Q_OBJECT

public:
    // disable move
    ImageUtils( ImageUtils&& ) = delete;
    ImageUtils& operator=( ImageUtils&& ) = delete;

    // constructor/destructor
    ImageUtils( QWidget *parent = nullptr );
    ~ImageUtils() override;

    bool loadImage( const QString &fileName );

    /**
     * @brief scale
     * @return
     */
    [[nodiscard]] qreal scale() const { return this->m_scale; }

    /**
     * @brief cropWidget
     * @return
     */
    [[nodiscard]] CropWidget *cropWidget() const { return this->m_cropWidget; }
    [[nodiscard]] static QImage autoCrop( const QImage &image, bool preserveAspectRatio = false );
    [[nodiscard]] static QImage colourToAlpha( const QImage &image, const QColor &key = Qt::white );
    [[nodiscard]] ImageWidget *imageWidget() const;
    [[nodiscard]] QImage image() const;
    [[nodiscard]] QString title() const;

public slots:
    void setImage( const QImage &image, bool reset = false );
    void scaleImage( qreal scale );
    void setViewMode();
    void setAddMode();

protected:
    void resizeEvent( QResizeEvent *event ) override;
    void showEvent( QShowEvent *event ) override;

private:
    Ui::ImageUtils *ui;
    qreal m_scale = 1.0;
    CropWidget *m_cropWidget = nullptr;
    QRect lastImageGeometry;
    QImage originalImage;
};
