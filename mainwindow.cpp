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

    // setup syntax highlighter
    this->highlighter = new SyntaxHighlighter( this->ui->calcView->document());

    QTextBrowser::connect( this->ui->calcView, &QTextBrowser::anchorClicked, [ this ]( const QUrl &url ) {
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
        this->ui->calcView->append( "<span>" + ( result.isError() ? line : replacedLine ) + "</span>" );
        if ( result.isError()) {
            this->ui->calcView->append( QString( "<span>%1</span><br>" ).arg( string ));
            Variable::setString( "calculator/ans", "" );
        } else {
            this->ui->calcView->append( QString( "<span>= <a href=\"ans;%1\">%1<\a></span><br>" ).arg( string ));
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

#if 0
    // TODO: add caching to this method

    bool ok;
    const QString identifier( QInputDialog::getText( this, MainWindow::tr( "Search" ), MainWindow::tr( "Name or CAS number" ), QLineEdit::Normal, QString(), &ok ));

    // NOTE: object facilitates safe disconnection of the following lambdas
    QObject *object( new QObject());

    if ( ok ) {
        QObject::connect( NetworkManager::instance(),
                                   &NetworkManager::finished,
                                   object,
                             [ this, object, identifier ]( const QString &, NetworkManager::Types type, const QVariant &, const QByteArray &data ) {
                QStringList cidList;

                switch ( type ) {
                // ignore these events
                case NetworkManager::NoType:
                case NetworkManager::FormulaRequest:
                case NetworkManager::FormulaRequestBrowser:
                case NetworkManager::DataRequest:
                case NetworkManager::IUPACName:
                case NetworkManager::FavIcon:
                    return;

                case NetworkManager::CIDRequestInitial:
                {
                    cidList << QString( data ).split( "\n" );
                    cidList.removeAll( "" );
                    if ( cidList.isEmpty()) {
                        QMessageBox::warning( this, MainWindow::tr( "Warning" ), MainWindow::tr( "Could not find any reagents matching your identifier" ));
                        break;
                    }

                    /*const QString cid( cidList.first());
                    StructureFragment sb( ListUtils::toNumericList<int>( cidList ), this );
                    sb.setSearchMode();

                    if ( sb.exec() != QDialog::Accepted )
                        break;

                    ReagentDock::instance()->addReagent( Id::Invalid, sb.name(), sb.cid());*/
                }
                    break;

                case NetworkManager::CIDRequestSimilar:
                {
                    cidList << QString( data ).split( "\n" );
                    cidList.removeAll( "" );

                    if ( cidList.isEmpty()) {
                        QMessageBox::warning( this, MainWindow::tr( "Warning" ), MainWindow::tr( "Could not find any reagents matching your identifier" ));
                        break;
                    }

                    QList<int> cidListInt( ListUtils::toNumericList<int>( qAsConst( cidList )));
                    cidListInt.removeAll( 0 );
                    std::sort( cidListInt.begin(), cidListInt.end());
                    cidListInt.erase( std::unique( cidListInt.begin(), cidListInt.end()), cidListInt.end());

                    /StructureBrowser sb( cidListInt, this );
                    sb.setSearchMode();
                    if ( sb.exec() != QDialog::Accepted )
                        break;

                    ReagentDock::instance()->addReagent( Id::Invalid, sb.name(), sb.cid());
                }
                    break;
            }
                // disconnect by deleting the object
                if ( object != nullptr )
                    object->deleteLater();
        } );

        QObject::connect( NetworkManager::instance(),
                             &NetworkManager::error,
                             object,
                       [ this, identifier, object ]( const QString &, NetworkManager::Types type, const QString & ) {

            switch ( type ) {
            // ignore these events
            case NetworkManager::NoType:
            case NetworkManager::FormulaRequest:
            case NetworkManager::FormulaRequestBrowser:
            case NetworkManager::DataRequest:
            case NetworkManager::IUPACName:
            case NetworkManager::FavIcon:
                return;

            case NetworkManager::CIDRequestInitial:
                // initial failed to yield a list, proceed to similar search
                NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/%1/cids/TXT?name_type=word" ).arg( QString( identifier ).replace( " ", "-" )), NetworkManager::CIDRequestSimilar );
                break;

            case NetworkManager::CIDRequestSimilar:
                QMessageBox::warning( this, MainWindow::tr( "Warning" ), MainWindow::tr( "Could not find any reagents matching your identifier" ));

                // disconnect by deleting the object
                if ( object != nullptr )
                    object->deleteLater();

                break;
           }
        } );

        // send the inital request
        NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/%1/cids/TXT" ).arg( QString( identifier ).replace( " ", "-" )), NetworkManager::CIDRequestInitial );
    }
#endif
}
