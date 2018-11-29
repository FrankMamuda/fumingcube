/*
 * Copyright (C) 2017-2018 Factory #12
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
LineEdit::LineEdit( QWidget *parent ) : QLineEdit( parent ), m_value( 0.0 ), m_mode( NoMode ) {
    // set text alignment
    this->setAlignment( Qt::AlignHCenter );

    // initialize units to avoid segfaults
    this->m_current = QVector<QString>( Count, "" );
    this->m_default = QVector<QString>( Count, "" );
    this->units = QVector<QMap<QString, qreal>>( Secondary + 1 );

    // as-you-type validation
    this->connect( this, &QLineEdit::textChanged, [ this ]() {
        const QRegExp re( this->pattern(), Qt::CaseInsensitive );

        if ( re.indexIn( this->text()) != -1 ) {
            // match units from the pattern and set as current if valid
            const QString primaryUnits( re.cap( 2 ).toLower());
            if ( this->units[Primary].contains( primaryUnits ))
                this->setCurrentUnits( primaryUnits, Primary, true );

            // match units from the pattern and set as current if valid
            const QString secondaryUnits( re.cap( 3 ).toLower());
            if ( this->units[Secondary].contains( secondaryUnits ))
                this->setCurrentUnits( secondaryUnits, Secondary, true );

            // get value from the pattern
            this->setValue( re.cap( 1 ).replace( ',', '.' ).toDouble());

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
        const QRegExp re( this->pattern(), Qt::CaseInsensitive );

        if ( re.indexIn( this->text()) == -1 )
            return;

        this->displayValue();
    } );
}

/**
 * @brief LineEdit::~LineEdit
 */
LineEdit::~LineEdit() {
    this->disconnect( this, &QLineEdit::textChanged, this, nullptr );
    this->disconnect( this, &QLineEdit::editingFinished, this, nullptr );
}

/**
 * @brief LineEdit::multiplier
 * @return
 */
qreal LineEdit::multiplier() const {
    bool primary = false, secondary = false;
    qreal value = 0.0;

    // check for primary units
    if ( this->units[Primary].contains( this->currentUnits( Primary )))
        primary = true;

    // check for secondary units
    if ( this->units[Secondary].contains( this->currentUnits( Secondary )))
        secondary = true;

    // calculate multiplier
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
    // check if the value has changed
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
    // scale value by unit multiplier
    this->setValue( value / this->multiplier());

    // display the new value
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
void LineEdit::setCurrentUnits( const QString &name, LineEdit::Units dest, bool update ) {
    // abort if no units have been mapped
    if ( this->units[dest].isEmpty())
        return;

    // match and set current units
    if ( this->units[dest].contains( name ) && ( QString::compare( this->currentUnits( dest ), name ))) {
        this->m_current[dest] = name;

        if ( update )
            emit this->valueChanged();
    }
}

/**
 * @brief LineEdit::setMode
 * @param mode
 */
void LineEdit::setMode( LineEdit::Modes mode ) {
    this->m_mode = mode;

    switch ( this->mode()) {
    case Mass:
        this->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(mg|g|kg)?[\\s|\\n]*$" );
        this->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );        
        break;

    case MolarMass:
        this->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(?:(mg|g|kg)\\s*[\\/|·]\\s*(mmol|mol|kmol|mol\\−1))?[\\s|\n]*$" );
        this->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );
        this->setUnits( QString( "mol,mmol,kmol" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000, LineEdit::Secondary );
        break;

    case Mol:
        this->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(mol|mmol|kmol)?[\\s|\\n]*$" );
        this->setUnits( QString( "mol,mmol,kmol" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );
        break;

    case Density:
        this->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(?:(mg|g|kg)\\s*\\/\\s*(ul|ml|l|cm3|m3|cm³|m³))?[\\s|\\n]*$" );
        this->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );
        this->setUnits( QString( "ml,ul,l,cm3,m3,cm³,m³" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 << 1 << 1000000 << 1 << 1000000, LineEdit::Secondary );
        break;

    case Assay:
        this->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(%)?[\\s|\\n]*$" );
        this->setUnits( QString( "%," ).split( "," ), QList<qreal>() << 0.01 << 1 );
        break;

    case Pure:
        this->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(mg|g|kg)?[\\s|\\n]*$" );
        this->setUnits( QString( "g,mg,kg" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 );
        break;

    case Volume:
        this->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(ul|ml|l|cm3|m3|cm³|m³)?[\\s|\\n]*$" );
        this->setUnits( QString( "ml,ul,l,cm3,m3,cm³,m³" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 << 1 << 1000000 << 1 << 1000000 );
        break;

    case Amount:
        this->setPattern( "\\s*(\\d+[\\.|,]?\\s*\\d*)\\s*(g|mg|kg|ul|ml|l|cm3|m3|cm\u00b9|m\u00b9)?[\\s|\\n]*$" );
        this->setUnits( QString( "g,mg,kg,ml,ul,l,cm3,m3,cm\u00b9,m\u00b9" ).split( "," ), QList<qreal>() << 1 << 0.001 << 1000 << 1 << 0.001 << 1000 << 1 << 1000000 << 1 << 1000000 );
        break;

    case NoMode:
        break;
    }
}

/**
 * @brief LineEdit::displayValue
 */
void LineEdit::displayValue( bool fullPrecision ) {
    int digits = 2;

    // we don't want any updates when simply displaying value0
    this->blockSignals( true );

    // currently hard-coded significant digit count
    // in future, make this configurable in settings
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

        case Amount:
        case NoMode:
            break;
        }
    }

    // display rounded up value with the corresponding units
    if ( this->units[Secondary].isEmpty())
        this->setText( QString( "%1 %2" ).arg( fullPrecision ? QString::number( this->value()) : QString::number( this->value(), 'f', digits )).arg( this->currentUnits()));
    else
        this->setText( QString( "%1 %2/%3" ).arg( fullPrecision ? QString::number( this->value()) : QString::number( this->value(), 'f', digits )).arg( this->currentUnits()).arg( this->currentUnits( Secondary )));

    if ( this->mode() == Assay )
        this->setText( this->text().remove( " " ));

    // update tooltips
    this->displayToolTips();

    // we're ready for updates again
    this->blockSignals( false );
}

/**
 * @brief LineEdit::displayToolTips
 */
void LineEdit::displayToolTips() {
    // display full precision in toolTips
    if ( this->units[Secondary].isEmpty()) {
        qreal value = this->scaledValue();

        if ( this->mode() == Assay )
            value = static_cast<qreal>( value * 100 );

        this->setToolTip( QString( "%1 %2" ).arg( QString::number( value )).arg( this->defaultUnits( Primary )));
    } else
        this->setToolTip( QString( "%1 %2/%3" ).arg( QString::number( this->scaledValue())).arg( this->defaultUnits( Primary )).arg( this->defaultUnits( Secondary )));
}

/**
 * @brief LineEdit::enterEvent
 * @param event
 */
void LineEdit::focusInEvent( QFocusEvent *event ) {
    // upon entering lineEdit, display value in full precision
    if ( !this->isReadOnly())
        this->displayValue( true );

    QLineEdit::focusInEvent( event );
}

/**
 * @brief LineEdit::copy
 */
void LineEdit::copy() {
    // copy non-scaled value
    // TODO: add custom context menu:
    //      copy -> with units    -> scaled
    //                            -> non-scaled
    //           -> without units -> scaled
    //                            -> non-scaled
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
