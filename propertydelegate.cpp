/*
 * Copyright (C) 2017-2018 Factory #12
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

//
// includes
//
#include "propertydelegate.h"
#include "property.h"
#include "nfpawidget.h"
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include <QApplication>
#include <QPalette>
#include <QTableView>

//
// TODO: precache document, instead of making it in paint and sizeHint
//

/**
 * @brief PropertyDelegate::setupDocument
 * @param index
 */
void PropertyDelegate::setupDocument( const QModelIndex &index ) const {
    const QTableView *view( qobject_cast<QTableView*>( this->parent()));
    const int width = view != nullptr ?
                ( static_cast<int>( view->viewport()->width() * ( index.column() == Property::HTML ? 0.66 : 0.33 ))) :
                64;

        this->document.setHtml( index.data( Property::HTML ).toString());
    this->document.setDocumentMargin( 2 );
    this->document.setTextWidth( width );
    this->document.setTextWidth( this->document.idealWidth());
}

/**
 * @brief PropertyDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void PropertyDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    // setup html document
    this->setupDocument( index );

    // draw custom selection hilight
    if ( option.state & QStyle::State_Selected ) {
        QColor hilight( qApp->palette().highlight().color());
        hilight.setAlpha( 128 );
        painter->fillRect( option.rect, QBrush( qAsConst( hilight )));
    }

    // TODO: must be a better way to detect this
    /*if ( index.column() == Property::Name ) {
        const QString plainName( Property::instance()->name( Property::instance()->row( index )).remove( QRegExp("<[^>]*>" )));
        if ( plainName.contains( "NFPA 704" ))
            return;
    }*/

    // draw html
    painter->save();
    painter->translate( option.rect.left(), option.rect.top() + option.rect.height() / 2 - document.size().height() / 2 );
    //painter->translate( qMax( option.rect.left(), static_cast<int>( option.rect.left() + option.rect.width() / 2 - document.size().width() / 2 )),  option.rect.top() + option.rect.height() / 2 - document.size().height() / 2 );
    document.drawContents( painter );
    painter->restore();
}

/**
 * @brief WordItemDelegate::sizeHint
 * @param option
 * @param index
 * @return
 */
QSize PropertyDelegate::sizeHint( const QStyleOptionViewItem &, const QModelIndex &index ) const {
    // setup html document
    this->setupDocument( index );

    //
    /*if ( index.column() == Property::Name ) {
        const QString plainName( Property::instance()->name( Property::instance()->row( index )).remove( QRegExp("<[^>]*>" )));
        if ( plainName.contains( "NFPA 704" )) {
            NFPAWidget nfpa( QStringList(), nullptr );
            return nfpa.size();
        }
    }*/

    // return document size
    return document.size().toSize();
}
