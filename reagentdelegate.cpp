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
#include "reagent.h"
#include "reagentdelegate.h"
#include "reagentdock.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QDate>

/**
 * @brief ReagentDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void ReagentDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    // abort on invalid indexes
    if (( this->model() == nullptr && !this->viewMode()) || !index.isValid())
        return QStyledItemDelegate::paint( painter, option, index );

    // abort if resizing
    if ( !this->viewMode()) {
        const ReagentView *view( qobject_cast<ReagentView *>( this->parentView()));
        if ( view->isResizeInProgress())
            return;
    }

    // draw custom selection highlight
    if ( option.state & QStyle::State_Selected ) {
        QColor highlight( QApplication::palette().highlight().color());
        highlight.setAlpha( 128 );
        painter->fillRect( option.rect, QBrush( qAsConst( highlight )));
    }

    // get the document
    QTextDocument *document( this->cache[index] );
    if ( document == nullptr )
        return;

    // save painter state (we do transformations)
    painter->save();

    // adjust position
    int x = option.rect.left();
    if ( !this->viewMode()) {
        const auto pixmap( this->sourceModel()->data( this->model()->mapToSource( index ), ReagentModel::Pixmap ).value<QPixmap>());
        if ( !pixmap.isNull())
            painter->drawPixmap( QRect(
                                     option.rect.left(),
                                     option.rect.top() + option.rect.height() / 2 - pixmap.height() / 2,
                                     pixmap.width(),
                                     pixmap.height()), pixmap );

        x += option.rect.left() + ( pixmap.isNull() ? 0 : pixmap.width());
    }

    // adjust position
    //i//f ( document->defaultTextOption().alignment() == Qt::AlignRight )
    //    x += 8;
    painter->translate( x, option.rect.top() + static_cast<int>( option.rect.height() / 2 ) - document->size().height() / 2 );

    // draw the actual html
    document->drawContents( painter );

    // restore state
    painter->restore();
}

/**
 * @brief ReagentDelegate::sizeHint
 * @param option
 * @param index
 * @return
 */
QSize ReagentDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    const QSize defaultSize( QStyledItemDelegate::sizeHint( option, index ));

    // viewMode fetches html directtly from the reagent
    if ( this->viewMode()) {
        if ( !index.isValid() || index.data( Qt::DisplayRole ).isNull())
            return defaultSize;

        // get reagent id
        const Id id = index.data( Qt::DisplayRole ).value<Id>();
        if ( id == Id::Invalid )
            return defaultSize;

        // get reagent parentId
        const Id parentId = Reagent::instance()->parentId( id );

        // setup html document and save to cache
        QTextDocument *document( new QTextDocument());
        document->setHtml( QString( parentId != Id::Invalid ? "&nbsp;&nbsp;&nbsp;&nbsp;" : "" ) + Reagent::instance()->name( id ) + ( parentId == Id::Invalid ? "" : QString( " (%1)" ).arg( Reagent::instance()->name( parentId ))));

        if ( parentId != Id::Invalid ) {
            QFont font( this->internalFont );
            font.setItalic( true );
            document->setDefaultFont( font );
        } else {
            document->setDefaultFont( this->internalFont );
        }

        this->cache[index] = document;

        // return document size
        return document->size().toSize();
    }

    // abort on invalid indexes
    if ( this->model() == nullptr || !index.isValid() || this->parentView() == nullptr )
        return defaultSize;

    const ReagentView *view( qobject_cast<ReagentView *>( this->parentView()));
    if ( view->isResizeInProgress())
        return QSize();

    // otherwise html is fetched through model
    const QFont font( this->internalFont );
    QDate date( this->sourceModel()->data( this->model()->mapToSource( index ), ReagentModel::DateTime ).toDate());
    const QString dateString( date.isValid() ? QString( " (%1)" ).arg( date.toString( Qt::SystemLocaleShortDate )) : "" );
    const QString html( this->sourceModel()->data( this->model()->mapToSource( index ), ReagentModel::HTML ).toString());

    // setup html document
    auto *document( new QTextDocument());
    const auto pixmap( this->sourceModel()->data( this->model()->mapToSource( index ), ReagentModel::Pixmap ).value<QPixmap>());
    QTextOption textOption( document->defaultTextOption());
    textOption.setWrapMode( QTextOption::WordWrap );
    document->setDefaultTextOption( textOption );
    document->setTextWidth( this->parentView()->columnWidth( 0 ) - this->parentView()->indentation() - ( pixmap.isNull() ? 0 : pixmap.width()));
    document->setHtml( QString( R"(<p style="font-size: %1pt; font-family: '%2'">%3</p>)" )
                       .arg( QString::number( font.pointSize()),
                             font.family(),
                             html + dateString ));

    // save to cache
    this->cache[index] = document;

    // return adjusted document size
    return QSizeF( document->idealWidth() + ( pixmap.isNull() ? 0 : pixmap.width()),
                   document->size().height()).toSize();
}
