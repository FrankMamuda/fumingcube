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
#include "math.h"
#include <QtMath>

/**
 * @brief Math::Math
 */
Math::Math() {}

/**
 * @brief Math::round
 * @param value
 * @param precision
 * @return
 */
QJSValue Math::round( qreal value, int precision ) {
    return QString::number( value, 'f', precision );
}

/**
 * @brief Math::floor
 * @param value
 * @return
 */
QJSValue Math::floor( qreal value ) {
    return QString::number( qFloor( value ));
}

/**
 * @brief Math::ceil
 * @param value
 * @return
 */
QJSValue Math::ceil( qreal value ) {
    return QString::number( qCeil( value ));
}

/**
 * @brief Math::abs
 * @param value
 * @return
 */
QJSValue Math::abs( qreal value ) {
    return QString::number( qAbs( value ));
}
