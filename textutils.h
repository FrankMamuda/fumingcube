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
#include <QString>

/**
 * @brief The TextUtils class
 */
class TextUtils final {
    Q_DISABLE_COPY( TextUtils )

public:
    // disable move
    TextUtils( TextUtils&& ) = delete;
    TextUtils& operator=( TextUtils&& ) = delete;

    /**
     * @brief elidedString
     * @param string
     * @param length
     * @return
     */
    [[nodiscard]]
    static QString elidedString( const QString &string, int length = 32 ) {
        return string.length() < length ? string : string.left( length - 3 ) + "...";
    }

    /**
     * @brief toBase64
     * @param string
     * @return
     */
    [[nodiscard]]
    static QString toBase64( const QString &string ) {
        return QByteArray( string.toUtf8().constData()).toBase64().constData();
    }

    /**
     * @brief fromBase64
     * @param string
     * @return
     */
    [[nodiscard]]
    static QString fromBase64( const QString &string ) {
        if ( string.isEmpty())
            return QString();

        return QByteArray::fromBase64( string.toUtf8().constData()).constData();
    }

private:
    explicit TextUtils() {}
};
