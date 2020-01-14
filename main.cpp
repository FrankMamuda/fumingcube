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
#include "theme.h"
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

/*
 TODO:

reagents:
 - richtext for names (currently not feasible)
 - multiple aliases?
 - groups and sorting (with drag and drop; reagents can be in multiple groups)
   Inorganic reagents
        \_Sodium hydroxide
   Bases
        \_Sodium hydroxide
 - up down arrow in dock does not change index (is this the intended behaviour)
 - sort alphabetically (currently broken)
 - option to duplicate reagent?
 - renaming should restore index
 - better yet -> store all opened nodes

properties:
 - for now we use built in property extractor from PubChem
   in the future this should be fully scripted (per tag) and from multiple
   sources
 - filter in dock
 - icons in "add property" menu
 - solubility data as a property
 - script editing in tag dialog
 - must remove orphaned properties on tag removal
 - state property
 - fetch 'other names' such as IUPAC name
 - add multiline edit option in textual properties (tagged, not custom)
 - decimal separator for copy actions

extraction:
 - unified caching solution (cidLists, images, etc.) (in progress)
 - tag selection for extraction (user might not need all tags)

completion:
 - fix reagent completion (does not work as indended)
 - complete batch from selected reagent, not the whole list
 - complete function( "CURSOR to function( "CURSOR"

scripting:
 - add additional functions such as mol( mass, reagent ) which returns:
   mol = mass * assay( reagent ) / molarMass( reagent )
 - add any as batch name (a whildcard that chooses any batch with the
   property)
 - check API
 - implement ans (history), Avogadro constant, etc.
 - fix ** comments in syntax highlighter
 - clickable calculator references (opens corresponding reagent)
 - smart formulas such as 'purity' (uses assay, HPLC, 100-related subtances,
   in that order; useful when assay is not defined)

settings:
 - option to change syntax highlighter (and font size) (partially supported)

misc:
 - store variables (for example F = molarMasss( "NaOH" )
   (not sure how to get a list of vars from globalObject, though)
 - unify text editor toolbars (in tagedit and propertyedit) as a separate
   class

variable:
 - automatically store QByteArray as base64
 - and QStringList as proper compressed string

future:
 - common reaction browser
 - molecule drawing (and search)
 - tables (make custom tables reagents and select properties)
*/

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

    // read initial history
    QFile file( ":/initial/calculator_history" );
    QString history;
    if ( file.open( QIODevice::ReadOnly )) {
        history = Variable::compressedString( file.readAll());
        file.close();
    }

    // set variable defaults
    Variable::instance()->add( "databasePath", "", Var::Flag::Hidden );
    //Variable::instance()->add( "decimalSeparator", ",", Var::Flag::Hidden );
    Variable::instance()->add( "calculator/commands", "", Var::Flag::ReadOnly );
    Variable::instance()->add( "calculator/history", qAsConst( history ), Var::Flag::ReadOnly );
    Variable::instance()->add( "mainWindow/geometry", QByteArray(), Var::Flag::ReadOnly );
    Variable::instance()->add( "mainWindow/state", QByteArray(), Var::Flag::ReadOnly );
    Variable::instance()->add( "reagentDock/selection", -1, Var::Flag::Hidden );
    Variable::instance()->add( "darkMode", false, Var::Flag::ReadOnly | Var::Flag::Hidden | Var::Flag::NoSave );
    Variable::instance()->add( "overrideTheme", false, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::instance()->add( "theme", "light", Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::instance()->add( "fetchPropertiesOnAddition", false, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::instance()->add( "alwaysOnTop", false, Var::Flag::ReadOnly | Var::Flag::Hidden );

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
        delete MainWindow::instance();
    } );

    // check for previous crashes
    if ( QFileInfo( apiFileName ).exists()) {
        const QFileInfo info( Variable::instance()->string( "databasePath" ));

        // just change path
        Variable::instance()->setString( "databasePath", info.absolutePath() + "/database_"
                                         + QDateTime::currentDateTime()
                                         .toString( "yyyyMMdd_hhmmss" ) +
                                         ".db" );

        // copy built-in demo version
        QFile::copy( ":/initial/database.db", Variable::instance()->string( "databasePath" ));
        QFile( Variable::instance()->string( "databasePath" )).setPermissions( QFileDevice::ReadOwner | QFileDevice::WriteOwner );

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

    // detect dark mode
    bool darkMode = false;
    bool darkModeWin10 = false;
#ifdef Q_OS_WIN
    const QVariant key( QSettings( "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat ).value( "AppsUseLightTheme" ));
    if ( key.isValid() && !key.toBool()) {
        darkMode = true;
        darkModeWin10 = true;

        if ( !Variable::instance()->isEnabled( "overrideTheme" ))
            Variable::instance()->setString( "theme", "dark" );
    }
#else
    if ( qGray( qApp->palette().color( QPalette::Base ).rgb()) < 128 )
        darkMode = true;
#endif

    // set default icon theme
    QIcon::setThemeName( darkMode ? "dark" : "light" );

    if ( Variable::instance()->isEnabled( "overrideTheme" ) || darkModeWin10 ) {
        // load theme from file
        Theme *theme( new Theme( QString( ":/themes/%1.theme" ).arg( Variable::instance()->string( "theme" ))));

        // override the variable
        Variable::instance()->setEnabled( "darkMode", theme->isDark());

        // override style and palette
        a.setStyle( theme->style());
        a.setPalette( theme->palette());

        // override icon theme and syntax highlighter theme
        QIcon::setThemeName( theme->isDark() ? "dark" : "light" );
        MainWindow::instance()->setTheme( theme );
    }

    // show main window
    MainWindow::instance()->setWindowFlag( Qt::WindowStaysOnTopHint, Variable::instance()->isEnabled( "alwaysOnTop" ));
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
