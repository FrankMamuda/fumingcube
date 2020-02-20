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
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDebug>

/**
 * @brief The ButtonBox class
 */
class ButtonBox final : public QDialogButtonBox {
    Q_OBJECT

public:
    /**
     * @brief ButtonBox
     * @param parent
     */
    explicit ButtonBox( QWidget *parent = nullptr ) : QDialogButtonBox( parent ) {}

public slots:

    void setStandardButtons( StandardButtons buttons ) {
        QDialogButtonBox::setStandardButtons( buttons );
        QPushButton *ok( this->button( QDialogButtonBox::Ok ));
        QPushButton *cc( this->button( QDialogButtonBox::Cancel ));

        if ( ok != nullptr && cc != nullptr ) {
            ok->setIcon( QIcon::fromTheme( "accept" ));
            cc->setIcon( QIcon::fromTheme( "remove" ));
        }
    }
};
