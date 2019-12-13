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
//     Groups with drag and drop (reagents can be in multiple groups)
//      Inorganic reagents
//          \_Sodium hydroxide
//      Bases
//          \_Sodium hydroxide
//
//  properties:
//  - for now we use built in property extractor from PubChem
//     in the future this should be fully scripted (per tag) and from multiple sources
//  - filter in dock
//  - icons in "add property" menu
//  - solubility data as a property
//
//  extraction:
//  - the current search URL is not exactly reliable
//    we must first search for the exact name, and then for synonyms
//    try butane (resolves to 1,4-butanediol)
//  - first search by exact name, if it returns no matches - search for
//    alike structures and present a cid list to choose from
//  - there must be an option to clear PubChem cache
//    if the wrong record was fetched, a proper one cannot be
//    fetched from the internet, since the app will use a cached
//    version by default
//    (OR FIND another way to store cached files)
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
//  settings:
//   - implement settings dialog and:
//     - option to change syntax highlighter (and font size)
//
//  misc:
//   - store images (formulas) fullsize but rescale in property view
//     this could be used for special props (add custom property -> image )
//     this however causes a performance penalty while resizing property
//     view. one option would be to use a precached image or sacrifice quality
//     with fast transform
//   - application icon for macOS and windows
//
//  variable:
//   - automatically store QByteArray as base64
//   - and QStringList as proper compressed string
//
//  future:
//   - common reaction browser
//   - molecule drawing (and search)
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
    //Variable::instance()->add( "decimalSeparator", ",", Var::Flag::Hidden );
    Variable::instance()->add( "propertyNameColumnSize", 128, Var::Flag::Hidden ); // // TODO: do we even need this now?
    Variable::instance()->add( "calculator/commands", "", Var::Flag::ReadOnly );
    Variable::instance()->add( "calculator/history", "", Var::Flag::ReadOnly );
    Variable::instance()->add( "mainWindow/geometry", QByteArray(), Var::Flag::ReadOnly );
    Variable::instance()->add( "mainWindow/state", QByteArray(), Var::Flag::ReadOnly );
    Variable::instance()->add( "reagentDock/selection", -1, Var::Flag::Hidden );
    Variable::instance()->add( "darkMode", false, Var::Flag::ReadOnly | Var::Flag::Hidden | Var::Flag::NoSave );
    Variable::instance()->add( "overrideTheme", false, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::instance()->add( "theme", "light", Var::Flag::ReadOnly | Var::Flag::Hidden );

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
    const QVariant key( QSettings( "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat ).value( "AppsUseLightTheme" ));
    if ( key.isValid() && !key.toBool())
        darkMode = true;
#else
    if ( qGray( qApp->palette().color( QPalette::Base ).rgb()) < 128 )
        darkMode = true;
#endif
    if ( Variable::instance()->isEnabled( "overrideTheme" )) {
        const QString theme( Variable::instance()->string( "theme" ));
        if ( !QString::compare( theme, "light" ))
            darkMode = false;
        else if ( !QString::compare( theme, "dark" ))
            darkMode = true;
    }

    QPalette palette;
    // apply fusion style
    // this does not look native, but is a good workaround for now
    a.setStyle( QStyleFactory::create( "Fusion" ));

    // TODO: sort this out
    // apply dark palette (macOS sets theme natively, so it might not need palette change?)
    if ( darkMode ) {
        // apply dark palette (taken from qt creator flat dark)
        palette.setColor( QPalette::Background, QColor::fromRgb( 46, 47, 48, 255 ));
        palette.setColor( QPalette::Window, QColor::fromRgb( 46, 47, 48, 255 ));
        palette.setColor( QPalette::WindowText, QColor::fromRgb( 208, 208, 208, 255 ));
        palette.setColor( QPalette::Base, QColor::fromRgb( 46, 47, 48, 255 ));
        palette.setColor( QPalette::AlternateBase, QColor::fromRgb( 53, 54, 55, 255 ));
        palette.setColor( QPalette::Button, QColor::fromRgb( 64, 66, 68, 255 ));
        palette.setColor( QPalette::BrightText, QColor::fromRgb( 255, 51, 51, 255 ));
        palette.setColor( QPalette::Text, QColor::fromRgb( 208, 208, 208, 255 ));
        palette.setColor( QPalette::ButtonText, QColor::fromRgb( 208, 208, 208, 255 ));
        palette.setColor( QPalette::ToolTipBase, QColor::fromRgb( 0, 0, 0, 102 ));
        palette.setColor( QPalette::Highlight, QColor::fromRgb( 96, 96, 96, 196 )); // a more gray-ish highlight than the default black
        palette.setColor( QPalette::Dark, QColor::fromRgb( 64, 66, 68, 255 ));
        palette.setColor( QPalette::HighlightedText, Qt::white );
        palette.setColor( QPalette::ToolTipText, QColor::fromRgb( 208, 208, 208, 255 ));
        palette.setColor( QPalette::Link, QColor::fromRgb( 0, 122, 244, 255 ));
        palette.setColor( QPalette::LinkVisited, QColor::fromRgb( 165, 122, 255, 255 ));
        palette.setColor( QPalette::Disabled, QPalette::ButtonText, QColor::fromRgb( 164, 166, 168, 96 ));
        palette.setColor( QPalette::Disabled, QPalette::Window, QColor::fromRgb( 68, 68, 68, 255 ));
        palette.setColor( QPalette::Disabled, QPalette::WindowText, QColor::fromRgb( 164, 166, 168, 96 ));
        palette.setColor( QPalette::Disabled, QPalette::Base, QColor::fromRgb( 68, 68, 68, 255 ));
        palette.setColor( QPalette::Disabled, QPalette::Text, QColor::fromRgb( 164, 166, 168, 96 ));
        palette.setColor( QPalette::Disabled, QPalette::HighlightedText, Qt::white );
        a.setPalette( qAsConst( palette ));

    } else {
        // apply dark palette (taken from default win10 theme)
        palette.setColor( QPalette::Background, QColor::fromRgb( 240, 240, 240, 255 ));
        palette.setColor( QPalette::Window, QColor::fromRgb( 240, 240, 240, 255 ));
        palette.setColor( QPalette::WindowText, QColor::fromRgb( 0, 0, 0, 255 ));
        palette.setColor( QPalette::Base, QColor::fromRgb( 255, 255, 255, 255 ));
        palette.setColor( QPalette::AlternateBase, QColor::fromRgb( 246, 246, 246, 255 ));
        palette.setColor( QPalette::Button, QColor::fromRgb( 240, 240, 240, 255 ));
        palette.setColor( QPalette::BrightText, QColor::fromRgb( 255, 255, 255, 255 ));
        palette.setColor( QPalette::Text, QColor::fromRgb( 0, 0, 0, 255 ));
        palette.setColor( QPalette::ButtonText, QColor::fromRgb( 0, 0, 0, 255 ));
        palette.setColor( QPalette::ToolTipBase, QColor::fromRgb( 255, 255, 220, 255 ));
        palette.setColor( QPalette::Highlight, QColor::fromRgb( 0, 120, 215, 255 ));
        palette.setColor( QPalette::Dark, QColor::fromRgb( 160, 160, 160, 255 ));
        palette.setColor( QPalette::HighlightedText, QColor::fromRgb( 255, 255, 255, 255 ));
        palette.setColor( QPalette::ToolTipText, QColor::fromRgb( 0, 0, 0, 255 ));
        palette.setColor( QPalette::Link, QColor::fromRgb( 0, 0, 255, 255 ));
        palette.setColor( QPalette::LinkVisited, QColor::fromRgb( 255, 0, 255, 255 ));
        palette.setColor( QPalette::Disabled, QPalette::ButtonText, QColor::fromRgb( 120, 120, 120, 255 ));
        palette.setColor( QPalette::Disabled, QPalette::Window, QColor::fromRgb( 240, 240, 240, 255 ));
        palette.setColor( QPalette::Disabled, QPalette::WindowText, QColor::fromRgb( 120, 120, 120, 255 ));
        palette.setColor( QPalette::Disabled, QPalette::Base, QColor::fromRgb( 240, 240, 240, 255 ));
        palette.setColor( QPalette::Disabled, QPalette::Text, QColor::fromRgb( 120, 120, 120, 255 ));
        palette.setColor( QPalette::Disabled, QPalette::HighlightedText, QColor::fromRgb( 255, 255, 255, 255 ));
        a.setPalette( qAsConst( palette ));
    }

    // set icon theme
    QIcon::setThemeName( darkMode ? "dark" : "light" );

    // store variable
    Variable::instance()->setEnabled( "darkMode", darkMode );

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
