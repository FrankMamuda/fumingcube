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
#include "scriptmath.h"
#include <QtMath>

/**
 * @brief ScriptMath::ScriptMath
 */
ScriptMath::ScriptMath() {}

/**
 * @brief ScriptMath::round
 * @param value
 * @param precision
 * @return
 */
QJSValue ScriptMath::round( qreal value, int precision ) {
    return QString::number( value, 'f', precision );
}

/**
 * @brief ScriptMath::floor
 * @param value
 * @return
 */
QJSValue ScriptMath::floor( qreal value ) {
    return QString::number( qFloor( value ));
}

/**
 * @brief ScriptMath::ceil
 * @param value
 * @return
 */
QJSValue ScriptMath::ceil( qreal value ) {
    return QString::number( qCeil( value ));
}

/**
 * @brief ScriptMath::abs
 * @param value
 * @return
 */
QJSValue ScriptMath::abs( qreal value ) {
    return QString::number( qAbs( value ));
}
