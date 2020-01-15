/*
 * Copyright (C) 2017-2018 Factory #12
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
#include "propertydelegate.h"
#include "property.h"
#include "textedit.h"
#include "tag.h"
#include "variable.h"
#include "imageutils.h"
#include "cache.h"
#include "reagent.h"
#include "reagentmodel.h"
#include "reagentdock.h"
#include "propertyview.h"
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
    const bool pixmapTag = ( tagId != Id::Invalid ) ? ( Tag::instance()->type( tagId ) == Tag::Formula || tagId == PixmapTag ) : false;
    const PropertyView *view( qobject_cast<PropertyView*>( this->parent()));

    // create a new document
    QTextDocument *document( new QTextDocument());

    // special handling of pixmaps
    if ( pixmapTag && index.column() == Property::PropertyData ) {
        if ( !view->isResizeInProgress()) {
            QSize scaledSize;
            QByteArray pixmapData;

            auto setupPixmap = [ this, data, tagId, &scaledSize, &pixmapData ]() {
                // get pixmap data and calculate a quick checksum for cache access
                const QByteArray storedData( data.toByteArray());
                const quint32 checksum = Cache::checksum( storedData.constData(), static_cast<size_t>( storedData.length()));

                // get sectionWidth (property value column width)
                const int sectionWidth = PropertyDock::instance()->sectionSize( Property::PropertyData );

                // get original pixmap width either by loading from widthCache or by loading pixmap
                QSize originalSize;
                QPixmap pixmap;
                if ( this->sizeCache.contains( checksum )) {
                    // get width from cache
                    originalSize = this->sizeCache[checksum];
                } else {
                    if ( !pixmap.loadFromData( qAsConst( storedData )))
                        return false;

                    // get width from pixmap and store into cache
                    originalSize = pixmap.size();
                    this->sizeCache[checksum] = originalSize;
                }

                // calculate the preferred with (to fit property value column)
                const int scaledWidth = ( originalSize.width() >= sectionWidth ) ? sectionWidth : originalSize.width();
                const int scaledHeight = static_cast<int>(( static_cast<qreal>( scaledWidth ) / static_cast<qreal>( originalSize.width())) * static_cast<qreal>( originalSize.height()));
                scaledSize = QSize( scaledWidth, scaledHeight );

                // generateMipMap lambda - downscales, inverts and crops pixmap
                auto generateMipMap = [ this, /*scaledWidth,*/ checksum, storedData, &pixmap, &pixmapData, tagId, originalSize, &scaledSize ]() {
                    if ( pixmap.isNull()) {
                        if ( !pixmap.loadFromData( qAsConst( storedData )))
                            return false;
                    }

                    // invert and autocrop it if necessary
                    if ( Tag::instance()->type( tagId ) == Tag::Formula ) {
                        pixmap = ImageUtils::autoCropPixmap( qAsConst( pixmap ));
                        if ( Variable::instance()->isEnabled( "darkMode" ))
                            pixmap = ImageUtils::invertPixmap( ImageUtils::autoCropPixmap( qAsConst( pixmap )));
                    }

                    // FAST downscale pixmap
                    if ( scaledSize.width() * 2 < originalSize.width())
                        pixmap = pixmap.scaled( QSize( scaledSize.width() * 2, scaledSize.height() * 2 ), Qt::IgnoreAspectRatio, Qt::FastTransformation );

                    // downscale pixmap
                    pixmap = pixmap.scaled( qAsConst( scaledSize ), Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
                    //qDebug() << "RECACHE with size" << scaledWidth << "ORIGINAL" << originalSize.width();

                    // convert it back to buffer
                    QBuffer buffer( &pixmapData );
                    buffer.open( QIODevice::WriteOnly );
                    pixmap.save( &buffer, "PNG" );
                    buffer.close();

                    // insert data into the cache
                    this->cache[checksum][scaledSize.width()] = pixmapData;
                   // qDebug() << "TO CACHE";

                    return true;
                };

                // check for pre-cached pixmaps with this scaledWidth, if none are available - make them
                if ( this->cache.contains( checksum )) {
                    if ( this->cache[checksum].contains( scaledSize.width())) {
                        pixmapData = this->cache[checksum][scaledSize.width()];
                        //qDebug() << "CACHE PASS" << checksum;
                    } else {
                        if ( !generateMipMap())
                            return false;
                    }
                } else {
                    if ( !generateMipMap())
                        return false;
                }

                if ( scaledSize.isEmpty())
                    return false;

                if ( pixmapData.isEmpty())
                    return false;

                return true;
            };


            // embed pixmap in html
            if ( setupPixmap())
                document->setHtml( QString( "<img width=\"%1\" height=\"%2\" src=\"data:image/png;base64,%3\">" ).arg( scaledSize.width()).arg( scaledSize.height()).arg( pixmapData.toBase64().constData()));
        }
    } else {
        QString html;

        if ( tagId == Id::Invalid || pixmapTag ) {
            // custom properties however do display their names
            html = ( index.column() == Property::Name ) ? Property::instance()->name( row ) : data.toString();
        } else {
            // properties with built-in tags don't use property names, but rather tag names
            const QString units( Tag::instance()->units( tagId ));

            QString stringData( data.toString());
            if ( tagId != Id::Invalid ) {
                if ( Tag::instance()->type( tagId ) == Tag::Real )
                    stringData.replace( QRegularExpression( "(\\d+)[,.](\\d+)" ), QString( "\\1%1\\2" ).arg( Variable::instance()->string( "decimalSeparator" )));
            }

            html = ( index.column() == Property::Name ? Tag::instance()->name( tagId ) : ( TextEdit::stripHTML( qAsConst( stringData ) + units )));
        }

        document->setHtml( QString( "<p style=\"font-size: %1pt; font-family: '%2'\">%3<\\p>" ).arg( font.pointSize()).arg( font.family()).arg( qAsConst( html )));
    }

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

    // draw custom selection highlight
    if ( option.state & QStyle::State_Selected ) {
        QColor highlight( qApp->palette().highlight().color());
        highlight.setAlpha( 128 );
        painter->fillRect( option.rect, QBrush( qAsConst( highlight )));
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
    // prevents caching before the model initializes
    const Row row = Property::instance()->row( index );
    if ( row == Row::Invalid )
        return QSize();

    const Id reagentId = Variable::instance()->value<Id>( "reagentDock/selection" );
    const Id parentId = Reagent::instance()->parentId( reagentId );
    const Id propertyParentId = Property::instance()->reagentId( row );
    if ( reagentId == Id::Invalid || propertyParentId == Id::Invalid )
        return QSize();

    if ( parentId == Id::Invalid && propertyParentId != reagentId )
        return QSize();

    if ( parentId != Id::Invalid && ( propertyParentId != reagentId && propertyParentId != parentId ))
        return QSize();

    // setup html document
    this->setupDocument( index, item.font );
    if ( !this->documentMap.contains( index ))
        return QStyledItemDelegate::sizeHint( item, index );

    // return document size
    return this->documentMap[index]->size().toSize();
}
