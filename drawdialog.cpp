/*
 * Copyright (C) 2020 Armands Aleksejevs
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

// TODO:
//       other configurable options
//       bind atom colours to cvar
//       zoom!
//       license copy?
//       ambiguous bond
//       pusher bonds
//       brackets
//       eraser icon for dark mode
//       cvar check in PropertyDialog
//       update window title to filename?
//       cyclobutadiene joke
//       subscript/superscript in labels
//       save last path
//       multiline labels
//
// BIG TODO: write own implementation of everything
//

/*
 * includes
 */
#include "drawdialog.h"
#include "drawlabeleditor.h"
#include "imageutils.h"
#include "mainwindow.h"
#include "networkmanager.h"
#include "pixmaputils.h"
#include "searchengine.h"
#include "ui_drawdialog.h"
#include "variable.h"
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QClipboard>
#include <QMessageBox>
#include <QBuffer>
#include <QImageWriter>
#include <QMimeData>
#include <QFileDialog>
#include <QWebChannel>
#include <QDesktopServices>

/**
 * @brief DrawDialog::DrawDialog
 * @param parent
 */
DrawDialog::DrawDialog( QWidget *parent, const QString &json, bool drawMode ) : QDialog( parent ), json( json ), ui( new Ui::DrawDialog ) {
    this->ui->setupUi( this );
    this->ui->contents->setWindowFlags( Qt::Widget );

    // no button box in draw mode
    if ( drawMode )
        this->ui->buttonBox->hide();

    // check if draw component has been downloaded
    QDir dir( QDir::currentPath() + "/chemDoodle/" );
    if ( !dir.exists()) {
        // make a directory in app path to store scripts
        dir.mkpath( QDir::currentPath() + "/chemDoodle/" );

        if ( !dir.exists()) {
            QMessageBox::critical( this, DrawDialog::tr( "Error" ),  DrawDialog::tr( "Could not setup paths" ));
            QMetaObject::invokeMethod( this, "close", Qt::QueuedConnection );
        }
    }

    // copy qwebchannel.js
    const QString webChannelPath( QDir::currentPath() + "/chemDoodle/qwebchannel.js" );
    if ( !QFileInfo::exists( webChannelPath )) {
        if ( !QFile::copy( ":/qtwebchannel/qwebchannel.js", webChannelPath )) {
            QMessageBox::critical( this, DrawDialog::tr( "Error" ), DrawDialog::tr( "Could not setup QWebChannel" ));
            QMetaObject::invokeMethod( this, "close", Qt::QueuedConnection );
        }
    }

    // copy supplementary script
    const QString scriptPath( QDir::currentPath() + "/chemDoodle/script.js" );
    if ( !QFileInfo::exists( scriptPath )) {
        if ( !QFile::copy( ":/chemDoodle/script.js", scriptPath )) {
            QMessageBox::critical( this, DrawDialog::tr( "Error" ), DrawDialog::tr( "Could not setup supplementary script" ));
            QMetaObject::invokeMethod( this, "close", Qt::QueuedConnection );
        }
    }

    // setup finished connection to the NetworkManager
    NetworkManager::connect( NetworkManager::instance(), &NetworkManager::finished, this, [ this ]( const QString &, NetworkManager::Types type, const QVariant &, const QByteArray &data ) {
        switch ( type ) {
        case NetworkManager::CDDemoPage:
        {
            // parse 2d sketcher demo page (so that we don't have to download the whole archive and bother with compression)
            const QString page( data );

            // we need three files - jQuery css and two js scripts
            QRegularExpression reJSQ( R"(href=\"(.+uis\/jquery.+?\.css))" );
            const QRegularExpressionMatch matchJSQ( reJSQ.match( page ));
            QRegularExpression reCDW( R"(src=\"(.+ChemDoodleWeb.js))" );
            const QRegularExpressionMatch matchCDW( reCDW.match( page ));
            QRegularExpression reUI( R"(src=\"(.+uis\/ChemDoodleWeb-uis.js))" );
            const QRegularExpressionMatch matchUI( reUI.match( page ));

            // find these in the demo page and downloaded them
            if ( matchJSQ.hasMatch() && matchCDW.hasMatch() && matchUI.hasMatch()) {
                NetworkManager::instance()->execute( "https://web.chemdoodle.com/" + matchJSQ.captured( 1 ), NetworkManager::CDjQueryCSS );
                NetworkManager::instance()->execute( "https://web.chemdoodle.com/" + matchCDW.captured( 1 ), NetworkManager::CDScript );
                NetworkManager::instance()->execute( "https://web.chemdoodle.com/" + matchUI.captured( 1 ), NetworkManager::CDUIScript );

                qDebug() << "Requesting scripts";
            } else {
                QMessageBox::critical( this, DrawDialog::tr( "Error" ), DrawDialog::tr( "Something went wrong.\nTry again later." ));
                QMetaObject::invokeMethod( this, "close", Qt::QueuedConnection );
                return;
            }

        }
            break;

        case NetworkManager::CDScript:
        {
            // write out base script
            if ( !data.isEmpty()) {
                QFile file( QDir::currentPath() + "/chemDoodle/ChemDoodleWeb.js" );
                if ( file.open( QIODevice::WriteOnly )) {
                    file.write( data );
                    file.close();
                    this->script = true;
                }
            }
        }
            break;

        case NetworkManager::CDUIScript:
        {
            // write out ui script
            if ( !data.isEmpty()) {
                QFile file( QDir::currentPath() + "/chemDoodle/ChemDoodleWeb-uis.js" );
                if ( file.open( QIODevice::WriteOnly )) {
                    file.write( data );
                    file.close();
                    this->scriptUI = true;
                }
            }
        }
            break;

        case NetworkManager::CDjQueryCSS:
        {
            // write out jQuery css
            if ( !data.isEmpty()) {
                QFile file( QDir::currentPath() + "/chemDoodle/jquery-ui.css" );
                if ( file.open( QIODevice::WriteOnly )) {
                    file.write( data );
                    file.close();
                    this->jQuery = true;
                }
            }
        }
            break;

        default:
            ;
        }

        // load draw component when we're done
        if ( this->script && this->scriptUI && this->jQuery )
            this->loadComponent();
    } );

    // setup error connection to the NetworkManager
    NetworkManager::connect( NetworkManager::instance(), &NetworkManager::error, this, [ this ]( const QString &, NetworkManager::Types type, const QVariant &, const QString & ) {
        switch ( type ) {
        case NetworkManager::CDDemoPage:
        case NetworkManager::CDScript:
        case NetworkManager::CDUIScript:
        case NetworkManager::CDjQueryCSS:
            // just throw a generic error for now
            QMessageBox::critical( this, DrawDialog::tr( "Error" ), DrawDialog::tr( "Something went wrong.\nTry again later." ));
            QMetaObject::invokeMethod( this, "close", Qt::QueuedConnection );
            return;

        default:
            ;
        }
    } );

    // download draw components if required
    if ( !QFileInfo( QDir::currentPath() + "/chemDoodle/ChemDoodleWeb.js" ).exists() || !QFileInfo( QDir::currentPath() + "/chemDoodle/ChemDoodleWeb-uis.js" ).exists() || !QFileInfo( QDir::currentPath() + "/chemDoodle/jquery-ui.css" ).exists()) {
        if ( QMessageBox::question( this, DrawDialog::tr( "Install draw tool component" ), DrawDialog::tr( "To use the draw tool, ChemDoodle Web Components must be downloaded.\nContinue with the download?" )) == QMessageBox::No ) {
            QMetaObject::invokeMethod( this, "close", Qt::QueuedConnection );
            return;
        }

        NetworkManager::instance()->execute( QString( "https://web.chemdoodle.com/demos/2d-sketcher" ), NetworkManager::CDDemoPage );
        return;
    } else {
        // load draw component if already available
        this->loadComponent();
    }
}

