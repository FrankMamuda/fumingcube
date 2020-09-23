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
#include <QObject>
#include <QMap>
#include <QColor>
#include <QPalette>
#include <QStyleFactory>
#include <QStyle>
#include <QPainter>
#include <QMenu>
#include "networkmanager.h"

/**
 * @brief The SearchEngine class
 */
class SearchEngine : public QObject {
    Q_OBJECT

public:
    explicit SearchEngine( const QString &name = QString()) : m_name( name ) {}

    /**
     * @brief name
     * @return
     */
    [[nodiscard]] QString name() const { return this->m_name; }

    /**
     * @brief url
     * @return
     */
    [[nodiscard]] QString url() const { return this->m_url; }

    /**
     * @brief separator
     * @return
     */
    [[nodiscard]] QString separator() const { return this->m_separator; }

    /**
     * @brief abbreviation
     * @return
     */
    [[nodiscard]] QString abbreviation() const { return this->m_abbreviation; }

    /**
     * @brief icon
     * @return
     */
    [[nodiscard]] QIcon icon() const { return this->m_icon; }

    /**
     * @brief hasIcon
     * @return
     */
    [[nodiscard]] bool hasIcon() const { return !this->m_icon.isNull(); }

    /**
     * @brief generateIcon
     * @param string
     * @return
     */
    [[nodiscard]] static QIcon generateIcon( const QString &string, const int scale = 16 ) {
        QPixmap pixmap( scale, scale );
        pixmap.fill( Qt::transparent );
        QPainter painter( &pixmap );
        QTextOption opt;
        opt.setAlignment( Qt::AlignCenter );
        painter.drawText( QRect( 0, 0, scale, scale ), string, opt );
        return QIcon( pixmap );
    }

    [[nodiscard]] QString searchUrl( const QString &query ) const;

public slots:
    /**
     * @brief setUrl
     * @param url
     */
    void setUrl( const QString &url ) { this->m_url = url; }

    /**
     * @brief setSeparator
     * @param separator
     */
    void setSeparator( const QString &separator ) { this->m_separator = separator; }

    /**
     * @brief setAbbreviation
     * @param abbreviation
     */
    void setAbbreviation( const QString &abbreviation ) { this->m_abbreviation = abbreviation; }

    /**
     * @brief setIcon
     * @param icon
     */
    void setIcon( const QIcon &icon ) { this->m_icon = icon; }

private:
    QString m_name;
    QString m_url;
    QString m_separator;
    QString m_abbreviation;
    QIcon m_icon;
};

/**
 * @brief The SearchEngineManager class
 */
class SearchEngineManager : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY( SearchEngineManager )

public:
    [[maybe_unused]][[nodiscard]] static QMap<QString, QString> availableSearchEngines();

    ~SearchEngineManager();

    // disable move
    SearchEngineManager( SearchEngineManager&& ) = delete;
    SearchEngineManager& operator=( SearchEngineManager&& ) = delete;

    /**
     * @brief instance
     * @return
     */
    static SearchEngineManager *instance() {
        static auto *instance( new SearchEngineManager());
        return instance;
    }

    /**
     * @brief path
     * @return
     */
    [[nodiscard]] QString path() const { return this->m_path; }

    /**
     * @brief iconPath
     * @param name
     * @return
     */
    [[nodiscard]] QString iconPath( const QString &name ) { return this->path() + "/" + name + ".png"; }

    void populateMenu( QMenu *searchMenu, const QString &query ) const;

public slots:
    void loadSearchEngines();
    void replyReceived( const QString &, NetworkManager::Types type, const QVariant &, const QByteArray &data );

private slots:
    void readFromFile( const QString &fileName );

private:
    explicit SearchEngineManager();
    QMap<QString, SearchEngine *> searchEngines;
    QString m_path;
};
