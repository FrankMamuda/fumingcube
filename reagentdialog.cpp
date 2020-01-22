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

/*
 * includes
 */
#include "charactermap.h"
#include "reagentdialog.h"
#include "ui_reagentdialog.h"
#include "variable.h"

#include <QRegularExpression>

/*
 * Reagent alias map
 */
const static QMap<QString, QString> reagentAliases {
    { "Sodium hydroxide", "NaOH" },
    { "Acetic acid", "AcOH" },
    { "Trifluoroacetic acid", "CF3COOH" },
    { "Trifluoroacetic acid anydride", "(CF3COOH)2O" },
    { "Water", "H2O" },
    { "Acetone", "dimethyl ketone" },
    { "Acetylene", "C2H2" },
    { "Ammonia", "NH3" },
    { "Ammonium hydroxide", "NH4OH" },
    { "n-Bromosuccinimide", "NBS" },
    { "Methyl ethyl ketone", "MEK" },
    { "Butanone", "MEK" },
    { "n-Butyllithium", "n-BuLi" },
    { "Carbon disulfide", "CS2" },
    { "Carbon tetrachloride", "CCl4" },
    { "Carbonyldiimidazole", "CDI" },
    { "Chloroform", "CHCl3" },
    { "Copper iodide", "CuI" },
    { "Borane", "BH3" },
    { "Diborane", "B2H6" },
    { "Diethyl ether", "Et2O" },
    { "Diisobutylaluminium hydride", "DIBAL-H" },
    { "Dimethylformamide", "DMF" },
    { "Dimethylsulfide", "DMS" },
    { "Dimethyl sulfoxide", "DMSO" },
    { "Ethanol", "EtOH" },
    { "Methanol", "MeOH" },
    { "Isobutanol", "i-BuOH" },
    { "Butanol", "n-BuOH" },
    { "2-butanol", "2-BuOH" },
    { "2-butanol", "2-BuOH" },
    { "Formaldehyde", "methanal" },
    { "Formic acid", "HCOOH" },
    { "Hydrazine", "N2H4" },
    { "Hydrochloric acid", "HCl" },
    { "Hydrofluoric acid", "HF" },
    { "Hydrobromic acid", "HBr" },
    { "Hydrogen peroxide", "H2O2" },
    { "Isopropyl alcohol", "i-PrOH" },
    { "Isopropanol", "i-PrOH" },
    { "Lithium aluminium hydride", "LiAlH4" },
    { "Manganese dioxide", "MnO2" },
    { "Methyl tert-butyl ether", "MTBE" },
    { "Nitric acid", "HNO3" },
    { "Oxalyl chloride", "(COCl)2" },
    { "Perchloric acid", "HClO4" },
    { "Phosphoric acid", "H3PO4" },
    { "Orthophosphoric acid", "H3PO4" },
    { "Phosphorus pentachloride", "PCl5" },
    { "Phosphorus pentoxide", "P2O5" },
    { "Phosphorus tribromide", "PBr3" },
    { "Phosphorus trichloride", "PCl3" },
    { "Phosphoryl chloride", "POCl3" },
    { "Potassium hydroxide", "KOH" },
    { "Potassium borohydride", "KBH4" },
    { "Potassium chloride", "KCl" },
    { "Potassium bromide", "KBr" },
    { "Potassium fluoride", "KF" },
    { "Potassium iodide", "KI" },
    { "Potassium bisulfite", "KHSO3" },
    { "Potassium bicarbonate", "KHCO3" },
    { "Potassium dithionite", "K2S2O4" },
    { "Potassium hydrosulfite", "K2S2O4" },
    { "Potassium sulfate", "KSO4" },
    { "Potassium carbonate", "K2CO3" },
    { "Potassium permanganate", "KMnO4" },
    { "Raney nickel", "RaNi" },
    { "Silver nitrate", "AgNO3" },
    { "Sodium amide", "NaNH2" },
    { "Sodium azide", "NaN3" },
    { "Sodium borohydride", "NaBH4" },
    { "Sodium chlorite", "NaClO2" },
    { "Sodium hydride", "NaH" },
    { "Sodium hydroxide", "NaOH" },
    { "Sodium hypochlorite", "NaClO" },
    { "Sodium nitrite", "NaNO2" },
    { "Sodium chloride", "NaCl" },
    { "Sodium bromide", "NaBr" },
    { "Sodium fluoride", "NaF" },
    { "Sodium iodide", "NaI" },
    { "Sodium bisulfite", "NaHSO3" },
    { "Sodium bicarbonate", "NaHCO3" },
    { "Sodium dithionite", "Na2S2O4" },
    { "Sodium hydrosulfite", "Na2S2O4" },
    { "Sodium sulfate", "NaSO4" },
    { "Sodium carbonate", "Na2CO3" },
    { "Magnesium sulfate", "MgSO4" },
    { "Tetrabutylammonium bromide", "TBAB" },
    { "Tetrabutylammonium fluoride", "TBAF" },
    { "Sulfuric acid", "H2SO4" },
    { "Tetrahydrofuran", "THF" },
    { "Thionyl chloride", "SOCl2" },
    { "Titanium tetrachloride", "TiCl4" },
    { "Triphenylphosphine", "PPh3" }
};

