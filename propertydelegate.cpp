/*
 * Copyright (C) 2017-2018 Factory #12
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
#include "propertydelegate.h"
#include "property.h"
#include "textedit.h"
#include "tag.h"
#include "variable.h"
//#include "nfpawidget.h"
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
void PropertyDelegate::setupDocument( const QModelIndex &index, const QFont &font ) const {
    if ( this->documentMap.contains( index ))
        return;

    const Row row = Property::instance()->row( index );
    const Id tagId =  Property::instance()->tagId( row );

    QTextDocument *document( new QTextDocument());
    if ( tagId != Id::Invalid ) {
        const Row tagRow = Tag::instance()->row( tagId );
        if ( tagRow == Row::Invalid )
            return;

        const QString units( Tag::instance()->units( tagRow ));
        const QString data( index.column() == Property::Name ? Tag::instance()->name( tagRow ) : ( TextEdit::stripHTML( Property::instance()->valueData( row ).constData() + units )));

        document->setHtml( QString( "<p style=\"font-size: %1pt; font-family: '%2'\">" )
                           .arg( font.pointSize())
                           .arg( font.family())
                           + data
                           + "<\\p>" );
    } else {
        document->setHtml(( index.column() == Property::Name ) ? Property::instance()->name( row ) : Property::instance()->valueData( row ).constData());
    }

    const QTableView *view( qobject_cast<QTableView*>( this->parent()));
    document->setDocumentMargin( 2 );
    document->setTextWidth( view != nullptr ? static_cast<int>( view->columnWidth( index.column())) : 128 );
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
    this->setupDocument( index, painter->font());
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
  //  qDebug() << "size hint";

    // setup html document
    this->setupDocument( index, item.font );
    if ( !this->documentMap.contains( index ))
        return QStyledItemDelegate::sizeHint( item, index );

    // return document size
    //qDebug() << "height" <<this->documentMap[index]->size().toSize();
    return this->documentMap[index]->size().toSize();
}
