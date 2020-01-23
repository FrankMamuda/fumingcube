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

/*
 * includes
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "propertydock.h"
#include "reagentdock.h"
#include "labeldock.h"
#include "variable.h"
#include "dockwidget.h"
#include "script.h"
#include "tag.h"
#include "tagdialog.h"
#include "settingsdialog.h"
#include "about.h"
#include <QScrollBar>
#include <QMenu>
#include <QSqlQuery>
#include <QTimer>

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ) {
    this->ui->setupUi( this );

    // restore geometry, state dock widgets (reagent and property)
    this->addDockWidget( Qt::LeftDockWidgetArea, ReagentDock::instance());
    this->addDockWidget( Qt::LeftDockWidgetArea, LabelDock::instance());
    this->addDockWidget( Qt::RightDockWidgetArea, PropertyDock::instance());
    ReagentDock::instance()->setup( this->ui->actionReagents );
    PropertyDock::instance()->setup( this->ui->actionProperties );
    LabelDock::instance()->setup( this->ui->actionLabels );

    if ( !Variable::instance()->value<QVariant>( "mainWindow/geometry" ).isNull() && !Variable::instance()->value<QVariant>( "mainWindow/state" ).isNull()) {
        this->restoreGeometry( Variable::instance()->compressedByteArray( "mainWindow/geometry" ));
        this->restoreState( Variable::instance()->compressedByteArray( "mainWindow/state" ));
        this->restoreDockWidget( ReagentDock::instance());
        this->restoreDockWidget( PropertyDock::instance());
        this->restoreDockWidget( LabelDock::instance());
    }

    // slightly increase font in calculator view
    QFont font( this->ui->calcView->font());
    font.setPointSizeF( font.pointSizeF() * 1.2 );
    this->ui->calcView->setFont( qAsConst( font ));

    // resture previous calculations
    this->ui->calcView->document()->setDefaultStyleSheet( "a { text-decoration:none; }" );
    this->ui->calcView->append( Variable::instance()->compressedString( "calculator/history" ));

    // setup syntax highlighter
    this->highlighter = new SyntaxHighlighter( this->ui->calcView->document());

    this->ui->calcView->connect( this->ui->calcView, &QTextBrowser::anchorClicked, [ this ]( const QUrl &url ) {
        const QStringList args( url.toString().split( ";" ));
        if ( args.count() < 2 || args.count() > 3 )
            return;

        // resuse result
        if ( !QString::compare( args.first(), "ans" )) {
            this->insertCommand( args.at( 1 ) + " " );
            return;
        }

        // get all arguments - property name, reagent name and batch name
        const QString property( args.at( 0 ));
        const QString reagent( args.at( 1 ));
        const QString batch( args.count() == 3 ? args.at( 2 ) : "" );

        // get reagentId (parent)
        Id reagentId = Script::instance()->getReagentId( reagent );
        if ( reagentId == Id::Invalid )
            return;

        // get batchId (if any)
        if ( !batch.isEmpty()) {
            const Id batchId = Script::instance()->getReagentId( batch, qAsConst( reagentId ));
            if ( batchId == Id::Invalid )
                return;

            reagentId = batchId;
        }

        // select reagent or batch
        Variable::instance()->setValue<int>( "reagentDock/selection", static_cast<int>( qAsConst( reagentId )));
        ReagentDock::instance()->view()->restoreIndex();

        // get property tagId
        const Id tagId = Script::instance()->getPropertyId( property );
        if ( tagId == Id::Invalid )
            return;

        // find property
        Row propertyRow = Row::Invalid;
        for ( int y = 0; y < Property::instance()->count(); y++ ) {
            const Row row = static_cast<Row>( y );
            if ( Property::instance()->tagId( row ) == tagId ) {
                propertyRow = row;
                break;
            }
        }

        if ( propertyRow == Row::Invalid )
            return;

        // select propery index
        const QModelIndex index( Property::instance()->index( static_cast<int>( propertyRow ), Property::PropertyData ));
        if ( !index.isValid())
            return;

        PropertyDock::instance()->setCurrentIndex( index );

        // add an option to paste reference to calculator
        QMenu menu( this->ui->calcView );
        menu.addAction( this->tr( "Paste" ), [ this, property, reagent, batch ]() {
            this->insertCommand( QString( "%1( \"%2\"%3 )" ).arg( property ).arg( reagent ).arg( batch.isEmpty() ? "" : ", \"" + batch + "\"" ));
        } );
        QTimer::singleShot( 2000, &menu, SLOT( close()));
        menu.exec( QCursor::pos());
    } );
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    // delete syntax highlighter
    delete this->highlighter;
    delete this->m_theme;

    // delete docks
    delete ReagentDock::instance();
    delete PropertyDock::instance();

    // clear ui
    delete this->ui;
}

/**
 * @brief MainWindow::commandWidget
 * @return
 */
