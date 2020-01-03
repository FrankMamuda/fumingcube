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
#include "syntaxhighlighter.h"
#include "reagentmodel.h"
#include "theme.h"
#include <QLineEdit>
#include <QMainWindow>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class MainWindow;
}

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    static MainWindow *instance() { static MainWindow *instance( new MainWindow()); return instance; }
    ~MainWindow() override;
    QLineEdit *calculatorWidget();
    Theme *theme() const { return this->m_theme; }

public slots:
    void appendToCalculator( const QString &line );
    void saveHistory();
    void scrollToBottom();
    void setTheme( Theme *theme ) { this->m_theme = theme; }

private slots:
    void on_actionClear_triggered();
    void on_actionTags_triggered();
    void on_actionSettings_triggered();

protected:
    void closeEvent( QCloseEvent *event ) override;

private:
    explicit MainWindow( QWidget *parent = nullptr );
    Ui::MainWindow *ui;
    SyntaxHighlighter *highlighter;
    Theme *m_theme = new Theme();
};
