﻿/*
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
#include "reagentdock.h"
#include "labeldock.h"
#include "theme.h"
#include "propertydock.h"
#include "searchengine.h"
#include "cache.h"
#include <QApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QSharedMemory>
#include <QSettings>
#include <QTranslator>
#include "pixmaputils.h"
#include "calcview.h"
#include "tableentry.h"
#include "tableproperty.h"
#ifdef Q_OS_WIN
#include "emfmime.h"
#endif
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QDesktopWidget>
#endif

/*
 TODO:

extraction:
  - better 'not found' and 'server busy' error handling in search
  - delete cache for a single reagent does not work

misc/unsorted:
 - view custom properties in a separate dialog
 - option to explicitly override property
 - ability to use labels in table
 - recursive filter pre 5.10
*/

/*
 NON-PRIORITY TODO:

future:
 - icons in "add property" menu
 - multiple references
 - use bind in queries
 - common reaction browser
 - molecule drawing (and search)
 - theme browser
 - all fileOpen dialogs must remember last location
 - warning when added property tag is hidden
 - first open dialog?
 - android companion app
 - scripted property extractor with multiple data sources
 - store variables (for example F = molarMass( "NaOH" )
   (not sure how to get a list of vars from globalObject, though)
 - property modifiers:
   boiling point: 43C @1 atm
                  11C @20 mbar
 - aliases (display names for reagents)
 - CoA search
 - molport search
 - label sub categories:
   Project1:
      solvents
      inorganics
   Project2:
      solvents
      organics
      intermediates
 - allow to display treeView in multiple columns
 - cut properties from reagents
 - sort tags instead of properties?
 - interactive label maker (label as in something you can
   print out and glue on a bottle)
 - tag sub categories
 - rely more on Variable::bind for updates
 - release candidate

properties:
 - filter in dock
 - experimental: replace QTextDocument delegates with QLabels
   and evalutate performance

i18n:
 - better i18n support
   input dialogs can have an i18n button, to localize names
   add reagent-> sodium hydroxide [flag button]
 - language selector in settings

database:
 - fix crash on argument count mismatch (can happen only when
   breaking API, so it is a non-issue right now)

scripting:
 - add additional functions such as mol( mass, reagent ) which returns:
   mol = mass * assay( reagent ) / molarMass( reagent )
 - add 'any' as batch name (a wildcard that chooses any batch with the
   property)
 - implement Avogadro constant, etc.
 - smart formulas such as 'purity' (uses assay, HPLC, 100-related substances,
   in that order; useful when assay is not defined)
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

    // register metaTypes
    //qRegisterMetaType<Reagent::Fields>();
    qRegisterMetaType<Id>();
    qRegisterMetaType<Row>();
    qRegisterMetaType<Table::Roles>();

    // i18n
    QTranslator translator;
#ifdef FORCE_EN_LOCALE
    const QString locale( "en_EN" );
#else
#ifndef FORCE_LV_LOCALE
    const QLocale locale( QLocale::system().name());
#else
    const QLocale locale( QLocale::Latvia );
#endif
#endif
    QLocale::setDefault( locale );
    if ( translator.load( ":/i18n/fumingCube_" + locale.name()))
        QApplication::installTranslator( &translator );

    // read initial history
    QFile file( !QString::compare( locale.name(), "lv_LV" ) ? ":/initial/calculator_history_lv_LV" : ":/initial/calculator_history" );
    QString history;
    if ( file.open( QIODevice::ReadOnly )) {
        history = Variable::compressString( file.readAll());
        file.close();
    }

    // set variable defaults
    Variable::add( "databasePath", "", Var::Flag::Hidden );
    Variable::add( "calculator/commands", "", Var::Flag::ReadOnly );
    Variable::add( "calculator/history", qAsConst( history ), Var::Flag::ReadOnly );
    Variable::add( "calculator/ans", "", Var::Flag::ReadOnly );
    Variable::add( "calculator/theme", "", Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::add( "calculator/zoom", 1.0, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::add( "mainWindow/geometry", QByteArray(), Var::Flag::ReadOnly );
    Variable::add( "mainWindow/state", QByteArray(), Var::Flag::ReadOnly );
    Variable::add( "reagentDock/selection", -1, Var::Flag::Hidden );
    Variable::add( "reagentDock/openNodes", "", Var::Flag::Hidden );
    Variable::add( "reagentDock/hiddenNodes", "", Var::Flag::Hidden );
    Variable::add( "reagentDock/deprecatedNodes", "", Var::Flag::Hidden );
    Variable::add( "propertyDock/hiddenTags", "", Var::Flag::Hidden );
    Variable::add( "darkMode", false, Var::Flag::ReadOnly | Var::Flag::Hidden | Var::Flag::NoSave );
    Variable::add( "overrideTheme", false, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::add( "theme", "light", Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::add( "fetchPropertiesOnAddition", false, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::add( "alwaysOnTop", false, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::add( "decimalSeparator", QString( QLocale::system().decimalPoint()), Var::Flag::Hidden );
    Variable::add( "searchFragment/history", "", Var::Flag::ReadOnly );
    Variable::add( "propertyFragment/selectedTags", "", Var::Flag::Hidden );
    Variable::add( "labelDock/selectedRows", "", Var::Flag::Hidden );

    // read configuration
    XMLTools::read();

#ifdef Q_OS_WIN
    EMFMime *emf( new EMFMime());
#endif

    // clean up on exit
    QApplication::connect( &a, &QApplication::aboutToQuit,
                           [
                       #ifdef Q_OS_WIN
                           emf
                       #endif
                           ]() {
#ifdef Q_OS_WIN
        delete emf;
#endif

        Variable::instance()->unbind( "labelDock/selectedRows" );

        // save zoom value
        Variable::setDecimalValue( "calculator/zoom", MainWindow::instance()->calcView()->zoom());

        Cache::instance()->writeReagentCache();

        NodeHistory::instance()->saveHistory();

        PropertyDock::instance()->saveHiddenTags();
        MainWindow::instance()->saveHistory();
        XMLTools::write();

        GarbageMan::instance()->clear();
        delete GarbageMan::instance();

        if ( Database::instance() != nullptr )
            delete Database::instance();

        delete Variable::instance();
        delete MainWindow::instance();
    } );

    // check for previous crashes
    const QString apiFileName( QDir::currentPath() + "/badapi" );
    if ( QFileInfo::exists( apiFileName )) {
        const QFileInfo info( Variable::string( "databasePath" ));

        // just change path
        Variable::setString( "databasePath", info.absolutePath() + "/database_"
                                                         + QDateTime::currentDateTime()
                             .toString( "yyyyMMdd_hhmmss" ) +
                             ".db" );
        // reset vars
        Variable::reset( "calculator/commands" );
        Variable::reset( "calculator/history" );
        Variable::reset( "calculator/ans" );
        Variable::reset( "reagentDock/selection" );
        Variable::reset( "reagentDock/openNodes" );
        Variable::reset( "propertyDock/hiddenTags" );
        Variable::reset( "reagentDock/hiddenNodes" );
        Variable::reset( "properyDock/selection" );
        Variable::reset( "searchFragment/history" );
        Variable::reset( "propertyFragment/selectedTags" );

        // copy built-in demo version
        QFile::copy( ":/initial/database.db", Variable::string( "databasePath" ));
        QFile( Variable::string( "databasePath" )).setPermissions(
                    QFileDevice::ReadOwner | QFileDevice::WriteOwner );

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
        success &= Database::instance()->add( TableEntry::instance());
        success &= Database::instance()->add( TableProperty::instance());


        if ( !Tag::instance()->count())
            Tag::instance()->populate();

        if ( !Label::instance()->count())
            Label::instance()->populate();

        Tag::instance()->sort( Tag::Name, Qt::AscendingOrder );
        //Tag::instance()->select();

        return success;
    };

    if ( !loadTables()) {
        QMessageBox::critical(
                    // FIXME
            #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                    QApplication::desktop(),
            #else
                    nullptr,
            #endif
                    QObject::tr( "Internal error" ),
                    QObject::tr( "Could not load database\n"
                                            "New database will be created\n"
                                            "Please restart the application" ),
                    QMessageBox::Ok );

        QFile badAPIFile( apiFileName );
        if ( badAPIFile.open( QIODevice::WriteOnly ))
            badAPIFile.close();

        QApplication::quit();
        return 0;
    }

    // detect dark mode
    bool darkMode = false;
    bool darkModeWin10 = false;
#ifdef Q_OS_WIN
    const QVariant key( QSettings( R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)",
                                   QSettings::NativeFormat ).value( "AppsUseLightTheme" ));
    if ( key.isValid() && !key.toBool()) {
        darkMode = true;
        darkModeWin10 = true;

        if ( !Variable::isEnabled( "overrideTheme" ))
            Variable::setString( "theme", "dark" );
    }
#else
    if ( qGray( QApplication::palette().color( QPalette::Base ).rgb()) < 128 )
        darkMode = true;
#endif

    // set default icon theme
    QIcon::setThemeName( darkMode ? "dark" : "light" );

    if ( Variable::isEnabled( "overrideTheme" ) || darkModeWin10 ) {
        // load theme from file
        auto *theme( new Theme( Variable::string( "theme" )));

        // override the variable
        Variable::setEnabled( "darkMode", theme->isDark());

        // override style and palette
        QApplication::setStyle( theme->style());
        QApplication::setPalette( theme->palette());

        // override icon theme and syntax highlighter theme
        QIcon::setThemeName( theme->isDark() ? "dark" : "light" );
        MainWindow::instance()->setTheme( theme );
    }

    // show main window
    MainWindow::instance()->setWindowFlag( Qt::WindowStaysOnTopHint, Variable::isEnabled( "alwaysOnTop" ));
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
    if ( Variable::value<QVariant>( "mainWindow/geometry" ).isNull() &&
         Variable::value<QVariant>( "mainWindow/state" ).isNull()) {
        MainWindow::instance()->resize( 1024, 650 );
        MainWindow::instance()->resizeDocks(
                    QList<QDockWidget *>() << PropertyDock::instance() << ReagentDock::instance() << LabelDock::instance(),
                    QList<int>() << 300 << 210 << 210, Qt::Horizontal );
        MainWindow::instance()->resizeDocks( QList<QDockWidget *>() << LabelDock::instance(), QList<int>() << 96,
                                             Qt::Vertical );
    }

    // restore last reagent selection
    ReagentDock::instance()->view()->updateView();

    // load search engines
    SearchEngineManager::instance()->loadSearchEngines();

    // read reagent cache
    Cache::instance()->readReagentCache();

    return QApplication::exec();
}
