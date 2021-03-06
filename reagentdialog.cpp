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
#include <QRegularExpressionValidator>
#include <QTimer>
#include "labeldock.h"
#include "label.h"
#include "tagdialog.h"
#include "htmlutils.h"

/*
 * Reagent reference map
 */
const static QMap<QString, QString> reagentReferences {
        { "Sodium hydroxide",              "NaOH" },
        { QT_TRANSLATE_NOOP( "Reagent", "Acetic acid" ),                   "AcOH" },
        { "Trifluoroacetic acid",          "CF<sub>3</sub>COOH" },
        { "Trifluoroacetic acid anydride", ReagentTools::DigitsToSubscript( "(CF3COOH)2O" ) },
        { QT_TRANSLATE_NOOP( "Reagent", "Water" ),                         ReagentTools::DigitsToSubscript( "H2O" ) },
        { QT_TRANSLATE_NOOP( "Reagent", "Acetone" ),                       "dimethyl ketone" },
        { QT_TRANSLATE_NOOP( "Reagent", "Acetonitrile" ),                  "MeCN" },
        { "Acetylene",                     ReagentTools::DigitsToSubscript( "C2H2" ) },
        { "Ammonia",                       ReagentTools::DigitsToSubscript( "NH3" ) },
        { "Ammonium hydroxide",            ReagentTools::DigitsToSubscript( "NH4OH" ) },
        { "n-Bromosuccinimide",            "NBS" },
        { "Methyl ethyl ketone",           "MEK" },
        { "Butanone",                      "MEK" },
        { "n-Butyllithium",                "<span style=\"font-style:italic;\">n</span>-BuLi" },
        { "Carbon disulfide",              ReagentTools::DigitsToSubscript( "CS2" ) },
        { "Carbon tetrachloride",          ReagentTools::DigitsToSubscript( "CCl4" ) },
        { "Carbonyldiimidazole",           "CDI" },
        { "Chloroform",                    ReagentTools::DigitsToSubscript( "CHCl3" ) },
        { "Copper iodide",                 "CuI" },
        { "Borane",                        ReagentTools::DigitsToSubscript( "BH3" ) },
        { "Diborane",                      ReagentTools::DigitsToSubscript( "B2H6" ) },
        { "Diethyl ether",                 ReagentTools::DigitsToSubscript( "Et2O" ) },
        { "Diisobutylaluminium hydride",   "DIBAL-H" },
        { "Dimethylformamide",             "DMF" },
        { "Dimethylsulfide",               "DMS" },
        { "Dimethyl sulfoxide",            "DMSO" },
        { QT_TRANSLATE_NOOP( "Reagent", "Ethanol" ),                       "EtOH" },
        { QT_TRANSLATE_NOOP( "Reagent", "Methanol" ),                      "MeOH" },
        { "Isobutanol",                    "<span style=\"font-style:italic;\">i</span>-BuOH" },
        { "Butanol",                       "<span style=\"font-style:italic;\">n</span>-BuOH" },
        { "2-butanol",                     "2-BuOH" },
        { "2-butanol",                     "2-BuOH" },
        { "Formaldehyde",                  "methanal" },
        { "Formic acid",                   "HCOOH" },
        { "Hydrazine",                     ReagentTools::DigitsToSubscript( "N2H4" ) },
        { QT_TRANSLATE_NOOP( "Reagent", "Hydrochloric acid" ),             "HCl" },
        { "Hydrofluoric acid",             "HF" },
        { "Hydrobromic acid",              "HBr" },
        { "Hydrogen peroxide",             ReagentTools::DigitsToSubscript( "H2O2" ) },
        { "Isopropyl alcohol",             "<span style=\"font-style:italic;\">i</span>-PrOH" },
        { "Isopropanol",                   "<span style=\"font-style:italic;\">i</span>-PrOH" },
        { "Lithium aluminium hydride",     "LiAlH4" },
        { "Manganese dioxide",             ReagentTools::DigitsToSubscript( "MnO2" ) },
        { "Methyl tert-butyl ether",       "MTBE" },
        { "Nitric acid",                   ReagentTools::DigitsToSubscript( "HNO3" ) },
        { "Oxalyl chloride",               ReagentTools::DigitsToSubscript( "(COCl)2" ) },
        { "Perchloric acid",               ReagentTools::DigitsToSubscript( "HClO4" ) },
        { "Phosphoric acid",               ReagentTools::DigitsToSubscript( "H3PO4" ) },
        { QT_TRANSLATE_NOOP( "Reagent", "Orthophosphoric acid" ),          ReagentTools::DigitsToSubscript( "H3PO4" ) },
        { "Phosphorus pentachloride",      ReagentTools::DigitsToSubscript( "PCl5" ) },
        { "Phosphorus pentoxide",          ReagentTools::DigitsToSubscript( "P2O5" ) },
        { "Phosphorus tribromide",         ReagentTools::DigitsToSubscript( "PBr3" ) },
        { "Phosphorus trichloride",        ReagentTools::DigitsToSubscript( "PCl3" ) },
        { "Phosphoryl chloride",           ReagentTools::DigitsToSubscript( "POCl3" ) },
        { QT_TRANSLATE_NOOP( "Reagent", "Potassium hydroxide" ),           "KOH" },
        { "Potassium borohydride",         ReagentTools::DigitsToSubscript( "KBH4" ) },
        { "Potassium chloride",            "KCl" },
        { "Potassium bromide",             "KBr" },
        { "Potassium fluoride",            "KF" },
        { "Potassium iodide",              "KI" },
        { "Potassium bisulfite",           ReagentTools::DigitsToSubscript( "KHSO3" ) },
        { "Potassium bicarbonate",         ReagentTools::DigitsToSubscript( "KHCO3" ) },
        { "Potassium dithionite",          ReagentTools::DigitsToSubscript( "K2S2O4" ) },
        { "Potassium hydrosulfite",        ReagentTools::DigitsToSubscript( "K2S2O4" ) },
        { "Potassium sulfate",             ReagentTools::DigitsToSubscript( "KSO4" ) },
        { "Potassium carbonate",           ReagentTools::DigitsToSubscript( "K2CO3" ) },
        { "Potassium permanganate",        ReagentTools::DigitsToSubscript( "KMnO4" ) },
        { "Raney nickel",                  "RaNi" },
        { "Silver nitrate",                ReagentTools::DigitsToSubscript( "AgNO3" ) },
        { "Sodium amide",                  ReagentTools::DigitsToSubscript( "NaNH2" ) },
        { "Sodium azide",                  ReagentTools::DigitsToSubscript( "NaN3" ) },
        { "Sodium borohydride",            ReagentTools::DigitsToSubscript( "NaBH4" ) },
        { "Sodium chlorite",               ReagentTools::DigitsToSubscript( "NaClO2" ) },
        { "Sodium hydride",                "NaH" },
        { QT_TRANSLATE_NOOP( "Reagent", "Sodium hydroxide" ),              "NaOH" },
        { "Sodium hypochlorite",           "NaClO" },
        { "Sodium nitrite",                ReagentTools::DigitsToSubscript( "NaNO2" ) },
        { QT_TRANSLATE_NOOP( "Reagent", "Sodium chloride" ),               "NaCl" },
        { "Sodium bromide",                "NaBr" },
        { "Sodium fluoride",               "NaF" },
        { "Sodium iodide",                 "NaI" },
        { "Sodium bisulfite",              ReagentTools::DigitsToSubscript( "NaHSO3" ) },
        { "Sodium bicarbonate",            ReagentTools::DigitsToSubscript( "NaHCO3" ) },
        { "Sodium dithionite",             ReagentTools::DigitsToSubscript( "Na2S2O4" ) },
        { "Sodium hydrosulfite",           ReagentTools::DigitsToSubscript( "Na2S2O4" ) },
        { "Sodium sulfate",                ReagentTools::DigitsToSubscript( "NaSO4" ) },
        { "Sodium carbonate",              ReagentTools::DigitsToSubscript( "Na2CO3" ) },
        { "Magnesium sulfate",             ReagentTools::DigitsToSubscript( "MgSO4" ) },
        { "Tetrabutylammonium bromide",    "TBAB" },
        { "Tetrabutylammonium fluoride",   "TBAF" },
        { QT_TRANSLATE_NOOP( "Reagent", "Sulfuric acid" ),                 ReagentTools::DigitsToSubscript( "H2SO4" ) },
        { "Tetrahydrofuran",               "THF" },
        { "Thionyl chloride",              ReagentTools::DigitsToSubscript( "SOCl2" ) },
        { "Titanium tetrachloride",        ReagentTools::DigitsToSubscript( "TiCl4" ) },
        { "Triphenylphosphine",            ReagentTools::DigitsToSubscript( "PPh3" ) }
};

