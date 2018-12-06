/*
 * Copyright (C) 2018 Factory #12
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

//
// includes
//
#include <QIcon>
#include <QPainter>
#include <QWidget>
#include <QtMath>
#include <QMap>
#include <QDebug>

/**
 * @brief The GHSWidget class
 */
class GHSWidget final : public QWidget {
public:
    const int scale = 48;
    explicit GHSWidget( const QStringList &st, QWidget *parent = nullptr ) : QWidget( parent ), statements( st ) {
        this->pictograms["GHS07"] = QIcon( ":/pictograms/harmful" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS02"] = QIcon( ":/pictograms/flammable" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS06"] = QIcon( ":/pictograms/toxic" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS05"] = QIcon( ":/pictograms/corrosive" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS09"] = QIcon( ":/pictograms/environment" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS08"] = QIcon( ":/pictograms/health" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS01"] = QIcon( ":/pictograms/explosive" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS03"] = QIcon( ":/pictograms/oxidizing" ).pixmap( this->scale, this->scale );
        this->pictograms["GHS04"] = QIcon( ":/pictograms/compressed" ).pixmap( this->scale, this->scale );

        foreach ( const QString &key, this->pictograms.keys()) {
            if ( !this->statements.contains( key ))
                this->statements.removeAll( key );
        }
        qDebug() << this->statements;
    }

protected:
    void paintEvent( QPaintEvent * ) override {
        QPainter painter( this );
        int xOffset = 0;
        int yOffset = 0;
        int index = 0;

        foreach ( const QString &name, this->statements ) {
            const QPixmap pixmap( this->pictograms[name] );

            if ( index == 4 || index == 8) {
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
        int height = this->scale;

        if ( this->statements.count() > 4 ) {
            height += this->scale;

            if ( this->statements.count() > 8 )
                height += this->scale;
        }

        return QSize( this->statements.count() * this->scale, height );
    }

private:
    QStringList statements;
    QMap<QString, QPixmap> pictograms;
};
