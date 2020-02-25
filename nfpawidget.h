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
#include "propertyviewwidget.h"
#include <QtMath>
#include <QMap>

/**
 * @brief The NFPAWidget class
 */
class NFPAWidget final : public PropertyViewWidget {
Q_OBJECT

public:
    explicit NFPAWidget( QWidget *parent = nullptr, const QStringList &parms = QStringList());

    /**
     * @brief scale
     * @return
     */
    [[nodiscard]] int scale() const { return this->m_scale; }

    /**
     * @brief vscale
     * @return
     */
    [[nodiscard]] qreal vscale() const { return this->m_vscale; }

public slots:
    /**
     * @brief update
     * @param parms
     */
    void update( const QStringList &parms ) override {
        if ( this->parameters() == parms )
            return;

        this->m_parameters = parms;
        this->repaint();
    }

    void setScale( const int &scale = 32 );

protected:
    void paintEvent( QPaintEvent * ) override;

    /**
     * @brief sizeHint
     * @return
     */
    [[nodiscard]] QSize sizeHint() const override {
        const qreal vScale = sqrt( 2 * ( this->scale() * this->scale()));
        return QSizeF( vScale * 2, vScale * 2 ).toSize();
    }

private:
    int m_scale = 0;
    qreal m_vscale = 0.0;
    const QMap<int, qreal> scales {{ 0, 0 },
                                   { 1, this->scale() * 0.5 },
                                   { 2, this->scale() * 0.42 },
                                   { 3, this->scale() * 0.35 },
                                   { 4, this->scale() * 0.22 }};
    QList<QRectF> rects;
};