/**
 * @brief DrawDialog::~DrawDialog
 */
DrawDialog::~DrawDialog() {
    // disconnect canvas resize
    QTimer::disconnect( &this->resizeTimer, &QTimer::timeout, this, nullptr );

    // disconnect edit callback
    QWebEnginePage::disconnect( this->ui->webView->page(), &QWebEnginePage::loadFinished, this, nullptr );

    // disconnect NetworkManager
    NetworkManager::disconnect( NetworkManager::instance(), &NetworkManager::finished, this, nullptr );
    NetworkManager::disconnect( NetworkManager::instance(), &NetworkManager::error, this, nullptr );

    // cleanup actions
    QAction::disconnect( this->ui->actionSaveAs, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionSaveImage, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionUndo, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionRedo, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionSave, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionAbout, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionSelectAll, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionOpen, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionNew, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionCopy, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionCut, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionZoomIn, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionZoomOut, &QAction::triggered, this, nullptr );

    // delete QWebChannel and DrawBridge
    delete this->bridge;
    if ( this->channel != nullptr )
        delete this->channel;

    // get rid of ui
    delete this->ui;
}

/**
 * @brie-> DrawDialog::getPixmapAndAccept
 */
void DrawDialog::getPixmapAndAccept() {
    // we do a couple of things here:
    //
    // 1) get internal JSON string, which contains all shapes and molecules
    // 2) render and upscale a PNG of the canvas
    // 3) store JSON inside PNG (for editing in the future)

    // retrieve json from canvas and store it
    this->ui->webView->page()->runJavaScript( "{ const shapes = sketcher.shapes; const molecules = sketcher.molecules; ChemDoodle.writeJSON( molecules, shapes ); }", [ this ]( const QVariant &v ) {
        this->json = v.toString();
    } );

    // make PNG of the canvas
    this->ui->webView->page()->runJavaScript( "exportTool();", [ & ]( const QVariant &v ) {
        QRegularExpression reg( "data:image\\/png;base64,(.+)" );

        // js should output PNG in base64 format, so there we capture it...
        const QRegularExpressionMatch match( reg.match( v.toString()));
        if ( match.hasMatch()) {
            QString captured( match.captured( 1 ));

            // ...and reconstruct a pixmap from base64 data
            QPixmap pixmap;
            pixmap.loadFromData( QByteArray::fromBase64( QByteArray( captured.toUtf8().constData())));

            // unfortunately, we cannot directly store textual data in QPixmap, so we have to convert it into QImage...
            QImage image( pixmap.toImage());
            image.setText( "ChemDoodleData", this->json );

            // ...and write it back to QByteArray via buffer and QImageWriter
            QByteArray byteArray;
            QBuffer buffer( &byteArray );
            buffer.open( QIODevice::WriteOnly );

            QImageWriter writer( &buffer, "png" );
            writer.write( image );
            buffer.close();

            // at last we store the data in class and return success
            this->data = byteArray;
            this->accept();
        }

        // failure
        QMetaObject::invokeMethod( this, "reject", Qt::QueuedConnection );
    } );
}

