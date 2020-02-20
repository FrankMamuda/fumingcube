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

        // what this does is:
        //   0) runs when column resize has been finished (200 msec)
        //   1) finds all formulas or other pixmaps
        //   2) removes them from document cache
        //   3) forces propertyView to resize, thus recreating missing documents
        //      a) here pixmaps are rescaled if required and stored into pixmap cache
        //      b) loaded from pixmap cache and displayed
        // all this is done to resolve performance issues in handling large pixmaps
        for ( int y = 0; y < Property::instance()->count(); y++ ) {
            const Row row = Property::instance()->row( y );
            const Id tagId = Property::instance()->tagId( row );
            const bool pixmapTag = ( tagId != Id::Invalid ) ? ( Tag::instance()->type( tagId ) == Tag::Formula ||
                                                                tagId == PixmapTag ) : false;

            if ( !pixmapTag )
                continue;

            const QModelIndex index( Property::instance()->index( y, Property::PropertyData ));
            if ( this->delegate->documentMap.contains( index ))
                this->delegate->documentMap.remove( index );

            this->update( index );
        }
        this->resizeRowsToContents();
    } );
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
    this->resizeRowsToContents();
}