/**
 * @brief ReagentDialog::ReagentDialog
 * @param parent
 */
ReagentDialog::ReagentDialog( QWidget *parent, const QString &name, const QString &reference, const Modes &mode )
        : QDialog( parent ), ui( new Ui::ReagentDialog ), m_mode( mode ) {
    this->completer = new QCompleter( reagentReferences.keys());
    this->completer->setCaseSensitivity( Qt::CaseInsensitive );
    this->ui->setupUi( this );
    this->ui->mainWindow->setWindowFlags( Qt::Widget );

    if ( this->mode() == AddMode || this->mode() == EditMode || this->mode() == SearchMode ) {
        // setup editor toolbar
        this->ui->editorToolBar->installFeature( EditorToolbar::Font );
        this->ui->editorToolBar->installFeature( EditorToolbar::VerticalAlignment );
        this->ui->editorToolBar->installFeature( EditorToolbar::CharacterMap );

        // actions performed upon entering name editor
        ReagentDialog::connect( this->ui->nameEdit, &TextEdit::entered, [ this ]() {
            this->ui->editorToolBar->setEditor( this->ui->nameEdit );
        } );

        // actions performed upon entering reference editor
        ReagentDialog::connect( this->ui->referenceEdit, &TextEdit::entered, [ this ]() {
            this->ui->editorToolBar->setEditor( this->ui->referenceEdit );
        } );

        // set completer
        this->ui->nameEdit->setCompleter( completer );
    }

    // focus on the name editor to begin with
    this->ui->nameEdit->setFocus();

    // setup reference lock button
    auto checkState = [ this ]( bool checked ) {
        this->ui->lockButton->setIcon( QIcon::fromTheme( checked ? "lock" : "unlock" ));
    };
    QToolButton::connect( this->ui->lockButton, &QToolButton::toggled, this, checkState );

    switch ( this->mode()) {
    case AddMode:
        checkState( false );
        break;

    case SearchMode:
        checkState( false );
        this->ui->propertyCheck->hide();
        break;

    case EditMode:
        this->setWindowTitle( ReagentDialog::tr( "Edit reagent" ));
        this->ui->lockButton->setChecked( this->mode() == EditMode );
        this->ui->propertyCheck->hide();
        this->ui->labelButton->hide();
        break;

    case BatchMode:
    case BatchEditMode:
        this->ui->referenceLabel->hide();
        this->ui->referenceEdit->hide();
        this->ui->labelButton->hide();
        this->ui->propertyCheck->hide();
        this->ui->nameEdit->setPlaceholderText( TextEdit::tr( "Enter batch name" ));
        this->ui->lockButton->hide();
        this->ui->editorToolBar->hide();

        this->setWindowTitle( this->mode() == BatchMode ? ReagentDialog::tr( "Add batch" ) : ReagentDialog::tr( "Rename batch" ));

        break;
    }

    if ( !name.isEmpty())
        this->ui->nameEdit->setText( name );

    if ( !reference.isEmpty())
        this->ui->referenceEdit->setText( reference );

    // bind property button
    this->variables << Variable::instance()->bind( "fetchPropertiesOnAddition", this->ui->propertyCheck );

    // set current label
    this->labels << LabelDock::instance()->currentLabels();
    ReagentDialog::connect( this->ui->labelButton, &QPushButton::pressed, [ this ]() {
        // modify labels in label selector
        LabelSelector ls( this, this->labels );
        ls.exec();
        this->labels = ls.labelIds;
    } );

    auto validate = []( const QString &text ) {
        QString string( text );
        int pos;
        const QRegularExpression re( R"([\p{L}0-9-+,.\)\(\[\]\{\}\@\s\p{No}\/\\:]+)" );
        const QRegularExpressionValidator validator( re );
        return validator.validate( string, pos ) != QRegularExpressionValidator::Invalid;
    };

    auto checkInputs = [ this, validate ]() {
        const bool validName = validate( this->ui->nameEdit->toPlainText());
        const bool validReference = validate( this->ui->referenceEdit->toPlainText());
        this->ui->nameEdit->setStyleSheet( validName ? "QTextEdit {}" : "QTextEdit { background-color: #ff5555; color: white; }" );
        this->ui->referenceEdit->setStyleSheet( validReference ? "QTextEdit {}" : "QTextEdit { background-color: #ff5555; color: white; }" );

        this->ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( validReference && validName );
    };

    // validate name & reference
    TextEdit::connect( this->ui->nameEdit, &TextEdit::textChanged, this, checkInputs );
    TextEdit::connect( this->ui->referenceEdit, &TextEdit::textChanged, this, checkInputs );

    // handle return keys
    this->ui->nameEdit->installEventFilter( this );
    this->ui->referenceEdit->installEventFilter( this );
}

