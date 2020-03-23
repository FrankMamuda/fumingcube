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
#include "fragmentnavigation.h"
#include <QDebug>

/**
 * @brief FragmentNavigation::FragmentNavigation
 */
FragmentNavigation::FragmentNavigation( QWidget *parent ) : QToolBar( parent ) {}

/**
 * @brief FragmentNavigation::~FragmentNavigation
 */
FragmentNavigation::~FragmentNavigation() {
    const QList<QAction*> actions( this->map.keys());
    for ( QAction *action : actions )
        QAction::disconnect( action, &QAction::toggled, this, nullptr );
}

/**
 * @brief FragmentNavigation::addFragment
 * @param name
 * @param icon
 * @param widget
 * @return
 */
QAction *FragmentNavigation::addFragment( const QString &name, const QIcon &icon, QWidget *widget ) {
    // abort if fragment host or widget is invalid
    if ( this->fragmentHost() == nullptr || widget == nullptr )
        return nullptr;

    qDebug() << "add widget";

    // get index and store fragment
    this->fragmentHost()->addWidget( widget );

    qDebug() << "add widget done";

    // create an action
    QAction *action( this->addAction( icon, name ));
    action->setCheckable( true );
    this->map[action] = widget;

    // connect action
    QAction::connect( action, &QAction::toggled, this, [ this, action ]() { this->setCurrentFragment( action ); } );

    qDebug() << "add done";

    return action;
}

/**
 * @brief FragmentNavigation::setCurrentFragment
 * @param action
 */
void FragmentNavigation::setCurrentFragment( QAction *action ) {
    if ( !this->map.contains( action ))
        return;

    if ( !action->isChecked()) {
        action->blockSignals( true );
        action->setChecked( true );
        action->blockSignals( false );
        return;
    }

    const QList<QAction*> actions( this->map.keys());
    for ( QAction *otherAction : actions ) {
        if ( otherAction != action ) {
            otherAction->blockSignals( true );
            otherAction->setChecked( false );
            otherAction->blockSignals( false );
        }
    }

    this->fragmentHost()->setCurrentWidget( this->map[action] );
    this->adjustSize();
}
