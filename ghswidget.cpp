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
#include "ghswidget.h"
#include <QPainter>
#include "ghspictograms.h"
#include "property.h"

/**
 * @brief GHSWidget::scale
 */
constexpr const int GHSWidget::scale = 48;

/**
 * @brief GHSWidget::GHSWidget
 * @param parent
 * @param parms
 */
GHSWidget::GHSWidget( QWidget *parent, const QStringList &parms ) : PropertyViewWidget( parent, parms ) {
    this->update( parms );

    foreach ( const QString &key, GHSHazards::Hazards.keys()) {
        if ( !this->parameters().contains( key ))
            this->parameters().removeAll( key );
    }
}

/**
 * @brief GHSWidget::paintEvent
 */
void GHSWidget::paintEvent( QPaintEvent * ) {
    QPainter painter( this );
    int xOffset = 0;
    int yOffset = 0;
    int index = 0;

    foreach ( const QString &key, this->parameters()) {
        const QPixmap pixmap( GHSPictograms::pixmap( key, GHSWidget::scale ));
        if ( xOffset >= this->iconsPerRow() * GHSWidget::scale ) {
            xOffset = 0;
            yOffset += GHSWidget::scale;
        }

        painter.drawPixmap( qAsConst( xOffset ), qAsConst( yOffset ), GHSWidget::scale, GHSWidget::scale, pixmap );
        xOffset += GHSWidget::scale;
        index++;
    }
}

/**
 * @brief GHSWidget::sizeHint
 * @return
 */
QSize GHSWidget::sizeHint() const {
    if ( this->m_linear ) {
        this->m_iconsPerRow = 9;
        return QSize( GHSWidget::scale * this->parameters().count(), GHSWidget::scale );
    }

    const int sectionSize = PropertyDock::instance()->sectionSize( Property::PropertyData );
    if ( sectionSize == 0 )
        return QSize();

    this->m_iconsPerRow = qMax( 1, ( sectionSize - ( sectionSize % GHSWidget::scale )) / GHSWidget::scale );
    const int numIcons = this->parameters().count();
    const int rows = ( numIcons - ( numIcons % this->iconsPerRow())) / this->iconsPerRow() + ( numIcons % this->iconsPerRow() > 0 ? 1 : 0 );

    int height = GHSWidget::scale;
    for ( int y = 1; y < rows; y++ )
        height += GHSWidget::scale;

    return QSize( this->parameters().count() * GHSWidget::scale, qAsConst( height ));
}
