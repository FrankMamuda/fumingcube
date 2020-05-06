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
#include "label.h"
#include "reagent.h"
#include "field.h"
#include "database.h"
#include "variable.h"
#include "listutils.h"
#include <QPixmap>
#include <QPainter>
#include <QSqlQuery>
#include <QApplication>

/**
 * @brief Label::Label
 */
Label::Label() : Table( "label" ) {
    this->addField( PRIMARY_FIELD( ID ) );
    this->addField( FIELD( Name, String ) );
    this->addField( FIELD( Colour, ByteArray ) );
}

/**
 * @brief Label::add
 * @param name
 * @return
 */
Row Label::add( const QString &name, const QColor &colour ) {
    return Table::add( QVariantList() << Database_::null << name << QVariant( colour ).toByteArray());
}

/**
 * @brief Label::data
 * @param index
 * @param role
 * @return
 */
QVariant Label::data( const QModelIndex &index, int role ) const {
    if ( role == Qt::DecorationRole ) {
        const QColor colour( this->colour( static_cast<Row>( index.row())));
        return this->pixmap( colour );
    }

    if ( role == Qt::DisplayRole ) {
        // NOTE: for now use this i18n method, in future replace with something better
        const QString originalString( Table::data( index, role ).toString());
        return QApplication::translate( "Label", originalString.toUtf8().constData());
    }

    if ( role == Qt::BackgroundRole ) {
        const QList<int> rows( ListUtils::toNumericList<int>( Variable::string( "labelDock/selectedRows" ).split( ";" )));
        if ( rows.contains( index.row()) || Reagent::instance()->filter().isEmpty()) {
            QColor highlight( QApplication::palette().highlight().color());
            highlight.setAlpha( 16 );
            return highlight;
        }
    }

    return Table::data( index, role );
}

/**
 * @brief Label::pixmap
 * @param colour
 * @return
 */
QPixmap Label::pixmap( const QColor &colour ) const {
    if ( this->cache.contains( colour.name()))
        return this->cache[colour.name()];

    QPixmap pixmap( Label::Width, Label::Height );
    pixmap.fill( Qt::transparent );
    QPainter painter( &pixmap );
    painter.setPen( Qt::transparent );
    painter.setBrush( colour );// QColor::fromRgb( colour.red(), colour.green(), colour.blue(), 128 ));
    painter.drawRoundedRect( QRect( 0, 0, Label::Width, Label::Height ), 3, 3 );
    this->cache[colour.name()] = pixmap;
    return pixmap;
}

/**
 * @brief Label::pixmap
 * @param colour
 * @return
 */
QPixmap Label::pixmap( const QList<QColor> &colourList ) const {

    if ( colourList.count() <= 1 )
        return this->pixmap( colourList.isEmpty() ? QColor() : colourList.first());

    QString name;
    for ( const QColor &colour : colourList )
        name.append( colour.name());

    if ( this->cache.contains( qAsConst( name )))
        return this->cache[qAsConst( name )];

    QPixmap pixmap( Label::Width, Label::Height );
    pixmap.fill( Qt::transparent );
    QPainter painter( &pixmap );
    painter.setPen( Qt::transparent );

    if ( colourList.count() > 4 ) {
        QLinearGradient gradient( 0, 0, Label::Width, Label::Height );
        gradient.setColorAt( 0.0, Qt::red );
        gradient.setColorAt( 1.0 / 6.0, QColor( 255, 100, 0 ));
        gradient.setColorAt( 2.0 / 6.0, Qt::yellow );
        gradient.setColorAt( 3.0 / 6.0, Qt::green );
        gradient.setColorAt( 4.0 / 6.0, Qt::cyan );
        gradient.setColorAt( 5.0 / 6.0, Qt::blue );
        gradient.setColorAt( 1.0, Qt::magenta );
        painter.setBrush( QBrush( gradient ));
        painter.drawRoundedRect( QRect( 0, 0, Label::Width, Label::Height ), 3, 3 );
    } else {
        const QMap<int, int> sizes {{ 2, 6 },
                                    { 3, 4 },
                                    { 4, 3 }};

        int offset = 0;
        for ( const QColor &colour : colourList ) {
            const int size = sizes[colourList.count()];
            painter.setBrush( colour );
            painter.drawRect( QRect( offset, 0, size, 8 ));
            offset += size;
        }
    }

    this->cache[name] = pixmap;
    return pixmap;
}

/**
 * @brief Label::removeOrphanedEntries
 */
void Label::removeOrphanedEntries() {
    // NOTE: labels do not have foreign ids, therefore they cannot be orphaned
}

/**
 * @brief Label::populate
 */
void Label::populate() {
    this->add( Label::tr( "Bases" ), QColor::fromRgb( 218, 32, 32, 255/*32*/ ));
    this->add( Label::tr( "Acids" ), QColor::fromRgb( 32, 218, 32, 255/*32*/ ));
    this->add( Label::tr( "Solvents" ), QColor::fromRgb( 32, 32, 218, 255/*32*/ ));
    this->add( Label::tr( "Inorganics" ), QColor::fromRgb( 128, 128, 128, 255/*32*/ ));
    this->add( Label::tr( "Alcohols" ), QColor::fromRgb( 66, 183, 255, 255/*32*/ ));
}
