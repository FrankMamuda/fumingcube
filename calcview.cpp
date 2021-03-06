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
#include "calcview.h"
#include "theme.h"
#include "variable.h"
#include "mainwindow.h"
#include <QTextBrowser>
#include <QDebug>
#include <QMenu>
#include <QContextMenuEvent>
#include <QApplication>
#include <QPainter>
#include <QRegularExpression>

CalcView::CalcView( QWidget *parent ) : QTextBrowser( parent ) {
    this->m_zoom = Variable::decimalValue( "calculator/zoom" );
    this->m_zoom = qMin( this->m_zoom, 4.0 );
    this->m_zoom = qMax( this->m_zoom, 0.5 );
    this->shortCutZoomIn = new QShortcut( QKeySequence::ZoomIn, this, SLOT( zoomIn()));
    this->shortCutZoomOut = new QShortcut( QKeySequence::ZoomOut, this, SLOT( zoomOut()));
}

/**
 * @brief CalcView::~CalcView
 */
CalcView::~CalcView() {
    delete this->shortCutZoomIn;
    delete this->shortCutZoomOut;
}

/**
 * @brief CalcView::fontSize
 * @return
 */
int CalcView::fontSize() const {
    return static_cast<int>( this->document()->defaultFont().pointSizeF() * this->zoom());
}

/**
 * @brief The CalcView class
 */
void CalcView::contextMenuEvent(QContextMenuEvent *event) {
    QMenu *menu( this->createStandardContextMenu());
    menu->setAttribute( Qt::WA_DeleteOnClose, true );
    QMenu *subMenu( menu->addMenu( CalcView::tr( "Override theme" )));
    subMenu->setIcon( QIcon::fromTheme( "colour" ));

    const QString currentTheme( Variable::string( "calculator/theme" ));
    const QStringList themes( Theme::availableThemes().keys());
    for ( const QString &themeName : themes ) {
        const bool current = !themeName.isEmpty() && !QString::compare( themeName, currentTheme );
        QAction *action( subMenu->addAction( themeName, [ current, themeName ]() {
            Variable::setString( "calculator/theme", current ? "" : themeName );

            // TODO: delete previous
            Theme *theme( current ? nullptr : new Theme( themeName ));
            MainWindow::instance()->setCalcTheme( theme );
        } ));
        action->setCheckable( true );
        action->setChecked( current );
    }

    menu->addAction( CalcView::tr( "Zoom in" ), this, SLOT( zoomIn()))->setIcon( QIcon::fromTheme( "zoom_in" ));
    menu->addAction( CalcView::tr( "Zoom out" ), this, SLOT( zoomOut()))->setIcon( QIcon::fromTheme( "zoom_out" ));
    menu->addAction( CalcView::tr( "Restore zoom" ), this, SLOT( zoomRestore()));

    menu->exec( event->globalPos());
}

/**
 * @brief CalcView::wheelEvent
 * @param e
 */
void CalcView::wheelEvent( QWheelEvent *event ){
    if (( event->modifiers() == Qt::ControlModifier ) && ( event->angleDelta().y() > 0 ))
       this->zoomIn();
    else if (( event->modifiers() == Qt::ControlModifier ) && ( event->angleDelta().y() < 0 ))
        this->zoomOut();
    else
        QTextBrowser::wheelEvent( event );
}

/**
 * @brief CalcView::showEvent
 * @param event
 */
void CalcView::showEvent( QShowEvent *event ) {
    QTextBrowser::showEvent( event );
    this->adjustFonts();
}

/**
 * @brief CalcView::adjustFonts
 */
void CalcView::adjustFonts() {
    const QString html( QString( this->document()->toHtml()).replace( QRegularExpression( "(<span\\s+style=\"\\s+font-size:)(\\d+)" ), QString( "\\1%1" ).arg( this->fontSize())));
    this->setHtml( html );
    MainWindow::instance()->scrollToBottom();
}
