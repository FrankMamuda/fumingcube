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

//
// includes
//
#include "gui_main.h"
#include "ui_gui_main.h"
#include <QMessageBox>
#include "database.h"

//
// TODO:
//  use actual units, not just parse values (partially done)
//  adjustable precision
//  remember units once typed in
//  disable properties dialog if no reagent?
//

/**
 * @brief Gui_Main::Gui_Main
 * @param parent
 */
Gui_Main::Gui_Main( QWidget *parent) : QMainWindow( parent ), curentReagent( NULL ), ui( new Ui::Gui_Main ), m_lock( true ), m_propertiesDialog( new Gui_Properties( this )) {
    this->ui->setupUi( this );

    m.initialise();

    this->unlock();
    this->recalculate();
    this->fillReagents();

    // default to water, if no reagents on init
    if ( this->curentReagent == NULL ) {
        this->lock();
        this->setMass( 1.0f );
        this->setVolume( 1.0f );
        this->setMolarMass( 18.02f );
        this->setAssay( 100.0f );
        this->setDensity( 1.0f );
        this->unlock();
        this->recalculate();
    }

    this->on_keepAboveAction_toggled( this->ui->keepAboveAction->isChecked());
    this->m_propertiesDialog->setWindowFlags( this->windowFlags());
}

/**
 * @brief Gui_Main::parseString
 * @param text
 * @param value
 * @param units
 * @return
 */
bool Gui_Main::parseString( const QString &text, double &value, QString &units ) const {
    QRegExp exp;
    bool ok = false;

    exp.setPattern( "(\\d+[\\.|\\,]?\\d*)[\\s]?(%|\\w+\\/?\\w*)*" );
    exp.setCaseSensitivity( Qt::CaseInsensitive );
    if ( exp.indexIn( text ) != -1 ) {
        value = exp.cap(1).replace( ',', '.' ).toDouble( &ok );
        units = exp.cap(2).toLower();
    }

    return ok;
}

/**
 * @brief Gui_Main::volume
 * @param volume
 * @return
 */
bool Gui_Main::volume( double &volume ) const {
    QString units;
    //double scaleFactor = 1.0f;

    if ( this->parseString( this->ui->volumeEdit->text(), volume, units )) {
        if ( units.isEmpty()) {
            this->ui->volumeLabel->setText( this->tr( "Volume, ml" ));
            return true;
        }

        if ( !QString::compare( units, "l" )) {
            this->ui->volumeLabel->setText( this->tr( "Volume, L" ));
            volume = volume * 1000.0f;
        } else if ( !QString::compare( units, "ml" ))
            this->ui->volumeLabel->setText( this->tr( "Volume, ml" ));
        else
            return false;

        return true;
    }

    return false;
}

/**
 * @brief Gui_Main::mass
 * @param mass
 * @return
 */
bool Gui_Main::mass( double &mass ) const {
    QString units;

    if ( this->parseString( this->ui->massEdit->text(), mass, units )) {
        if ( units.isEmpty()) {
            this->ui->massLabel->setText( this->tr( "Mass, g" ));
            return true;
        }

        if ( !QString::compare( units, "kg" )) {
            this->ui->massLabel->setText( this->tr( "Mass, kg" ));
            mass = mass * 1000.0f;
        } else if ( !QString::compare( units, "mg" )) {
            this->ui->massLabel->setText( this->tr( "Mass, mg" ));
            mass = mass / 1000.0f;
        } else if ( !QString::compare( units, "g" ))
            this->ui->massLabel->setText( this->tr( "Mass, g" ));
        else
            return false;

        return true;
    }

    return false;
}

/**
 * @brief Gui_Main::mol
 * @param mol
 * @return
 */
bool Gui_Main::mol( double &mol ) const {
    QString units;

    if ( this->parseString( this->ui->molEdit->text(), mol, units )) {
        if ( units.isEmpty()) {
            this->ui->molLabel->setText( "mol" );
            return true;
        }

        if ( !QString::compare( units, "mmol" )) {
            this->ui->molLabel->setText( "mmol" );
            mol = mol / 1000.0f;
        } else if ( !QString::compare( units, "mol" ))
            this->ui->molLabel->setText( "mol" );
        else if ( !QString::compare( units, "kmol" )) {
            this->ui->molLabel->setText( "kmol" );
            mol = mol * 1000.0f;
        } else
            return false;

        return true;
    }

    return false;
}

/**
 * @brief Gui_Main::density
 * @param density
 * @return
 */
