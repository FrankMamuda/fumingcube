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
#include <QCompleter>
#include <QMainWindow>
#include <QSignalMapper>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class MainWindow;
}

//
// classes
//
class LineEdit;
class MessageDock;
class ReagentModel;
class TemplateModel;
class Reagent;
class Template;

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow( QWidget *parent = nullptr );
    ~MainWindow();
    Reagent *currentReagent() const;
    Template *currentTemplate() const;

public slots:
    void restoreIndexes();

private slots:
    void on_actionAdd_triggered();
    void fillTemplates();
    void calculate( int mode );
    void on_actionEdit_triggered();
    void on_actionRemove_triggered();
    void on_actionProperties_triggered();

protected:
    void resizeEvent( QResizeEvent *event ) override;
    void closeEvent( QCloseEvent *event ) override;

private:
    Ui::MainWindow *ui;
    QList<LineEdit*> inputList;
    QSignalMapper *signalMapper;
    MessageDock *messageDock;
    ReagentModel *reagentModel;
    TemplateModel *templateModel;
    QCompleter *reagentCompleter;
};
