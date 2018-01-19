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
#include <QLoggingCategory>
#include <QNetworkReply>

/**
 * @brief NetworkManager::NetworkManager
 * @param parent
 */
NetworkManager::NetworkManager( QObject *parent ) : QObject( parent ), accessManager( new QNetworkAccessManager ), m_running( false ) {
    // disable ssl warnings
    QLoggingCategory::setFilterRules( "qt.network.ssl.warning=false" );

    // connect accessManager
    this->connect( this->accessManager, SIGNAL( finished( QNetworkReply * )), this, SLOT( replyReceived( QNetworkReply * )));
}

/**
 * @brief NetworkManager::~NetworkManager
 */
NetworkManager::~NetworkManager() {
    this->disconnect( this->accessManager, SIGNAL( finished( QNetworkReply * )));
    delete this->accessManager;
}

/**
 * @brief NetworkManager::clear
 */
void NetworkManager::clear() {
    this->requestList.clear();
    this->activeRequests.clear();
}

/**
 * @brief NetworkManager::add
 * @param request
 */
void NetworkManager::add( const QString &url, Type type, const QVariant &userData, bool priority ) {
    QNetworkRequest request;

    // create request with requested url, store request type and user data
    request.setAttribute( QNetworkRequest::User, static_cast<int>( type ));
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ), userData );
    request.setUrl( QUrl( url ));

    // push to queue (redirects take priority)
    if ( !priority || this->requestList.isEmpty())
        this->requestList << request;
    else
        this->requestList.insert( 0, request );
}

/**
 * @brief NetworkManager::run
 * @param request
 */
void NetworkManager::run() {
    int free, y;

    // stop if queue is empty
    if ( this->requestList.isEmpty() && this->activeRequests.isEmpty()) {
        this->m_running = false;
        emit this->stopped();
        return;
    } else {
        this->m_running = true;
    }

    // abort if all slots are full
    free = Network::MaxRequests - this->activeRequests.count();
    if ( !free )
        return;

    // determine request count
    if ( free > this->requestList.count())
        free = this->requestList.count();

    // perform request
    for ( y = 0; y < free; y++ ) {
        QNetworkRequest request = this->requestList.takeFirst();
        this->activeRequests << request;
        this->accessManager->get( request );
    }
}

/**
 * @brief NetworkManager::replyReceived
 * @param networkReply
 */
void NetworkManager::replyReceived( QNetworkReply *networkReply ) {
    Type type;
    int statusCode;
    QVariant userData;
    bool error = false, redirect = false;

    // handle errors externally
    if ( networkReply->error() != QNetworkReply::NoError ) {
        qCritical() << networkReply->error();
        error = true;
    }

    // get request type and user data
    type = static_cast<Type>( networkReply->request().attribute( QNetworkRequest::User ).toInt());
    userData = networkReply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ));

    // handle redirects
    statusCode = networkReply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    if ( statusCode == 301 || statusCode == 302 ) {
        QString redirectURL( networkReply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toString());
        this->add( redirectURL, type, userData, true );
        redirect = true;
    }

    // emit signal on finish
    if ( !redirect )
        emit this->finished( networkReply->url().toString(), type, userData, networkReply->readAll(), error );

    // remove request from active queue (must do this manually)
    foreach ( const QNetworkRequest request, this->activeRequests ) {
        Type requestType;

        requestType = static_cast<Type>( request.attribute( QNetworkRequest::User ).toInt());
        if ( request.url() == networkReply->url() && requestType == type ) {
            this->activeRequests.removeOne( request );
            this->requestList.removeOne( request );
            break;
        }
    }

    // continue running
    if ( this->isRunning())
        this->run();

    // clean up
    networkReply->deleteLater();
}
