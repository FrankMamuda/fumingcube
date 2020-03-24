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

#pragma once

/*
 * includes
 */
#include <QStackedWidget>
#include <QToolBar>
#include "fragment.h"
#include <QDebug>

/**
 * @brief The FragmentNavigation class
 */
class FragmentNavigation : public QToolBar {
    Q_OBJECT
    Q_DISABLE_COPY( FragmentNavigation )

public:
    explicit FragmentNavigation( QWidget *parent = nullptr );
    ~FragmentNavigation();

    // disable move
    FragmentNavigation( FragmentNavigation&& ) = delete;
    FragmentNavigation& operator=( FragmentNavigation&& ) = delete;

    /**
     * @brief fragmentHost
     * @return
     */
    [[nodiscard]] QStackedWidget *fragmentHost() const { return this->m_fragmentHost; }

    /**
     * @brief addFragmentX
     * @param name
     * @param icon
     * @param parent
     * @param toolTip
     * @return
     */
    template<class T>
    [[nodiscard]] T *addFragment( const QString &name, const QIcon &icon, QWidget *parent = nullptr, const QString &toolTip = QString()) {
        // abort if fragment host or widget is invalid
        if ( this->fragmentHost() == nullptr )
            return nullptr;

        // get index and store fragment
        Fragment *fragment( new T());
        this->fragmentHost()->addWidget( fragment );
        fragment->setHost( parent );

        // create an action
        QAction *action( new QAction( icon, name ));
        this->insertAction( this->rightSpacer, action );
        action->setCheckable( true );
        action->setToolTip( toolTip );
        this->actionFragmentMap[action] = fragment;
        this->fragmentActionMap[fragment] = action;

        // connect action
        QAction::connect( action, &QAction::toggled, this, [ this, action ]() { this->setCurrentFragment( action ); } );
        return qobject_cast<T*>( fragment );
    }

public slots:
    /**
     * @brief setFragmentHost
     * @param fragmentHost
     */
    void setFragmentHost( QStackedWidget *fragmentHost ) { this->m_fragmentHost = fragmentHost; }
    void setCurrentFragment( QAction *action );
    void setCurrentFragment( Fragment *fragment );
    void installCloseButton( QWidget *parent = nullptr );
    void setFragmentEnabled( Fragment *fragment, bool enabled );

private:
    QStackedWidget *m_fragmentHost = nullptr;
    QMap<QAction*, Fragment*> actionFragmentMap;
    QMap<Fragment*, QAction*> fragmentActionMap;
    QAction *rightSpacer = nullptr;
};
