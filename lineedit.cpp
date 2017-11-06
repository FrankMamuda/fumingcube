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
#include <QApplication>
#include <QClipboard>
#include "lineedit.h"

/**
 * @brief LineEdit::LineEdit
 */
LineEdit::LineEdit( QWidget *parent ) : QLineEdit( parent ), m_value( 0.0 ), m_current{ "", "" }, m_default{ "", "" }, m_mode( NoMode ) {
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

            // display amount in default units
            this->displayToolTips();
        } else {
            // paint background red to indicate an invalid input
            this->setStyleSheet( "QLineEdit { background-color: #ff9999; }" );
            this->setToolTip( "could not parse input" );
        }
    } );

    // as-you-leave captured value and unit display
    this->connect( this, &QLineEdit::editingFinished, [ this ]() {
        QRegExp re;

        re.setPattern( this->pattern());
        re.setCaseSensitivity( Qt::CaseInsensitive );
        if ( re.indexIn( this->text()) == -1 )
            return;

        this->displayValue();
    } );
}

/**
 * @brief LineEdit::multiplier
 * @return
 */
qreal LineEdit::multiplier() const {
    bool primary = false, secondary = false;
    qreal value = 0.0;

    if ( this->units[Primary].contains( this->currentUnits( Primary )))
        primary = true;

    if ( this->units[Secondary].contains( this->currentUnits( Secondary )))
        secondary = true;

    if ( primary && !secondary )
        value = this->units[Primary][this->currentUnits( Primary )];
    else if ( primary && secondary )
        value = this->units[Primary][this->currentUnits( Primary )] / this->units[Secondary][this->currentUnits( Secondary )];

    return value;
}

/**
 * @brief LineEdit::setValue
 * @param value
 */
void LineEdit::setValue( qreal value ) {
    if ( !qFuzzyCompare( this->value(), value )) {
        this->m_value = value;
        emit this->valueChanged();
    }
}

/**
 * @brief LineEdit::setScaledValue
 * @param value
 */
void LineEdit::setScaledValue( qreal value ) {
    this->setValue( value / this->multiplier());
    this->displayValue();
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
    this->m_default[dest] = names.first();
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

/**
 * @brief LineEdit::displayValue
 */
void LineEdit::displayValue( bool fullPrecision ) {
    int digits = 2;

    if ( !fullPrecision ) {
        switch ( this->mode()) {
        case Volume:
        case Mass:
            digits = 1;
            break;

        case Assay:
        case Pure:
        case Density:
        case MolarMass:
            digits = 2;
            break;

        case Mol:
            digits = 3;
            break;

        case NoMode:
            break;
        }
    }

    if ( this->units[Secondary].isEmpty())
        this->setText( QString( "%1 %2" ).arg( fullPrecision ? QString::number( this->value()) : QString::number( this->value(), 'f', digits )).arg( this->currentUnits()));
    else
        this->setText( QString( "%1 %2/%3" ).arg( fullPrecision ? QString::number( this->value()) : QString::number( this->value(), 'f', digits )).arg( this->currentUnits()).arg( this->currentUnits( Secondary )));

    if ( this->mode() == Assay )
        this->setText( this->text().remove( " " ));

    this->displayToolTips();
}

/**
 * @brief LineEdit::displayToolTips
 */
void LineEdit::displayToolTips() {
    if ( this->units[Secondary].isEmpty())
        this->setToolTip( QString( "%1 %2" ).arg( QString::number( this->scaledValue())).arg( this->defaultUnits( Primary )));
    else
        this->setToolTip( QString( "%1 %2/%3" ).arg( QString::number( this->scaledValue())).arg( this->defaultUnits( Primary )).arg( this->defaultUnits( Secondary )));
}

/**
 * @brief LineEdit::enterEvent
 * @param event
 */
void LineEdit::focusInEvent( QFocusEvent *event ) {
    if ( !this->isReadOnly()) {
        this->blockSignals( true );
        this->displayValue( true );
        this->blockSignals( false );
    }

    QLineEdit::focusInEvent( event );
}

/**
 * @brief LineEdit::copy
 */
void LineEdit::copy() {
    qApp->clipboard()->setText( QString::number( this->value()));
}

/**
 * @brief LineEdit::cut
 */
void LineEdit::cut() {
    this->copy();
    this->setValue( 0.0 );
    this->clear();
}
