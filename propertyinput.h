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
#include <QKeyEvent>
#include <QLineEdit>
#include <QDebug>
#include <QRegularExpression>
#include <QClipboard>
#include <QGuiApplication>

/**
 * @brief The PropertyInput class
 */
class PropertyInput : public QLineEdit {
    Q_OBJECT
    Q_DISABLE_COPY( PropertyInput )

public:
    /**
     * @brief PropertyInput
     * @param parent
     */
    explicit PropertyInput( QWidget *parent = nullptr ) : QLineEdit( parent ) {}

    // disable move
    PropertyInput( PropertyInput&& ) = delete;
    PropertyInput& operator=( PropertyInput&& ) = delete;

protected:
    void keyPressEvent( QKeyEvent *event ) override {
        // pre-process pasted text
        if ( event->matches( QKeySequence::Paste )) {
            const QChar neg( 0x2212 );
            const QRegularExpression re( QString( R"((-|%1?[0-9]\d*(?:[\.|,]\d+)?))" ).arg( neg ));
            const QRegularExpressionMatch match( re.match( QGuiApplication::clipboard()->text()));
            if ( match.hasMatch()) {
                this->setText( match.captured( 1 ).replace( neg, "-" ));
                event->ignore();
                return;
            }
        }

        QLineEdit::keyPressEvent( event );
    }
};
