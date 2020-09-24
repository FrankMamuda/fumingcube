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
#include "textutils.h"
#include "networkmanager.h"
#include <QScrollBar>
#include <QMenu>
#include <QSqlQuery>
#include <QTimer>
#include <utility>
#include <QInputDialog>
#include <QMessageBox>
#include "extractiondialog.h"
#include "listutils.h"
#include "structurefragment.h"
#include "labelset.h"
#include "label.h"
#include "tabledialog.h"
#include "drawdialog.h"

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

    if ( !Variable::value<QVariant>( "mainWindow/geometry" ).isNull() &&
         !Variable::value<QVariant>( "mainWindow/state" ).isNull()) {
        this->restoreGeometry( Variable::compressedByteArray( "mainWindow/geometry" ));
        this->restoreState( Variable::compressedByteArray( "mainWindow/state" ));
        this->restoreDockWidget( ReagentDock::instance());
        this->restoreDockWidget( PropertyDock::instance());
        this->restoreDockWidget( LabelDock::instance());
    }

    // slightly increase font in calculator view
    QFont font( this->ui->calcView->font());
    font.setPointSizeF( font.pointSizeF() * 1.2 );
    this->ui->calcView->setFont( qAsConst( font ));

    // restore previous calculations
    this->ui->calcView->document()->setDefaultStyleSheet( "a { text-decoration:none; } ​p { margin: 0px; }​" );
    this->ui->calcView->append( Variable::compressedString( "calculator/history" ));

    // set calculator theme
    const QString currentTheme( Variable::string( "calculator/theme" ));
    if ( !currentTheme.isEmpty()) {
        this->m_calcTheme = new Theme( currentTheme );
        this->ui->calcView->setPalette( this->calcTheme()->palette());
    }

    // setup syntax highlighter
    this->highlighter = new SyntaxHighlighter( this->ui->calcView->document());

    QTextBrowser::connect( this->calcView(), &QTextBrowser::anchorClicked, [ this ]( const QUrl &url ) {
        const QStringList args( url.toString().split( ";" ));
        if ( args.count() < 2 || args.count() > 3 )
            return;

        // reuse result
        if ( !QString::compare( args.first(), "ans" )) {
            this->insertCommand( args.at( 1 ) + " " );
            return;
        }

        // get all arguments - property name, reagent name and batch name
        const QString &property( args.at( 0 ));
        const QString &reagent( TextUtils::fromBase64( args.at( 1 )));
        const QString batch( args.count() == 3 ? TextUtils::fromBase64( args.at( 2 )) : "" );

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

        // NOTE: sometimes the reagent list is filtered and reagent might be hidden
        //       therefore we must find its label and select it
        {
            QSqlQuery query;
            query.exec( QString( "select %1 from %2 where %3=%4" )
                                .arg( LabelSet::instance()->fieldName( LabelSet::LabelId ),
                                      LabelSet::instance()->tableName(),
                                      LabelSet::instance()->fieldName( LabelSet::ReagentId ),
                                      QString::number( static_cast<int>( reagentId ))));

            // find the first label the reagent has
            if ( query.next()) {
                const Id labelId = query.value( 0 ).value<Id>();
                if ( labelId != Id::Invalid ) {
                    const QList<Row> list( ListUtils::toNumericList<Row>( Variable::string( "labelDock/selectedRows" ).split( ";" )));
                    const Row labelRow = Label::instance()->row( labelId );

                    // test if label has already been selected
                    if ( !list.contains( labelRow )) {
                        // if not, append it to the selection and reset view
                        Variable::setString( "labelDock/selectedRows", Variable::string( "labelDock/selectedRows" ).append( ";" ).append( QString::number( static_cast<int>( labelRow ))));
                        LabelDock::instance()->restoreFilter();

                        //qDebug() << "reagentId" << reagentId << "label" << labelId << labelRow;
                        //LabelDock::instance()->setFilter( )
                    }
                }
            }
        }


        // select reagent or batch
        Variable::setValue<int>( "reagentDock/selection", static_cast<int>( qAsConst( reagentId )));
        ReagentDock::instance()->view()->restoreIndex();

        // get property tagId
        const Id tagId = Script::instance()->getPropertyId( property );
        if ( tagId == Id::Invalid )
            return;

        // find property
        Row propertyRow = Row::Invalid;
        for ( int y = 0; y < Property::instance()->count(); y++ ) {
            const auto row = static_cast<Row>( y );
            if ( Property::instance()->tagId( row ) == tagId ) {
                propertyRow = row;
                break;
            }
        }

        if ( propertyRow == Row::Invalid )
            return;

        // select property index
        const QModelIndex index(
                Property::instance()->index( static_cast<int>( propertyRow ), Property::PropertyData ));
        if ( !index.isValid())
            return;

        PropertyDock::instance()->setCurrentIndex( index );

        // add an option to paste reference to calculator
        QMenu menu( this->ui->calcView );
        menu.addAction( MainWindow::tr( "Paste" ), this, [ this, property, reagent, batch ]() {
            this->insertCommand( QString( "%1( \"%2\"%3 )" )
                                 .arg( property, reagent,
                                       batch.isEmpty() ?
                                           "" :
                                           ", \"" + batch + "\"" ));
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
 * @brief MainWindow::calcView
 * @return
 */
CalcView *MainWindow::calcView() {
    return this->ui->calcView;
}

/**
 * @brief MainWindow::appendToCalculator
 * @param line
 */
void MainWindow::appendToCalculator( const QString &line, bool debug ) {
    // ignore empty lines
    if ( line.isEmpty())
        return;

    // evaluate expression
    const QJSValue result( Script::instance()->evaluate( line ));

    // handle system commands
    if ( line.contains( "sys." )) {
        this->ui->calcView->append( QString( "<span style=\"font-size: %1pt\">" ).arg( calcView()->fontSize()) + "# " + line + "</span>" );
        this->scrollToBottom();
        return;
    } else if ( debug ) {
        this->ui->calcView->append( QString( "<span style=\"font-size: %1pt\">" ).arg( calcView()->fontSize()) + "$ " + line + "</span>" );
        this->scrollToBottom();
        return;
    }

    // get result string
    const QString string( result.toString());

    // output either error or result
    if ( !string.isEmpty()) {
        const QStringList functions( Tag::instance()->getFunctionList());

        /**
         * @brief The Match struct
         */
        struct Match {
            QString args;
            int start;
            int end;

            Match( QString args, int start, int end ) : args( std::move( args )), start( start ), end( end ) {}
        };


        // NOTE: this may not be the best implementation, but it works
        //       what it does is:
        //       1) finds function( args, .. )
        //       2) encloses this with an anchor
        const QRegularExpression functionExpression(
                  QString( "(?<function>%1)\\s*\\(\\s*(?<arguments>\".+?(?=\")\"(?:\\s*,\\s*?(?:\".+?(?=\"))\")?)\\s*\\)" )
                    .arg( functions.join( "|" ))
                    );

        QString replacedLine( line );
        replacedLine = replacedLine.remove( "\n" );
        QRegularExpressionMatchIterator functionIterator( functionExpression.globalMatch( qAsConst( replacedLine )));
        QList<Match> matches;

        // first match the function + args
        while ( functionIterator.hasNext()) {
            const QRegularExpressionMatch functionMatch( functionIterator.next());

            // then separate args
            const QRegularExpression argsExpression( "\"(.+?(?=\"))\"" );
            QRegularExpressionMatchIterator argsIterator(
                    argsExpression.globalMatch( functionMatch.captured( "arguments" )));
            QStringList args;
            while ( argsIterator.hasNext()) {
                const QRegularExpressionMatch argsMatch( argsIterator.next());
                args << TextUtils::toBase64( argsMatch.captured( 1 ));
            }

            // build a list of functionNames, args, capture start and end positions for proper string replacement
            matches << Match( functionMatch.captured( "function" ) + ";" + args.join( ";" ),
                              functionMatch.capturedStart(), functionMatch.capturedEnd());
        }

        // perform string replacement
        int offset = 0;
        for ( const Match &match : qAsConst( matches )) {
            const QString link( QString( "<a href=\"%1\">" ).arg( match.args ));

            replacedLine.insert( match.start + offset, link );
            offset += link.length();
            replacedLine.insert( match.end + offset, "</a>" );
            offset += 4;
        }

        // finally append the end result to the calculator
        // NOTE: must be wrapped in <span></span> to avoid trailing anchors
        this->ui->calcView->append( QString( "<span style=\"font-size: %1pt\">" ).arg( calcView()->fontSize()) + ( result.isError() ? line : replacedLine ) + "</span>" );
        if ( result.isError()) {
            this->ui->calcView->append( QString( "<span style=\"font-size: %1pt\">%2</span><br>" ).arg( calcView()->fontSize()).arg( string ));
            Variable::setString( "calculator/ans", "" );
        } else {
            this->ui->calcView->append( QString( "<span style=\"font-size: %1pt\">= <a href=\"ans;%2\">%2<\a></span><br>" ).arg( calcView()->fontSize()).arg( string ));
            Variable::setString( "calculator/ans", string );
        }
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
    Variable::setCompressedString( "calculator/history", this->ui->calcView->toHtml());

    // save expression editor history
    this->ui->calcEdit->saveHistory();
}

/**
 * @brief MainWindow::scrollToBottom
 */
void MainWindow::scrollToBottom() {
    this->ui->calcView->verticalScrollBar()->setValue( this->ui->calcView->verticalScrollBar()->maximum());
}

/**
 * @brief MainWindow::setCalcTheme
 * @param theme
 */
void MainWindow::setCalcTheme( Theme *theme ) {
    if ( this->m_calcTheme != nullptr )
        delete this->m_calcTheme;

    this->m_calcTheme = theme;

    // reset document
    this->highlighter->setDocument( this->calcView()->document());
    this->calcView()->setPalette( this->calcTheme()->palette());
}

/**
 * @brief MainWindow::on_actionClear_triggered
 */
void MainWindow::on_actionClear_triggered() {
    if ( QMessageBox::question( this, MainWindow::tr( "Confirm action" ), MainWindow::tr( "Clear calculator history?" )) == QMessageBox::Yes )
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
    Variable::setCompressedByteArray( "mainWindow/geometry", MainWindow::instance()->saveGeometry());
    Variable::setCompressedByteArray( "mainWindow/state", MainWindow::instance()->saveState());
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

/**
 * @brief MainWindow::on_actionSearch_triggered
 */
void MainWindow::on_actionSearch_triggered() {
    ExtractionDialog ed( this );
    ed.exec();
}

/**
 * @brief MainWindow::on_actionTables_triggered
 */
void MainWindow::on_actionTables_triggered() {
    TableDialog td( this );
    td.exec();
}

/**
 * @brief MainWindow::on_actionDraw_triggered
 */
void MainWindow::on_actionDraw_triggered() {
    DrawDialog dd( this, "", true );
    dd.exec();
}
