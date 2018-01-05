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
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include <QApplication>
#include <QPalette>

/**
 * @brief PropertyDelegate::PropertyDelegate
 * @param parent
 */
PropertyDelegate::PropertyDelegate( QObject *parent ) :  QStyledItemDelegate( parent ) {
}

/**
 * @brief PropertyDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void PropertyDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    QStyleOptionViewItem customOptions( option );
    QTextDocument document;

    // init style options from index
    this->initStyleOption( &customOptions, index );

    // save painter state due to translation manipulations
    painter->save();

    // set html from property
    document.setHtml( customOptions.text );

    // draw custom selection hilight
    if ( option.state & QStyle::State_Selected ) {
        QColor hilight( qApp->palette().highlight().color());
        hilight.setAlpha( 128 );

        painter->fillRect( option.rect, QBrush( hilight ) );
    }

    // align text in the middle
    //painter->translate( customOptions.rect.left() + customOptions.rect.width() / 2 - document.idealWidth() / 2,
    //                    customOptions.rect.top() + customOptions.rect.height() / 2 - document.size().height() / 2 );
    painter->translate( customOptions.rect.left(), customOptions.rect.top());
    QRect rect( 0, 0, customOptions.rect.width(), customOptions.rect.height());

    // paint html
    document.drawContents( painter, rect );

    // restore painter state
    painter->restore();
}

/**
 * @brief WordItemDelegate::sizeHint
 * @param option
 * @param index
 * @return
 */
QSize PropertyDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    QStyleOptionViewItem customOptions( option );
    QTextDocument document;

    // init style options from index
    this->initStyleOption( &customOptions, index );

    // set html from property
    document.setHtml( customOptions.text );
    document.setTextWidth( customOptions.rect.width());
    document.setDocumentMargin(0);

    // return ideal size
    qDebug() << customOptions.text << QSizeF( document.idealWidth(), document.size().height()).toSize();
    return document.documentLayout()->documentSize().toSize();//QSizeF( document.idealWidth(), document.size().height()).toSize();
}
