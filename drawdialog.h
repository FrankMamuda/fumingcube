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

#pragma once

/*
 * includes
 */
#include <QDialog>
#include <QTimer>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class DrawDialog;
}

/**
 * @brief The DrawDialog class
 */
class DrawDialog : public QDialog {
    Q_OBJECT

public:
    explicit DrawDialog( QWidget *parent = nullptr, const QString &json = QString());
    ~DrawDialog() override;
    void getPixmapAndAccept();
    QByteArray data;
    QString json;

protected:
    void resizeEvent( QResizeEvent * ) override;

private slots:
    void on_scriptButton_clicked();
    void loadComponent();
    void on_buttonBox_accepted();

private:
    Ui::DrawDialog *ui;
    QTimer resizeTimer;
    bool m_resizeInProgress = false;
    bool script = false;
    bool scriptUI = false;
    bool jQuery = false;
};
