/*
===========================================================================
Copyright (C) 2016 Avotu Briezhaudzetava

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

#ifndef GUI_MAIN_H
#define GUI_MAIN_H

//
// includes
//
#include <QLineEdit>
#include <QMainWindow>
#include "gui_addtemplate.h"
#include "template.h"
#include "ui_gui_main.h"
#include "gui_properties.h"

//
// namespace: Ui
//
namespace Ui {
class Gui_Main;
}

//
// class: Main (gui)
//
class Gui_Main : public QMainWindow {
    Q_OBJECT
    Q_PROPERTY( Template::State state READ state WRITE setState )

public:
    explicit Gui_Main( QWidget *parent = 0 );
    ~Gui_Main() { delete this->ui; delete this->m_propertiesDialog; }

    // unit enums
    enum MassUnits {
        Gram = 0,
        Milligram,
        Kilogram
    };
    enum VolumeUnits {
        Milliliter = 0,
        Liter
    };
    enum DensityUnits {
        GramPerMilliliter = 0,
        KilogramPerMilliliter,
        MilligramPerMilliliter
    };
    enum MolarMassUnits {
        GramPerMol = 0,
        KilogramPerMol
    };
    enum MolUnits {
        Mol = 0,
        Millimol,
        Kilomol
    };

    // variable getters
    bool volume( double &volume ) const;
    bool mass( double &mass ) const;
    bool mol( double &mol ) const;
    double density( double &density ) const;
    bool assay( double &assay ) const;
    double molarMass( double &molarMass ) const;

    // variable lock state related
    bool locked() const { return this->m_lock; }
    Template::State state() const { return m_state; }

    Template *curentTemplate;

public slots:
    void addTemplate( const QString &name );

private slots:
    // variable lock state related
    void setState( const Template::State &state );
    void unlock() { this->m_lock = false; }
    void lock() { this->m_lock = true; }

    // ui elements
    void on_assayEdit_textChanged();
    void on_densityEdit_textChanged();
    void on_molarEdit_textChanged();
    void on_molEdit_textChanged();
    void on_volumeEdit_textChanged();
    void on_solidCheck_stateChanged(int);
    void on_massEdit_textChanged();
    void on_templateCombo_currentIndexChanged( int index );
    void on_addAction_triggered() { Gui_AddTemplate dialog( this ); dialog.exec(); }
    void on_keepAboveAction_toggled( bool );
    void on_removeAction_triggered();
    void on_saveAction_triggered();
    void on_closeAction_triggered();
    void on_infoButton_clicked() { if ( this->curentTemplate != NULL ) this->m_propertiesDialog->setReagentId( this->curentTemplate->id()); this->m_propertiesDialog->show(); }

    // calculation related
    void recalculate();
    void calculateMass();

    void fillTemplates( int forceId = -1 );
    void setValue( QLineEdit *container, double value, int precision ) { container->setText( QString( "%1" ).arg( value, 0, 'f', precision, 0 )); }
    void setVolume( double value, int precision = 2 ) { this->setValue( this->ui->volumeEdit, value, precision ); }
    void setMol( double value, int precision = 3 ) { this->setValue( this->ui->molEdit, value, precision ); }
    void setMass( double value, int precision = 1 ) { this->setValue( this->ui->massEdit, value, precision ); }
    void setAssay( double value, int precision = 2 ) { this->setValue( this->ui->assayEdit, value, precision ); }
    void setDensity( double value, int precision = 3 ) { this->setValue( this->ui->densityEdit, value, precision ); }
    void setMolarMass( double value, int precision = 2 ) { this->setValue( this->ui->molarEdit, value, precision ); }

    // string parsing related
    bool parseString( const QString &text, double &value, QString &units ) const;

    // unit related
    // TODO: perform actual conversions
    void setMassUnits( const MassUnits units = Gram ) { this->m_massUnits = units; }
    void setVolumeUnits( const VolumeUnits units = Milliliter ) { this->m_volumeUnits = units; }
    void setDensityUnits( const DensityUnits units = GramPerMilliliter ) { this->m_densityUnits = units; }
    void setMolarMassUnits( const MolarMassUnits units = GramPerMol ) { this->m_molarMassUnits = units; }
    void setMolUnits( const MolUnits units = Mol ) { this->m_molUnits = units; }

private:
    Ui::Gui_Main *ui;

    // variable lock state related
    bool m_lock;
    Template::State m_state;

    // units
    MassUnits m_massUnits;
    VolumeUnits m_volumeUnits;
    DensityUnits m_densityUnits;
    MolarMassUnits m_molarMassUnits;
    MolUnits m_molUnits;
    Gui_Properties *m_propertiesDialog;
};

#endif // GUI_MAIN_H
