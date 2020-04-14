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
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QPalette>
#include <QTableView>
#include <QBuffer>
#include <QSqlQuery>
#include <QTranslator>
#include "htmlutils.h"
#include "pixmaputils.h"
#include "propertydock.h"

/**
 * @brief PropertyDelegate::setupDocument
 * @param index
 */
void PropertyDelegate::setupDocument( const QModelIndex &index, const QFont &defaultFont ) const {
    QFont font( defaultFont );

    // reuse document from cache if any
    if ( this->documentMap.contains( index ) || !index.isValid())
        return;

    // get data and tag/property info
    const Row propertyRow = Property::instance()->row( index );
    const QVariant data( Property::instance()->propertyData( propertyRow ));
    const Id tagId = Property::instance()->tagId( propertyRow );
    const Tag::Types tagType = Tag::instance()->type( tagId );

    // create a new document
    auto *document( new QTextDocument());

    // set special modifiers
    if ( index.column() == Property::Name )
       this->setSpecialModifiers( font, tagId, propertyRow );

    // special handling of pixmaps and formulas
    if ( tagType == Tag::Formula || tagId == PixmapTag ) {
        // name column
        if ( index.column() == Property::Name ) {
            this->setupTextDocument( index, document, tagType == Tag::Formula ?
                                         QApplication::translate( "Tag", Tag::instance()->name( tagId ).toUtf8().constData()) :
                                         Property::instance()->name( propertyRow ),
                                     font );
        } else if ( index.column() == Property::PropertyData ) {
            this->setupPixmapDocument( index, document, data.toByteArray(), Tag::instance()->type( tagId ) == Tag::Formula );
        }

        return;
    }

    // handle custom properties
    if ( tagId == Id::Invalid ) {
        // custom properties however do display their names
        this->setupTextDocument( index, document, ( index.column() == Property::Name ) ? Property::instance()->name( propertyRow ) : data.toString(), qAsConst( font ));
        return;
    }

    // properties with built-in tags don't use property names, but rather tag names
    const QString units( Tag::instance()->units( tagId ).remove( QRegularExpression( R"(<\s*br\s*\/>)" )));
    QString stringData( data.toString());
    if ( tagId != Id::Invalid ) {
        const Tag::Types type = Tag::instance()->type( tagId );
        if ( type == Tag::Real ) {
            stringData.replace( QRegularExpression( "(\\d+)[,.](\\d+)" ),
                                QString( "\\1%1\\2" ).arg( Variable::string( "decimalSeparator" )));
        } else if ( type == Tag::State ) {
            bool ok;
            int stateIndex = stringData.toInt( &ok );

            if ( !ok )
                stateIndex = -1;

            switch ( stateIndex ) {
            case 0:
                stringData = PropertyDelegate::tr( "Solid" );
                break;

            case 1:
                stringData = PropertyDelegate::tr( "Liquid" );
                break;

            case 2:
                stringData = PropertyDelegate::tr( "Gaseous" );
                break;

            default:
                stringData = PropertyDelegate::tr( "Unknown" );
            }
        } else if ( type == Tag::Date ) {
            const QDate date( stringData.isEmpty() ? QDate() : QDate::fromJulianDay( stringData.toInt()));
            stringData = date.isValid() ? date.toString( Qt::DateFormat::SystemLocaleDate ) : "";
        }
    }

    // setup document
    this->setupTextDocument( index,
                             document,
                             ( index.column() == Property::Name ?
                             QApplication::translate( "Tag", Tag::instance()->name( tagId ).toUtf8().constData()) :
                             ( HTMLUtils::simplify( qAsConst( stringData ) + units ))),
                             font );
}

/**
 * @brief PropertyDelegate::setupPixmapDocument
 * @param document
 */
void PropertyDelegate::setupPixmapDocument( const QModelIndex &index, QTextDocument *document, const QByteArray &data, bool isFormula ) const {
    // failsafes
    if ( document == nullptr || data.isEmpty())
        return;

    // read pixmap header
    PixmapInfo info;
    if ( !PixmapUtils::readHeader( data, &info ))
        return;

    // get section width
    const int sectionWidth = PropertyDock::instance()->sectionSize( Property::PropertyData );

    // determine required mipmap size
    const int needsScaling = info.width > sectionWidth - 16;
    const int width = needsScaling ? sectionWidth - 16 : info.width;
    const int height = needsScaling ? static_cast<int>(( static_cast<qreal>( width ) / static_cast<qreal>( info.width )) * static_cast<qreal>( info.height )) : info.height;
    const bool isDarkMode = Variable::isEnabled( "darkMode" );
    QString key( QString( "%1/%2%3.png" ).arg( info.crc ).arg( width ).arg( isDarkMode && isFormula ? "d" : "" ));

    // check if mipmap already exists in cache
    bool isCached = Cache::instance()->contains( "property", key );
    QByteArray pixmapData( isCached ? Cache::instance()->getData( "property", key ) : data );

    // make mipmap if it is necessary
    if ( !isCached && ( needsScaling || ( isDarkMode && isFormula ))) {
        QPixmap pixmap;
        if ( !pixmap.loadFromData( qAsConst( pixmapData )))
            return;

        // special handling of formulas
        if ( isFormula ) {
            pixmap = PixmapUtils::autoCrop( qAsConst( pixmap ));
            if ( isDarkMode )
                pixmap = PixmapUtils::invert( PixmapUtils::autoCrop( qAsConst( pixmap )));
        }

        // FAST downscale pixmap
        // NOTE: use pixmap.width(), since autocrop could have made it smaller
        if ( width * 2 < pixmap.width())
            pixmap = pixmap.scaled( width * 2, height, Qt::IgnoreAspectRatio, Qt::FastTransformation );

        // SLOW downscale pixmap
        pixmap = pixmap.scaled( width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );

        // convert it back to buffer
        pixmapData = PixmapUtils::toData( pixmap );
        if ( pixmapData.isEmpty())
            return;

        // insert into cache
        Cache::instance()->insert( "property", key, pixmapData );
    }

    // embed pixmap in html
    document->setHtml( QString( R"(<table cellpadding="4"><tr><td><img width="%1" height="%2" src="data:image/png;base64,%3"></td></tr></table>)" )
                       .arg( width )
                       .arg( height )
                       .arg( pixmapData.toBase64().constData()));

    // just add to cache
    this->documentMap[index] = document;
}

