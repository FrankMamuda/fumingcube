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
#include <QDir>
#include <QDebug>
#include <QRegularExpressionValidator>
#include <QBuffer>
#include <QDataStream>

/**
 * @brief Cache::Cache
 */
Cache::Cache() {
    // add to garbage collector
    GarbageMan::instance()->add( this );

    // make cache dir
    this->m_path = QDir( QDir::homePath() + "/" + Main::Path + "/cache/" ).absolutePath();
    const QDir dir( this->path());
    if ( !dir.exists()) {
        dir.mkpath( dir.absolutePath());
        if ( !dir.exists())
            return;
    }
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

/**
 * @brief Cache::contains
 * @param context
 * @param key
 * @return
 */
bool Cache::contains( const QString &context, const QString &key ) const {
    // failsafe
    if ( !Cache::validate( context, key ))
        return false;

    return QFileInfo::exists( this->contextPath( context, key ));
}

/**
 * @brief Cache::getData
 * @param context
 * @param key
 * @return
 */
QByteArray Cache::getData( const QString &context, const QString &key, bool compressed ) const {
    if ( this->contains( context, key )) {
        // read cache file
        QFile file( this->contextPath( context, key ));
        if ( file.open( QIODevice::ReadOnly )) {
            const QByteArray data( file.readAll());

            // close file and return data
            file.close();
            return compressed ? qUncompress( data ) : data;
        }
    }

    return QByteArray();
}

/**
 * @brief Cache::insert
 * @param context
 * @param key
 * @param data
 * @return
 */
bool Cache::insert( const QString &context, const QString &key, const QByteArray &data, bool compress ) {
    // failsafe
    if ( !Cache::validate( context, key ))
        return false;

    // make context path if non-existant
    const QDir dir( this->contextPath( context ));
    if ( !dir.exists()) {
        dir.mkpath( dir.absolutePath());
        if ( !dir.exists()) {
            qCritical() << Cache::tr( R"(could not create context - "%1")" ).arg( context );
            return false;
        }
    }

    // write cache file
    QFile file( this->contextPath( context, key ));
    qDebug() << this->contextPath( context, key );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate )) {
        QByteArray out( compress ? qCompress( data ) : data );
        file.write( out.constData(), out.length());
        file.close();

        // close file
        file.close();

        // return success
        return true;
    }

    qCritical() << Cache::tr( R"(could not write cache - "%1/%2")" ).arg( key, context );
    return false;
}

/**
 * @brief Cache::clear
 * @param context
 * @param key
 */
void Cache::clear( const QString &context, const QString &key ) {
    // failsafe
    if ( !Cache::validate( context, key ))
        return;

    if ( this->contains( context, key )) {
        //.qDebug() << "DEL" << context << key << this->contextPath( context, key ) << QFile::exists( this->contextPath( context, key ));
        QFile::remove( this->contextPath( context, key ));
    }
}

/**
 * @brief Cache::contextPath
 * @param context
 * @return
 */
QString Cache::contextPath( const QString &context, const QString &key ) const {
    return QString( this->path() + "/" + context + "/" + ( key.isEmpty() ? "" : key ));
}

/**
 * @brief Cache::validate
 * @param text
 * @param key
 * @return
 */
bool Cache::validate( const QString &text, const QString &key ) {
    if ( text.isEmpty() || key.isEmpty())
        return false;

    QString string( text + key );
    int pos;
    const QRegularExpression re( R"([a-zA-z0-9-+,.]+)" );
    const QRegularExpressionValidator validator( re );
    return validator.validate( string, pos ) != QRegularExpressionValidator::Invalid;
}

/**
 * @brief Cache::readReagentCache
 */
void Cache::readReagentCache() {
    // look for id map in cache
    if ( Cache::instance()->contains( Cache::IdMapContext, "data.map" )) {
        // read serialized map
        QByteArray byteArray( Cache::instance()->getData( Cache::IdMapContext, "data.map", true ));
        QBuffer buffer( &byteArray );
        buffer.open( QIODevice::ReadOnly );
        QDataStream in( &buffer );

        // check version to avoid segfaults
        // if we fail, new cache information will just be overwritten
        int version;
        in >> version;
        if ( version != Cache::Version )
            return;

        // finally read in the maps
        in >> this->idNameMap >> this->nameIdMap;
    }
}

/**
 * @brief Cache::writeReagentCache
 */
void Cache::writeReagentCache() {
    // serialize maps
    QByteArray byteArray;
    QBuffer buffer( &byteArray );
    buffer.open( QIODevice::WriteOnly );
    QDataStream out( &buffer );
    out << Cache::Version << this->idNameMap << this->nameIdMap;

    // store maps into disk cache
    Cache::instance()->insert( Cache::IdMapContext, "data.map", byteArray, true );
}
