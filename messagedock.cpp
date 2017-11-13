/*
 * Copyright (C) 2017 Factory #12
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

#include "messagedock.h"
#include <QStyle>
#include <QApplication>

/**
 * @brief MessageDock::MessageDock
 * @param parent
 */
MessageDock::MessageDock( QWidget *parent ) :
    QMainWindow( parent ),
    dockWidget( new QDockWidget()),
    titleBar( new QWidget( this )),
    titleBarLayout( new QHBoxLayout()),
    titleLabel( new QLabel( this )),
    closeButton( new QPushButton( this )),
    opacityEffect( new QGraphicsOpacityEffect( this ))
{
    // set dock widget
    this->dockWidget->setAllowedAreas( Qt::TopDockWidgetArea );
    this->dockWidget->setFloating( false );
    this->dockWidget->setFeatures( QDockWidget::NoDockWidgetFeatures );
    this->addDockWidget( Qt::TopDockWidgetArea, this->dockWidget );

    // remove Window flag, to allow embedding
    this->setWindowFlags( this->windowFlags() & ~Qt::Window );

    // convert icon lambda
    auto convertIcon = []( const QIcon &icon ) {
        QIcon out;

        foreach( QSize size, icon.availableSizes()) {
            QImage tmp( icon.pixmap( size ).toImage());
            tmp.invertPixels();
            out.addPixmap( QPixmap::fromImage( tmp ));
        }

        return out;
    };

    // make 'close message' button
    this->closeButton->setIcon( convertIcon( qApp->style()->standardIcon( QStyle::SP_TitleBarCloseButton )));
    this->closeButton->setFlat( true );
    this->closeButton->setFixedSize( 16, 16 );
    this->closeButton->setStyleSheet( "QPushButton { padding: 0px; outline: 0px; border: 0px; } QPushButton:hover { background-color: rgba(100, 100, 100, 150); }" );
    this->connect( this->closeButton, &QPushButton::clicked, [ this ]() {
        this->hide();
    } );

    // make title bar
    this->titleLabel->setAlignment( Qt::AlignCenter );
    this->titleLabel->setWordWrap( true );
    this->titleLabel->setStyleSheet( "QLabel { font-weight: bold; color: white; }" );

    // add widgets to title bar layout
    this->titleBarLayout->addWidget( this->titleLabel );
    this->titleBarLayout->addWidget( this->closeButton );
    this->titleBarLayout->setContentsMargins( 0, 0, 0, 0 );
    this->titleBarLayout->setSpacing( 0 );

    // set styleSheet and layout to title bar
    this->titleBar->setLayout( this->titleBarLayout );

    // override dockWidget's title bar
    this->dockWidget->setTitleBarWidget( this->titleBar );

    // set opacity effect
    this->setGraphicsEffect( this->opacityEffect );
    this->setAutoFillBackground( true );
}

/**
 * @brief displayMessage
 * @param message
 * @param mode
 * @param timeOut
 */
void MessageDock::displayMessage( const QString &message, Modes mode, int timeout, qreal opacity ) {
    // change style
    switch ( mode ) {
    case Plain:
        this->titleBar->setStyleSheet( "background: #777777;" );
        this->opacityEffect->setOpacity( opacity );
        break;

    case Warning:
        this->titleBar->setStyleSheet( "background: #eeaa00;" );
        this->opacityEffect->setOpacity( opacity );
        break;

    case Error:
        this->titleBar->setStyleSheet( "background: #ff0000;" );
        this->opacityEffect->setOpacity( opacity );
        break;

    case NoMode:
        break;
    }

    // set titleBar message
    this->titleLabel->setText( message );

    // set timeOut
    if ( timeout > 0 ) {
        this->timer.stop();
        this->timer.setInterval( timeout );
        this->timer.singleShot( timeout, this, SLOT( hideMessage()));
    }

    // show the widget
    this->show();
    this->raise();
}

/**
 * @brief MessageDock::hideMessage
 */
void MessageDock::hideMessage() {
    this->timer.stop();
    this->hide();
}

/**
 * @brief MessageDock::~MessageDock
 */
MessageDock::~MessageDock() {
    delete this->dockWidget;
    delete this->opacityEffect;
}
