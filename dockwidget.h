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

#pragma once

/*
 * includes
 */
#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QMainWindow>
#include <QMoveEvent>
#include "variable.h"

class DockWidget : public QDockWidget {
    Q_OBJECT

public:
    /**
     * @brief DockWidget
     * @param variable
     * @param parent
     */
    explicit DockWidget( QWidget *parent = nullptr ) : QDockWidget( parent ) {
        DockWidget::connect( this, &QDockWidget::visibilityChanged, [ this ]( const bool &visible ) {
            if ( this->action() != nullptr ) {
                if ( this->action()->isChecked() != visible ) {
                    this->action()->blockSignals( true );
                    this->action()->setChecked( visible );
                    this->action()->blockSignals( false );
                }
            }
        } );
    }

    /**
     * @brief action
     * @return
     */
    [[nodiscard]] QAction *action() const { return this->m_action; }

public slots:
    /**
     * @brief setup
     * @param action
     */
     void setup( QAction *action = nullptr ) {
        this->m_action = action;
        if ( this->action() != nullptr ) {
            this->action()->setChecked( this->isVisible());
            QAction::connect( this->action(), SIGNAL( toggled( bool )), this, SLOT( setVisible( bool )));
        }
    }


private:
    QAction *m_action = nullptr;
};