/**
 * @brief DrawDialog::fetchIcon
 * @param path
 * @return
 */
QIcon DrawDialog::fetchIcon( const QString &name ) const {
    const bool isDarkMode = Variable::isEnabled( "darkMode" );

    // get icon from internal resources and invert it for dark mode when anebled
    QIcon icon( QString( "://icons/draw/%1.png" ).arg( name ));
    if ( isDarkMode ) {
        QImage image( icon.pixmap( 24, 24 ).toImage());
        image.invertPixels();
        icon = QIcon( QPixmap::fromImage( image ));
    }

    return icon;
}

/**
 * @brief DrawDialog::resizeEvent
 */
void DrawDialog::resizeEvent( QResizeEvent *event ) {
    // don't resize too often
    this->resizeTimer.start( 200 );
    this->m_resizeInProgress = true;
    QDialog::resizeEvent( event );
}

/**
 * @brief DrawDialog::closeEvent
 */
void DrawDialog::closeEvent( QCloseEvent *event ) {
    if ( this->hasInitialized()) {
        Variable::setCompressedByteArray( "drawDialog/geometry", this->saveGeometry());
        Variable::setCompressedByteArray( "drawDialog/state", this->ui->contents->saveState());
    }

    QDialog::closeEvent( event );
}

/**
 * @brief DrawDialog::loadComponent
 */
