/*
 * Copyright (C) 2016 Zvaigznu Planetarijs
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
#include "singleton.h"
#include <QObject>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

//
// namespace Network
//
namespace Network {
static const int MaxRequests = 10;
}

/**
 * @brief The NetworkManager class
 */
class NetworkManager : public QObject {
    Q_OBJECT
    Q_ENUMS( Type )

public:
    enum Type {
        NoType = -1,
        HTML
    };

    static NetworkManager *instance() { return Singleton<NetworkManager>::instance( NetworkManager::createInstance ); }
    ~NetworkManager();
    bool isRunning() const { return this->m_running; }

signals:
    void finished( const QString &url, Type type, const QVariant &userData, QByteArray data, bool error );
    void stopped();

public slots:
    void add( const QString &url, Type type = HTML, const QVariant &userData = QVariant(), bool priority = false );
    void execute( const QString &url, Type type = HTML, const QVariant &userData = QVariant(), bool priority = false ) { this->add( url, type, userData, priority ); this->run(); }
    void run();
    void stop() { this->m_running  = false; }
    void clear();

private slots:
    void replyReceived( QNetworkReply *reply );

private:
    explicit NetworkManager( QObject *parent = nullptr );
    static NetworkManager *createInstance() { return new NetworkManager(); }
    QNetworkAccessManager *accessManager;
    bool m_running;
    QList<QNetworkRequest> activeRequests;
    QList<QNetworkRequest> requestList;
};
