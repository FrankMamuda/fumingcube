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
#include "searchfragment.h"
#include "structurefragment.h"
#include "networkmanager.h"
#include "extractiondialog.h"
#include "ui_searchfragment.h"
#include "listutils.h"
#include "cache.h"
#include "propertyfragment.h"
#include "fragmentnavigation.h"
#include "variable.h"
#include <QCompleter>
#include <QMessageBox>
#include <QStringListModel>

/**
 * @brief SearchFragment::SearchFragment
 * @param parent
 */
SearchFragment::SearchFragment( QWidget *parent ) : Fragment( parent ), ui( new Ui::SearchFragment ) {
    // setup ui
    this->ui->setupUi( this );

    // set tip icons
    const QPixmap pixmap( QIcon::fromTheme( "info" ).pixmap( 16, 16 ));
    const QList<QLabel*> tips( QList<QLabel*>() << this->ui->cacheTipIcon << this->ui->searchTipIcon );
    for ( QLabel *tip : tips )
        tip->setPixmap( pixmap );

    this->history = Variable::compressedString( "searchFragment/history" ).split( ";" );
    this->completer = new QCompleter( this->history, this );
    this->completer->setCaseSensitivity( Qt::CaseInsensitive );
    this->completer->setCompletionMode( QCompleter::InlineCompletion );
    this->ui->identifierEdit->setCompleter( this->completer );

    // setup finished connection to the NetworkManager
    NetworkManager::connect( NetworkManager::instance(), &NetworkManager::finished, this, [ this ]( const QString &, NetworkManager::Types type, const QVariant &, const QByteArray &data ) {
        switch ( type ) {
        case NetworkManager::CIDRequestInitial:
            qDebug() << "network->idList" << this->identifier();
            if ( !this->parseIdListRequest( data )) {
                qDebug() << "  parseIdListRequest failed";

                // try similar structure search if initial request fails
                this->sendSimilarRequest();
                return;
            }
            this->toggleControls( true );
            break;

        case NetworkManager::CIDRequestSimilar:
            this->toggleControls( true );

            qDebug() << "network->idList (similar)" << this->identifier();
            if ( !this->parseIdListRequest( data )) {
                qDebug() << "  parseIdListRequest failed";
                this->host()->setErrorMessage( SearchFragment::tr( "Could not parse request" ));

                // disable fragments
                this->host()->fragmentNavigation()->setFragmentEnabled( this->host()->structureFragment(), false );
                this->host()->fragmentNavigation()->setFragmentEnabled( this->host()->propertyFragment(), false );

                return;
            }
            break;

        default:
            ;
        }
    } );

    // setup error connection to the NetworkManager
    NetworkManager::connect( NetworkManager::instance(), &NetworkManager::error, this, [ this ]( const QString &, NetworkManager::Types type, const QVariant &, const QString &errorMessage ) {
        switch ( type ) {
        case NetworkManager::CIDRequestInitial:
            // try similar structure search if initial request fails
            this->sendSimilarRequest();
            break;

        case NetworkManager::CIDRequestSimilar:
            this->host()->setErrorMessage( errorMessage.contains( "PUGREST.NotFound" ) ? SearchFragment::tr( "Error: could not find requested reagent" ) : SearchFragment::tr( "Error: " ) + errorMessage );

            // disable fragments
            this->disableFragments();

            this->toggleControls( true );
            break;

        default:
            ;
        }
    } );

    // setup fetch action and line edit
    QAction::connect( this->ui->actionFetch, &QAction::triggered, this, &SearchFragment::sendInitialRequest );
    QLineEdit::connect( this->ui->identifierEdit, &QLineEdit::returnPressed, this, &SearchFragment::sendInitialRequest );

    // check name
    auto checkName = [ this ]() { this->ui->actionFetch->setEnabled( !this->ui->identifierEdit->text().isEmpty()); };
    QLineEdit::connect( this->ui->identifierEdit, &QLineEdit::textChanged, checkName );
    checkName();

    // setup cache deletion actions
    QAction::connect( this->ui->actionClear, &QAction::triggered, this, [ this ]() {
        if ( this->identifier().isEmpty())
            return;

        // check for id in cache
        if ( Cache::instance()->nameIdMap.contains( this->identifier().toLower())) {
            // get a list of ids for the identifier
            // make sure to reverse the list, since the most relavant entries are at the end
            const QList<int> idList( Cache::instance()->nameIdMap.values( this->identifier()));

            for ( const int id : idList ) {
                Cache::instance()->clear( Cache::FormulaContext, QString( "%1.png" ).arg( id ));
                Cache::instance()->clear( Cache::NameContext, QString( "%1" ).arg( id ));
                Cache::instance()->clear( Cache::DataContext, QString( "%1.dat" ).arg( id ));
            }
        }
    } );


    // setup cache deletion actions
    QAction::connect( this->ui->actionDeleteCache, &QAction::triggered, this, [ this ]() {
        if ( QMessageBox::question( this, SearchFragment::tr( "Confirm deletion" ), SearchFragment::tr( "Delete all cache?" )) != QMessageBox::Yes )
            return;

        const QList<int> values( Cache::instance()->nameIdMap.values());
        for ( const int id : values ) {
            Cache::instance()->clear( Cache::FormulaContext, QString( "%1.png" ).arg( id ));
            Cache::instance()->clear( Cache::NameContext, QString( "%1" ).arg( id ));
            Cache::instance()->clear( Cache::DataContext, QString( "%1.dat" ).arg( id ));
        }
        Cache::instance()->clear( Cache::IdMapContext, "data.map" );
    } );
}

/**
 * @brief SearchFragment::~SearchFragment
 */
