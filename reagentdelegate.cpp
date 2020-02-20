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
#include "reagentdelegate.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>

/**
 * @brief ReagentDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void ReagentDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    if ( this->model() == nullptr || !index.isValid())
        return QStyledItemDelegate::paint( painter, option, index );

    const QString html(
            this->sourceModel()->data( this->model()->mapToSource( index ), ReagentModel::HTML ).toString());
    if ( !this->cache.contains( html ))
        return QStyledItemDelegate::paint( painter, option, index );

    // draw custom selection highlight
    if ( option.state & QStyle::State_Selected ) {
        QColor highlight( QApplication::palette().highlight().color());
        highlight.setAlpha( 128 );
        painter->fillRect( option.rect, QBrush( qAsConst( highlight )));
    }

    // get document
    QTextDocument *document( this->cache[html] );
    painter->save();

    const auto pixmap(
            this->sourceModel()->data( this->model()->mapToSource( index ), ReagentModel::Pixmap ).value<QPixmap>());
    if ( !pixmap.isNull())
        painter->drawPixmap(
                QRect( option.rect.left(), option.rect.top() + option.rect.height() / 2 - pixmap.height() / 2,
                       pixmap.width(), pixmap.height()), pixmap );

    painter->translate( option.rect.left() + ( pixmap.isNull() ? 0 : pixmap.width()),
                        option.rect.top() + static_cast<int>( option.rect.height() / 2 ) - document->size().height() / 2 );
    document->drawContents( painter );
    painter->restore();
}

/**
 * @brief ReagentDelegate::sizeHint
 * @param option
 * @param index
 * @return
 */
QSize ReagentDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    if ( this->model() == nullptr || !index.isValid())
        return QStyledItemDelegate::sizeHint( option, index );

    const QFont font( option.font );
    const QString html(
            this->sourceModel()->data( this->model()->mapToSource( index ), ReagentModel::HTML ).toString());
    auto *document( new QTextDocument());
    document->setHtml( QString( R"(<p style="font-size: %1pt; font-family: '%2'">%3<\p>)" ).arg( font.pointSize()).arg(
            font.family()).arg( html ));
    this->cache[html] = document;

    const auto pixmap(
            this->sourceModel()->data( this->model()->mapToSource( index ), ReagentModel::Pixmap ).value<QPixmap>());
    return QSizeF( document->idealWidth() + ( pixmap.isNull() ? 0 : pixmap.width()),
                   document->size().height()).toSize();
}