void DrawDialog::loadComponent() {
    // get background colours from theme (to make an uniform look)
    const QColor baseColour( QApplication::palette().color( QPalette::Window ));
    QString colour( QString( "rgb( %1, %2, %3 )" ).arg( baseColour.red()).arg( baseColour.green()).arg( baseColour.blue()));

    // create and render a webpage, using draw components
    QFile file( ":/chemDoodle/index.html" );
    if ( file.open( QFile::ReadOnly )) {
        QString buffer( file.readAll());

        // update paths to app path and update colours to match app theme
        const QString path( QDir::currentPath() + "/chemDoodle" );
        buffer.replace( "[_PATH]", path );
        buffer.replace( "[_COLOUR]", colour );

        // close file
        file.close();

        // render page
        this->ui->webView->setHtml( buffer, path );

        // initialize communications between cpp and js (QWebChannel)
        this->channel = new QWebChannel( this->ui->webView->page());
        this->ui->webView->page()->setWebChannel( this->channel );
        channel->registerObject( "bridge", this->bridge );
    }

    // setup timer (so that canvas is resized, when dialog window is resized)
    this->resizeTimer.setSingleShot( true );
    const QString resizeScript( "if ( typeof( sketcher) !== \"undefined\" ) { sketcher.resize( window.innerWidth, window.innerHeight ); sketcher.repaint(); }" );
    QTimer::connect( &this->resizeTimer, &QTimer::timeout, this, [ this, resizeScript ]() {
        this->m_resizeInProgress = false;
        this->ui->webView->page()->runJavaScript( resizeScript );
    } );

    // reload content (edit mode)
    if ( !json.isEmpty()) {
        QWebEnginePage::connect( this->ui->webView->page(), &QWebEnginePage::loadFinished, [ this ]() {
            this->ui->webView->page()->runJavaScript( QString( "{ const { molecules, shapes } = ChemDoodle.readJSON( '%1' ); sketcher.loadContent( molecules, shapes ); sketcher.repaint(); }" ).arg( this->json ));
        } );
    }

    // initialize supplementary script when page is loaded
    // setup toolBars and actions
    QWebEnginePage::connect( this->ui->webView->page(), &QWebEnginePage::loadFinished, [ this, resizeScript ]() {
        // create and render a webpage, using draw components
        QFile file( "chemDoodle/script.js" );
        if ( file.open( QFile::ReadOnly )) {
            const QString buffer( file.readAll());

            // close file
            file.close();

            // render page
            this->ui->webView->page()->runJavaScript( buffer );

            // setup toolBars
            QToolBar *bondToolBar = this->ui->contents->addToolBar( DrawDialog::tr( "Bond toolbar" ));
            bondToolBar->setObjectName( "bondToolBar" );
            this->ui->contents->addToolBarBreak();
            QToolBar *ringToolBar = this->ui->contents->addToolBar( DrawDialog::tr( "Ring toolbar" ));
            ringToolBar->setObjectName( "ringToolBar" );
            QToolBar *arrowToolBar = this->ui->contents->addToolBar( DrawDialog::tr( "Arrow toolbar" ));
            arrowToolBar->setObjectName( "arrowToolBar" );
            QToolBar *chargeToolBar = this->ui->contents->addToolBar( DrawDialog::tr( "Charge toolbar" ));
            chargeToolBar->setObjectName( "chargeToolBar" );
            this->ui->contents->addToolBarBreak();
            QToolBar *elementToolBar = this->ui->contents->addToolBar( DrawDialog::tr( "Element toolbar" ));
            elementToolBar->setObjectName( "elementToolBar" );

            // make actions exclusive via QActionGroup
            QActionGroup *group( new QActionGroup( this ));
            QMap<QString, QAction*> *toolButtons = new QMap<QString, QAction*>();

            // add action to toolBar lambda
            auto addToolButton = [ this, toolButtons, group ]( QToolBar *toolBar, const QIcon &icon, const QString &name, const QString &script, bool toggle ) {
                QAction *action( toolBar->addAction( icon, name, [ this, script ]() {
                    this->ui->webView->page()->runJavaScript( QString( "%1();" ).arg( script ));
                } ));

                if ( toggle ) {
                    //group->setExclusionPolicy()
                    action->setCheckable( true );
                    group->addAction( action );
                }

                toolButtons->insert( script, action );
                return action;
            };

            // add action to element toolBar lambda
            auto addElementButton = [ this, toolButtons, group, elementToolBar ]( const QIcon &icon, const QString &name, const QString &script, bool toggle ) {
                QAction *action( elementToolBar->addAction( icon, name, [ this, script ]() {
                    this->ui->webView->page()->runJavaScript( QString( "elementTool( '%1' );" ).arg( script ));
                } ));

                if ( toggle ) {
                    //group->setExclusionPolicy()
                    action->setCheckable( true );
                    group->addAction( action );
                }

                toolButtons->insert( script, action );
                return action;
            };

            // setup main toolBar
            addToolButton( this->ui->primaryToolBar, this->fetchIcon( "marquee" ), DrawDialog::tr( "Use marquee tool" ), "marqueeTool", true );
            QAction *lasso( addToolButton( this->ui->primaryToolBar, this->fetchIcon( "lasso" ), DrawDialog::tr( "Use lasso tool" ), "lassoTool", true ));
            addToolButton( this->ui->primaryToolBar, this->fetchIcon( "flip_horizontal" ), DrawDialog::tr( "Flip horizontally" ), "flipHTool", false );
            addToolButton( this->ui->primaryToolBar, this->fetchIcon( "flip_vertical" ), DrawDialog::tr( "Flip vertically" ), "flipVTool", false );
            addToolButton( this->ui->primaryToolBar, this->fetchIcon( "eraser" ), DrawDialog::tr( "Use eraser tool" ), "eraserTool", true );
            addToolButton( this->ui->primaryToolBar, this->fetchIcon( "centre" ), DrawDialog::tr( "Centre canvas" ), "centreTool", false );

            // setup arrow toolBar
            addToolButton( arrowToolBar, this->fetchIcon( "arrow" ), DrawDialog::tr( "Use arrow tool" ), "arrowTool", true );
            addToolButton( arrowToolBar, this->fetchIcon( "retrosynthetic" ), DrawDialog::tr( "Use retrosynthetic arrow tool" ), "retrosyntheticTool", true );
            addToolButton( arrowToolBar, this->fetchIcon( "resonance" ), DrawDialog::tr( "Use resonance arrow tool" ), "resonanceTool", true );
            addToolButton( arrowToolBar, this->fetchIcon( "equilibrium" ), DrawDialog::tr( "Use equilibrium arrow tool" ), "equilibriumTool", true );

            // setup element toolBar
            addElementButton( this->fetchIcon( "hydrogen" ), DrawDialog::tr( "Hydrogen atom" ), "H", true );
            addElementButton( this->fetchIcon( "carbon" ), DrawDialog::tr( "Carbon atom" ), "C", true );
            addElementButton( this->fetchIcon( "nitrogen" ), DrawDialog::tr( "Nitrogen atom" ), "N", true );
            addElementButton( this->fetchIcon( "oxygen" ), DrawDialog::tr( "Oxygen atom" ), "O", true );
            addElementButton( this->fetchIcon( "fluorine" ), DrawDialog::tr( "Fluorine atom" ), "F", true );
            addElementButton( this->fetchIcon( "chlorine" ), DrawDialog::tr( "Chlorine atom" ), "Cl", true );
            addElementButton( this->fetchIcon( "bromine" ), DrawDialog::tr( "Bromine atom" ), "Br", true );
            addElementButton( this->fetchIcon( "iodine" ), DrawDialog::tr( "Iodine atom" ), "I", true );
            addElementButton( this->fetchIcon( "phosphorus" ), DrawDialog::tr( "Phosphorus atom" ), "P", true );
            addElementButton( this->fetchIcon( "sulphur" ), DrawDialog::tr( "Sulphur atom" ), "S", true );
            addElementButton( this->fetchIcon( "silicon" ), DrawDialog::tr( "Silicon atom" ), "Si", true );
            addElementButton( this->fetchIcon( "label" ), DrawDialog::tr( "Label tool" ), "", true );

            // setup ring toolBar
            addToolButton( ringToolBar, this->fetchIcon( "benzene" ), DrawDialog::tr( "Draw benzene ring" ), "benzeneTool", true );
            addToolButton( ringToolBar, this->fetchIcon( "cyclohexane" ), DrawDialog::tr( "Draw cyclohexane ring" ), "cyclohexaneTool", true );
            addToolButton( ringToolBar, this->fetchIcon( "cyclopentane" ), DrawDialog::tr( "Draw cyclopentane ring" ), "cyclopentaneTool", true );
            addToolButton( ringToolBar, this->fetchIcon( "cyclopropane" ), DrawDialog::tr( "Draw cyclopropane ring" ), "cyclopropaneTool", true );
            addToolButton( ringToolBar, this->fetchIcon( "cyclobutane" ), DrawDialog::tr( "Draw cyclobutane ring" ), "cyclobutaneTool", true );
            addToolButton( ringToolBar, this->fetchIcon( "ring" ), DrawDialog::tr( "Draw variable ring" ), "varRingTool", true );
            addToolButton( ringToolBar, this->fetchIcon( "chain" ), DrawDialog::tr( "Draw chains" ), "chainTool", true );

            // setup bond toolBar
            addToolButton( bondToolBar, this->fetchIcon( "bond_solid" ), DrawDialog::tr( "Solid bond" ), "solidBondTool", true );
            addToolButton( bondToolBar, this->fetchIcon( "bond_double" ), DrawDialog::tr( "Double bond" ), "doubleBondTool", true );
            addToolButton( bondToolBar, this->fetchIcon( "bond_triple" ), DrawDialog::tr( "Triple bond" ), "tripleBondTool", true );
            addToolButton( bondToolBar, this->fetchIcon( "bond_wedged" ), DrawDialog::tr( "Wedged bond" ), "wedgedBondTool", true );
            addToolButton( bondToolBar, this->fetchIcon( "bond_wedged_hashed" ), DrawDialog::tr( "Wedged hashed bond" ), "wedgedHashedBondTool", true );
            addToolButton( bondToolBar, this->fetchIcon( "bond_wavy" ), DrawDialog::tr( "Wavy bond" ), "wavyBondTool", true );
            addToolButton( bondToolBar, this->fetchIcon( "bond_dashed" ), DrawDialog::tr( "Dashed bond" ), "dashedBondTool", true );
            addToolButton( bondToolBar, this->fetchIcon( "bond_double_dashed" ), DrawDialog::tr( "Double dashed bond" ), "doubleDashedBondTool", true );
            addToolButton( bondToolBar, this->fetchIcon( "bond_dotted" ), DrawDialog::tr( "Dotted bond" ), "dottedBondTool", true );

            // setup charge toolBar
            // TODO: add other buttons
            addToolButton( chargeToolBar, this->fetchIcon( "positive_charge" ), DrawDialog::tr( "Increase charge" ), "positiveChargeTool", true );
            addToolButton( chargeToolBar, this->fetchIcon( "negative_charge" ), DrawDialog::tr( "Decrease charge" ), "negativeChargeTool", true );

            // zoom actions
            QAction::connect( this->ui->actionZoomIn, &QAction::triggered, [ this ]() { this->ui->webView->page()->runJavaScript( "zoomInTool();" ); } );
            QAction::connect( this->ui->actionZoomOut, &QAction::triggered, [ this ]() { this->ui->webView->page()->runJavaScript( "zoomOutTool();" ); } );

            // copyCut lambda
            auto copyCut = [ this ]( bool cut = false ) {
                this->ui->webView->page()->runJavaScript( "{ const shapes = sketcher.shapes; const molecules = sketcher.molecules; ChemDoodle.writeJSON( molecules, shapes ); }", [ this, cut ]( const QVariant &v ) {
                    this->json = v.toByteArray();

                    this->ui->webView->page()->runJavaScript( cut ? "cutTool();" : "copyTool();", [ this ]( const QVariant &v ) {

                        QRegularExpression reg( "data:image\\/png;base64,(.+)" );
                        const QRegularExpressionMatch match( reg.match( v.toString()));
                        if ( match.hasMatch()) {
                            QString captured( match.captured( 1 ));

                            // get pixmap from base64 data
                            QByteArray array( QByteArray::fromBase64( QByteArray( captured.toUtf8().constData())));
                            QPixmap pixmap;
                            pixmap.loadFromData( array );

                            // the original image does not have aliasing and has ugly white artefacts
                            // therefore we must make alpha internally
                            const QPixmap out( PixmapUtils::cropAndRemoveAlpha( pixmap ));
                            const QByteArray pixmapData( PixmapUtils::toData( pixmap ));

                            // windows clipboard is terrible for transparent images
                            // therefore we put PNGs via mime data
                            QMimeData *propertyData( new QMimeData());
                            propertyData->setImageData( pixmap.toImage());
                            propertyData->setData( "PNG", pixmapData );
                            propertyData->setData( "image/png", pixmapData );

                            // also store json data
                            propertyData->setData( "application/json", this->json.toUtf8().constData());

                            // set clipboard
                            QGuiApplication::clipboard()->setMimeData( propertyData );
                        }
                    } );
                } );
            };

            // copy action
            QAction::connect( this->ui->actionCopy, &QAction::triggered, [ copyCut ]() {
                copyCut( false );
            } );

            // cut action
            QAction::connect( this->ui->actionCut, &QAction::triggered, [ copyCut ]() {
                copyCut( true );
            } );

            // paste action
            QAction::connect( this->ui->actionPaste, &QAction::triggered, [ this ]() {
                this->ui->webView->page()->runJavaScript( "pasteTool();" );
            } );

            // new action
            QAction::connect( this->ui->actionNew, &QAction::triggered, [ this ]() {
                this->ui->webView->page()->runJavaScript( "{ let hasContent = function() { const shapes = sketcher.shapes; const molecules = sketcher.molecules; return shapes.length > 0 || molecules.length > 0; }; hasContent(); }", [ this ]( const QVariant &v ) {
                    const bool hasContent = v.toBool();
                    if ( hasContent ) {
                        if ( QMessageBox::question( this, "", "Clear page?" ) == QMessageBox::No )
                            return;
                    }

                    this->ui->webView->page()->runJavaScript( "clearTool();" );
                    this->fileName.clear();
                } );
            } );

            // open action
            QAction::connect( this->ui->actionOpen, &QAction::triggered, [ this ]() {
                const QString fileName( QFileDialog::getOpenFileName( this, QWidget::tr( "Open canvas" ), "", QWidget::tr( "ChemDoodle canvas (*.json)" )));
                if ( fileName.isEmpty()) {
                    this->json.clear();
                    return;
                }

                QFile file( fileName );
                if ( file.open( QIODevice::ReadOnly )) {
                    this->json = file.readAll().constData();
                    file.close();

                    this->fileName = fileName;
                    this->ui->webView->page()->runJavaScript( QString( "{ const { molecules, shapes } = ChemDoodle.readJSON( '%1' ); sketcher.loadContent( molecules, shapes ); sketcher.repaint(); }" ).arg( this->json ));
                    return;
                }

                this->fileName.clear();
            } );

            // selectAll action
            QAction::connect( this->ui->actionSelectAll, &QAction::triggered, [ this, lasso ]() {
                lasso->trigger();
                this->ui->webView->page()->runJavaScript( QString( "selectAllTool();" ));
            } );

            // about action
            QAction::connect( this->ui->actionAbout, &QAction::triggered, []() { QDesktopServices::openUrl( QUrl( "https://web.chemdoodle.com/#explanation" )); } );

            // write lambda
            auto write = [ this ]( const QString &fileName, const bool storeFileName ) {
                this->ui->webView->page()->runJavaScript( "{ const shapes = sketcher.shapes; const molecules = sketcher.molecules; ChemDoodle.writeJSON( molecules, shapes ); }", [ this, storeFileName, fileName ]( const QVariant &v ) {
                    this->json = v.toString();

                    QFile file( fileName );
                    if ( file.open( QIODevice::WriteOnly )) {
                        file.write( this->json.toUtf8().constData());
                        file.close();

                        if ( storeFileName && this->fileName.isEmpty())
                            this->fileName = fileName;
                    } else {
                        this->fileName.clear();
                    }
                } );
            };

            // save lambda
            auto save = [ this, write ]( bool storeFileName ) {
                const QString fileName( QFileDialog::getSaveFileName( this, QWidget::tr( "Save canvas" ), "", QWidget::tr( "ChemDoodle canvas (*.json)" )));
                if ( fileName.isEmpty())
                    return;

                write( fileName, storeFileName );
            };

            // save action
            QAction::connect( this->ui->actionSave, &QAction::triggered, [ this, save, write ]() {
                if ( this->fileName.isEmpty()) {
                    save( true );
                } else {
                    write( this->fileName, false );
                }
            } );

            // undo action
            QAction::connect( this->ui->actionUndo, &QAction::triggered, [ this ]() { this->ui->webView->page()->runJavaScript( "sketcher.historyManager.undo();" );  } );

            // redo action
            QAction::connect( this->ui->actionRedo, &QAction::triggered, [ this ]() { this->ui->webView->page()->runJavaScript( "sketcher.historyManager.redo();" ); } );

            // save image action
            QAction::connect( this->ui->actionSaveImage, &QAction::triggered, [ this  ]() {
                const QString fileName( QFileDialog::getSaveFileName( this, QWidget::tr( "Save image" ), "", QWidget::tr( "Image (*.png)" )));
                if ( fileName.isEmpty())
                    return;

                this->ui->webView->page()->runJavaScript( "saveImageTool();", [ fileName ]( const QVariant &v ) {
                    QRegularExpression reg( "data:image\\/png;base64,(.+)" );

                    // js should output PNG in base64 format, so there we capture it...
                    const QRegularExpressionMatch match( reg.match( v.toString()));
                    if ( match.hasMatch()) {
                        QString captured( match.captured( 1 ));

                        // ...and reconstruct a pixmap from base64 data
                        QPixmap pixmap;
                        pixmap.loadFromData( QByteArray::fromBase64( QByteArray( captured.toUtf8().constData())));

                        if ( pixmap.isNull())
                            return ;

                        pixmap.save( fileName );
                    }
                } );
            } );

            // saveAs action
            QAction::connect( this->ui->actionSaveAs, &QAction::triggered, [ save ]() { save( false ); } );

            // finally resize to fit screen
            QTimer::singleShot( 50, [ this, resizeScript ]() { this->ui->webView->page()->runJavaScript( resizeScript ); } );

            QAction::connect( this->ui->actionColouredAtoms, &QAction::toggled, [ this ]( bool state ) {
                this->ui->webView->page()->runJavaScript( QString( "sketcher.styles.atoms_useJMOLColors = %1; sketcher.repaint();" ).arg( state ? "true" : "false" ));
            } );

            // we have fully initialized
            this->m_initialized = true;

            // restore state
            if ( !Variable::value<QVariant>( "drawDialog/geometry" ).isNull() && !Variable::value<QVariant>( "drawDialog/state" ).isNull()) {
                this->restoreGeometry( Variable::compressedByteArray( "drawDialog/geometry" ));
                this->ui->contents->restoreState( Variable::compressedByteArray( "drawDialog/state" ));
            }
        }
    } );
}

/**
 * @brief DrawDialog::on_buttonBox_accepted
 */
void DrawDialog::on_buttonBox_accepted() {
    this->getPixmapAndAccept();
}

/**
 * @brief DrawBridge::getLabel
 * @param defaultLabel
 */
void DrawBridge::getLabel( const QString &defaultLabel ) {
    DrawLabelEditor labelEditor( nullptr, defaultLabel );

    if ( labelEditor.exec() != QDialog::Accepted )
        return;

    const QString label( labelEditor.label());
    this->m_label = label.isEmpty() ? "C" : label;
    emit this->labelReady( label );
}
