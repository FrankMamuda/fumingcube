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
#include "labelselector.h"
#include "reagentdialog.h"
#include "ui_reagentdialog.h"
#include "variable.h"
#include <QRegularExpression>
#include "labeldock.h"
#include "label.h"

/*
 * Reagent alias map
 */
const static QMap<QString, QString> reagentAliases {
    { "Sodium hydroxide", "NaOH" },
    { "Acetic acid", "AcOH" },
    { "Trifluoroacetic acid", ReagentTools::DigitsToSubscript( "CF3COOH" ) },
    { "Trifluoroacetic acid anydride", ReagentTools::DigitsToSubscript( "(CF3COOH)2O" ) },
    { "Water", ReagentTools::DigitsToSubscript( "H2O" ) },
    { "Acetone", "dimethyl ketone" },
    { "Acetylene", ReagentTools::DigitsToSubscript( "C2H2" ) },
    { "Ammonia", ReagentTools::DigitsToSubscript( "NH3" ) },
    { "Ammonium hydroxide", "NH4OH" },
    { "n-Bromosuccinimide", "NBS" },
    { "Methyl ethyl ketone", "MEK" },
    { "Butanone", "MEK" },
    { "n-Butyllithium", "n-BuLi" },
    { "Carbon disulfide", ReagentTools::DigitsToSubscript( "CS2" ) },
    { "Carbon tetrachloride", ReagentTools::DigitsToSubscript( "CCl4" ) },
    { "Carbonyldiimidazole", "CDI" },
    { "Chloroform", ReagentTools::DigitsToSubscript( "CHCl3" ) },
    { "Copper iodide", "CuI" },
    { "Borane", ReagentTools::DigitsToSubscript( "BH3" ) },
    { "Diborane", ReagentTools::DigitsToSubscript( "B2H6" ) },
    { "Diethyl ether", ReagentTools::DigitsToSubscript( "Et2O" ) },
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
    { "Hydrazine", ReagentTools::DigitsToSubscript( "N2H4" ) },
    { "Hydrochloric acid", "HCl" },
    { "Hydrofluoric acid", "HF" },
    { "Hydrobromic acid", "HBr" },
    { "Hydrogen peroxide", ReagentTools::DigitsToSubscript( "H2O2" ) },
    { "Isopropyl alcohol", "i-PrOH" },
    { "Isopropanol", "i-PrOH" },
    { "Lithium aluminium hydride", "LiAlH4" },
    { "Manganese dioxide", ReagentTools::DigitsToSubscript( "MnO2" ) },
    { "Methyl tert-butyl ether", "MTBE" },
    { "Nitric acid", ReagentTools::DigitsToSubscript( "HNO3" ) },
    { "Oxalyl chloride", ReagentTools::DigitsToSubscript( "(COCl)2" ) },
    { "Perchloric acid", ReagentTools::DigitsToSubscript( "HClO4" ) },
    { "Phosphoric acid", ReagentTools::DigitsToSubscript( "H3PO4" ) },
    { "Orthophosphoric acid", ReagentTools::DigitsToSubscript( "H3PO4" ) },
    { "Phosphorus pentachloride", ReagentTools::DigitsToSubscript( "PCl5" ) },
    { "Phosphorus pentoxide", ReagentTools::DigitsToSubscript( "P2O5" ) },
    { "Phosphorus tribromide", ReagentTools::DigitsToSubscript( "PBr3" ) },
    { "Phosphorus trichloride", ReagentTools::DigitsToSubscript( "PCl3" ) },
    { "Phosphoryl chloride", ReagentTools::DigitsToSubscript( "POCl3" ) },
    { "Potassium hydroxide", "KOH" },
    { "Potassium borohydride", ReagentTools::DigitsToSubscript( "KBH4" ) },
    { "Potassium chloride", "KCl" },
    { "Potassium bromide", "KBr" },
    { "Potassium fluoride", "KF" },
    { "Potassium iodide", "KI" },
    { "Potassium bisulfite", ReagentTools::DigitsToSubscript( "KHSO3" ) },
    { "Potassium bicarbonate", ReagentTools::DigitsToSubscript( "KHCO3" ) },
    { "Potassium dithionite", ReagentTools::DigitsToSubscript( "K2S2O4" ) },
    { "Potassium hydrosulfite", ReagentTools::DigitsToSubscript( "K2S2O4" ) },
    { "Potassium sulfate", ReagentTools::DigitsToSubscript( "KSO4" ) },
    { "Potassium carbonate", ReagentTools::DigitsToSubscript( "K2CO3" ) },
    { "Potassium permanganate", ReagentTools::DigitsToSubscript( "KMnO4" ) },
    { "Raney nickel", "RaNi" },
    { "Silver nitrate", ReagentTools::DigitsToSubscript( "AgNO3" ) },
    { "Sodium amide", ReagentTools::DigitsToSubscript( "NaNH2" ) },
    { "Sodium azide", ReagentTools::DigitsToSubscript( "NaN3" ) },
    { "Sodium borohydride", ReagentTools::DigitsToSubscript( "NaBH4" ) },
    { "Sodium chlorite", ReagentTools::DigitsToSubscript( "NaClO2" ) },
    { "Sodium hydride", "NaH" },
    { "Sodium hydroxide", "NaOH" },
    { "Sodium hypochlorite", "NaClO" },
    { "Sodium nitrite", ReagentTools::DigitsToSubscript( "NaNO2" ) },
    { "Sodium chloride", "NaCl" },
    { "Sodium bromide", "NaBr" },
    { "Sodium fluoride", "NaF" },
    { "Sodium iodide", "NaI" },
    { "Sodium bisulfite", ReagentTools::DigitsToSubscript( "NaHSO3" ) },
    { "Sodium bicarbonate", ReagentTools::DigitsToSubscript( "NaHCO3" ) },
    { "Sodium dithionite", ReagentTools::DigitsToSubscript( "Na2S2O4" ) },
    { "Sodium hydrosulfite", ReagentTools::DigitsToSubscript( "Na2S2O4" ) },
    { "Sodium sulfate", ReagentTools::DigitsToSubscript( "NaSO4" ) },
    { "Sodium carbonate", ReagentTools::DigitsToSubscript( "Na2CO3" ) },
    { "Magnesium sulfate", ReagentTools::DigitsToSubscript( "MgSO4" ) },
    { "Tetrabutylammonium bromide", "TBAB" },
    { "Tetrabutylammonium fluoride", "TBAF" },
    { "Sulfuric acid", ReagentTools::DigitsToSubscript( "H2SO4" ) },
    { "Tetrahydrofuran", "THF" },
    { "Thionyl chloride", ReagentTools::DigitsToSubscript( "SOCl2" ) },
    { "Titanium tetrachloride", ReagentTools::DigitsToSubscript( "TiCl4" ) },
    { "Triphenylphosphine", ReagentTools::DigitsToSubscript( "PPh3" ) }
};

