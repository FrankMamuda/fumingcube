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
    if ( this->model() == nullptr || !index.isValid())
        return QStyledItemDelegate::paint( painter, option, index );

    const ReagentView *view( qobject_cast<ReagentView *>( this->parentView()));
    if ( view->isResizeInProgress())
        return;

    // NOTE: not the most elegant way to map data, but it works
    const QString html( this->sourceModel()->data( this->model()->mapToSource( index ), ReagentModel::HTML ).toString() + QString::number( this->sourceModel()->data( this->model()->mapToSource( index ), ReagentModel::DateTime ).toDate().toJulianDay()));
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

    const auto pixmap( this->sourceModel()->data( this->model()->mapToSource( index ), ReagentModel::Pixmap ).value<QPixmap>());
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
    if ( this->model() == nullptr || !index.isValid() || this->parentView() == nullptr )
        return QStyledItemDelegate::sizeHint( option, index );

    const ReagentView *view( qobject_cast<ReagentView *>( this->parentView()));
    if ( view->isResizeInProgress())
        return QSize();

    const QFont font( option.font );
    QDate date( this->sourceModel()->data( this->model()->mapToSource( index ), ReagentModel::DateTime ).toDate());
    const QString dateString( date.isValid() ? QString( " (%1)" ).arg( date.toString( Qt::SystemLocaleShortDate )) : "" );
    const QString html( this->sourceModel()->data( this->model()->mapToSource( index ), ReagentModel::HTML ).toString());

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

    // NOTE: not the most elegant way to map data, but it works
    this->cache[html + QString::number( date.toJulianDay())] = document;
    return QSizeF( document->idealWidth() + ( pixmap.isNull() ? 0 : pixmap.width()),
                   document->size().height()).toSize();
}
