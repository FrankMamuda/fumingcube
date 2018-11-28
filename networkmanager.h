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

#pragma once

//
// includes
//
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLoggingCategory>

/**
 * @brief The NetworkManager class
 */
class NetworkManager: public QObject {
    Q_OBJECT
    Q_ENUMS( Type )

public:
    enum Type {
        NoType = -1,
        BasicProperties,
        Properties
    };

    /**
     * @brief instance
     * @return
     */
    static NetworkManager *instance() { NetworkManager *instance( new NetworkManager()); return instance; }

signals:
    /**
     * @brief finished
     * @param url
     * @param data
     */
    void finished( const QString &url, NetworkManager::Type type, const QVariant &userData, const QByteArray &data );

public slots:
    /**
     * @brief execute
     * @param url
     * @param type
     */
    void execute( const QString &url, Type type, const QVariant &userData = QVariant()) {
        QNetworkRequest request;

        request.setUrl( QUrl::fromEncoded( url.toLocal8Bit()));
        request.setAttribute( QNetworkRequest::User, static_cast<int>( type ));
        request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ), userData );

        this->activeRequests << this->manager.get( request );;
    }

    /**
     * @brief requestCompleted
     * @param reply
     */
    void requestCompleted( QNetworkReply *reply ) {
        int status;
        Type type;
        QVariant userData;
        QUrl url( reply->url());

        if ( reply->error()) {
            qCritical() << this->tr( "request \"%1\" failed: %2" ).arg( url.toEncoded().constData()).arg( reply->errorString());
        } else {
            status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
            type = static_cast<Type>( reply->request().attribute( QNetworkRequest::User ).toInt());
            userData = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ));

            if ( status == 301 || status == 302 || status == 303 || status == 305 || status == 307 || status == 308 ) {
                qWarning()  << this->tr( "request \"%1\" redirected" );
                QString redirectURL( reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toString());
                this->execute( redirectURL, type, userData );
            } else
                emit this->finished( url.toString(), type, userData, reply->readAll());
        }

        this->activeRequests.removeAll( reply );
        reply->deleteLater();
    }

private:
    /**
     * @brief NetworkManager
     */
    NetworkManager() {
        this->connect( &this->manager, SIGNAL( finished( QNetworkReply* )), SLOT( requestCompleted( QNetworkReply* )));

        // disable ssl warnings
        QLoggingCategory::setFilterRules( "qt.network.ssl.warning=false" );
    }

    /**
     * @brief createInstance
     * @return
     */
    static NetworkManager *createInstance() { return new NetworkManager(); }

    QNetworkAccessManager manager;
    QList<QNetworkReply* > activeRequests;
};
