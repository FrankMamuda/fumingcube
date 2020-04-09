/*
 * Copyright (C) 2019 Armands Aleksejevs
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
#include "searchengine.h"
#include <QColor>
#include <QDesktopServices>
#include <QDir>
#include <QRegularExpression>
#include <QSettings>
#include "networkmanager.h"
#include "variable.h"
#include <QApplication>

/**
 * @brief SearchEngineManager::SearchEngineManager
 */
SearchEngineManager::SearchEngineManager() {
    NetworkManager::connect( NetworkManager::instance(), SIGNAL( finished( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )), this, SLOT( replyReceived( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )));

    // make cache dir
    this->m_path = QDir( QDir::homePath() + "/" + Main::Path + "/cache/icons/" ).absolutePath();
    const QDir dir( this->path());
    if ( !dir.exists()) {
        dir.mkpath( dir.absolutePath());
        if ( !dir.exists())
            return;
    }

    // add to garbage collector
    GarbageMan::instance()->add( this );
}

/**
 * @brief SearchEngineManager::availableSearchEngines
 * @return
 */
QMap<QString, QString> SearchEngineManager::availableSearchEngines() {
    QMap<QString, QString> searchEngines;

    QDir dir( QString( ":/searchEngines/" ));
    dir.setNameFilters( QStringList() << "*.conf" );

    const QStringList internalList( dir.entryList( QDir::Files | QDir::NoDotDot ));
    for ( const QString &name : internalList )
        searchEngines[QString( name ).remove( ".conf" )] = dir.filePath( name );

    // extenal search engines take priority (override internal search engines)
    dir.setPath( QDir::currentPath() + "/searchEngines/" );
    const QStringList externalList( dir.entryList( QDir::Files | QDir::NoDotDot ));
    for ( const QString &name : externalList )
        searchEngines[QString( name ).remove( ".conf" )] = dir.absoluteFilePath( name );

    return searchEngines;
}

/**
 * @brief SearchEngineManager::~SearchEngineManager
 */
SearchEngineManager::~SearchEngineManager() {
    NetworkManager::disconnect( NetworkManager::instance(), SIGNAL( finished( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )), this, SLOT( replyReceived( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )));
    qDeleteAll( this->searchEngines );
}

/**
 * @brief SearchEngineManager::populateMenu
 * @param searchMenu
 */
void SearchEngineManager::populateMenu( QMenu *searchMenu, const QString &query ) const {
    searchMenu->setIcon( QIcon::fromTheme( "structure" ));

    const QList<SearchEngine *> engines( this->searchEngines.values());
    for ( SearchEngine *engine : engines ) {
        if ( !engine->hasIcon() && !engine->abbreviation().isEmpty())
            engine->setIcon( SearchEngine::generateIcon( engine->abbreviation()));

        searchMenu->addAction( engine->name(), this, [ engine, query ]() {
            QDesktopServices::openUrl( engine->searchUrl( query ));
        } )->setIcon( engine->icon());
    }
}

/**
 * @brief SearchEngineManager::loadSearchEngines
 */
void SearchEngineManager::loadSearchEngines() {
    const QMap<QString, QString> searchEngines( this->availableSearchEngines());

    const QStringList fileNames( searchEngines.values());
    for ( const QString &fileName : fileNames ) {
        if ( QFileInfo::exists( fileName ))
            this->readFromFile( fileName );
    }
}

/**
 * @brief SearchEngineManager::replyReceived
 * @param type
 * @param data
 */
void SearchEngineManager::replyReceived( const QString &, NetworkManager::Types type, const QVariant &userData, const QByteArray &data ) {
    switch ( type ) {
    case NetworkManager::FavIcon:
    {
        // make sure we use a valid name
        const QString name( userData.toString());
        if ( name.isEmpty() || !this->searchEngines.contains( name ))
            return;

        // make a pixmap from data
        QPixmap pixmap;
        pixmap.loadFromData( data );

        // find the search engine
        SearchEngine *engine( this->searchEngines[name] );

        // if pixmap is invalid, return
        if ( pixmap.isNull())
            return;

        // save icon to cache and add it to the search engine
        pixmap.save( this->iconPath( name ));
        engine->setIcon( QIcon( pixmap ));
    }
        break;

    default:
        ;
    }
}

/**
 * @brief SearchEngineManager::readFromFile
 * @param fileName
 */
void SearchEngineManager::readFromFile( const QString &fileName ) {
    // open settings file (plain INI format)
    QSettings settings( fileName, QSettings::IniFormat );

    // parse general settings
    settings.beginGroup( "SearchEngine" );

    QString name;
    QString url;
    QString domain;
    QString separator;
    QString abbreviation;

    for ( const QString &key : settings.allKeys()) {
        if ( !QString::compare( key, "Name" ))
            name = settings.value( key ).toString();

        if ( !QString::compare( key, "URL" )) {
            url = settings.value( key ).toString();

            const QRegularExpression re( R"(^(?:https?:\/\/)?(?:[^@\/\n]+@)?(?:www\.)?([^:\/?\n]+))" );
            const QRegularExpressionMatch match( re.match( url ));

            if ( match.hasMatch())
                domain = match.captured( 1 );
        }

        if ( !QString::compare( key, "Separator" ))
            separator = settings.value( key ).toString();

        if ( !QString::compare( key, "Abbreviation" ))
            abbreviation = settings.value( key ).toString();
    }

    // make sure we have a valid name and url
    if ( name.isEmpty() || url.isEmpty() || domain.isEmpty())
        return;

    // override any previous entries
    if ( this->searchEngines.contains( name ))
        delete this->searchEngines.take( name );

    // make a new search engine
    SearchEngine *engine( new SearchEngine( name ));
    this->searchEngines[name] = engine;

    // get icon
    if ( QFileInfo::exists( this->iconPath( name )))
        engine->setIcon( QIcon( this->iconPath( name )));
    else
        NetworkManager::instance()->execute( QString( "https://icons.duckduckgo.com/ip3/%1.ico" ).arg( domain ), NetworkManager::FavIcon, name );

    // set values
    engine->setUrl( url );
    engine->setSeparator( separator );
    engine->setAbbreviation( abbreviation );

    settings.endGroup();
}

/**
 * @brief SearchEngine::searchUrl
 * @param query
 * @return
 */
QString SearchEngine::searchUrl( const QString &query ) const {
    return this->url().replace( ":query", ( this->separator().isEmpty() ? query : QString( query ).replace( " ", this->separator())));
}