double Gui_Main::density( double &density ) const {
    QString units;

    if ( this->parseString( this->ui->densityEdit->text(), density, units )) {
        if ( units.isEmpty()) {
            this->ui->densityLabel->setText( this->tr( "Density, g/ml" ));
            return true;
        }

        if ( !QString::compare( units, "g/l" )) {
            this->ui->densityLabel->setText( this->tr( "Density, g/L" ));
            density = density / 1000.0f;
        } else if ( !QString::compare( units, "g/ml" ) || !QString::compare( units, "g/cm3" ))
            this->ui->densityLabel->setText( this->tr( "Density, g/ml" ));
        else if ( !QString::compare( units, "mg/ml" )) {
            this->ui->densityLabel->setText( this->tr( "Density, mg/ml" ));
            density = density / 1000.0f;
        } else
            return false;

        return true;
    }

    return false;
}

/**
 * @brief Gui_Main::assay
 * @param assay
 * @return
 */
bool Gui_Main::assay( double &assay ) const {
    QString units;

    if ( this->parseString( this->ui->assayEdit->text(), assay, units )) {
        assay = assay / 100.0f;

        if ( units.isEmpty()) {
            this->ui->assayLabel->setText( this->tr( "Assay, %" ));
            return true;
        }

        if ( !QString::compare( units, "%" ))
            this->ui->assayLabel->setText( this->tr( "Assay, %" ));
        else
            return false;

        return true;
    }

    return false;
}

/**
 * @brief Gui_Main::molarMass
 * @param molarMass
 * @return
 */
double Gui_Main::molarMass( double &molarMass ) const {
    QString units;

    if ( this->parseString( this->ui->molarEdit->text(), molarMass, units )) {
        if ( units.isEmpty()) {
            this->ui->molarLabel->setText( "M, g/mol" );
            return true;
        }

        if ( !QString::compare( units, "g/mol" ))
            this->ui->molarLabel->setText( "M, g/mol" );
        else if ( !QString::compare( units, "kg/mol" )) {
            this->ui->molarLabel->setText( "M, kg/mol" );
            molarMass = molarMass * 1000.0f;
        } else
            return false;

        return true;
    }

    return false;
}

/**
 * @brief Gui_Main::fillReagents
 * @param forceId
 */
void Gui_Main::fillReagents( int forceId ) {
    int newId = -1, y;
    Reagent *reagentPtr;

    if ( forceId != -1 )
        newId = forceId;

    this->ui->reagentCombo->clear();

    foreach ( Reagent *reagentPtr, db.reagentList )
        this->ui->reagentCombo->addItem( reagentPtr->name(), reagentPtr->id());

    if ( newId != -1 ) {
        reagentPtr = Reagent::fromId( newId );
        if ( reagentPtr != NULL ) {
            for ( y = 0; y < this->ui->reagentCombo->count(); y++ ) {
                if ( this->ui->reagentCombo->itemData( y, Qt::UserRole ).toInt() == newId ) {
                    this->ui->reagentCombo->setCurrentIndex( y );
                    return;
                }
            }
        }
    }

    if ( db.reagentList.count())
        this->curentReagent = db.reagentList.first();
}

/**
 * @brief Gui_Main::recalculate
 */
void Gui_Main::recalculate() {
    this->on_solidCheck_stateChanged( this->ui->solidCheck->checkState());

    if ( this->state() == Reagent::Liquid )
        this->on_volumeEdit_textChanged();
    else if ( this->state() == Reagent::Solid )
        this->on_massEdit_textChanged();
}

/**
 * @brief Gui_Main::on_molEdit_textChanged
 */
void Gui_Main::on_molEdit_textChanged() {
    double mol, assay, density, molarMass;

    if ( this->locked())
        return;

    if ( !this->mol( mol ) || !this->assay( assay ) || !this->density( density ) || !this->molarMass( molarMass )) {
        this->ui->molEdit->setStyleSheet( "QLineEdit { background-color: #ff9999; }" );
        return;
    }
    this->ui->molEdit->setStyleSheet( "QLineEdit {}" );

    this->lock();

    if ( this->state() == Reagent::Liquid )
        this->setVolume(( mol * molarMass ) / assay / density );
    else if ( this->state() == Reagent::Solid )
        this->setMass(( mol * molarMass ) / assay );

    this->calculateMass();
    this->unlock();
}

/**
 * @brief Gui_Main::on_volumeEdit_textChanged
 */
