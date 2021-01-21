/*
 * Copyright (C) 2019-2020 Armands Aleksejevs
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
#include "theme.h"
#include <QLineEdit>
#include <QMainWindow>
#include <QTextBrowser>
#include <QSystemTrayIcon>

/**
 * @brief The Ui namespace
 */
namespace Ui {
    class MainWindow;
}

/*
 * classes
 */
class CalcView;

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY( MainWindow )

public:
    // disable move
    MainWindow( MainWindow&& ) = delete;
    MainWindow& operator=( MainWindow&& ) = delete;

    /**
     * @brief instance
     * @return
     */
    static MainWindow *instance() {
        static auto *instance( new MainWindow());
        return instance;
    }
    ~MainWindow() override;

    /**
     * @brief theme
     * @return
     */
    [[nodiscard]] auto *theme() const { return this->m_theme; }

    /**
     * @brief calcTheme
     * @return
     */
    [[nodiscard]] auto *calcTheme() const { return this->m_calcTheme == nullptr ? this->m_theme : this->m_calcTheme; }

    /**
     * @brief calculator
     * @return
     */
    [[nodiscard]] CalcView *calcView();

public slots:
    void appendToCalculator( const QString &line, bool debug = false );
    void insertCommand( const QString &command );
    void saveHistory();
    void scrollToBottom();

    /**
     * @brief setTheme
     * @param theme
     */
    void setTheme( Theme *theme ) { this->m_theme = theme; }
    void setCalcTheme( Theme *theme );

private slots:
    void on_actionClear_triggered();
    void on_actionTags_triggered();
    void on_actionSettings_triggered();
    void on_actionAbout_triggered();
    void on_actionSearch_triggered();
    void on_actionTables_triggered();
    void on_actionDraw_triggered();

protected:
    void closeEvent( QCloseEvent *event ) override;

private:
    explicit MainWindow( QWidget *parent = nullptr );
    Ui::MainWindow *ui;
    SyntaxHighlighter *highlighter;
    Theme *m_theme = new Theme();
    Theme *m_calcTheme = nullptr;
    QSystemTrayIcon *tray = nullptr;
    bool forceQuit = false;
};
