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
#include "propertyview.h"
#include "variable.h"
#include <QHeaderView>
#include "property.h"
#include "propertydock.h"

/**
 * @brief PropertyView::PropertyView
 * @param parent
 */
PropertyView::PropertyView( QWidget *parent ) : QTableView( parent ) {
    this->setModel( Property::instance());
    this->hideColumn( Property::ID );
    this->hideColumn( Property::TagID );
    this->hideColumn( Property::ReagentID );
    this->hideColumn( Property::Index );
    this->delegate = new PropertyDelegate( this );
    this->setItemDelegateForColumn( Property::Value, this->delegate );
    this->setItemDelegateForColumn( Property::Name, this->delegate );

    this->connect( this->horizontalHeader(), &QHeaderView::sectionResized, [ this ]( const int column, const int oldWidth, const int newWidth ) {
        if ( oldWidth == newWidth )
            return;

        if ( column == Property::Name ) {
            Variable::instance()->setInteger( "propertyNameColumnSize", newWidth );
            this->resizeToContents();
        }
    } );

    this->horizontalHeader()->show();
    //this->resizeToContents();
}

/**
 * @brief PropertyView::resizeEvent
 * @param event
 */
void PropertyView::resizeEvent( QResizeEvent *event ) {
    QTableView::resizeEvent( event );
    this->resizeToContents();
}

/**
 * @brief PropertyView::resizeToContents
 */
void PropertyView::resizeToContents() {
    PropertyDock::instance()->clearDocumentCache();
    this->setColumnWidth( Property::Name, Variable::instance()->integer( "propertyNameColumnSize" ));
    this->resizeRowsToContents();
}
