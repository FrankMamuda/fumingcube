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

#pragma once

//
// includes
//
#include <QLineEdit>

/**
 * @brief The LineEdit class
 */
class LineEdit : public QLineEdit {
    Q_OBJECT
    Q_ENUMS( Modes )
    Q_ENUMS( Units )
    Q_PROPERTY( qreal value READ value WRITE setValue NOTIFY valueChanged )
    Q_PROPERTY( QString pattern READ pattern WRITE setPattern )
    Q_PROPERTY( qreal scaledValue READ scaledValue WRITE setScaledValue )
    Q_PROPERTY( Modes mode READ mode WRITE setMode )

public:
    enum Modes {
        NoMode = -1,
        Mass,
        MolarMass,
        Mol,
        Density,
        Assay,
        Pure,
        Volume,
        Amount
    };

    enum Units {
        NoUnits = -1,
        Primary,
        Secondary,

        Count
    };

    explicit LineEdit( QWidget *parent = nullptr );
    ~LineEdit();
    QString pattern() const { return this->m_pattern; }
    qreal value() const { return this->m_value; }
    qreal scaledValue() const { return this->value() * this->multiplier(); }
    qreal multiplier() const;
    QString currentUnits( Units dest = Primary ) const { return this->m_current[dest]; }
    QString defaultUnits( Units dest = Primary ) const { return this->m_default[dest]; }
    Modes mode() const { return this->m_mode; }

signals:
    void valueChanged();

public slots:
    void setPattern( const QString &pattern ) { this->m_pattern = pattern; }
    void setValue( qreal value = 0.0 );
    void setScaledValue( qreal value = 0.0 );
    void setUnits( const QStringList &names, const QList<qreal> multipliers, Units dest = Primary );
    void setCurrentUnits( const QString &name, Units dest = Primary, bool update = false );
    void setMode( Modes mode );
    void displayValue( bool fullPrecision = false );
    void displayToolTips();
    void copy();
    void cut();

protected:
    void focusInEvent( QFocusEvent *event );

private:
    QString m_pattern;
    qreal m_value;
    QVector<QMap<QString, qreal>> units;
    QVector<QString> m_current;
    QVector<QString> m_default;
    Modes m_mode;
};
