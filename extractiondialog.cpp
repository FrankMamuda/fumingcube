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

/*
 * includes
 */
#include "extractiondialog.h"
#include "ui_extractiondialog.h"
#include "cache.h"
#include "fragmentnavigation.h"
#include "fragment.h"
#include "searchfragment.h"
#include "structurefragment.h"
#include "propertyfragment.h"

/**
 * @brief ExtractionDialog::ExtractionDialog
 * @param parent
 */
ExtractionDialog::ExtractionDialog( QWidget *parent, const Id &, const Modes &mode ) : QDialog( parent ), ui( new Ui::ExtractionDialog ), m_mode( mode )/*, m_reagentId( reagentId )*/ {
    // setup ui and get rid of verticalHeader in property view
    this->ui->setupUi( this );
    this->ui->ExtractionDialogContents->setWindowFlags( Qt::Widget );

#if 0
    this->buttonTest();

    // forced cid
    if ( cid > 0 ) {
        this->ui->nameEdit->setText( "cid_" + QString::number( cid ));
        this->ui->nameEdit->hide();
        this->ui->cidEdit->setText( QString::number( cid ));
        this->ui->cidEdit->setDisabled( true );

        // begin data request immediately
        this->getFormula( QString::number( cid ));

        // GET DATA
        if ( !this->readFromCache())
            NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/%1/JSON" ).arg( cid ), NetworkManager::DataRequest );
    } 
#endif

    this->m_mode = ExistingMode;

    this->fragmentNavigation()->setFragmentHost( this->fragmentHost());
    this->m_searchFragment = this->fragmentNavigation()->addFragment<SearchFragment>( ExtractionDialog::tr( "Search" ), QIcon::fromTheme( "find" ), this, ExtractionDialog::tr( "Switch to the Search fragment" ));
    this->m_structureFragment = this->fragmentNavigation()->addFragment<StructureFragment>( ExtractionDialog::tr( "Structure\nbrowser" ), QIcon::fromTheme( "reagent" ), this, ExtractionDialog::tr( "Switch to the Structure browser fragment" ));
    this->m_propertyFragment = this->fragmentNavigation()->addFragment<PropertyFragment>( ExtractionDialog::tr( "Properties" ), QIcon::fromTheme( "property" ), this, ExtractionDialog::tr( "Switch to the Property extraction fragment" ));
    this->setCurrentFragment( this->searchFragment());
    this->fragmentNavigation()->installCloseButton( this );

    this->setFragmentEnabled( this->structureFragment(), false );
    this->setFragmentEnabled( this->propertyFragment(), false );

    // set reagent name
    //this->searchFragment()->setIdentifier( HTMLUtils::convertToPlainText( Reagent::instance()->name( reagentId )));
}

/**
 * @brief ExtractionDialog::~ExtractionDialog
 */
ExtractionDialog::~ExtractionDialog() {
    // delete ui
    delete this->ui;
}

/**
 * @brief ExtractionDialog::searchFragment
 * @return
 */
SearchFragment *ExtractionDialog::searchFragment() const {
    return this->m_searchFragment;
}

/**
 * @brief ExtractionDialog::structureFragment
 * @return
 */
StructureFragment *ExtractionDialog::structureFragment() const {
    return this->m_structureFragment;
}

/**
 * @brief ExtractionDialog::propertyFragment
 * @return
 */
PropertyFragment *ExtractionDialog::propertyFragment() const {
    return this->m_propertyFragment;
}

/**
 * @brief ExtractionDialog::fragmentHost
 * @return
 */
QStackedWidget *ExtractionDialog::fragmentHost() const {
    return this->ui->fragmentHost;
}

/**
 * @brief ExtractionDialog::fragmentNavigation
 * @return
 */
FragmentNavigation *ExtractionDialog::fragmentNavigation() const {
    return this->ui->fragmentNavigation;
}

/**
 * @brief ExtractionDialog::setCurrentFragment
 * @param fragment
 */