/**
 * @brief ReagentDialog::~ReagentDialog
 */
ReagentDialog::~ReagentDialog() {
    // unbind vars
    for ( const QString &key : qAsConst( this->variables ))
        Variable::instance()->unbind( key );

    delete this->completer;
    delete this->ui;
}

/**
 * @brief ReagentDialog::name
 * @return
 */
QString ReagentDialog::name() const {
    return HTMLUtils::captureBody( this->ui->nameEdit->toHtml());
}

/**
 * @brief ReagentDialog::reference
 * @return
 */
QString ReagentDialog::reference() const {
    return HTMLUtils::captureBody( this->ui->referenceEdit->toHtml());
}

/**
 * @brief ReagentDialog::eventFilter
 * @param object
 * @param event
 * @return
 */
bool ReagentDialog::eventFilter( QObject *object, QEvent *event ) {
    if ( event->type() == QEvent::KeyPress && ( object == this->ui->nameEdit || object == this->ui->referenceEdit )) {
        const QKeyEvent *keyEvent( dynamic_cast<QKeyEvent *>( event ));
        if ( keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter ) {
            if ( object == this->ui->nameEdit ) {
                if ( this->mode() == AddMode || this->mode() == EditMode || this->mode() == SearchMode )
                    this->ui->referenceEdit->setFocus();
                else
                    this->ui->buttonBox->button( QDialogButtonBox::Ok )->animateClick();
            } else if ( object == this->ui->referenceEdit && this->ui->buttonBox->button( QDialogButtonBox::Ok )->isEnabled())
                this->ui->buttonBox->button( QDialogButtonBox::Ok )->animateClick();

            return true;
        }
    }
    return object->eventFilter( object, event );
}