/**
 * @brief ReagentDialog::ReagentDialog
 * @param parent
 */
ReagentDialog::ReagentDialog( QWidget *parent, const QString &name, const QString &alias, const Modes &mode ) : QDialog( parent ), ui( new Ui::ReagentDialog ) {
    this->completer = new QCompleter( reagentAliases.keys());
    this->completer->setCaseSensitivity( Qt::CaseInsensitive );
    this->ui->setupUi( this );
    this->ui->nameEdit->setCompleter( completer );

    if ( mode == EditMode ) {
        this->setWindowTitle( this->tr( "Edit reagent" ));
        this->ui->labelButton->hide();
    }

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

    auto subSupModifier = [ this ]( const QList<QChar> list ) {
        QLineEdit *editor = qobject_cast<QLineEdit*>( this->focusWidget());
        if ( editor != this->ui->nameEdit && editor != this->ui->aliasEdit )
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
            return;

        editor->blockSignals( true );
        editor->setText( QString( editor->text()).replace( selectionStart, out.length(), qAsConst( out )));
        editor->blockSignals( false );
        editor->setCursorPosition( cursorPos );
        editor->setSelection( selectionStart, out.length());
    };

    this->connect( this->ui->supButton, &QToolButton::pressed, std::bind( subSupModifier, ReagentTools::SuperscriptDigits ));
    this->connect( this->ui->subButton, &QToolButton::pressed, std::bind( subSupModifier, ReagentTools::SubscriptDigits ));

    this->ui->nameEdit->connect( this->ui->nameEdit, &QLineEdit::textChanged, [ this ]( const QString &text ) {
        QLineEdit *a( this->ui->aliasEdit );

        a->blockSignals( true );
        a->setText( reagentAliases.keys().contains( text ) ? reagentAliases[text] : QString( text ).remove( ' ' ));
        a->blockSignals( false );
    } );

    this->ui->aliasEdit->connect( this->ui->aliasEdit, &QLineEdit::textChanged, [ this ]( const QString &text ) {
        QLineEdit *a( this->ui->aliasEdit );
        QString alias( QString( text ).remove( ' ' ));
        const QString plain( ReagentTools::ScriptToDigits( alias ));

        const int cursorPos = a->cursorPosition();
        if ( cursorPos > 1 && a->cursorPosition() == plain.length()) {
            const QRegularExpression re( "([a-zA-Z])(\\d+)$" );
            const QRegularExpressionMatch match( re.match( plain ));
            if ( match.hasMatch()) {

                int index = match.capturedStart() + match.captured( 1 ).length();
                const QString replaced( ReagentTools::DigitsToSubscript( match.captured( 2 )));
                alias.replace( index, replaced.length(), replaced );
            }
        }

        a->blockSignals( true );
        a->setText( alias );
        a->setCursorPosition( cursorPos );
        a->blockSignals( false );
    } );

    // set current label
    this->labels << LabelDock::instance()->currentLabel();
    this->connect( this->ui->labelButton, &QPushButton::pressed, [ this ]() {
        // modify labels in label selector
        LabelSelector ls( this, this->labels );
        ls.exec();
        this->labels = ls.labelIds;
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