void Gui_Main::on_volumeEdit_textChanged() {
    double volume, assay, density, molarMass;

    if ( this->state() != Reagent::Liquid || this->locked())
        return;

    if ( !this->volume( volume ) || !this->assay( assay ) || !this->density( density ) || !this->molarMass( molarMass )) {
        this->ui->volumeEdit->setStyleSheet( "QLineEdit { background-color: #ff9999; }" );
        return;
    }
    this->ui->volumeEdit->setStyleSheet( "QLineEdit {}");

    this->lock();
    this->calculateMass();
    this->setMol( volume * density * assay / molarMass );
    this->unlock();
}

/**
 * @brief Gui_Main::calculateMass
 */
void Gui_Main::calculateMass() {
    double assay, density;

    this->ui->massEdit->setStyleSheet( "QLineEdit {}" );

    if ( !this->assay( assay ) || !this->density( density )) {
        this->ui->massEdit->setStyleSheet( "QLineEdit { background-color: #ff9999; }" );
        return;
    }

    if ( this->state() == Reagent::Liquid ) {
        double volume;

        if ( !this->volume( volume ))
            return;

        this->ui->massEdit->setText( QString( "%1" ).arg( volume * density, 0, 'f', 1, 0 ));
        this->ui->pureEdit->setText( QString( "%1" ).arg( volume * density * assay, 0, 'f', 2, 0 ));
    } else if ( this->state() == Reagent::Solid ) {
        double mass;

        if ( !this->mass( mass ))
            return;

        this->ui->pureEdit->setText( QString( "%1" ).arg( mass * assay, 0, 'f', 2, 0 ));
    }
}

/**
 * @brief Gui_Main::on_solidCheck_stateChanged
 * @param state
 */
void Gui_Main::on_solidCheck_stateChanged( int state ) {
    if ( this->locked())
        return;

    switch ( state ) {
    case Qt::Checked:
        this->ui->massEdit->setReadOnly( false );
        //this->ui->massEdit->setEnabled( true );
        this->ui->volumeEdit->setDisabled( true );
        this->ui->densityEdit->setDisabled( true );
        this->setState( Reagent::Solid );
        //this->ui->massEdit->setFocus();
        break;

    case Qt::Unchecked:
        this->ui->massEdit->setReadOnly( true );
        //this->ui->massEdit->setEnabled( false );
        this->ui->volumeEdit->setEnabled( true );
        this->ui->densityEdit->setEnabled( true );
        this->setState( Reagent::Liquid );
        this->on_volumeEdit_textChanged();
        //this->ui->volumeEdit->setFocus();
        break;

    default:
        break;
    }
}

/**
 * @brief Gui_Main::on_massEdit_textChanged
 */
void Gui_Main::on_massEdit_textChanged() {
    double mass, assay, molarMass;

    if ( this->state() != Reagent::Solid )
        return;

    if ( this->locked())
        return;

    if ( !this->mass( mass ) || !this->assay( assay ) || !this->molarMass( molarMass )) {
        this->ui->massEdit->setStyleSheet( "QLineEdit { background-color: #ff9999; }" );
        return;
    }
    this->ui->massEdit->setStyleSheet( "QLineEdit {}" );

    this->lock();
    this->calculateMass();
    this->setMol( mass * assay / molarMass );
    this->unlock();
}

/**
 * @brief Gui_Main::addReagent
 * @param name
 */
void Gui_Main::addReagent( const QString &name ) {
    double amount = 0.0f, assay, density, molarMass;
    bool amountParsed = false;

    if ( this->state() == Reagent::Solid )
        amountParsed = this->mass( amount );
    else if ( this->state() == Reagent::Liquid )
        amountParsed = this->volume( amount );

    if ( !amountParsed || !this->assay( assay ) || !this->density( density ) || !this->molarMass( molarMass )) {
        QMessageBox msgBox;
        msgBox.setWindowFlags( this->windowFlags());
        msgBox.setIcon( QMessageBox::Critical );
        msgBox.setText( this->tr( "Invalid amount specified" ));
        msgBox.exec();
        return;
    }

    Reagent::add( name, amount, density, assay, molarMass, this->state());
    this->fillReagents( db.reagentList.last()->id());
}

/**
 * @brief Gui_Main::setState
 * @param state
 */
void Gui_Main::setState( const Reagent::State &state ) {
    this->m_state = state;

    if ( state == Reagent::Solid )
        this->ui->solidCheck->setChecked( true );
    else if ( state == Reagent::Liquid )
        this->ui->solidCheck->setChecked( false );
}

/**
 * @brief Gui_Main::on_reagentCombo_currentIndexChanged
 * @param index
 */