void ExtractionDialog::setCurrentFragment( Fragment *fragment ) {
    this->fragmentNavigation()->setCurrentFragment( fragment );
}

/**
 * @brief ExtractionDialog::setFragmentEnabled
 * @param fragment
 * @param enabled
 */
void ExtractionDialog::setFragmentEnabled( Fragment *fragment, bool enabled ) {
    this->fragmentNavigation()->setFragmentEnabled( fragment, enabled );
}

#if 0

/**
 * @brief ExtractionDialog::buttonTest
 */
void ExtractionDialog::buttonTest() {
    this->ui->addAllButton->setDisabled( true );
    this->ui->addSelectedButton->setDisabled( true );
    this->ui->clearCacheButton->setDisabled( true );
    this->ui->extractButton->setDisabled( true );

    if ( this->status() == Idle ) {
        if ( this->ui->propertyView->model()->rowCount() > 1 ) {
            this->ui->addAllButton->setEnabled( true );
            this->ui->addSelectedButton->setEnabled( true );
        }

        this->ui->clearCacheButton->setEnabled( true );
        this->ui->extractButton->setEnabled( true );
        return;
    }
}


/**
 * @brief ExtractionDialog::on_clearCacheButton_clicked
 */
void ExtractionDialog::on_clearCacheButton_clicked() {
    if ( this->ui->nameEdit->text().isEmpty())
        return;

    const QString cachedName( this->cachedName());
    Cache::instance()->clear( ExtractionDialog::FormulaContext, cachedName + ".png" );
    Cache::instance()->clear( ExtractionDialog::DataContext, cachedName );
}


/**
 * @brief ExtractionDialog::error
 */
void ExtractionDialog::error( const QString &, NetworkManager::Types type, const QString &/*errorMessage*/ ) {
    //qDebug() << errorMessage;
    this->setStatus( Error );
    this->buttonTest();

    switch ( type ) {
    case NetworkManager::CIDRequestInitial:
    {
        this->ui->cidEdit->setText( ExtractionDialog::tr( "Could not get a valid CID, trying similar" ));

        // get formula
        const QString cachedName( this->cachedName() + ".cid" );
        if ( Cache::instance()->contains( ExtractionDialog::CIDContext, cachedName )) {
            this->cidList = QString( Cache::instance()->getData( ExtractionDialog::CIDContext, cachedName )).split( "\n" );

            QList<int> cidListInt( ListUtils::toNumericList<int>( qAsConst( this->cidList )));
            cidListInt.removeAll( 0 );
            std::sort( cidListInt.begin(), cidListInt.end());
            cidListInt.erase( std::unique( cidListInt.begin(), cidListInt.end()), cidListInt.end());

            if ( cidListInt.count() == 1 ) {
                NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/%1/JSON" ).arg( cidListInt.first()), NetworkManager::DataRequest );
                this->getFormula( QString::number( cidListInt.first()));
            } else
                this->getSimilar( qAsConst( cidListInt ));

            return;
        }

        // initial failed to yield a list, proceed to similar search
        NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/%1/cids/TXT?name_type=word" ).arg( this->ui->nameEdit->text().replace( " ", "-" )), NetworkManager::CIDRequestSimilar );
    }
        break;

    case NetworkManager::CIDRequestSimilar:
        this->ui->cidEdit->setText( ExtractionDialog::tr( "Could not get a valid CID" ));
        break;

    case NetworkManager::DataRequest:
        this->ui->cidEdit->setText( ExtractionDialog::tr( "Could retrieve data associated with CID" ));
        break;

    case NetworkManager::FormulaRequest:
        this->ui->cidEdit->setText( ExtractionDialog::tr( "Could retrieve formula associated with CID" ));
        break;

        // do not handle errors related to other classes
    case NetworkManager::IUPACName:
    case NetworkManager::NoType:
    case NetworkManager::FormulaRequestBrowser:
    case NetworkManager::FavIcon:
        break;
    }
}
#endif

