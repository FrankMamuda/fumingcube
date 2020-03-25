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
#include "fragment.h"
#include "extractiondialog.h"

/**
 * @brief The Ui namespace
 */
namespace Ui {
class PropertyFragment;
}

/**
 * @brief The PropertyFragment class
 */
class PropertyFragment final : public Fragment {
    Q_OBJECT
    Q_DISABLE_COPY( PropertyFragment )

public:
    explicit PropertyFragment( QWidget *parent = nullptr );

    // disable move
    PropertyFragment( PropertyFragment&& ) = delete;
    PropertyFragment& operator=( PropertyFragment&& ) = delete;

    ~PropertyFragment() override;

    [[nodiscard]] ExtractionDialog *host() const { return qobject_cast<ExtractionDialog *>( Fragment::host()); }

public slots:
    bool getDataAndFormula( const int &id );

    void sendFormulaRequest();
    void sendDataRequest();

private slots:
    bool parseFormulaRequest( const QByteArray &data );
    bool parseDataRequest( const QByteArray &data );

    void readData( const QByteArray &uncompressed );
    void readFormula( const QByteArray &data );

protected:
    void keyPressEvent( QKeyEvent *event ) override;

private:
    Ui::PropertyFragment *ui;
};
