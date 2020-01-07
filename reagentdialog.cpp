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
#include "reagentdialog.h"
#include "ui_reagentdialog.h"

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
}

/**
 * @brief ReagentDialog::~ReagentDialog
 */
ReagentDialog::~ReagentDialog() {
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

/**
 * @brief ReagentDialog::on_nameEdit_textChanged
 * @param text
 */
void ReagentDialog::on_nameEdit_textChanged( const QString &text ) {
    this->ui->aliasEdit->setText( reagentAliases.keys().contains( text ) ? reagentAliases[text] : QString( text ).remove( ' ' ));
}
