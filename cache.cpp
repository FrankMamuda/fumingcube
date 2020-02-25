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
#include "cache.h"
#include "main.h"
#include <QCryptographicHash>

/**
 * @brief Cache::Cache
 */
Cache::Cache() {
    // add to garbage collector
    GarbageMan::instance()->add( this );
}

/**
 * @brief Cache::data
 * @param type
 * @param key
 * @return
 */
QVariant Cache::data( const Cache::Types &type, const QString &key ) const {
    Q_UNUSED( type )
    Q_UNUSED( key )

    // STUB
    return QVariant();
}

/**
 * @brief Cache::checksum
 * @param data
 * @param len
 * @return
 */
QString Cache::checksum( const QByteArray &array ) {
    QByteArray data( array );

    // if array is larger than 1K, take samples from start, mid and end
    if ( array.size() > 1024 ) {
        data.clear();
        data.append( array.left( 32 ));
        data.append( array.mid( array.size() / 2, 32 ));
        data.append( array.mid( array.length() - 32 - 1, 32 ));
    }

    return QString( QCryptographicHash::hash( data, QCryptographicHash::Md5 ).toHex());
}
