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
#include "table.h"

/**
 * @brief The Ui namespace
 */
namespace Ui {
    class ReagentDialog;
}

namespace ReagentTools {
    /**
     * @brief DigitsToScript
     * @param string
     * @return
     */
    static QString DigitsToScript( const QString &string, bool subScript = false ) {
        QString out( string );

        for ( const QChar &ch : out ) {
            if ( ch.isDigit()) {
                out.replace( ch, subScript ? QString( "<span style=\"vertical-align:sub;\">%1</span>" ).arg( ch )
                                           : QString( "<span style=\"vertical-align:sup;\">%1</span>" ).arg( ch ));
            }
        }

        return out;
    }

    [[maybe_unused]]
    static QString DigitsToSubscript( const QString &string ) { return DigitsToScript( string, true ); }
    [[maybe_unused]]
    static QString DigitsToSuperscript( const QString &string ) { return DigitsToScript( string, false ); }
}

/**
 * @brief The ReagentDialog class
 */
class ReagentDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY( ReagentDialog )

public:
    /**
     * @brief The Modes enum
     */
    enum Modes {
        AddMode,
        EditMode
    };
    Q_ENUM( Modes )

    explicit ReagentDialog( QWidget *parent = nullptr, const QString &name = QString(),
                            const QString &reference = QString(), const Modes &mode = AddMode );

    // disable move
    ReagentDialog( ReagentDialog&& ) = delete;
    ReagentDialog& operator=( ReagentDialog&& ) = delete;

    ~ReagentDialog() override;
    [[nodiscard]] QString name() const;
    [[nodiscard]] QString reference() const;
    QList<Id> labels;

protected:
    void showEvent( QShowEvent *event ) override;
    bool eventFilter( QObject *object, QEvent *event ) override;

private:
    Ui::ReagentDialog *ui;
    QCompleter *completer;
    QStringList variables;
};
