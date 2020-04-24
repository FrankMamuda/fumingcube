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
#include <QHBoxLayout>
#include <QSizeGrip>

/**
 * @brief The RubberBandWidget class
 */
class CropWidget : public QWidget {
    Q_OBJECT

public:
    // disable move
    CropWidget( CropWidget&& ) = delete;
    CropWidget& operator=( CropWidget&& ) = delete;

    // constructor/destructor
    explicit CropWidget( QWidget *parent = nullptr );
    ~CropWidget() override;

    /**
     * @brief isDragged
     * @return
     */
    [[nodiscard]] bool isDragged() const { return this->m_drag; }

    /**
     * @brief intialPosition
     * @return
     */
    [[nodiscard]] QPoint intialPosition() const { return this->m_initial; }
    [[nodiscard]] bool validateGeometry( QRect &geometry );

private:
    QSizeGrip *topLeftSizeGrip = new QSizeGrip( this );
    QSizeGrip *bottomRightSizeGrip = new QSizeGrip( this );
    QHBoxLayout *boxLayout = new QHBoxLayout( this );
    QPoint m_initial;
    bool m_drag = false;

protected:
    void moveEvent( QMoveEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
};
