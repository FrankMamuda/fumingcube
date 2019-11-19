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

#pragma once

/*
 * includes
 */
#include <QCompleter>
#include <QDialog>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class ReagentDialog;
}

/**
 * @brief The ReagentDialog class
 */
class ReagentDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReagentDialog( QWidget *parent = nullptr );
    ~ReagentDialog();
    QString name() const;
    QString alias() const;

private slots:
    void on_nameEdit_textChanged( const QString &text );

private:
    Ui::ReagentDialog *ui;
    QCompleter *completer;
};
