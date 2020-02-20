/*
 * Copyright (C) 2017-2018 Factory #12
 * Copyright (C) 2019 Armands Aleksejevs
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

/**
 * @brief The PropertyViewWidget class
 */
class PropertyViewWidget : public QWidget {
    Q_OBJECT

public:
    explicit PropertyViewWidget( QWidget *parent = nullptr, const QStringList & = QStringList()) : QWidget( parent ) {}
    [[nodiscard]] QStringList parameters() const { return this->m_parameters; }

public slots:
    virtual void update( const QStringList &parms ) = 0;

protected:
    QStringList m_parameters;
};
