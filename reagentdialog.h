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

#pragma once

//
// includes
//
#include <QDialog>
#include <QToolButton>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class ReagentDialog;
}

//
// classes
//
class TemplateWidget;
class Reagent;

/**
 * @brief The ReagentDialog class
 */
class ReagentDialog : public QDialog {
    Q_OBJECT
    Q_ENUMS( Modes )

public:
    enum Modes {
        NoMode = -1,
        Add,
        Edit
    };
    explicit ReagentDialog( QWidget *parent = nullptr );
    ~ReagentDialog();
    Modes mode() const { return this->m_mode; }

public slots:
    void add();
    void setMode( Modes mode ) { this->m_mode = mode; }
    void setReagent( Reagent *reagent );

private slots:
    void addNewTab();
    void on_tabWidget_tabCloseRequested( int index );

private:
    Ui::ReagentDialog *ui;
    QToolButton *newTab;
    QList<TemplateWidget*> widgetList;
    Modes m_mode;
    Reagent *reagent;
};