QLineEdit *MainWindow::commandWidget() {
    return this->ui->calcEdit;
}

/**
 * @brief MainWindow::calculatorWidget
 * @return
 */
QTextBrowser *MainWindow::calculatorWidget() {
    return this->ui->calcView;
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
    const QJSValue result( Script::instance()->evaluate( line ));
    const QString string( result.toString());

    // output either error or result
    if ( !string.isEmpty()) {
        // TODO: make a global static
        QStringList functions;
        QSqlQuery query;
        query.exec( QString( "select %1, %2 from %3 where %2 not null" )
                    .arg( Tag::instance()->fieldName( Tag::ID ))
                    .arg( Tag::instance()->fieldName( Tag::Function ))
                    .arg( Tag::instance()->tableName()));
        while ( query.next()) {
            const QString functionName( query.value( 1 ).toString());
            if ( !functionName.isEmpty())
                functions << functionName;
        }

        /**
         * @brief The Match struct
         */
        struct Match {
            QString args;
            int start;
            int end;
            Match( const QString &args, int start, int end ) : args( args ), start( start ), end( end ) {}
        };


        // NOTE: this may not be the best implementation, but it works
        //       what it does is:
        //       1) finds function( args, .. )
        //       2) encloses this with an anchor
        const QRegularExpression functionExpresstion( QString( "(?<function>%1)\\s*\\(\\s*(?<arguments>.+?(?=\\)|\\s*\\)))\\s*\\)" ).arg( functions.join( "|" )));
        QString replacedLine( line );
        replacedLine = replacedLine.remove( "\n" );
        QRegularExpressionMatchIterator functionIterator( functionExpresstion.globalMatch( qAsConst( replacedLine )));
        QList<Match> matches;

        // first match the function + args
        while ( functionIterator.hasNext()) {
            const QRegularExpressionMatch functionMatch( functionIterator.next());

            // then separate args
            const QRegularExpression argsExpression( "\"(.+?(?=\"))\"" );
            QRegularExpressionMatchIterator argsIterator( argsExpression.globalMatch( functionMatch.captured( "arguments" )));
            QStringList args;
            while ( argsIterator.hasNext()) {
                const QRegularExpressionMatch argsMatch( argsIterator.next());
                args << argsMatch.captured( 1 );
            }

            // build a list of functionNames, args, capture start and end positions for proper string replacement
            matches << Match( functionMatch.captured( "function" ) + ";" + args.join( ";" ), functionMatch.capturedStart(), functionMatch.capturedEnd());
        }

        // perform strring replacement
        int offset = 0;
        foreach ( const Match &match, matches ) {
            const QString link( QString( "<a href=\"%1\">" ).arg( match.args ));
            replacedLine.insert( match.start + offset, link );
            offset += link.length();
            replacedLine.insert( match.end + offset, "</a>" );
            offset += 4;
        }

        // finally append the end result to the calculator
        // NOTE: must be wrapped in <span></span> to avoid trailing anchors
        this->ui->calcView->append( "<span>" + ( result.isError() ? line : replacedLine ) + "</span>" );
        this->ui->calcView->append( QString( "<span>" ) + ( !result.isError() ? "= " : "" ) + QString( "<a href=\"ans;%1\">%1<\a>" ).arg( string ) + "</span>" + "<br>" );
    }

    // ensure the result is visible
    this->scrollToBottom();
}

/**
 * @brief MainWindow::insertCommand
 * @param command
 */
void MainWindow::insertCommand( const QString &command ) {
    QLineEdit *calc( this->ui->calcEdit );
    if ( calc->text().isEmpty())
        calc->setText( command );
    else
        calc->insert( " " + command );

    // must activate MainWindow first
    this->activateWindow();
    calc->setFocus();
}

/**
 * @brief MainWindow::saveHistory
 */
void MainWindow::saveHistory() {
    // save calculator view history
    Variable::instance()->setCompressedString( "calculator/history", this->ui->calcView->toHtml());

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
    ReagentDock::instance()->view()->updateView();
}

/**
 * @brief MainWindow::closeEvent
 * @param event
 */
void MainWindow::closeEvent( QCloseEvent *event ) {
    Variable::instance()->setCompressedByteArray( "mainWindow/geometry", MainWindow::instance()->saveGeometry());
    Variable::instance()->setCompressedByteArray( "mainWindow/state", MainWindow::instance()->saveState());
    QMainWindow::closeEvent( event );
}

/**
 * @brief MainWindow::on_actionSettings_triggered
 */
void MainWindow::on_actionSettings_triggered() {
    SettingsDialog( this ).exec();
}

/**
 * @brief MainWindow::on_actionAbout_triggered
 */
void MainWindow::on_actionAbout_triggered() {
    About( this ).exec();
}

