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
#include "imageutils.h"
//#include "nfpawidget.h"
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include <QApplication>
#include <QPalette>
#include <QTableView>
#include <QBuffer>
#include "propertydock.h"

/**
 * @brief PropertyDelegate::setupDocument
 * @param index
 */
void PropertyDelegate::setupDocument( const QModelIndex &index, const QFont &font ) const {
    // reuse document from cache if any
    if ( this->documentMap.contains( index ) || !index.isValid())
        return;

    // get property row, data and tagId
    const Row row = Property::instance()->row( index );
    const QVariant data( Property::instance()->propertyData( row ));
    const Id tagId = Property::instance()->tagId( row );

    // create a new document
    QTextDocument *document( new QTextDocument());

    bool pixmap = false;
    if ( tagId != Id::Invalid )
        pixmap = Tag::instance()->type( tagId ) == Tag::Formula || tagId == PixmapTag;

    // special handling of pixmaps
    if ( pixmap && index.column() == Property::PropertyData ) {
        QByteArray pixmapData( data.toByteArray());
        QPixmap propertyPixmap;
        propertyPixmap.loadFromData( pixmapData );

        // NOTE: experimental dark mode converter
        const bool darkMode = Variable::instance()->isEnabled( "darkMode" );
        if ( darkMode && Tag::instance()->type( tagId ) == Tag::Formula ) {
            if ( !this->cache.contains( pixmapData )) {
                propertyPixmap = ImageUtils::invertPixmap( ImageUtils::autoCropPixmap( propertyPixmap ));
                QByteArray replacedData;
                QBuffer buffer( &replacedData );
                buffer.open( QIODevice::WriteOnly );
                propertyPixmap.save( &buffer, "PNG" );
                buffer.close();

                this->cache[pixmapData] = replacedData;
                pixmapData = replacedData;
            } else {
                pixmapData = this->cache[pixmapData];
            }
        }

        // scale pixmap to fit property view value column
        const qreal aspect = static_cast<qreal>( propertyPixmap.height()) / static_cast<qreal>( propertyPixmap.width());
        const int preferredWidth = qMin( PropertyDock::instance()->sectionSize( 1 ), propertyPixmap.width());
        const int preferredHeight = static_cast<int>( preferredWidth * aspect );
        document->setHtml( QString( "<img width=\"%1\" height=\"%2\" src=\"data:image/png;base64,%3\">" ).arg( preferredWidth ).arg( preferredHeight ).arg( pixmapData.toBase64().constData()));
    } else {
        QString html;

        if ( tagId == Id::Invalid || pixmap ) {
            // custom properties however do display their names
            html = ( index.column() == Property::Name ) ? Property::instance()->name( row ) : data.toString();
        } else {
            // properties with built-in tags don't use property names, but rather tag names
            const QString units( Tag::instance()->units( tagId ));
            html = ( index.column() == Property::Name ? Tag::instance()->name( tagId ) : ( TextEdit::stripHTML( data.toString() + units )));
        }

        document->setHtml( QString( "<p style=\"font-size: %1pt; font-family: '%2'\">%3<\\p>" ).arg( font.pointSize()).arg( font.family()).arg( qAsConst( html )));
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
    // setup html document
    this->setupDocument( index, item.font );
    if ( !this->documentMap.contains( index ))
        return QStyledItemDelegate::sizeHint( item, index );

    // return document size
    return this->documentMap[index]->size().toSize();
}