void Gui_Main::on_reagentCombo_currentIndexChanged( int index ) {
    if ( this->locked())
        return;

    if ( this->state() == Reagent::Solid )
        this->ui->massEdit->setFocus();
    else if ( this->state() == Reagent::Liquid )
        this->ui->volumeEdit->setFocus();

    this->curentReagent = Reagent::fromId( this->ui->reagentCombo->itemData( index, Qt::UserRole ).toInt());
    if ( this->curentReagent != NULL ) {
        if ( this->curentReagent->state() == Reagent::Solid )
            this->setMass( this->curentReagent->amount());
        else if ( this->curentReagent->state() == Reagent::Liquid )
            this->setVolume( this->curentReagent->amount());

        this->setDensity( this->curentReagent->density());
        this->setAssay( this->curentReagent->assay() * 100.0f );
        this->setMolarMass( this->curentReagent->molarMass());
        this->setState( this->curentReagent->state());
    }
}

/**
 * @brief Gui_Main::on_keepAboveAction_toggled
 * @param toggle
 */
void Gui_Main::on_keepAboveAction_toggled( bool toggle ) {
    if ( toggle )
        this->setWindowFlags( this->windowFlags() | Qt::WindowStaysOnTopHint );
    else
        this->setWindowFlags( this->windowFlags() ^= Qt::WindowStaysOnTopHint );

    this->show();
}

/**
 * @brief Gui_Main::on_densityEdit_textChanged
 */
void Gui_Main::on_densityEdit_textChanged() {
    double density;

    if ( this->locked())
        return;

    if ( !this->density( density )) {
        this->ui->densityEdit->setStyleSheet( "QLineEdit { background-color: #ff9999; }" );
        return;
    }
    this->ui->densityEdit->setStyleSheet( "QLineEdit {}" );
    this->recalculate();
}

/**
 * @brief Gui_Main::on_molarEdit_textChanged
 */
void Gui_Main::on_molarEdit_textChanged() {
    double molarMass;

    if ( this->locked())
        return;

    if ( !this->molarMass( molarMass )) {
        this->ui->molarEdit->setStyleSheet( "QLineEdit { background-color: #ff9999; }" );
        return;
    }
    this->ui->molarEdit->setStyleSheet( "QLineEdit {}" );
    this->recalculate();
}

/**
 * @brief Gui_Main::on_assayEdit_textChanged
 */
void Gui_Main::on_assayEdit_textChanged() {
    double assay;

    if ( this->locked())
        return;

    if ( !this->assay( assay )) {
        this->ui->assayEdit->setStyleSheet( "QLineEdit { background-color: #ff9999; }" );
        return;
    }
    this->ui->assayEdit->setStyleSheet( "QLineEdit {}" );
    this->recalculate();
}

/**
 * @brief Gui_Main::on_removeAction_triggered
 */
void Gui_Main::on_removeAction_triggered() {
    int state;
    QMessageBox msgBox;
    QSqlQuery query;
    Reagent *reagentPtr;

    reagentPtr = Reagent::fromId( this->ui->reagentCombo->itemData( this->ui->reagentCombo->currentIndex(), Qt::UserRole ).toInt());

    if ( reagentPtr != NULL ) {
        msgBox.setWindowFlags( this->windowFlags());
        msgBox.setText( this->tr( "Do you really want to remove '%1'?" ).arg( reagentPtr->name()));
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
        msgBox.setDefaultButton( QMessageBox::Yes );
        msgBox.setIcon( QMessageBox::Warning );
        msgBox.setWindowIcon( QIcon( ":/icons/reagent_remove_24" ));
        state = msgBox.exec();

        // check options
        switch ( state ) {
        case QMessageBox::Yes:
            // remove from memory and database
            db.reagentList.removeOne( reagentPtr );
            query.exec( QString( "delete from reagents where id=%1" ).arg( reagentPtr->id()));

            // refill list
            this->fillReagents();
            break;

        case QMessageBox::No:
        default:
            return;
        }
    }
}

/**
 * @brief Gui_Main::on_saveAction_triggered
 */
void Gui_Main::on_saveAction_triggered() {
    double amount, molarMass, density, assay;

    // failsafe
    if ( this->curentReagent == NULL )
        return;

    if ( !this->molarMass( molarMass ) || !this->density( density ) || !this->assay( assay ))
        return;

    if ( this->state() == Reagent::Solid && !this->mass( amount ))
        return;

    if ( this->state() == Reagent::Liquid && !this->volume( amount ))
        return;

    this->curentReagent->setAmount( amount );
    this->curentReagent->setMolarMass( molarMass );
    this->curentReagent->setAssay( assay );
    this->curentReagent->setState( this->state());

    if ( this->state() == Reagent::Liquid )
        this->curentReagent->setDensity( density );
}

