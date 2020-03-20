/*
 * Copyright (C) 2017-2018 Factory #12
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
#include <QLabel>
#include <QLayout>
#include <QMutex>
#include <qtoolbutton.h>
#include "networkmanager.h"
#include "table.h"
#include "tag.h"
#include "ghswidget.h"
#include "nfpawidget.h"

//
// classes
//
class ExtractionModel;

/**
 * @brief The Ui namespace
 */
namespace Ui {
    class ExtractionDialog;
}

/**
 * @brief The ExtractionDialog class
 */
class ExtractionDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY( ExtractionDialog )

public:
    explicit ExtractionDialog( QWidget *parent = nullptr, const Id &reagentId = Id::Invalid, const int cid = 0 );

    // disable move
    ExtractionDialog( ExtractionDialog&& ) = delete;
    ExtractionDialog& operator=( ExtractionDialog&& ) = delete;

    ~ExtractionDialog() override;

    /**
     * @brief reagentId
     * @return
     */
    [[nodiscard]] Id reagentId() const { return this->m_reagentId; }

    /**
     * @brief The StatusOption enum
     */
    enum StatusOption {
        Idle = 0x0,
        Busy = 0x1,
        Error = 0x2
    };
    Q_DECLARE_FLAGS( Status, StatusOption )

    /**
     * @brief status
     * @return
     */
    [[nodiscard]] Status status() const { return this->m_status; }

    void readData( const QByteArray &uncompressed ) const;

    [[nodiscard]] QString cachedName() const;

    bool readFromCache();


    [[nodiscard]] QString name() const;
    [[nodiscard]] int id() const;


public slots:
    void replyReceived( const QString &url, NetworkManager::Types type, const QVariant &userData, const QByteArray &data );
    void error( const QString &, NetworkManager::Types, const QString & );
    void readFormula( const QByteArray &data );
    /*void getFormula( const QString &cid );
    void getSimilar( const QList<int> &cidListInt );
    void buttonTest();*/
    void sendInitialRequest();
    void sendSimilarRequest();
    void sendFormulaRequest();
    void sendDataRequest();
    bool parseFormulaRequest( const QByteArray &data );
    bool parseDataRequest( const QByteArray &data );
    bool parseIdListRequest( const QByteArray &data );
    bool parseIdList( const QList<int> &idList );
    bool getDataAndFormula( const int &id );
    void getFormula();
    void getData();

private slots:
    void readCache();
    void writeCache();
    void on_actionFetch_triggered();
    //void on_clearCacheButton_clicked();

    /**
     * @brief setStatus
     * @param status
     */
    void setStatus( const ExtractionDialog::Status &status ) { this->m_status = status; }
private:
    Ui::ExtractionDialog *ui;
    ExtractionModel *model = nullptr;
    Id m_reagentId = Id::Invalid;
    QNetworkRequest request;
    QStringList cidList;
    mutable QMutex mutex;
    Status m_status = Idle;
};
