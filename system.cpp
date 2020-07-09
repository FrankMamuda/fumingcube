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

/*
 * includes
 */
#include "system.h"
#include "mainwindow.h"
#include "calcview.h"
#include "reagent.h"
#include "property.h"
#include "label.h"
#include "tableentry.h"
#include "variable.h"
#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QMessageBox>
#include <QSqlQuery>

/**
 * @brief System::System
 */
System::System() {}

/**
 * @brief System::print
 * @param message
 */
void System::print( const QString &message ) {
    MainWindow::instance()->appendToCalculator( message + "\n\n", true );
}

/**
 * @brief System::replaceGreeting
 */
void System::replaceGreeting() {
    // TODO: reuse code

    // read initial history
    QFile file( ":/initial/calculator_history" );
    QString history;
    if ( file.open( QIODevice::ReadOnly )) {
        const QString html( file.readAll());
        MainWindow::instance()->calcView()->setHtml( html );
        MainWindow::instance()->calcView()->adjustFonts();
        file.close();
    }
}

/**
 * @brief System::dbInfo
 */
void System::dbInfo() {
    int reagents = 0, properties = 0;

    // avoid filters
    QSqlQuery queryR( "select count(*) from " + Reagent::instance()->tableName()); queryR.exec(); if ( queryR.next()) reagents = queryR.value( 0 ).toInt();
    QSqlQuery queryP( "select count(*) from " + Property::instance()->tableName()); queryP.exec(); if ( queryP.next()) properties = queryP.value( 0 ).toInt();

    const QString string( QString( "Reagents: %1\nProperties: %2\nLabels: %3\nTables: %4\nPath: %5" )
                          .arg( reagents ).arg( properties ).arg( Label::instance()->count()).arg( TableEntry::instance()->count()).arg( Variable::string( "databasePath" )));


    QMessageBox::information( MainWindow::instance(), System::tr( "Database info" ), string );
}

/**
 * @brief System::clearCommandHistory
 */
void System::clearCommandHistory() {
    Variable::reset( "calculator/commands" );
}

/**
 * @brief System::printVariableValue
 */
void System::printVariableValue( const QString &key ) {
    if ( Variable::instance()->contains( key ))
        MainWindow::instance()->appendToCalculator( key + ": " +  Variable::string( key ) + "<br>", true );
    else
        MainWindow::instance()->appendToCalculator( "Unknown variable: " + key + "<br>", true );
}
