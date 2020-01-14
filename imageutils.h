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

#pragma once

/*
 * includes
 */
#include <QDialog>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class ImageUtils;
static const int MaxImageSize = 512;
}

/**
 * @brief The ImageUtils class
 */
class ImageUtils : public QDialog {
    Q_OBJECT

public:
    explicit ImageUtils( QWidget *parent = nullptr, const QPixmap &pixmap = QPixmap(), const int &preferredWidth = 0, bool view = false );
    ~ImageUtils();
    QPixmap pixmap;

    static QPixmap autoCropPixmap( const QPixmap &pixmap, const QColor &key = Qt::white );
    static QPixmap brightenPixmap( const QPixmap &pixmap, const int factor = 150 );
    static QPixmap invertPixmap( const QPixmap &pixmap );

signals:
    void accepted( const QPixmap &pixmap );

private:
    Ui::ImageUtils *ui;
    QSize size;
};
