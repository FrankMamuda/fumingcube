/*
 * Copyright (C) 2018-2019 Armands Aleksejevs
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

#pragma once

/*
 * includes
 */
#include <QDialog>

/**
 * @brief The Ui namespace
 */
namespace Ui {
    class SettingsDialog;
}

/**
 * @brief The SettingsDialog class
 */
class SettingsDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY( SettingsDialog )

public:
    explicit SettingsDialog( QWidget *parent = nullptr );

    // disable move
    SettingsDialog( SettingsDialog&& ) = delete;
    SettingsDialog& operator=( SettingsDialog&& ) = delete;

    ~SettingsDialog() override;

private:
    QStringList variables;
    Ui::SettingsDialog *ui;
};
