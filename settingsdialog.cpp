/*
 * Copyright (C) 2018-2020 Armands Aleksejevs
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
#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "variable.h"
#include <QFileDialog>
#include <QMessageBox>

/**
 * @brief Settings::Settings
 * @param parent
 */
SettingsDialog::SettingsDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::SettingsDialog ) {
    this->ui->setupUi( this );

    // handle database path
    this->connect( this->ui->pathButton, &QPushButton::clicked, [ this ]() {
        const QString fileName( QFileDialog::getSaveFileName
                                ( this, this->tr( "Open database" ),
                                  QFileInfo( Variable::instance()->string( "databasePath" )).absolutePath(),
                                  this->tr( "Database (*.db *.sqlite)" ), nullptr, QFileDialog::DontConfirmOverwrite ));

        if ( fileName.isEmpty()) {
            QMessageBox::warning( this,
                                  this->tr( "Settings" ),
                                  this->tr( "Invalid database selection" ),
                                  QMessageBox::Close );
            return;
        }

        QMessageBox::warning( this,
                              this->tr( "Settings" ),
                              this->tr( "Application must be restarted" ),
                              QMessageBox::Ok );
        Variable::instance()->setString( "databasePath", fileName );
        QApplication::quit();

    } );

    this->ui->themeCombo->model()->setData( this->ui->themeCombo->model()->index( 0, 0 ), "light", Qt::UserRole );
    this->ui->themeCombo->model()->setData( this->ui->themeCombo->model()->index( 1, 0 ), "dark", Qt::UserRole );
    this->connect( this->ui->overrideCheck, &QCheckBox::toggled, [ this ]( bool checked ) { this->ui->themeCombo->setEnabled( checked ); } );

    // bind database path to edit
    this->variables << Variable::instance()->bind( "overrideTheme", this->ui->overrideCheck );
    this->variables << Variable::instance()->bind( "databasePath", this->ui->pathEdit );
    this->variables << Variable::instance()->bind( "theme", this->ui->themeCombo );
    this->variables << Variable::instance()->bind( "alwaysOnTop", this->ui->onTopCheck );

    // setup decimal separator comboBox
    this->ui->decimalSepCombo->model()->setData( this->ui->decimalSepCombo->model()->index( 0, 0 ), ".", Qt::UserRole );
    this->ui->decimalSepCombo->model()->setData( this->ui->decimalSepCombo->model()->index( 1, 0 ), ",", Qt::UserRole );
    this->variables << Variable::instance()->bind( "decimalSeparator", this->ui->decimalSepCombo );
}

/**
 * @brief SettingsDialog::~SettingsDialog
 */
SettingsDialog::~SettingsDialog() {
    // unbind vars
    foreach ( const QString &key, this->variables )
        Variable::instance()->unbind( key );

    delete this->ui;
}