/**
 * @brief ReagentDialog::ReagentDialog
 * @param parent
 */
ReagentDialog::ReagentDialog( QWidget *parent , const QString &name, const QString &alias ) : QDialog( parent ), ui( new Ui::ReagentDialog ) {
    this->completer = new QCompleter( reagentAliases.keys());
    this->completer->setCaseSensitivity( Qt::CaseInsensitive );
    this->ui->setupUi( this );
    this->ui->nameEdit->setCompleter( completer );

    if ( !name.isEmpty())
        this->ui->nameEdit->setText( name );

    if ( !alias.isEmpty())
        this->ui->aliasEdit->setText( alias );

    // bind property button
    this->variables << Variable::instance()->bind( "fetchPropertiesOnAddition", this->ui->propertyCheck );

    this->connect( this->ui->charButton, &QToolButton::pressed, [ this ]() {
        CharacterMap cm( this );

        // add character map action
        this->connect( &cm, &CharacterMap::characterSelected, [ this ]( const QString &character ) {
            QLineEdit *editor = qobject_cast<QLineEdit*>( this->focusWidget());

            if ( editor != this->ui->nameEdit )
                return;

            editor->insert( character );
        } );

        cm.exec();
    } );

    const QList<QChar> supList {
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

    const QList<QChar> subList {
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

    auto subSupModifier = [ this ]( const QList<QChar> list ) {
        QLineEdit *editor = qobject_cast<QLineEdit*>( this->focusWidget());
        if ( editor != this->ui->nameEdit )
            return;

        const int cursorPos = editor->cursorPosition();
        const int selectionStart = editor->selectionStart();

        bool allDigits = true;


        QString out;
        foreach ( const QChar &ch, editor->selectedText()) {
            if ( !ch.isDigit()) {
                if ( list.contains( ch )) {
                    out.append( QString::number( list.indexOf( ch )));
                    continue;
                }

                allDigits = false;
                break;
            }

            const int digit = QString( ch ).toInt();
            out.append( list[digit] );
        }
        if ( !allDigits)
            return ;

        this->ui->nameEdit->setText( QString( editor->text()).replace( selectionStart, out.length(), qAsConst( out )));
        this->ui->nameEdit->setCursorPosition( cursorPos );
        this->ui->nameEdit->setSelection( selectionStart, out.length());
    };

    this->connect( this->ui->subButton, &QToolButton::pressed, std::bind( subSupModifier, subList ));
    this->connect( this->ui->supButton, &QToolButton::pressed, std::bind( subSupModifier, supList ));

    this->ui->nameEdit->connect( this->ui->nameEdit, &QLineEdit::textChanged, [ this, supList, subList ]( const QString &text ) {
        QString out( text );
        QString out2( text );

#if 0
        const QLineEdit *n( this->ui->nameEdit );

        if ( n->cursorPosition() > 1 && n->cursorPosition() == n->text().length()) {
            //const QString prev( QString( n->text().at( n->cursorPosition() - 1 )).prepend(( n->cursorPosition() > 1 ) ? n->text().at( n->cursorPosition() - 2 ) : QChar()));
            //const QString current( n->text().at( n->cursorPosition()));
            //qDebug() << prev << prev.at( 0 ).isLetter() << current;

            //QString out( text );

            const QRegularExpression re( "(\\w)(\\d+)$" );
            const QRegularExpressionMatch match( re.match( n->text()));
            if ( match.hasMatch()) {

                int index = match.capturedStart() + match.captured( 1 ).length();
                foreach ( const QChar &ch, match.captured( 2 )) {
                    qDebug() << index << QString( ch ).toInt() << supList[QString( ch ).toInt()];
                    out2 = out2.replace( index, 1, subList[QString( ch ).toInt()] );
                    index++;
                }

                qDebug() << "REPLACED" << out2;
            }
        }
#endif

        // replace back
        foreach ( const QChar &ch, supList ) {
            const int index = supList.indexOf( ch );
            if ( index >= 0 )
                out.replace( ch, QString::number( index ));
        }

        // replace back
        foreach ( const QChar &ch, subList ) {
            const int index = subList.indexOf( ch );
            if ( index >= 0 )
                out.replace( ch, QString::number( index ));
        }

#if 0
        qDebug() << out2;
        if ( QString::compare( n->text(), out2 ))
            this->ui->nameEdit->setText( out2 );
#endif

        this->ui->aliasEdit->setText( reagentAliases.keys().contains( qAsConst( out )) ? reagentAliases[qAsConst( out )] : QString( qAsConst( out )).remove( ' ' ));
    } );
}

/**
 * @brief ReagentDialog::~ReagentDialog
 */
ReagentDialog::~ReagentDialog() {    
    // unbind vars
    foreach ( const QString &key, this->variables )
        Variable::instance()->unbind( key );

    delete this->completer;
    delete this->ui;
}

/**
 * @brief ReagentDialog::name
 * @return
 */
QString ReagentDialog::name() const {
    return this->ui->nameEdit->text();
}

/**
 * @brief ReagentDialog::alias
 * @return
 */
QString ReagentDialog::alias() const {
    return this->ui->aliasEdit->text();
}
