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

//
// This is currently a STUB
//

//
// TODO:
//      merge with closed-source drawtool project

/*
 * includes
 */
#include "drawdialog.h"
#include "ui_drawdialog.h"
#include "variable.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QApplication>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QClipboard>
#include <QMimeData>
#include <QActionGroup>

/**
 * @brief DrawDialog::DrawDialog
 * @param parent
 */
DrawDialog::DrawDialog( QWidget *parent, const QString &, bool drawMode ) : QDialog( parent ), ui( new Ui::DrawDialog ) {
    this->ui->setupUi( this );
    this->ui->contents->setWindowFlags( Qt::Widget );

    // no button box in draw mode
    if ( drawMode )
        this->ui->buttonBox->hide();

    // make qml window
    this->quickView = new QQuickView();
    this->quickView->setResizeMode( QQuickView::SizeRootObjectToView );

    // set antialiasing
    QSurfaceFormat format;
    format.setSamples( 8 );
    this->quickView->setFormat( format );

    //Exporter exporter;
    //this->quickView->rootContext()->setContextProperty("Exporter", &exporter);

    // set source
    this->quickView->setSource( QUrl( "qrc:/main.qml" ));

    // make window container
    QWidget *container = QWidget::createWindowContainer( this->quickView );
    this->ui->contentsGrid->addWidget( container );


    // load draw component if already available
    this->loadComponent();
}

/**
 * @brief DrawDialog::~DrawDialog
 */
DrawDialog::~DrawDialog() {
    // disconnect canvas resize
    QTimer::disconnect( &this->resizeTimer, &QTimer::timeout, this, nullptr );

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

    // get rid of ui
    delete this->ui;
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
    qApp->quit();
    QMetaObject::invokeMethod( qApp, "quit", Qt::QueuedConnection );
}

/**
 * @brief DrawDialog::keyReleaseEvent
 */
void DrawDialog::keyReleaseEvent( QKeyEvent *event ) {
    if ( event->key() == Qt::Key_Delete ) {
        //QMetaObject::invokeMethod( this->ui->quick->rootObject(), "deleteContent" );
        //event->ignore();
        return;
    }

    QDialog::keyReleaseEvent( event );
}

/**
 * @brief DrawDialog::loadComponent
 */
