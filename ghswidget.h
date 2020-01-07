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
#include "propertydock.h"

/**
 * @brief The GHSWidget class
 */
class GHSWidget final : public PropertyViewWidget {
public:
    static constexpr const int scale = 48;
    explicit GHSWidget( QWidget *parent = nullptr, const QStringList &parms = QStringList());
    int iconsPerRow() const { return this->m_iconsPerRow; }

public slots:
    void update( const QStringList &parms ) override {
        //if ( this->parameters() == parms )
        //     return;
        this->m_parameters = parms;
        this->repaint();
    }
    void setLinear() { this->m_linear = true; }

protected:
    void paintEvent( QPaintEvent * ) override;

    /**
     * @brief sizeHint
     * @return
     */
    QSize sizeHint() const override;

    /**
     * @brief minimumSizeHint
     * @return
     */
    QSize minimumSizeHint() const override { return this->sizeHint(); }

private:
    mutable int m_iconsPerRow = 0;
    bool m_linear = false;
};
