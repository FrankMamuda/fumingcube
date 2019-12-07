/*
 * Copyright (C) 2017-2018 Factory #12
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
#include <QDialog>
#include <QLabel>
#include <QLayout>
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
 * @brief The PropertyValueWidget class
 */
class PropertyValueWidget : public QWidget {
    Q_OBJECT

public:
    explicit PropertyValueWidget( QWidget *parent = nullptr, const QList<QStringList> &values = QList<QStringList>(), const Id &tagId = Id::Invalid );
    ~PropertyValueWidget() override {
        this->disconnect( this->left, &QToolButton::pressed, this, nullptr );
        this->disconnect( this->right, &QToolButton::pressed, this, nullptr );

        delete this->ghs;
        delete this->nfpa;
        delete this->label;
        delete this->left;
        delete this->right;
        delete this->layout;
    }

    int position() const { return this->m_position; }
    Id tagId() const { return this->m_tagId; }

    static QStringList parseGHS( const QStringList &list ) {
        QStringList parms;
        foreach ( const QString &parm, list ) {
            if ( parm.contains( QRegularExpression( "[Ee]xplosive" )))
                parms << "GHS01";
            if ( parm.contains( QRegularExpression( "[Ff]lammable" )))
                parms << "GHS02";
            if ( parm.contains( QRegularExpression( "[Oo]xidizing" )))
                parms << "GHS03";
            if ( parm.contains( QRegularExpression( "[Cc]ompressed\\s[Gg]as" )))
                parms << "GHS04";
            if ( parm.contains( QRegularExpression( "[Cc]orrosive" )))
                parms << "GHS05";
            if ( parm.contains( QRegularExpression( "[Tt]oxic" )))
                parms << "GHS06";
            if ( parm.contains( QRegularExpression( "[Hh]armful" )) || parm.contains( QRegularExpression( "[Ii]rritant" )))
                parms << "GHS07";
            if ( parm.contains( QRegularExpression( "[Hh]ealth\\s[Hh]azard" )))
                parms << "GHS08";
            if ( parm.contains( QRegularExpression( "[Ee]nvironmental\\s[Hh]azard" )))
                parms << "GHS09";
        }
        return qAsConst( parms );
    }

public slots:
    void add( const Id &id );

private:
    int m_position = -1;
    QMap<int, QString> displayValues;
    QMap<int, QStringList> propertyValues;
    QLabel *label = new QLabel();
    QToolButton *left = new QToolButton();
    QToolButton *right = new QToolButton();
    QHBoxLayout *layout = new QHBoxLayout();

    // FIXME: don't initialize these if not needed
    NFPAWidget *nfpa = new NFPAWidget();
    GHSWidget *ghs = new GHSWidget();

    Id m_tagId = Id::Invalid;
};

/**
 * @brief The ExtractionDialog class
 */
class ExtractionDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExtractionDialog( QWidget *parent = nullptr, const Id &reagentId = Id::Invalid );
    ~ExtractionDialog() override;
    Id reagentId() const { return this->m_reagentId; }
    QString path() const { return this->m_path; }
    QString cache() const { return this->m_cache; }

    enum RequestTypes {
        NoType = -1,
        CIDRequest,
        DataRequest
    };

public slots:
    void readData( const QByteArray &uncompressed );

private slots:
    void on_extractButton_clicked();

private:
    Ui::ExtractionDialog *ui;
    ExtractionModel *model = nullptr;
    Id m_reagentId = Id::Invalid;
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkRequest request;
    QStringList cidList;
    QString m_path;
    QString m_cache;
};
