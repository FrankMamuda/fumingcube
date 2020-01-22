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
#include "label.h"
#include "labelset.h"
#include "tag.h"
#include "script.h"
#include "reagentdock.h"
#include "labeldock.h"
#include "theme.h"
#include "reagentdock.h"
#include "propertydock.h"
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
 - multiple aliases?
 - up down arrow in dock does not change index (is this the intended behaviour)
 - store selection in NodeHisotry

database:
 - check API
 - crash on argument count mismatch

properties:
 - for now we use built in property extractor from PubChem
   in the future this should be fully scripted (per tag) and from multiple
   sources
 - filter in dock
 - icons in "add property" menu
 - solubility data as a property
 - must remove orphaned properties on tag removal
 - fetch 'other names' such as IUPAC name

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
 - add 'any' as batch name (a whildcard that chooses any batch with the
   property)
 - implement ans (history), Avogadro constant, etc.
 - smart formulas such as 'purity' (uses assay, HPLC, 100-related subtances,
   in that order; useful when assay is not defined)

settings:
 - option to change syntax highlighter (and font size) (partially supported)

misc:
 - store variables (for example F = molarMasss( "NaOH" )
   (not sure how to get a list of vars from globalObject, though)
 - unify text editor toolbars (in tagedit and propertyedit) as a separate
   class
 - some widgets (QListView in TagEditor, QTextEdit in PropertyEditor) are
   dark in light mode when windows theme set to dark

variables:
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
    Variable::instance()->add( "reagentDock/openNodes", "", Var::Flag::Hidden );
    Variable::instance()->add( "darkMode", false, Var::Flag::ReadOnly | Var::Flag::Hidden | Var::Flag::NoSave );
    Variable::instance()->add( "overrideTheme", false, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::instance()->add( "theme", "light", Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::instance()->add( "fetchPropertiesOnAddition", false, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::instance()->add( "alwaysOnTop", false, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::instance()->add( "decimalSeparator", ",", Var::Flag::Hidden );

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
        success &= Database::instance()->add( Label::instance());
        success &= Database::instance()->add( LabelSet::instance());

        if ( !Tag::instance()->count())
            Tag::instance()->populate();

        if ( !Label::instance()->count())
            Label::instance()->populate();

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
    LabelDock::instance()->setFloating( ReagentDock::instance()->isFloating());
    ReagentDock::instance()->setVisible( ReagentDock::instance()->isVisible());
    PropertyDock::instance()->setVisible( PropertyDock::instance()->isVisible());
    LabelDock::instance()->setVisible( PropertyDock::instance()->isVisible());
#endif

    // set initial sizes
    if ( Variable::instance()->value<QVariant>( "mainWindow/geometry" ).isNull() && Variable::instance()->value<QVariant>( "mainWindow/state" ).isNull()) {
        MainWindow::instance()->resize( 1024, 650 );
        MainWindow::instance()->resizeDocks( QList<QDockWidget*>() << PropertyDock::instance() << ReagentDock::instance() << LabelDock::instance(), QList<int>() << 300 << 210 << 210, Qt::Horizontal );
        MainWindow::instance()->resizeDocks( QList<QDockWidget*>() << LabelDock::instance(), QList<int>() << 96, Qt::Vertical );
    }

    // restore last reagent selection
    ReagentDock::instance()->view()->updateView();

    return a.exec();
}
