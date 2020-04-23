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
#include <QWidget>
#include "imageutils.h"

/**
 * @brief The ImageWidget class
 */
class ImageWidget : public QWidget {
    Q_OBJECT

public:
    // constructor
    explicit ImageWidget( QWidget *parent = nullptr ) : QWidget( parent ) {}

    // disable move
    ImageWidget( ImageUtils&& ) = delete;
    ImageWidget& operator=( ImageWidget&& ) = delete;

    /**
     * @brief imageUtilsParent
     * @return
     */
    [[nodiscard]] ImageUtils *imageUtilsParent() const { return this->m_iu; }
    [[nodiscard]] QRect imageGeometry() const;

    /**
     * @brief zoomScale
     * @return
     */
    [[nodiscard]] qreal zoomScale() const { return this->m_zoomScale; }

    /**
     * @brief image
     * @return
     */
    [[nodiscard]] QImage image() const { return this->m_image; }

protected:
    void paintEvent( QPaintEvent * ) override;

public slots:
    void setImageUtilsParent( ImageUtils *iu ) { this->m_iu = iu; }
    void setZoomScale( qreal zoomScale ) { this->m_zoomScale = zoomScale; this->repaint(); }
    void setImage( const QImage &image ) { this->m_image = image; this->repaint(); }

private:
    ImageUtils *m_iu = nullptr;
    qreal m_zoomScale = 1.0;
    QImage m_image;
};
