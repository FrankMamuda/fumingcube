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
#include "fragment.h"
#include "fragmentnavigation.h"
#include <QDebug>

/**
 * @brief FragmentNavigation::FragmentNavigation
 */
FragmentNavigation::FragmentNavigation( QWidget *parent ) : QToolBar( parent ) {
    auto leftSpacer( new QWidget( this ));
    leftSpacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    auto rightSpacer( new QWidget( this ));
    rightSpacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    this->addWidget( leftSpacer );
    this->rightSpacer = this->addWidget( rightSpacer );
}

/**
 * @brief FragmentNavigation::~FragmentNavigation
 */
FragmentNavigation::~FragmentNavigation() {
    const QList<QAction*> actions( this->actionFragmentMap.keys());
    for ( QAction *action : actions )
        QAction::disconnect( action, &QAction::toggled, this, nullptr );
}

/**
 * @brief FragmentNavigation::setCurrentFragment
 * @param action
 */
void FragmentNavigation::setCurrentFragment( QAction *action ) {
    if ( !this->actionFragmentMap.contains( action ) || this->fragmentHost() == nullptr )
        return;

    if ( !action->isChecked()) {
        action->blockSignals( true );
        action->setChecked( true );
        action->blockSignals( false );
        return;
    }

    // enable action of the active fragment
    if ( !action->isEnabled())
        action->setEnabled( true );

    const QList<QAction*> actions( this->actionFragmentMap.keys());
    for ( QAction *otherAction : actions ) {
        if ( otherAction != action ) {
            otherAction->blockSignals( true );
            otherAction->setChecked( false );
            otherAction->blockSignals( false );
        }
    }

    this->fragmentHost()->setCurrentWidget( this->actionFragmentMap[action] );
    this->adjustSize();
}

/**
 * @brief FragmentNavigation::setCurrentFragment
 * @param fragment
 */
void FragmentNavigation::setCurrentFragment( Fragment *fragment ) {
    if ( !this->fragmentActionMap.contains( fragment ))
        return;

    this->fragmentActionMap[fragment]->trigger();
}

/**
 * @brief FragmentNavigation::installCloseButton
 */
void FragmentNavigation::installCloseButton( QWidget *parent ) {
    QAction *action( new QAction( QIcon::fromTheme( "close" ), FragmentNavigation::tr( "Close" )));
    if ( parent != nullptr )
        QAction::connect( action, &QAction::triggered, parent, &QWidget::close );

    this->addAction( action );
}

/**
 * @brief FragmentNavigation::setFragmentEnabled
 * @param fragment
 * @param enabled
 */
void FragmentNavigation::setFragmentEnabled( Fragment *fragment, bool enabled ) {
    if ( !this->fragmentActionMap.contains( fragment ))
        return;

    this->fragmentActionMap[fragment]->setEnabled( enabled );
}
