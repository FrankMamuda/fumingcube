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
#include <QShortcut>
#include <QTextBrowser>

/**
 * @brief The CalcView class
 */
class CalcView final : public QTextBrowser {
    Q_OBJECT

public:
    explicit CalcView( QWidget *parent = nullptr );
    ~CalcView() override;

    /**
     * @brief zoom
     * @return
     */
    [[nodiscard]] qreal zoom() const { return this->m_zoom; }

public slots:
    /**
     * @brief zoomIn
     */
    void zoomIn() {
        this->m_zoom += 0.1;
        this->m_zoom = qMin( this->m_zoom, 4.0 );
        this->repaint();
    }

    /**
     * @brief zoomOut
     */
    void zoomOut() {
        this->m_zoom -= 0.1;
        this->m_zoom = qMax( this->m_zoom, 0.5 );
        this->repaint();
    }

    /**
     * @brief zoomRestore
     */
    void zoomRestore() {
        this->m_zoom = 1.0;
        this->repaint();
    }

protected:
    void contextMenuEvent( QContextMenuEvent *event ) override;
    void paintEvent( QPaintEvent *e ) override;

private:
    qreal m_zoom = 1.0;
    QShortcut *shortCutZoomIn;
    QShortcut *shortCutZoomOut;
};
