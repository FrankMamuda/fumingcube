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
quint32 Cache::checksum( const char *data, size_t len ) {
    const quint32 m = 0x5bd1e995, r = 24;
    quint32 h = 0, w;
    const char *l = data + len;

    while ( data + 4 <= l ) {
        w = *( reinterpret_cast<const quint32 *>( data ));
        data += 4;
        h += w;
        h *= m;
        h ^= ( h >> 16 );
    }

    switch ( l - data ) {
        case 3:
            h += static_cast<quint32>( data[2] << 16 );
            [[fallthrough]];

        case 2:
            h += static_cast<quint32>( data[1] << 8 );
            [[fallthrough]];

        case 1:
            h += static_cast<quint32>( data[0] );
            h *= m;
            h ^= ( h >> r );
            break;
    }
    return h;
}
