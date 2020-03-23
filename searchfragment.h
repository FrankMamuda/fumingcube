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
#include "fragment.h"

/*
 * classes
 */
class ExtractionDialog;

/**
 * @brief The Ui namespace
 */
namespace Ui {
class SearchFragment;
}

/**
 * @brief The SearchFragment class
 */
class SearchFragment final : public Fragment {
    Q_OBJECT

public:
    explicit SearchFragment( QWidget *parent = nullptr );
    ~SearchFragment();
    [[nodiscard]] QString identifier( bool clean = false ) const;

public slots:
    void setIdentifier( const QString &string );

signals:
    void status( const QString &message );
 //   void idListReady( )

private slots:
    void sendInitialRequest();
    void sendSimilarRequest();
    bool parseIdList( const QList<int> &idList );
    bool parseIdListRequest( const QByteArray &data );
    void toggleControls( bool enabled );

private:
    Ui::SearchFragment *ui;
};