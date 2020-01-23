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
#include <QCompleter>
#include <QDialog>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class ReagentDialog;
}

namespace ReagentTools {
const static QList<QChar> SuperscriptDigits {
    0x2070,
    0x00b9,
    0x00b2,
    0x00b3,
    0x2074,
    0x2075,
    0x2076,
    0x2077,
    0x2078,
    0x2079
};

const static QList<QChar> SubscriptDigits {
    0x2080,
    0x2081,
    0x2082,
    0x2083,
    0x2084,
    0x2085,
    0x2086,
    0x2087,
    0x2088,
    0x2089
};

/**
 * @brief ScriptToDigits
 * @param string
 * @param list
 * @return
 */
static __attribute__((unused)) QString ScriptToDigits( const QString &string ) {
    QString out( string );

    auto back = [ &out ]( const QList<QChar> &list ) {
        foreach ( const QChar &ch, out ) {
            const int index = list.indexOf( ch );
            if ( index >= 0 )
                out.replace( ch, QString::number( index ));
        }
    };
    back( SubscriptDigits );
    back( SuperscriptDigits );

    return out;
}


/**
 * @brief DigitsToScript
 * @param string
 * @return
 */
static QString DigitsToScript( const QString &string, const QList<QChar> &list ) {
    QString out( string );

    foreach ( const QChar &ch, out ) {
        if ( ch.isDigit())
            out.replace( ch, list[QString( ch ).toInt()] );
    }

    return out;
}

static __attribute__((unused)) QString DigitsToSubscript( const QString &string ) { return DigitsToScript( string, ReagentTools::SubscriptDigits ); }
static __attribute__((unused)) QString DigitsToSuperscript( const QString &string ) { return DigitsToScript( string, ReagentTools::SuperscriptDigits ); }
};

/**
 * @brief The ReagentDialog class
 */
class ReagentDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReagentDialog( QWidget *parent = nullptr, const QString &name = QString(), const QString &alias = QString());
    ~ReagentDialog();
    QString name() const;
    QString alias() const;

private:
    Ui::ReagentDialog *ui;
    QCompleter *completer;
    QStringList variables;
};
