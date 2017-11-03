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
#include <QLineEdit>

/**
 * @brief The LineEdit class
 */
class LineEdit : public QLineEdit {
    Q_OBJECT
    Q_ENUMS( Modes )
    // props!!!

public:
    enum Modes {
        NoMode = -1,
        Mass,
        MolarMass,
        Density,
        Assay
    };

    enum Units {
        NoUnits = -1,
        Primary,
        Secondary
    };

    explicit LineEdit( QWidget *parent = nullptr );
    QString pattern() const { return this->m_pattern; }
    qreal value() const { return this->m_value; }
    QString currentUnits( Units dest = Primary ) const { return this->m_current[dest]; }

public slots:
    void setPattern( const QString &pattern ) { this->m_pattern = pattern; }
    void setValue( qreal value = 0.0 ) { this->m_value = value; }
    void setUnits( const QStringList &names, const QList<qreal> multipliers, Units dest = Primary );
    void setCurrentUnits( const QString &name, Units dest = Primary );

private:
    QString m_pattern;
    qreal m_value;
    QMap<QString, qreal> units[2];
    QString m_current[2];
};
