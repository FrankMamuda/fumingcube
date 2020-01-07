/*
 * Copyright (C) 2017-2018 Factory #12
 * Copyright (C) 2019-2020 Armands Aleksejevs
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
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLoggingCategory>
#include "main.h"

/**
 * @brief The NetworkManager class
 */
class NetworkManager: public QObject {
    Q_OBJECT

public:
    enum Types {
        NoType = -1,
        CIDRequestInitial,
        CIDRequestSimilar,
        DataRequest,
        FormulaRequest,
        FormulaRequestBrowser,
        IUPACName
    };
    Q_ENUM( Types )

    /**
     * @brief instance
     * @return
     */
    static NetworkManager *instance() { static NetworkManager *instance( new NetworkManager()); return instance; }

signals:
    /**
     * @brief finished
     * @param url
     * @param data
     */
    void finished( const QString &url, NetworkManager::Types type, const QVariant &userData, const QByteArray &data );
    void error( const QString &url, NetworkManager::Types type, const QString &errorString );

public slots:
    void execute( const QString &url, Types type, const QVariant &userData = QVariant());

private:
    /**
     * @brief NetworkManager
     */
    explicit NetworkManager() {
        // disable ssl warnings
        QLoggingCategory::setFilterRules( "qt.network.ssl.warning=false" );

        // add to garbage collector
        GarbageMan::instance()->add( this );
    }

    QNetworkAccessManager manager;
};
