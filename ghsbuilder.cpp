/*
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
#include "ghsbuilder.h"

// TODO: gray out properties (with a tag) that have been already set

/**
 * @brief GHSBuilder::GHSBuilder
 * @param parent
 */
GHSBuilder::GHSBuilder( QWidget *parent, const QStringList &parameters ) : QDialog( parent ) {
    grid->setSpacing( 0 );

    /*
     * grayScale pixmap lambda
     */
    auto grayPixmap = []( const QString &name ) {
        QImage image( name );

        for ( int w = 0; w < image.width(); w++ ) {
             for ( int h = 0; h < image.height(); h++ ) {
                 const int gray = qGray( image.pixel( w, h ));
                 const int alpha = image.pixelColor( w, h ).alpha();
                 image.setPixel( w, h, qRgba( gray, gray, gray, alpha ));
             }
         }

        return QPixmap::fromImage( qAsConst( image ));
    };

    int index = 1;
    for ( int y = 0; y < 3; y++ ) {
        for ( int k = 0; k < 3; k++ ) {
            const QString iconName( QString( "GHS0%1" ).arg( index ));

            // setup icon (unchecked: grayed out; checked: coloured)
            QIcon icon;
            icon.addPixmap( grayPixmap( ":/pictograms/" + iconName ), QIcon::Normal, QIcon::Off );
            icon.addPixmap( QPixmap( ":/pictograms/" + iconName ), QIcon::Normal, QIcon::On );

            // set up tool button
            QToolButton *button( new QToolButton());
            button->setCheckable( true );
            button->setIcon( icon );
            button->setIconSize( { 64, 64 } );
            button->setAutoRaise( true );
            button->setStyleSheet( "QToolButton { background: rgba( 255, 255, 255, 0 ); outline: 0px; }" );
            button->setText( iconName );
            button->setToolButtonStyle( Qt::ToolButtonIconOnly );

            // set previous value
            if ( parameters.contains( iconName ))
                button->setChecked( true );

            // add to layout and garbage collector
            this->buttons << button;
            grid->addWidget( button, y, k );

            // advance index
            index++;
        }
    }

    // setup dialog buttonbox
    QDialogButtonBox *buttonBox( new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel ));
    buttonBox->connect( buttonBox, SIGNAL( accepted()), this, SLOT( accept()));
    buttonBox->connect( buttonBox, SIGNAL( rejected()), this, SLOT( reject()));
    grid->addWidget( buttonBox, 3, 0, 1, 3 );

    // set grid as the dialog's layout
    this->setLayout( grid );

    // setup dialog
    this->setWindowIcon( QIcon::fromTheme( "hazard" ));
    this->setWindowTitle( this->tr( "Select pictograms" ));
}

/**
 * @brief GHSBuilder::~GHSBuilder
 */
GHSBuilder::~GHSBuilder() {
    // get rid of buttons and grid
    qDeleteAll( this->buttons );
    delete this->buttonBox;
    delete this->grid;
}

/**
 * @brief GHSBuilder::parameters
 * @return
 */
QStringList GHSBuilder::parameters() const {
    QStringList parms;

    foreach ( const QToolButton *button, this->buttons ) {
        if ( button->isChecked())
            parms << button->text();
    }

    return parms;
}
