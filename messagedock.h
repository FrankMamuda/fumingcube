/*
 * Copyright (C) 2017-2018 Factory #12
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
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
#include <QDockWidget>

/**
 * @brief The MessageDock class
 */
class MessageDock : public QMainWindow {
    Q_OBJECT
    Q_ENUMS( Modes )

public:
    enum Modes {
        NoMode = -1,
        Plain,
        Warning,
        Error
    };
    explicit MessageDock( QWidget *parent = 0 );
    ~MessageDock();

public slots:
    void displayMessage( const QString &message, Modes mode = Error, int timeout = 2500, qreal opacity = 1.0 );
    void hideMessage();

protected:
    void showEvent( QShowEvent *event ) override;
    void hideEvent( QHideEvent *event ) override;

private:
    QDockWidget *dockWidget;
    QWidget *titleBar;
    QHBoxLayout *titleBarLayout;
    QLabel *titleLabel;
    QPushButton *closeButton;
    QGraphicsOpacityEffect *opacityEffect;
    QTimer timer;
};
