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
#include <QWebChannel>
#include <QMessageBox>
#include <QInputDialog>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class DrawDialog;
}

/**
 * @brief The Core class
 */
class Core : public QObject {
    Q_OBJECT

public:
    Core( QObject *parent = nullptr ) : QObject( parent ) {}
   // QString getLabel() const { return this->m_label; }
    QString m_label;

signals:
    void sendText( const QString &text );

public slots:
    void receiveText( const QString &defaultLabel );

private:
};

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
    QString fileName;
    QIcon fetchIcon( const QString &name ) const;

protected:
    void resizeEvent( QResizeEvent * ) override;

private slots:
    void loadComponent();
    void on_buttonBox_accepted();

private:
    Ui::DrawDialog *ui;
    QTimer resizeTimer;
    bool m_resizeInProgress = false;
    bool script = false;
    bool scriptUI = false;
    bool jQuery = false;
    QWebChannel *channel = nullptr;
};
