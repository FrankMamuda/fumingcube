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
    explicit DockWidget( const QString &variable, QWidget *parent = nullptr ) : QDockWidget( parent ), m_variable( variable ) {
        if ( this->variable().isEmpty())
            return;

        this->connect( this, &QDockWidget::dockLocationChanged, [ this ]( const Qt::DockWidgetArea &area ) {
            if ( this->hasInitialized())
                Variable::instance()->setInteger( this->variable() + "Area", area );
        } );

        this->connect( this, &QDockWidget::topLevelChanged, [ this ]( const bool &floating ) {
            if ( this->hasInitialized())
                Variable::instance()->setEnabled( this->variable() + "Floating", floating );
        } );

        this->connect( this, &QDockWidget::visibilityChanged, [ this ]( const bool &visible ) {
            if ( this->hasInitialized()) {
                Variable::instance()->setEnabled( this->variable() + "Visible", visible );
                if ( this->action() != nullptr ) {
                    if( this->action()->isChecked() != visible ) {
                        this->action()->blockSignals( true );
                        this->action()->setChecked( visible );
                        this->action()->blockSignals( false );
                    }
                }
            }
        } );
    }

    /**
     * @brief variable
     * @return
     */
    QString variable() const { return this->m_variable; }

    /**
     * @brief action
     * @return
     */
    QAction *action() const { return this->m_action; }

    /**
     * @brief initalize
     * @param variable
     * @param defaultDockArea
     */
    static void initalize( const QString &variable, const Qt::DockWidgetArea &defaultDockArea = Qt::LeftDockWidgetArea ) {
        if ( variable.isEmpty())
            return;

        Variable::instance()->add( variable + "Visible",  true, Var::Flag::Hidden );
        Variable::instance()->add( variable + "Area",     defaultDockArea, Var::Flag::Hidden );
        Variable::instance()->add( variable + "Geometry", QRect(), Var::Flag::Hidden );
        Variable::instance()->add( variable + "Floating", false, Var::Flag::Hidden );
    }

    /**
     * @brief hasInitialized
     * @return
     */
    bool hasInitialized() const {   return this->m_init; }

    /**
     * @brief parentWidget
     * @return
     */
    QMainWindow *parentWidget() const {
        return this->m_parentWidget;
    }

public slots:
    /**
     * @brief setup
     * @param parentWidget
     * @param action
     */
    virtual void setup( QMainWindow *parentWidget, QAction *action = nullptr ) {
        if ( this->variable().isEmpty())
            return;

        this->m_parentWidget = parentWidget;
        if ( this->parentWidget() != nullptr ) {
            this->parentWidget()->addDockWidget( Variable::instance()->value<Qt::DockWidgetArea>( this->variable() + "Area" ), this );
            this->parentWidget()->installEventFilter( this );
        }

        const bool isVisible = Variable::instance()->isEnabled( this->variable() + "Visible" );
        this->setFloating( Variable::instance()->isEnabled( this->variable() + "Floating" ));
        this->setVisible( isVisible );
        this->setGeometry( Variable::instance()->value<QRect>( this->variable() + "Geometry" ));

        this->m_action = action;
        if ( this->action() != nullptr ) {
            this->action()->setChecked( isVisible );
            this->action()->connect( this->action(), SIGNAL( toggled( bool )), this, SLOT( setVisible( bool )));
        }

        this->m_init = true;
    }

protected:
    /**
     * @brief eventFilter
     * @param obj
     * @param event
     * @return
     */
    bool eventFilter( QObject *obj, QEvent *event ) override {
        if ( event->type() == QEvent::Close )
            this->m_init = false;

        return QObject::eventFilter(obj, event);
    }

    /**
     * @brief moveEvent
     * @param event
     */
    void moveEvent( QMoveEvent *event ) override {
        if ( !this->variable().isEmpty() && this->hasInitialized())
            Variable::instance()->setValue<QRect>( this->variable() + "Geometry", QRect( this->pos(), this->size()));

        QDockWidget::moveEvent( event );
    }

    /**
     * @brief resizeEvent
     * @param event
     */
    void resizeEvent( QResizeEvent *event ) override {
        if ( !this->variable().isEmpty() && this->hasInitialized())
            Variable::instance()->setValue<QRect>( this->variable() + "Geometry", QRect( this->pos(), this->size()));

        QDockWidget::resizeEvent( event );
    }

private:
    QString m_variable;
    bool m_init = false;
    QAction *m_action;
    QMainWindow *m_parentWidget;
};