/**
 * @brief PropertyDelegate::setupTextDocument
 * @param document
 * @param text
 * @param font
 */
void PropertyDelegate::setupTextDocument( const QModelIndex &index, QTextDocument *document, const QString &text, const QFont &font ) const {
    // failsafes
    if ( document == nullptr || text.isEmpty())
        return;

    // apply font and generate initial html
    QString html( QString( R"(<p style="font-size: %1pt; font-family: '%2'">%3</p>)" ).arg( QString::number( font.pointSize()), font.family(), qAsConst( text )));

    // italic modifier
    if ( font.italic()) {
        html.prepend( "<i>" );
        html.append( "</i>" );
    }

    // strikeout
    if ( font.strikeOut()) {
        html.prepend( "<s>" );
        html.append( "</s>" );
    }

    // add star
    if ( index.column() == Property::Name && font.italic() && !font.strikeOut()) {
        //   PixmapDa ( !Cache::instance()->getData( "pixmap", "star_12" ))
        //
        //
        //   html.prepend( QString( R"(<img width="12" height="12" src="data:image/png;base64,%1">)" )
        //                 .arg( PixmapUtils::toData( QIcon::fromTheme( "star" ).pixmap( 12, 12 ), "star_12" ).toBase64().constData()));
    }

    // set html to the document
    document->setHtml( qAsConst( html ));

    // finialize and add to cache
    this->finializeDocument( index, document );
}

/**
 * @brief PropertyDelegate::finializeDocument
 * @param document
 */
void PropertyDelegate::finializeDocument( const QModelIndex &index, QTextDocument *document ) const {
    const PropertyView *view( qobject_cast<PropertyView *>( this->parent()));

    // setup margins and word wrap
    document->setDocumentMargin( 2 );
    document->setTextWidth( view != nullptr ? static_cast<int>( view->columnWidth( index.column())) : 128 );
    document->setTextWidth( document->idealWidth());

    // add to cache
    this->documentMap[index] = document;
}

/**
 * @brief PropertyDelegate::setSpecialModifiers
 * @param document
 */
void PropertyDelegate::setSpecialModifiers( QFont &font, const Id &tagId, const Row &propertyRow ) const {
    // get reagent info
    const Id reagentId = Property::instance()->reagentId( propertyRow );
    const Id reagentParentId = Reagent::instance()->parentId( reagentId );

    // check if property belongs to the batch
    const bool isBatchProperty = Reagent::instance()->parentId( reagentId ) != Id::Invalid;

    // check for overrides
    bool isOverridden = false;
    if ( isBatchProperty ) {
        QSqlQuery query;
        query.exec( QString( "select * from %1 where %2=%3 and %4=%5" )
                    .arg( Property::instance()->tableName(),
                          Property::instance()->fieldName( Property::ReagentId ),
                          QString::number( static_cast<int>( reagentParentId )),
                          Property::instance()->fieldName( Property::TagId ),
                          QString::number( static_cast<int>( tagId ))));
        if ( query.next())
            isOverridden = true;
    }

    // check for duplicates
    bool isDuplicate = false;
    if ( tagId != Id::Invalid ) {
        if ( !Tag::instance()->function( tagId ).isEmpty()) {
            // check backwards all scriptable tags for duplicates
            const int startIndex = static_cast<int>( propertyRow ) - 1;
            if ( startIndex > 0 ) {
                for ( int y = startIndex; y >= 0; y-- ) {
                    const Row propRow = static_cast<Row>( y );
                    const Id dupTagId = Property::instance()->tagId( propRow );
                    if ( dupTagId == tagId ) {
                        isDuplicate = true;
                        break;
                    }
                }
            }
        }
    }

    // NOTE: batch properties are displayed in italic (at least for now)
    if ( isBatchProperty && !isDuplicate )
        font.setItalic( true );

    // TODO: add tooltip?
    if ( isDuplicate )
        font.setStrikeOut( true );
}

/**
 * @brief PropertyDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void PropertyDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {    
    const PropertyView *view( qobject_cast<PropertyView *>( this->parent()));
    if ( view->isResizeInProgress())
        return;

    // draw custom selection highlight
    if ( option.state & QStyle::State_Selected ) {
        QColor highlight( QApplication::palette().highlight().color());
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
    painter->translate( option.rect.left(),
                        option.rect.top() + static_cast<int>( option.rect.height() / 2 ) -
                        document->size().height() / 2 );
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
    const PropertyView *view( qobject_cast<PropertyView *>( this->parent()));
    if ( view->isResizeInProgress())
        return QSize();

    // prevents caching before the model initializes
    const Row row = Property::instance()->row( index );
    if ( row == Row::Invalid )
        return QSize();

    const auto reagentId = Variable::value<Id>( "reagentDock/selection" );
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
