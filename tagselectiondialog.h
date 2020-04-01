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

#pragma once

/*
 * includes
 */
#include <QCheckBox>
#include <QDialog>
#include "table.h"

/**
 * @brief The Ui namespace
 */
namespace Ui {
class TagSelectionDialog;
}

/**
 * @brief The TagSelectionDialog class
 */
class TagSelectionDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY( TagSelectionDialog )

public:
    explicit TagSelectionDialog( QWidget *parent = nullptr );
    ~TagSelectionDialog() override;

    // disable move
    TagSelectionDialog( TagSelectionDialog&& ) = delete;
    TagSelectionDialog& operator=( TagSelectionDialog&& ) = delete;

private slots:
    void on_actionSelectAll_triggered();

    void on_actionDeselectAll_triggered();

private:
    Ui::TagSelectionDialog *ui;
    QList<Id> selectedTags;
    QMap<Id, QCheckBox*> map;
};
