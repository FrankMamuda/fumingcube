﻿/*
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
#include "database.h"
#include "main.h"
#include "mainwindow.h"
#include "table.h"
#include "reagent.h"
#include "variable.h"
#include "xmltools.h"
#include "property.h"
#include "tag.h"
#include "script.h"
#include "reagentdock.h"
#ifdef Q_OS_LINUX
#include "reagentdock.h"
#include "propertydock.h"
#endif
#include <QApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QSharedMemory>
#include <QSettings>
#include <QStyleFactory>

//
// TODO:
//
//  reagents:
//   - richtext for names?
//   - multiple aliases?
//   - groups, sorting
//
//  properties:
//   - fix extraction and add support for propery mapping to tags
//   - use pubchem.ncbi.nlm.nih.gov
//     first get cid https://pubchem.ncbi.nlm.nih.gov/rest/pug/substance/name/sodium-hydroxide/cids/TXT
//     then fetch properties https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/Sodium-hydroxide/property/MolecularWeight/TXT
//          CAS, MolecularWeight, formula (not much else)
//     or parse https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/CID/JSON (CID=14798 for sodium hydroxide)
//     for additional properties such as solubility, etc.
//     https://pubchem.ncbi.nlm.nih.gov/rest/pug_view/data/compound/14798/JSON?heading=Molecular+Weight
//     https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/14798/PNG
//  - for now we use built in property extractor from PubChem
//     in the future this should be fully scripted (per tag) and from multiple sources
//  - fix negative values from extraction (melting point, etc.)
//
//  completion:
//   - complete batch from selected reagent, not the whole list
//   - complete function( "CURSOR to function( "CURSOR"
//
//  scripting:
//   - add additional functions such as mol( mass, reagent ) which returns:
//     mol = mass * assay( reagent ) / molarMass( reagent )
//   - add any as batch name (a whildcard that chooses any batch with the property)
//   - check API
//
//  misc:
//   - store images (formulas) fullsize but rescale in property view
//     this could be used for special props (add custom property -> image )
//     this however causes a performance penalty while resizing property
//     view. one option would be to use a precached image or sacrifice quality
//     with fast transform
//
//  variable:
//   - automatically store QByteArray as base64
//
//  future:
//   - common reaction browser
//   - molecule drawing (and search)
//
//

/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char *argv[] ) {
    QApplication a( argc, argv );

    // simple single instance implementation
    // NOTE: this however will fail if app crashes during startup
#ifndef QT_DEBUG
    class SharedMemory : public QSharedMemory {
    public:
        SharedMemory( const QString &key, QObject *parent = nullptr ) : QSharedMemory( key, parent ) {}
        ~SharedMemory() { if ( this->isAttached()) { this->detach(); } }

        bool lock() {
            if ( this->isAttached()) return false;
            if ( this->attach( QSharedMemory::ReadOnly )) { this->detach(); return false; }
            return this->create( sizeof( quint64 ));
        }
    };
    QSharedPointer<SharedMemory> sharedMemory( new SharedMemory( "fumingCube_singleInstance", &a ));
    if ( !sharedMemory->lock())
        return 0;
#endif

    // dummy file
    const QString apiFileName( QDir::currentPath() + "/badapi" );

    // register metatypes
    //qRegisterMetaType<Reagent::Fields>();
    qRegisterMetaType<Id>();
    qRegisterMetaType<Row>();
    qRegisterMetaType<Table::Roles>();
    qRegisterMetaType<QList<QList<qreal> >>( "QList<QList<qreal> >");

    // set variable defaults
    Variable::instance()->add( "databasePath", "", Var::Flag::Hidden );
    Variable::instance()->add( "decimalSeparator", ",", Var::Flag::Hidden );
    Variable::instance()->add( "propertyNameColumnSize", 128, Var::Flag::Hidden );
    Variable::instance()->add( "system/consoleHistory", "", Var::Flag::ReadOnly );
    Variable::instance()->add( "calculator/history", "", Var::Flag::ReadOnly );
    Variable::instance()->add( "mainWindow/geometry", QByteArray(), Var::Flag::ReadOnly );
    Variable::instance()->add( "mainWindow/state", QByteArray(), Var::Flag::ReadOnly );
    Variable::instance()->add( "reagentDock/selection", -1, Var::Flag::Hidden );

    // read configuration
    XMLTools::instance()->read();

    // clean up on exit
    qApp->connect( qApp, &QApplication::aboutToQuit, []() {
        MainWindow::instance()->saveHistory();
        XMLTools::instance()->write();

        GarbageMan::instance()->clear();
        delete GarbageMan::instance();

        if ( Database::instance() != nullptr )
            delete Database::instance();

        delete Variable::instance();
    } );

    // check for previous crashes
    if ( QFileInfo( apiFileName ).exists()) {
        const QFileInfo info( Variable::instance()->string( "databasePath" ));

        // just change path
        Variable::instance()->setString( "databasePath", info.absolutePath() + "/database_"
                                         + QDateTime::currentDateTime()
                                         .toString( "yyyyMMdd_hhmmss" ) +
                                         ".db" );
        QFile::remove( apiFileName );
    }

    // initialize database and its tables
    Database::instance();
    auto loadTables = []() {
        bool success = true;
        success &= Database::instance()->add( Reagent::instance());
        success &= Database::instance()->add( Property::instance());
        success &= Database::instance()->add( Tag::instance());

        if ( !Tag::instance()->count())
            Tag::instance()->populate();

        return success;
    };

    if ( !loadTables()) {
        QMessageBox::critical( QApplication::desktop(),
                               QObject::tr( "Internal error" ),
                               QObject::tr( "Could not load database\n"
                                            "New database will be created\n"
                                            "Please restart the application" ),
                               QMessageBox::Ok );

        QFile file( QDir::currentPath() + "/badapi" );
        if ( file.open( QIODevice::WriteOnly ))
            file.close();

        QApplication::quit();
        return 0;
    }

    bool darkMode = false;


#ifdef Q_OS_WIN
    if ( !QSettings( "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat ).value( "AppsUseLightTheme" ).toBool()) {
        darkMode = true;
        // TODO: apply dark palette for windows
    }
#else
    if ( qGray( qApp->palette().color( QPalette::Base ).rgb()) < 128 ) {
        darkMode = true;

#ifdef Q_OS_LINUX
        // TODO: apply dark palette for linux (macOS sets dark mode natively)
#endif
    }
#endif

    // set icon theme
    QIcon::setThemeName( darkMode ? "dark" : "light" );

    // show main window
    MainWindow::instance()->show();
    MainWindow::instance()->scrollToBottom();

#ifdef Q_OS_LINUX
    // fixes issues with dockwidgets on linux
    ReagentDock::instance()->setFloating( ReagentDock::instance()->isFloating());
    PropertyDock::instance()->setFloating( PropertyDock::instance()->isFloating());
    ReagentDock::instance()->setVisible( ReagentDock::instance()->isVisible());
    PropertyDock::instance()->setVisible( PropertyDock::instance()->isVisible());
#endif

    // restore last reagent selection
    ReagentDock::instance()->restoreIndex();

    return a.exec();
}
