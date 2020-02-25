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
#include <QDialog>
#include "networkmanager.h"

/**
 * @brief The Ui namespace
 */
namespace Ui {
    class StructureBrowser;
}

/**
 * @brief The StructureBrowser class
 */
class StructureBrowser : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY( StructureBrowser )

public:
    explicit StructureBrowser( QList<int> cidList, QWidget *parent = nullptr );

    // disable move
    StructureBrowser( StructureBrowser&& ) = delete;
    StructureBrowser& operator=( StructureBrowser&& ) = delete;

    ~StructureBrowser() override;

    /**
     * @brief The StatusOption enum
     */
    enum StatusOption {
        Idle = 0x0,
        FetchName = 0x1,
        FetchFormula = 0x2,
        Error = 0x4
    };
    Q_DECLARE_FLAGS( Status, StatusOption )

    /**
     * @brief status
     * @return
     */
    [[nodiscard]] Status status() const { return this->m_status; }
    [[nodiscard]] int cid() const;

public slots:
    void replyReceived( const QString &url, NetworkManager::Types type, const QVariant &userData, const QByteArray &data );
    void error( const QString &, NetworkManager::Types type, const QString &errorString );

private slots:
    void getInfo();
    void getFormula( int cid );
    void getName( int cid );
    void readFormula( const QByteArray &data );

    /**
     * @brief setStatus
     * @param status
     */
    void setStatus( const StructureBrowser::Status &status ) { this->m_status = status; }
    void buttonTest();

private:
    /**
     * @brief index
     * @return
     */
    [[nodiscard]] int index() const { return this->m_index; }

    /**
     * @brief path
     * @return
     */
    [[nodiscard]] QString path() const { return this->m_path; }

    Ui::StructureBrowser *ui;
    QList<int> cidList;
    int m_index = 0;
    QString m_path;
    Status m_status = Idle;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( StructureBrowser::Status )