SearchFragment::~SearchFragment() {
    NetworkManager::disconnect( NetworkManager::instance(), &NetworkManager::finished, this, nullptr );
    NetworkManager::disconnect( NetworkManager::instance(), &NetworkManager::error, this, nullptr );
    QAction::disconnect( this->ui->actionFetch, &QAction::triggered, this, &SearchFragment::sendInitialRequest );
    QLineEdit::disconnect( this->ui->identifierEdit, &QLineEdit::returnPressed, this, &SearchFragment::sendInitialRequest );
    QAction::disconnect( this->ui->actionDeleteCache, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionClear, &QAction::triggered, this, nullptr );

    Variable::setCompressedString( "searchFragment/history", this->history.join( ";" ));

    delete this->completer;
    delete this->ui;
}

/**
 * @brief SearchFragment::identifier
 * @return
 */
QString SearchFragment::identifier( bool clean ) const {
    return clean ? this->ui->identifierEdit->text().replace( " ", "-" ) : this->ui->identifierEdit->text();
}

/**
 * @brief SearchFragment::setIdentifier
 * @param string
 */
void SearchFragment::setIdentifier( const QString &string ) {
    this->ui->identifierEdit->setText( string );
}

/**
 * @brief SearchFragment::parseIdList
 * @param idList
 * @return
 */
bool SearchFragment::parseIdList( const QList<int> &idList ) {
    // abort on empty id lists
    if ( idList.isEmpty())
        return false;

    // remove null ids
    QList<int> cleanList( idList );
    cleanList.removeAll( 0 );

    // setup structureBrowser
    this->host()->structureFragment()->setup( qAsConst( cleanList ));

    // save history on success
    if ( !this->history.contains( this->identifier()))
        this->history << this->identifier();

    if ( this->history.count() >= 32 )
        this->history.removeFirst();

    delete this->completer->model();
    this->completer->setModel( new QStringListModel( this->history ));

    // select an id
    if ( qAsConst( cleanList ).count() > 1 || this->host()->mode() == ExtractionDialog::SearchMode ) {
        // if we have multiple ids in the list, open the StructureBrowser and let the user decide
        // NOTE: this is also the default behaviour in SearchMode, because we do need to add a new reagent
        this->host()->setCurrentFragment( this->host()->structureFragment());
        this->host()->structureFragment()->getNameAndFormula();
        return true;
    }

    // if the list has only one entry, continue with that
    const int id = qAsConst( cleanList ).first();

    // make sure it is a valid id
    if ( id <= 0 ) {
        this->host()->setErrorMessage( SearchFragment::tr( "Could not parse request" ));

        // disable fragments
        this->disableFragments();

        return false;
    }

    // all ok, continue with data extraction
    this->host()->setCurrentFragment( this->host()->propertyFragment());
    this->host()->fragmentNavigation()->setFragmentEnabled( this->host()->structureFragment(), false );
    this->host()->propertyFragment()->getDataAndFormula( id );

    // return success
    return true;
}

/**
 * @brief SearchFragment::parseIdListRequest
 * @param data
 * @return
 */
bool SearchFragment::parseIdListRequest( const QByteArray &data ) {
    // get id list from data
    const QList<int>idList( ListUtils::toNumericList<int>( QString( data ).split( "\n" )));

    if ( idList.isEmpty())
        return false;

    // store ids into cache
    for ( const int &id : idList )
        Cache::instance()->nameIdMap.insert( this->identifier().toLower(), id );

    // continue on
    return this->parseIdList( idList );
}

/**
 * @brief SearchFragment::toggleControls
 * @param enabled
 */
void SearchFragment::toggleControls( bool enabled ) {
    this->ui->identifierEdit->setEnabled( enabled );
    this->ui->actionClear->setEnabled( enabled );
    this->ui->actionFetch->setEnabled( enabled );
    this->ui->actionDeleteCache->setEnabled( enabled );
}

/**
 * @brief SearchFragment::disableFragments
 */
void SearchFragment::disableFragments() {
    this->host()->fragmentNavigation()->setFragmentEnabled( this->host()->structureFragment(), false );
    this->host()->fragmentNavigation()->setFragmentEnabled( this->host()->propertyFragment(), false );
}

/**
 * @brief SearchFragment::sendInitialRequest
 */
void SearchFragment::sendInitialRequest() {
    // disable fragments
    this->disableFragments();
    this->host()->clearStatusMessage();

    if ( this->identifier().isEmpty())
#if 1
        return;
#else
        // TODO: delete this when done with debugging
        this->setIdentifier( "dimethylbenzene" );
#endif

    // check for id in cache
    if ( Cache::instance()->nameIdMap.contains( this->identifier().toLower())) {
        // get a list of ids for the identifier
        // make sure to reverse the list, since the most relavant entries are at the end
        QList<int> idList( Cache::instance()->nameIdMap.values( this->identifier().toLower()));
        std::reverse( idList.begin(), idList.end());

        if ( !idList.isEmpty()) {
            qDebug() << "cache->idList";
            if ( this->parseIdList( idList ))
                return;
        }
    }

    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/%1/cids/TXT" ).arg( this->identifier( true )), NetworkManager::CIDRequestInitial );
    this->host()->setStatusMessage( SearchFragment::tr( "Searching for %1 (exact match)" ).arg( this->identifier()));
    this->toggleControls( false );
}

/**
 * @brief SearchFragment::sendSimilarRequest
 */
void SearchFragment::sendSimilarRequest() {
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/%1/cids/TXT?name_type=word" ).arg( this->identifier( true )), NetworkManager::CIDRequestSimilar );
    this->host()->setStatusMessage( SearchFragment::tr( "Searching for %1 (similiar structures)" ).arg( this->identifier()));
}