void DrawDialog::loadComponent() {
    // setup resize timer
    this->resizeTimer.setSingleShot( true );
    QTimer::connect( &this->resizeTimer, &QTimer::timeout, this, []() {
        // STUB: do smth
    } );

    // setup toolBars
    QToolBar *bondToolBar = this->ui->contents->addToolBar( DrawDialog::tr( "Bond toolbar" ));
    bondToolBar->setObjectName( "bondToolBar" );
    this->ui->contents->addToolBarBreak();
    QToolBar *ringToolBar = this->ui->contents->addToolBar( DrawDialog::tr( "Ring toolbar" ));
    ringToolBar->setObjectName( "ringToolBar" );
    QToolBar *shapeToolBar = this->ui->contents->addToolBar( DrawDialog::tr( "Shape toolbar" ));
    shapeToolBar->setObjectName( "shapeToolBar" );
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
        QAction *action( toolBar->addAction( icon, name, [ this, script, toggle ]() {
            if ( toggle )
                this->rootObject()->setProperty( "currentTool", script );
            else
                QMetaObject::invokeMethod( this->rootObject(), script.toUtf8().constData());
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
            // use Silicon debugger for now
            if ( !QString::compare( script, "Si" )) {
                this->rootObject()->setProperty( "currentTool", "debugTool" );
                return;
            }

            this->rootObject()->setProperty( "currentLabel", script );
            this->rootObject()->setProperty( "currentTool", "labelTool" );

            // STUB
            qDebug() << script;
        } ));

        if ( toggle ) {
            //group->setExclusionPolicy()
            action->setCheckable( true );
            group->addAction( action );
        }

        toolButtons->insert( script, action );
        return action;
    };


    // add action to bond toolBar lambda
    auto addBondButton = [ this, toolButtons, group, bondToolBar ]( const QIcon &icon, const QString &name, const QString &script, bool toggle ) {
        QAction *action( bondToolBar->addAction( icon, name, [ this, script ]() {
            this->rootObject()->setProperty( "currentBondType", script );
            this->rootObject()->setProperty( "currentTool", "bondTool" );
        } ));

        if ( toggle ) {
            action->setCheckable( true );
            group->addAction( action );
        }

        toolButtons->insert( script, action );
        return action;
    };


    // add action to shape toolBar lambda
    auto addShapeButton = [ this, toolButtons, group, shapeToolBar ]( const QIcon &icon, const QString &name, const QString &script, bool toggle ) {
        QAction *action( shapeToolBar->addAction( icon, name, [ this, script ]() {
            this->rootObject()->setProperty( "currentShapeType", script );
            this->rootObject()->setProperty( "currentTool", "shapeTool" );
        } ));

        if ( toggle ) {
            action->setCheckable( true );
            group->addAction( action );
        }

        toolButtons->insert( script, action );
        return action;
    };

    // setup main toolBar
    QAction *marqueeTool( addToolButton( this->ui->primaryToolBar, this->fetchIcon( "marquee" ), DrawDialog::tr( "Use marquee tool" ), "marqueeTool", true ));
    addToolButton( this->ui->primaryToolBar, this->fetchIcon( "lasso" ), DrawDialog::tr( "Use lasso tool" ), "lassoTool", true );
    addToolButton( this->ui->primaryToolBar, this->fetchIcon( "flip_horizontal" ), DrawDialog::tr( "Flip horizontally" ), "flipHorizontally", false );
    addToolButton( this->ui->primaryToolBar, this->fetchIcon( "flip_vertical" ), DrawDialog::tr( "Flip vertically" ), "flipVertically", false );
    addToolButton( this->ui->primaryToolBar, this->fetchIcon( "eraser" ), DrawDialog::tr( "Use eraser tool" ), "eraserTool", true );
    addToolButton( this->ui->primaryToolBar, this->fetchIcon( "info" ), DrawDialog::tr( "Paste info label" ), "infoTool", false );

    // setup shape toolBar
    addShapeButton( this->fetchIcon( "arrow" ), DrawDialog::tr( "Use arrow tool" ), "arrow", true );
    addShapeButton( this->fetchIcon( "retrosynthetic" ), DrawDialog::tr( "Use retrosynthetic arrow tool" ), "retrosyntheticArrow", true );
    addShapeButton( this->fetchIcon( "resonance" ), DrawDialog::tr( "Use resonance arrow tool" ), "resonanceArrow", true );
    addShapeButton( this->fetchIcon( "equilibrium" ), DrawDialog::tr( "Use equilibrium arrow tool" ), "equilibriumArrow", true );

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

    addBondButton( this->fetchIcon( "bond_solid" ), DrawDialog::tr( "Solid bond" ), "single", true );
    addBondButton( this->fetchIcon( "bond_double" ), DrawDialog::tr( "Double bond" ), "double", true );
    addBondButton( this->fetchIcon( "bond_triple" ), DrawDialog::tr( "Triple bond" ), "triple", true );
    addBondButton( this->fetchIcon( "bond_wedged" ), DrawDialog::tr( "Wedged bond" ), "wedged", true );
    addBondButton( this->fetchIcon( "bond_wedged_hashed" ), DrawDialog::tr( "Hashed wedged bond" ), "hashedWedged", true ); // rename me
    addBondButton( this->fetchIcon( "bond_wavy" ), DrawDialog::tr( "Wavy bond" ), "wavy", true );
    addBondButton( this->fetchIcon( "bond_dashed" ), DrawDialog::tr( "Dashed bond" ), "dashed", true );
    //addBondButton( this->fetchIcon( "bond_double_dashed" ), DrawDialog::tr( "Double dashed bond" ), "doubleDashed", true );
    //addBondButton( this->fetchIcon( "bond_dotted" ), DrawDialog::tr( "Dotted bond" ), "dotted", true );
    addBondButton( this->fetchIcon( "bond_hashed" ), DrawDialog::tr( "Hashed bond" ), "hashed", true );
    addBondButton( this->fetchIcon( "bond_bold" ), DrawDialog::tr( "Bold bond" ), "bold", true );
    addBondButton( this->fetchIcon( "bond_hollow_wedged" ), DrawDialog::tr( "Hollow wedged bond" ), "hollowWedged", true );

    // setup charge toolBar
    // TODO: add other buttons
    addToolButton( chargeToolBar, this->fetchIcon( "positive_charge" ), DrawDialog::tr( "Increase charge" ), "positiveChargeTool", true );
    addToolButton( chargeToolBar, this->fetchIcon( "negative_charge" ), DrawDialog::tr( "Decrease charge" ), "negativeChargeTool", true );

    // zoom actions
    QAction::connect( this->ui->actionZoomIn, &QAction::triggered, []() { qDebug() << "zoomInTool();"; } );
    QAction::connect( this->ui->actionZoomOut, &QAction::triggered, []() { qDebug() << "zoomOutTool();"; } );

    // copy action
    QAction::connect( this->ui->actionCopy, &QAction::triggered, [ this ]() { qDebug() << "copy();"; QMetaObject::invokeMethod( this->rootObject(), "copy" ); } );

    // cut action
    QAction::connect( this->ui->actionCut, &QAction::triggered, []() { qDebug() << "cut();"; } );

    // paste action
    QAction::connect( this->ui->actionPaste, &QAction::triggered, [ this ]() {
        qDebug() << "paste();";

        const QString data( QGuiApplication::clipboard()->mimeData()->text());

        if ( !data.isEmpty())
            QMetaObject::invokeMethod( this->rootObject(), "paste", Qt::AutoConnection, Q_ARG( QVariant, data )); }
    );

    // new action
    QAction::connect( this->ui->actionNew, &QAction::triggered, []() { qDebug() << "new();"; } );

    // open action
    QAction::connect( this->ui->actionOpen, &QAction::triggered, [ this ]() {
        const QString fileName( QFileDialog::getOpenFileName( this, QWidget::tr( "Open canvas" ), "", QWidget::tr( "JSON canvas (*.json)" )));
        if ( fileName.isEmpty()) {
            this->json.clear();
            return;
        }

        QFile file( fileName );
        if ( file.open( QIODevice::ReadOnly )) {
            this->json = file.readAll().constData();
            file.close();

            this->fileName = fileName;

            QMetaObject::invokeMethod( this->rootObject(), "quickLoad", Q_ARG( QVariant, QVariant( this->json )));

            return;
        }

        this->fileName.clear();
    } );

    // selectAll action
    QAction::connect( this->ui->actionSelectAll, &QAction::triggered, [ this ]() { QMetaObject::invokeMethod( this->rootObject(), "selectAll" ); } );

    // about action
    QAction::connect( this->ui->actionAbout, &QAction::triggered, []() { qDebug() << "about();"; } );

    // write lambda
    /*auto write = [ this ]( const QString &fileName, const bool storeFileName ) {
        QFile file( fileName );
        if ( file.open( QIODevice::WriteOnly )) {
            file.write( this->json.toUtf8().constData());
            file.close();

            if ( storeFileName && this->fileName.isEmpty())
                this->fileName = fileName;
        } else {
            this->fileName.clear();
        }
    };*/

    // save lambda
    /**auto save = [ this, write ]( bool storeFileName ) {
        const QString fileName( QFileDialog::getSaveFileName( this, QWidget::tr( "Save canvas" ), "", QWidget::tr( "JSON canvas (*.json)" )));
        if ( fileName.isEmpty())
            return;

        qDebug()  << "SAVE LAMBDA" << fileName << storeFileName;

        write( fileName, storeFileName );
    };*/

    // save action
   /*QAction::connect( this->ui->actionSave, &QAction::triggered, [ this, save, write ]() {
        QVariant json;
        QMetaObject::invokeMethod( this->rootObject(), "quickSave", Q_RETURN_ARG( QVariant, json ));
        this->json = json.toString();

        qDebug() << "save from dialog";

        if ( this->fileName.isEmpty()) {
            save( true );
        } else {
            write( this->fileName, false );
        }
    } );*/

    // save action
    QAction::connect( this->ui->actionSave, &QAction::triggered, [ this ]() {
        QMetaObject::invokeMethod( this->rootObject(), "save" );
    } );

    // undo action
    QAction::connect( this->ui->actionUndo, &QAction::triggered, [ this ]() {
        QMetaObject::invokeMethod( this->rootObject(), "undo" );
    } );

    // redo action
    QAction::connect( this->ui->actionRedo, &QAction::triggered, [ this ]() {
        QMetaObject::invokeMethod( this->rootObject(), "redo" );
    } );

    // save image action
    QAction::connect( this->ui->actionSaveImage, &QAction::triggered, [ this ]() {
        QMetaObject::invokeMethod( this->rootObject(), "saveImage" );
    } );

    // saveAs action
    //QAction::connect( this->ui->actionSaveAs, &QAction::triggered, [ save ]() { qDebug() << "SAVEAS"; save( false ); } );

    // we have fully initialized
    this->m_initialized = true;

    // restore state
    if ( !Variable::value<QVariant>( "drawDialog/geometry" ).isNull() && !Variable::value<QVariant>( "drawDialog/state" ).isNull()) {
        this->restoreGeometry( Variable::compressedByteArray( "drawDialog/geometry" ));
        this->ui->contents->restoreState( Variable::compressedByteArray( "drawDialog/state" ));
    }

    // trigger
    marqueeTool->trigger();
}

/**
 * @brief DrawDialog::rootObject
 * @return
 */
QQuickItem *DrawDialog::rootObject() const {
    return this->quickView->rootObject();
}

/**
 * @brief DrawDialog::on_commandEdit_returnPressed
 */
void DrawDialog::on_commandEdit_returnPressed() {
    const QString command( this->ui->commandEdit->text());

    if ( !command.isEmpty()) {
        QMetaObject::invokeMethod( this->rootObject(), command.toUtf8().constData());
        this->ui->commandEdit->clear();
    }
}
