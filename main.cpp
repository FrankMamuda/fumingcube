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
#include <QDesktopWidget>
#include <QSharedMemory>
#include <QSettings>
#include <QTranslator>

/*
 TODO:

database:
 - fix crash on argument count mismatch
 - expand built-in database with more reagents

i18n:
 - language selector in settings

properties:
 - filter in dock

extraction:
 - unified caching solution (cidLists, images, etc.) (in progress)
 - tag selection for extraction (user might not need all tags)

theming:
 - separate app theme from calculator theme
   (calculator window background styling to be defined independantly from
    app window background)
 - option to change syntax highlighter (and font size) (partially supported)

future/non-priority:
 - icons in "add property" menu
 - multiple references
 - use bind in queries
 - common reaction browser
 - molecule drawing (and search)
 - tables (make custom tables reagents and select properties)
 - theme browser
 - menu option to add image from clipboard
 - all fileOpen dialogs must remember last location
 - warning when added property tag is hidden
 - first open dialog?
 - android companion app
 - scripted property extractor with multiple data sources
 - store variables (for example F = molarMass( "NaOH" )
   (not sure how to get a list of vars from globalObject, though)

scripting/non-priority:
 - add additional functions such as mol( mass, reagent ) which returns:
   mol = mass * assay( reagent ) / molarMass( reagent )
 - add 'any' as batch name (a wildcard that chooses any batch with the
   property)
 - implement Avogadro constant, etc.
 - smart formulas such as 'purity' (uses assay, HPLC, 100-related substances,
   in that order; useful when assay is not defined)

misc/unsorted:
  - cut properties from reagents
  - allow to display treeView in multiple columns
  - remove extra <br> at the end of some properties
  - double check all add/edit/delete buttons for when they should be enabled or not
  - sort batches by addition date
  - better i18n support
  - caching for search dialog
  - better ui for search dialog (less dialogs)
    (currently four steps search->similar->add->extract)
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
#ifndef FORCE_LV_LOCALE
    const QString locale( QLocale::system().name());
#else
    const QString locale( "lv_LV" );
    QLocale::setDefault( locale );
#endif
    translator.load( ":/i18n/fumingCube_" + locale );
    QApplication::installTranslator( &translator );

    // read initial history
    QFile file( !QString::compare( locale, "lv_LV" ) ? ":/initial/calculator_history_lv_LV" : ":/initial/calculator_history" );
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
    Variable::add( "mainWindow/geometry", QByteArray(), Var::Flag::ReadOnly );
    Variable::add( "mainWindow/state", QByteArray(), Var::Flag::ReadOnly );
    Variable::add( "reagentDock/selection", -1, Var::Flag::Hidden );
    Variable::add( "reagentDock/openNodes", "", Var::Flag::Hidden );
    Variable::add( "reagentDock/hiddenNodes", "", Var::Flag::Hidden );
    Variable::add( "propertyDock/hiddenTags", "", Var::Flag::Hidden );
    Variable::add( "darkMode", false, Var::Flag::ReadOnly | Var::Flag::Hidden | Var::Flag::NoSave );
    Variable::add( "overrideTheme", false, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::add( "theme", "light", Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::add( "fetchPropertiesOnAddition", false, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::add( "alwaysOnTop", false, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::add( "decimalSeparator", QString( QLocale::system().decimalPoint()), Var::Flag::Hidden );

    // read configuration
    XMLTools::read();

    // clean up on exit
    QApplication::connect( &a, &QApplication::aboutToQuit, []() {
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

    // test cache
    /*qDebug() << Cache::instance()->contains( "temp", "value" );

    Cache::instance()->insert( "temp", "value", "testValue" );
    qDebug() << Cache::instance()->getData( "temp", "value" );

    qDebug() << Cache::instance()->contains( "temp", "value" );

    Cache::instance()->insert( "temp", "value", "override" );
    qDebug() << Cache::instance()->getData( "temp", "value" );

    Cache::instance()->clear( "temp", "value" );

    qDebug() << Cache::instance()->contains( "temp", "value" );
    qDebug() << Cache::instance()->getData( "temp", "value" );

    qDebug() << Cache::instance()->getData( "temp", "value" );
    qDebug() << "validate 1" << ( Cache::validate( "penguin", "key" ));
    qDebug() << "validate 2" << ( Cache::validate( "penguin", "ke%y" ));
    */

    return QApplication::exec();
}
