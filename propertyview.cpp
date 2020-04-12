/*
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
#include "propertyview.h"
#include "variable.h"
#include <QHeaderView>
#include "property.h"
#include "propertydock.h"
#include "tag.h"

/**
 * @brief PropertyView::PropertyView
 * @param parent
 */
PropertyView::PropertyView( QWidget *parent ) : QTableView( parent ) {
    this->setModel( Property::instance());
    this->hideColumn( Property::ID );
    this->hideColumn( Property::TagId );
    this->hideColumn( Property::ReagentId );
    this->hideColumn( Property::TableOrder );
    this->delegate = new PropertyDelegate( this );
    this->setItemDelegateForColumn( Property::PropertyData, this->delegate );
    this->setItemDelegateForColumn( Property::Name, this->delegate );

    PropertyView::connect( this->horizontalHeader(), &QHeaderView::sectionResized,
                           [ this ]( const int column, const int oldWidth, const int newWidth ) {
                               if ( oldWidth == newWidth )
                                   return;

                               this->resizeTimer.start( 200 );
                               this->m_resizeInProgress = true;
                               if ( column == Property::Name )
                                   this->resizeToContents();
                           } );

    this->horizontalHeader()->show();
    //this->resizeRowsToContents();

    // setup timer
    this->resizeTimer.setSingleShot( true );
    QTimer::connect( &this->resizeTimer, &QTimer::timeout, [ this ]() {
        this->m_resizeInProgress = false;
        this->setWidgetsEnabled( true );
        this->resizeToContents();
    } );
}

/**
 * @brief PropertyView::resizeEvent
 * @param event
 */
void PropertyView::resizeEvent( QResizeEvent *event ) {
    this->setWidgetsEnabled( false );
    QTableView::resizeEvent( event );
}

/**
 * @brief PropertyView::setWidgetsEnabled
 * @param enabled
 */
void PropertyView::setWidgetsEnabled( bool enabled ) {
    for ( int y = 0; y < Property::instance()->count(); y++ ) {
        QWidget *widget( this->indexWidget( Property::instance()->index( y, Property::PropertyData )));
        if ( widget != nullptr )
            widget->setUpdatesEnabled( enabled );
    }
}

/**
 * @brief PropertyView::resizeToContents
 */
void PropertyView::resizeToContents() {
    //qDebug() << "resizeToContents called";

    this->clearDocumentCache();
    for ( int y = 0; y < Property::instance()->count(); y++ ) {
        this->update( Property::instance()->index( y, Property::Name ));
        this->update( Property::instance()->index( y, Property::PropertyData ));
    }

    this->resizeRowsToContents();
}
