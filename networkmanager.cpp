/*
 * Copyright (C) 2017-2018 Factory #12
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

//
// includes
//
#include "networkmanager.h"

/**
 * @brief NetworkManager::execute
 * @param url
 * @param type
 * @param userData
 */
void NetworkManager::execute( const QString &url, NetworkManager::Type type, const QVariant &userData ) {
    QNetworkRequest request;

    // add user data to request
    request.setUrl( QUrl::fromEncoded( url.toLocal8Bit()));
    request.setAttribute( QNetworkRequest::User, static_cast<int>( type ));
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ), userData );

    // get reply
    QNetworkReply *reply( this->manager.get( request ));

    // handle replies
    this->connect( reply, &QNetworkReply::finished, [ this, reply ]() mutable {
        const Type type = static_cast<Type>( reply->request().attribute( QNetworkRequest::User ).toInt());
        const QVariant userData( reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 )));

        // abort on errors
        if ( reply->error()) {
            reply->deleteLater();
            return;
        }

        // handle redirects
        if ( !reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).isNull())
            this->execute( reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toString(), type, userData );

        // emit downloaded data
        const QByteArray data( reply->readAll());
        emit this->finished( reply->url().toString(), type, userData, data );

        // clean up
        reply->disconnect( reply, SIGNAL( finished()));
        reply->deleteLater();
        reply = nullptr;
    } );
}