/**
 * @brief ReagentDialog::showEvent
 * @param event
 */
void ReagentDialog::showEvent( QShowEvent *event ) {
    QDialog::showEvent( event );

    this->ui->nameEdit->document()->setDocumentMargin( 2 );
    this->ui->nameEdit->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->nameEdit->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->nameEdit->setCleanHTML( true );
    this->ui->nameEdit->setSimpleEditor( true );

    this->ui->referenceEdit->document()->setDocumentMargin( 2 );
    this->ui->referenceEdit->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->referenceEdit->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->referenceEdit->setCleanHTML( true );
    this->ui->referenceEdit->setSimpleEditor( true );

    // force widget to resize to minimum
    QApplication::processEvents();
    QTimer::singleShot( 0, this, [ this ]() {
        this->resize( this->width(), this->minimumSizeHint().height());
    } );

    // copy reference from name by default
    ReagentDialog::connect( this->ui->nameEdit, &TextEdit::textChanged, this, [ this ]() {
        if ( this->ui->lockButton->isChecked())
            return;

        const QString plain( this->ui->nameEdit->toPlainText());
        const QString html( this->ui->nameEdit->toHtml());

        if ( plain.isEmpty()) {
            this->ui->referenceEdit->setPlainText( "" );
            return;
        }

        QString match;
        const QStringList list( reagentReferences.keys());
        for ( const QString &key : list ) {
            if ( !QString::compare( key, plain, Qt::CaseInsensitive )) {
                match = key;
                break;
            }
        }
        this->ui->referenceEdit->setText( !match.isEmpty() ? reagentReferences[match] : QString( html ).remove( ' ' ));
    } );

    //this->ui->nameEdit->setMaximumHeight( 128 );
    //this->ui->referenceEdit->setMaximumHeight( 128 );
}
