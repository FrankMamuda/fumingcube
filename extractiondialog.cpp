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
#include "reagent.h"
#include "htmlutils.h"

// TODO: disable structure and property fragment on error

/**
 * @brief ExtractionDialog::ExtractionDialog
 * @param parent
 */
ExtractionDialog::ExtractionDialog( QWidget *parent, const Id &reagentId ) : QDialog( parent ), ui( new Ui::ExtractionDialog ), m_reagentId( reagentId ) {
    // setup ui and get rid of verticalHeader in property view
    this->ui->setupUi( this );
    this->ui->ExtractionDialogContents->setWindowFlags( Qt::Widget );

    // if invalid reagentId is passed, default to SearchMode (find and add new reagents and their properties)
    // otherwise set ExistingMode (get properties for existing reagents)
    this->m_mode = ( this->reagentId() == Id::Invalid ) ? SearchMode : ExistingMode;

    // setup fragment navigation and initialize fragments
    this->fragmentNavigation()->setFragmentHost( this->fragmentHost());
    this->m_searchFragment = this->fragmentNavigation()->addFragment<SearchFragment>( ExtractionDialog::tr( "Search" ), QIcon::fromTheme( "find" ), this, ExtractionDialog::tr( "Switch to the Search fragment" ));
    this->m_structureFragment = this->fragmentNavigation()->addFragment<StructureFragment>( ExtractionDialog::tr( "Structure\nbrowser" ), QIcon::fromTheme( "structure" ), this, ExtractionDialog::tr( "Switch to the Structure browser fragment" ));
    this->m_propertyFragment = this->fragmentNavigation()->addFragment<PropertyFragment>( ExtractionDialog::tr( "Properties" ), QIcon::fromTheme( "property" ), this, ExtractionDialog::tr( "Switch to the Property extraction fragment" ));
    this->setCurrentFragment( this->searchFragment());
    this->fragmentNavigation()->installCloseButton( this );

    if ( this->reagentId() != Id::Invalid )
        this->searchFragment()->setIdentifier( HTMLUtils::convertToPlainText( Reagent::instance()->name( this->reagentId())));

    // hide initial fragments for now
    this->setFragmentEnabled( this->structureFragment(), false );
    this->setFragmentEnabled( this->propertyFragment(), false );
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

/**
 * @brief ExtractionDialog::setStatusMessage
 * @param message
 */
void ExtractionDialog::setStatusMessage( const QString &message ) {
    this->ui->statusbar->setStyleSheet( "" );
    this->ui->statusbar->showMessage( message );
}

/**
 * @brief ExtractionDialog::setStatusErrorMessage
 * @param message
 */
void ExtractionDialog::setErrorMessage( const QString &message ) {
    this->ui->statusbar->setStyleSheet( "background-color: red; color: white;" );
    this->ui->statusbar->showMessage( message );
}

/**
 * @brief ExtractionDialog::clearStatusMessage
 */
void ExtractionDialog::clearStatusMessage() {
    this->ui->statusbar->setStyleSheet( "" );
    this->ui->statusbar->clearMessage();
}
