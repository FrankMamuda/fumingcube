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

/**
 * @brief PropertyDelegate::setupDocument
 * @param index
 */
void PropertyDelegate::setupDocument( const QModelIndex &index ) const {
    const QTableView *view( qobject_cast<QTableView*>( this->parent()));
    const int width = view != nullptr ? ( static_cast<int>( view->viewport()->width() * ( index.column() == Property::HTML ? 0.66 : 0.33 ))) : 64;

    if ( this->documentMap.contains( index ))
        return;

    QTextDocument *document( new QTextDocument());
    document->setHtml( index.data( Property::HTML ).toString());
    document->setDocumentMargin( 2 );
    document->setTextWidth( width );
    document->setTextWidth( document->idealWidth());
    this->documentMap[index] = document;
}

/**
 * @brief PropertyDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void PropertyDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    const QTableView *view( qobject_cast<QTableView*>( this->parent()));

    // draw custom selection hilight
    if ( option.state & QStyle::State_Selected ) {
        QColor hilight( qApp->palette().highlight().color());
        hilight.setAlpha( 128 );
        painter->fillRect( option.rect, QBrush( qAsConst( hilight )));
    }

    // don't draw anything else if a custom widget is used
    if ( view->indexWidget( index ) != nullptr )
        return;

    // setup html document
    this->setupDocument( index );
    if ( !this->documentMap.contains( index ))
        return;

    // get document
    QTextDocument *document( this->documentMap[index] );

    // draw html
    painter->save();
    painter->translate( option.rect.left(), option.rect.top() + option.rect.height() / 2 - document->size().height() / 2 );
    document->drawContents( painter );
    painter->restore();
}

/**
 * @brief WordItemDelegate::sizeHint
 * @param option
 * @param index
 * @return
 */
QSize PropertyDelegate::sizeHint( const QStyleOptionViewItem &item, const QModelIndex &index ) const {
    // setup html document
    this->setupDocument( index );
    if ( !this->documentMap.contains( index ))
        return QStyledItemDelegate::sizeHint( item, index );

    // return document size
    return this->documentMap[index]->size().toSize();
}
