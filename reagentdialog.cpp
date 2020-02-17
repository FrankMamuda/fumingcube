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
#include <QTimer>
#include "labeldock.h"
#include "label.h"
#include "tagdialog.h"

/*
 * Reagent alias map
 */
const static QMap<QString, QString> reagentAliases {
    { "Sodium hydroxide", "NaOH" },
    { "Acetic acid", "AcOH" },
    { "Trifluoroacetic acid", "CF<sub>3</sub>COOH" },
    { "Trifluoroacetic acid anydride", ReagentTools::DigitsToSubscript( "(CF3COOH)2O" ) },
    { "Water", ReagentTools::DigitsToSubscript( "H2O" ) },
    { "Acetone", "dimethyl ketone" },
    { "Acetylene", ReagentTools::DigitsToSubscript( "C2H2" ) },
    { "Ammonia", ReagentTools::DigitsToSubscript( "NH3" ) },
    { "Ammonium hydroxide", "NH4OH" },
    { "n-Bromosuccinimide", "NBS" },
    { "Methyl ethyl ketone", "MEK" },
    { "Butanone", "MEK" },
    { "n-Butyllithium", "<span style=\"font-style:italic;\">n</span>-BuLi" },
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
    { "Isobutanol", "<span style=\"font-style:italic;\">i</span>-BuOH" },
    { "Butanol", "<span style=\"font-style:italic;\">n</span>-BuOH" },
    { "2-butanol", "2-BuOH" },
    { "2-butanol", "2-BuOH" },
    { "Formaldehyde", "methanal" },
    { "Formic acid", "HCOOH" },
    { "Hydrazine", ReagentTools::DigitsToSubscript( "N2H4" ) },
    { "Hydrochloric acid", "HCl" },
    { "Hydrofluoric acid", "HF" },
    { "Hydrobromic acid", "HBr" },
    { "Hydrogen peroxide", ReagentTools::DigitsToSubscript( "H2O2" ) },
    { "Isopropyl alcohol", "<span style=\"font-style:italic;\">i</span>-PrOH" },
    { "Isopropanol", "<span style=\"font-style:italic;\">i</span>-PrOH" },
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
    this->ui->mainWindow->setWindowFlags( Qt::Widget );

    // setup editor toolbar
    this->ui->editorToolBar->installFeature( EditorToolbar::Font );
    this->ui->editorToolBar->installFeature( EditorToolbar::VerticalAlignment );
    this->ui->editorToolBar->installFeature( EditorToolbar::CharacterMap );

    // actions performed upon entering name editor
    this->connect( this->ui->nameEdit, &TextEdit::entered, [ this ]() {
        this->ui->editorToolBar->setEditor( this->ui->nameEdit );
    } );

    // actions performed upon entering reference editor
    this->connect( this->ui->referenceEdit, &TextEdit::entered, [ this ]() {
        this->ui->editorToolBar->setEditor( this->ui->referenceEdit );
    } );

    // copy reference from name by default
    this->connect( this->ui->nameEdit, &TextEdit::textChanged, [ this ]() {
        const QString plain( this->ui->nameEdit->toPlainText());
        const QString html( this->ui->nameEdit->toHtml());

        if ( plain.isEmpty()) {
            this->ui->referenceEdit->setPlainText( "" );
            return;
        }

        QString match;
        foreach ( const QString &key, reagentAliases.keys()) {
            if ( !QString::compare( key, plain, Qt::CaseInsensitive )) {
                match = key;
                break;
            }
        }
        this->ui->referenceEdit->setText( !match.isEmpty() ? reagentAliases[match] : QString( html ).remove( ' ' ));
    } );

    // focus on the name editor to begin with
    this->ui->nameEdit->setFocus();

    // TODO:
    this->ui->nameEdit->setCompleter( completer );

    if ( mode == EditMode ) {
        this->setWindowTitle( this->tr( "Edit reagent" ));
        this->ui->labelButton->hide();
    }

    if ( !name.isEmpty())
        this->ui->nameEdit->setText( name );

    if ( !alias.isEmpty())
        this->ui->referenceEdit->setText( alias );

    // bind property button
    this->variables << Variable::instance()->bind( "fetchPropertiesOnAddition", this->ui->propertyCheck );

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
    // TODO: for now
    return TagDialog::captureBody( this->ui->nameEdit->toHtml());
}

/**
 * @brief ReagentDialog::reference
 * @return
 */
QString ReagentDialog::reference() const {
    // TODO: for now
    return TagDialog::captureBody( this->ui->referenceEdit->toHtml());
}

/**
 * @brief ReagentDialog::showEvent
 * @param event
 */
void ReagentDialog::showEvent( QShowEvent *event ) {
    QDialog::showEvent( event );

    // steal height from a line edit widget
    this->ui->nameEdit->setMaximumHeight( this->ui->lineEdit->height());
    this->ui->nameEdit->document()->setDocumentMargin( 2 );
    this->ui->nameEdit->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->nameEdit->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->nameEdit->setCleanHTML( true );
    this->ui->nameEdit->setSimpleEditor( true );
    this->ui->nameEdit->installEventFilter( this );

    this->ui->referenceEdit->setMaximumHeight( this->ui->lineEdit->height());
    this->ui->referenceEdit->document()->setDocumentMargin( 2 );
    this->ui->referenceEdit->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->referenceEdit->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->referenceEdit->setCleanHTML( true );
    this->ui->referenceEdit->setSimpleEditor( true );
    this->ui->referenceEdit->installEventFilter( this );

    // dummy line edit widget is not needed anymore
    this->ui->lineEdit->hide();

    // force widget to resize to minimum
    QApplication::processEvents();
    QTimer::singleShot( 0, [this]() {
        this->resize( this->minimumSizeHint());
    });
}

/**
 * @brief ReagentDialog::eventFilter
 * @param object
 * @param event
 * @return
 */
bool ReagentDialog::eventFilter( QObject *object, QEvent *event ) {
    if ( object == this->ui->nameEdit || object == this->ui->referenceEdit ) {
        if ( event->type() == QEvent::KeyPress ) {
            const QKeyEvent *keyEvent( static_cast<QKeyEvent*>( event ));

            if ( keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Return )
                return true;
        }
    }

    return QDialog::eventFilter( object, event );
}

