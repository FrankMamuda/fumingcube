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

#pragma once

/*
 * includes
 */
#include "propertywidget.h"
#include <QIcon>
#include <QPainter>
#include <QWidget>
#include <QtMath>
#include <QMap>
#include <QDebug>
#include <QRegularExpression>
#include "propertydock.h"

/**
 * @brief The GHSWidget class
 */
class GHSWidget final : public PropertyWidget {
public:
    static constexpr const int scale = 48;
    explicit GHSWidget( QWidget *parent = nullptr, const QStringList &parms = QStringList()) : PropertyWidget( parent, parms ) {

        // TODO: make these global to be shared
        this->pictograms["GHS07"] = QIcon( ":/pictograms/GHS07" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS02"] = QIcon( ":/pictograms/GHS02" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS06"] = QIcon( ":/pictograms/GHS06" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS05"] = QIcon( ":/pictograms/GHS05" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS09"] = QIcon( ":/pictograms/GHS09" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS08"] = QIcon( ":/pictograms/GHS08" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS01"] = QIcon( ":/pictograms/GHS01" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS03"] = QIcon( ":/pictograms/GHS03" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS04"] = QIcon( ":/pictograms/GHS04" ).pixmap( this->scale, this->scale );

        this->update( parms );

        foreach ( const QString &key, this->pictograms.keys()) {
            if ( !this->parameters().contains( key ))
                this->parameters().removeAll( key );
        }
    }

    int iconsPerRow() const { return this->m_iconsPerRow; }

public slots:
    void update( const QStringList &parms ) override {
        //if ( this->parameters() == parms )
        //     return;
        this->m_parameters = parms;
        this->repaint();
    }
    void setLinear() { this->m_linear = true; }

protected:
    void paintEvent( QPaintEvent * ) override {
        QPainter painter( this );
        int xOffset = 0;
        int yOffset = 0;
        int index = 0;

        foreach ( const QString &name, this->parameters()) {
            const QPixmap pixmap( this->pictograms[name] );
            if ( xOffset >= this->iconsPerRow() * this->scale ) {
                xOffset = 0;
                yOffset += this->scale;
            }

            painter.drawPixmap( xOffset, yOffset, this->scale, this->scale, pixmap );
            xOffset += this->scale;
            index++;
        }
    }

    /**
     * @brief sizeHint
     * @return
     */
    QSize sizeHint() const override {
        if ( this->m_linear )
            return QSize( this->scale, this->scale );

        const int sectionSize = PropertyDock::instance()->sectionSize( 1 );
        if ( sectionSize == 0 )
            return QSize();

        this->m_iconsPerRow = qMax( 1, ( sectionSize - ( sectionSize % this->scale )) / this->scale );
        const int numIcons = this->parameters().count();
        const int rows = ( numIcons - ( numIcons % this->iconsPerRow())) / this->iconsPerRow() + ( numIcons % this->iconsPerRow() > 0 ? 1 : 0 );

        int height = this->scale;
        for ( int y = 1; y < rows; y++ )
            height += this->scale;

        return QSize( this->parameters().count() * this->scale, height );
    }

private:
    QMap<QString, QPixmap> pictograms;
    mutable int m_iconsPerRow = 0;
    bool m_linear = false;
};
