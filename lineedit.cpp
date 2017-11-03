/*
 * Copyright (C) 2017 Factory #12
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

//
// includes
//
#include <QDebug>
#include "lineedit.h"

/**
 * @brief LineEdit::LineEdit
 */
LineEdit::LineEdit( QWidget *parent ) : QLineEdit( parent ), m_value( 0.0 ) {
    this->setAlignment( Qt::AlignHCenter );

    // as-you-type validation
    this->connect( this, &QLineEdit::textChanged, [ this ]() {
        QRegExp re;
        QString primaryUnits, secondaryUnits;

        re.setPattern( this->pattern());
        re.setCaseSensitivity( Qt::CaseInsensitive );
        if ( re.indexIn( this->text()) != -1 ) {
            // get value from the pattern
            this->setValue( re.cap( 1 ).replace( ',', '.' ).toDouble());

            // match units from the pattern and set as current if valid
            primaryUnits = re.cap( 2 ).toLower();
            if ( this->units[Primary].contains( primaryUnits ))
                this->setCurrentUnits( primaryUnits, Primary );

            // match units from the pattern and set as current if valid
            secondaryUnits = re.cap( 3 ).toLower();
            if ( this->units[Secondary].contains( secondaryUnits ))
                this->setCurrentUnits( secondaryUnits, Secondary );

            // all ok
            this->setStyleSheet( "QLineEdit {}" );
        } else {
            // paint background red to indicate an invalid input
            this->setStyleSheet( "QLineEdit { background-color: #ff9999; }" );
        }
    } );

    // as-you-leave captured value and unit display
    this->connect( this, &QLineEdit::editingFinished, [ this ]() {
        QRegExp re;

        re.setPattern( this->pattern());
        re.setCaseSensitivity( Qt::CaseInsensitive );
        if ( re.indexIn( this->text()) == -1 )
            return;

        if ( this->units[Secondary].isEmpty())
            this->setText( QString( "%1 %2" ).arg( QString::number( this->value())).arg( this->currentUnits()));
        else
            this->setText( QString( "%1 %2/%3" ).arg( QString::number( this->value())).arg( this->currentUnits()).arg( this->currentUnits( Secondary )));
    } );
}

/**
 * @brief LineEdit::setUnits
 * @param names
 * @param multipliers
 * @param units
 */
void LineEdit::setUnits( const QStringList &names, const QList<qreal> multipliers, LineEdit::Units dest ) {
    int y;

    // abort on empty lists
    if ( names.isEmpty())
        return;

    // make sure name and multiplier list count matches
    if ( names.count() != multipliers.count()) {
        qCritical() << this->tr( "invalid units" );
        return;
    }

    // map unit names to multipliers
    for ( y = 0; y < names.count(); y++ )
        this->units[dest][names.at( y )] = multipliers.at( y );

    // set the first entry as current
    this->setCurrentUnits( names.first(), dest );
}

/**
 * @brief LineEdit::setCurrentUnits
 * @param name
 * @param dest
 */
void LineEdit::setCurrentUnits( const QString &name, LineEdit::Units dest ) {
    // abort if no units have been mapped
    if ( this->units[dest].isEmpty())
        return;

    // match and set current units
    if ( this->units[dest].contains( name ))
        this->m_current[dest] = name;
}
