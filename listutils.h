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
#include <QList>

/**
 * @brief The Script class
 */
class ListUtils final {

public:
    /**
     * @brief toNumericList
     * @param list
     * @return
     */
    template<typename T>
    [[nodiscard]]
    static
    QList<T> toNumericList( const QStringList &list ) {
        QList<T> out;
        for ( const QString &str : list )
            out << QVariant( str ).value<T>();

        return qAsConst( out );
    }

    /**
     * @brief toStringList
     * @param list
     * @return
     */
    template<typename T>
    [[nodiscard]]
    static
    QStringList toStringList( const QList<T> &list ) {
        QStringList out;
        for ( const T &val : list ) {
            const QVariant var( QVariant::fromValue( val ));

            if ( var.canConvert<QString>())
                out << var.toString();
            else if ( var.canConvert<int>())
                out << QString::number( var.toInt());
        }
        return qAsConst( out );
    }
};
