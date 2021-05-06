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
#include <QQuickItem>
#include <QQuickView>
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
    explicit DrawDialog( QWidget *parent = nullptr, const QString &json = QString(), bool drawMode = false );
    ~DrawDialog() override;
    QString fileName;
    QString json;
    QIcon fetchIcon( const QString &name ) const;
    bool hasInitialized() const { return this->m_initialized; }

protected:
    void resizeEvent( QResizeEvent * ) override;
    void keyReleaseEvent( QKeyEvent * ) override;

private slots:
    void loadComponent();
    void on_commandEdit_returnPressed();

private:
    Ui::DrawDialog *ui;
    QTimer resizeTimer;
    bool m_resizeInProgress = false;
    bool m_initialized = false;
    QQuickItem *historyManager = nullptr;
    QQuickItem *rootObject() const;
    QQuickView *quickView = nullptr;
};
