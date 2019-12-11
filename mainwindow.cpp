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

/*
 * includes
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "propertydock.h"
#include "reagentdock.h"
#include "variable.h"
#include "dockwidget.h"
#include "script.h"
#include "tag.h"
#include "tagdialog.h"
#include "settingsdialog.h"
#include <QScrollBar>
#include <QMenu>
#include <QSqlQuery>

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ) {
    this->ui->setupUi( this );

    // restore geometry, state dock widgets (reagent and property)
    this->addDockWidget( Qt::LeftDockWidgetArea, ReagentDock::instance());
    this->addDockWidget( Qt::RightDockWidgetArea, PropertyDock::instance());
    ReagentDock::instance()->setup( this->ui->actionReagents );
    PropertyDock::instance()->setup( this->ui->actionProperties );
    this->restoreGeometry( QByteArray::fromBase64( Variable::instance()->value<QByteArray>( "mainWindow/geometry" )));
    this->restoreState( QByteArray::fromBase64( Variable::instance()->value<QByteArray>( "mainWindow/state" )));
    this->restoreDockWidget( ReagentDock::instance());
    this->restoreDockWidget( PropertyDock::instance());

    // slightly increase font in calculator view
    QFont font( this->ui->calcView->font());
    font.setPointSizeF( font.pointSizeF() * 1.2 );
    this->ui->calcView->setFont( qAsConst( font ));

    // resture previous calculations
    this->ui->calcView->setText( Variable::uncompressedString( Variable::instance()->string( "calculator/history" )));

    // setup syntax highlighter
    this->highlighter = new SyntaxHighlighter( this->ui->calcView->document());
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    // delete syntax highlighter
    delete this->highlighter;

    // delete docks
    delete ReagentDock::instance();
    delete PropertyDock::instance();

    // clear ui
    delete this->ui;
}

/**
 * @brief MainWindow::calculatorWidget
 * @return
 */
QLineEdit *MainWindow::calculatorWidget() {
    return this->ui->calcEdit;
}

/**
 * @brief MainWindow::appendToCalculator
 * @param line
 */
void MainWindow::appendToCalculator( const QString &line ) {
    // ignore empty lines
    if ( line.isEmpty())
        return;

    // evaluate expression
    const QJSValue result( Script::instance()->evaluate( QString( line )));
    const QString string( result.toString());

    // output either error or result
    if ( !string.isEmpty())
        this->ui->calcView->append( line + "\n" + ( !result.isError() ? "= " : "" ) + string + "\n" );

    // ensure the result is visible
    this->scrollToBottom();
}

/**
 * @brief MainWindow::saveHistory
 */
void MainWindow::saveHistory() {
    // save calculator view history
    Variable::instance()->setString( "calculator/history", Variable::compressedString( this->ui->calcView->toPlainText()));

    // save expression editor hidtory
    this->ui->calcEdit->saveHistory();
}

/**
 * @brief MainWindow::scrollToBottom
 */
void MainWindow::scrollToBottom() {
    this->ui->calcView->verticalScrollBar()->setValue( this->ui->calcView->verticalScrollBar()->maximum());
}

/**
 * @brief MainWindow::on_actionClear_triggered
 */
void MainWindow::on_actionClear_triggered() {
    this->ui->calcView->clear();
}

/**
 * @brief MainWindow::on_actionTags_triggered
 */
void MainWindow::on_actionTags_triggered() {
    TagDialog td( this );
    td.exec();
    PropertyDock::instance()->updateView();
}

/**
 * @brief MainWindow::closeEvent
 * @param event
 */
void MainWindow::closeEvent( QCloseEvent *event ) {
    Variable::instance()->setValue( "mainWindow/geometry", MainWindow::instance()->saveGeometry().toBase64());
    Variable::instance()->setValue( "mainWindow/state", MainWindow::instance()->saveState().toBase64());
    QMainWindow::closeEvent( event );
}

/**
 * @brief MainWindow::on_actionSettings_triggered
 */
void MainWindow::on_actionSettings_triggered() {
    SettingsDialog().exec();
}
