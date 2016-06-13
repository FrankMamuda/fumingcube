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
//

/*
==========
constructor
==========
*/
Gui_Main::Gui_Main( QWidget *parent) : QMainWindow( parent ), ui( new Ui::Gui_Main ), m_lock( true ), curentTemplate( NULL ) {
    this->ui->setupUi( this );

    m.initialise();

    this->unlock();
    this->recalculate();
    this->fillTemplates();

    this->on_keepAboveAction_toggled( this->ui->keepAboveAction->isChecked());
}

/*
==========
parseString
==========
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

/*
==========
volume
==========
*/
bool Gui_Main::volume( double &volume ) const {
    QString units;
    //double scaleFactor = 1.0f;

    if ( this->parseString( this->ui->volumeEdit->text(), volume, units )) {
        if ( units.isEmpty()) {
            this->ui->volumeLabel->setText( "Volume, ml" );
            return true;
        }

        if ( !QString::compare( units, "l" )) {
            this->ui->volumeLabel->setText( "Volume, L" );
            volume = volume * 1000.0f;
        } else if ( !QString::compare( units, "ml" ))
            this->ui->volumeLabel->setText( "Volume, ml" );
        else
            return false;

        return true;
    }

    return false;
}

/*
==========
mass
==========
*/
bool Gui_Main::mass( double &mass ) const {
    QString units;

    if ( this->parseString( this->ui->massEdit->text(), mass, units )) {
        if ( units.isEmpty()) {
            this->ui->massLabel->setText( "Mass, g" );
            return true;
        }

        if ( !QString::compare( units, "kg" )) {
            this->ui->massLabel->setText( "Mass, kg" );
            mass = mass * 1000.0f;
        } else if ( !QString::compare( units, "mg" )) {
            this->ui->massLabel->setText( "Mass, mg" );
            mass = mass / 1000.0f;
        } else if ( !QString::compare( units, "g" ))
            this->ui->massLabel->setText( "Mass, g" );
        else
            return false;

        return true;
    }

    return false;
}

/*
==========
mol
==========
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

/*
==========
density
==========
*/
double Gui_Main::density( double &density ) const {
    QString units;

    if ( this->parseString( this->ui->densityEdit->text(), density, units )) {
        if ( units.isEmpty()) {
            this->ui->densityLabel->setText( "Density, g/ml" );
            return true;
        }

        if ( !QString::compare( units, "g/l" )) {
            this->ui->densityLabel->setText( "Density, g/L" );
            density = density / 1000.0f;
        } else if ( !QString::compare( units, "g/ml" ) || !QString::compare( units, "g/cm3" ))
            this->ui->densityLabel->setText( "Density, g/ml" );
        else if ( !QString::compare( units, "mg/ml" )) {
            this->ui->densityLabel->setText( "Density, mg/ml" );
            density = density / 1000.0f;
        } else
            return false;

        return true;
    }

    return false;
}

/*
==========
assay
==========
*/
bool Gui_Main::assay( double &assay ) const {
    QString units;

    if ( this->parseString( this->ui->assayEdit->text(), assay, units )) {
        assay = assay / 100.0f;

        if ( units.isEmpty()) {
            this->ui->assayLabel->setText( "Assay, %" );
            return true;
        }

        if ( !QString::compare( units, "%" ))
            this->ui->assayLabel->setText( "Assay, %" );
        else
            return false;

        return true;
    }

    return false;
}

/*
==========
molarMass
==========
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

/*
==========
fillTemplates
==========
*/
void Gui_Main::fillTemplates( int forceId ) {
    int newId, y;
    Template *templatePtr;

    if ( forceId != -1 )
        newId = forceId;

    this->ui->templateCombo->clear();

    foreach ( Template *templatePtr, db.templateList )
        this->ui->templateCombo->addItem( templatePtr->name(), templatePtr->id());

    if ( newId != -1 ) {
        templatePtr = Template::fromId( newId );
        if ( templatePtr != NULL ) {
            for ( y = 0; y < this->ui->templateCombo->count(); y++ ) {
                if ( this->ui->templateCombo->itemData( y, Qt::UserRole ).toInt() == newId ) {
                    this->ui->templateCombo->setCurrentIndex( y );
                    return;
                }
            }
        }
    }

    if ( db.templateList.count())
        this->curentTemplate = db.templateList.first();
}

/*
==========
recalc
==========
*/
void Gui_Main::recalculate() {
    this->on_solidCheck_stateChanged( this->ui->solidCheck->checkState());

    if ( this->state() == Template::Liquid )
        this->on_volumeEdit_textChanged();
    else if ( this->state() == Template::Solid )
        this->on_massEdit_textChanged();
}

/*
==========
molEdit->textChanged
==========
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
    this->setVolume(( mol * molarMass ) / assay / density );
    this->calculateMass();
    this->unlock();
}

/*
==========
volumeEdit->textChanged
==========
*/
void Gui_Main::on_volumeEdit_textChanged() {
    double volume, assay, density, molarMass;

    if ( this->state() != Template::Liquid || this->locked())
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

/*
==========
calculateMass
==========
*/
void Gui_Main::calculateMass() {
    double assay, density;

    this->ui->massEdit->setStyleSheet( "QLineEdit {}" );

    if ( !this->assay( assay ) || !this->density( density )) {
        this->ui->massEdit->setStyleSheet( "QLineEdit { background-color: #ff9999; }" );
        return;
    }

    if ( this->state() == Template::Liquid ) {
        double volume;

        if ( !this->volume( volume ))
            return;

        this->ui->massEdit->setText( QString( "%1" ).arg( volume * density, 0, 'f', 1, 0 ));
        this->ui->pureEdit->setText( QString( "%1" ).arg( volume * density * assay, 0, 'f', 2, 0 ));
    } else if ( this->state() == Template::Solid ) {
        double mass;

        if ( !this->mass( mass ))
            return;

        this->ui->pureEdit->setText( QString( "%1" ).arg( mass * assay, 0, 'f', 2, 0 ));
    }
}

/*
==========
solidCheck->stateChanged
==========
*/
void Gui_Main::on_solidCheck_stateChanged( int state ) {
    if ( this->locked())
        return;

    switch ( state ) {
    case Qt::Checked:
        this->ui->massEdit->setReadOnly( false );
        this->ui->volumeEdit->setDisabled( true );
        this->ui->densityEdit->setDisabled( true );
        this->setState( Template::Solid );
        //this->ui->massEdit->setFocus();
        break;

    case Qt::Unchecked:
        this->ui->massEdit->setReadOnly( true );
        this->ui->volumeEdit->setEnabled( true );
        this->ui->densityEdit->setEnabled( true );
        this->setState( Template::Liquid );
        this->on_volumeEdit_textChanged();
        //this->ui->volumeEdit->setFocus();
        break;

    default:
        break;
    }
}

/*
==========
massEdit->textChanged
==========
*/
void Gui_Main::on_massEdit_textChanged() {
    double mass, assay, molarMass;

    if ( this->state() != Template::Solid )
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

/*
==========
addTemplate
==========
*/
void Gui_Main::addTemplate( const QString &name ) {
    double amount = 0.0f, assay, density, molarMass;
    bool amountParsed;

    if ( this->state() == Template::Solid )
        amountParsed = this->mass( amount );
    else if ( this->state() == Template::Liquid )
        amountParsed = this->volume( amount );

    if ( !amountParsed || !this->assay( assay ) || !this->density( density ) || !this->molarMass( molarMass )) {
        QMessageBox msgBox;
        msgBox.setIcon( QMessageBox::Critical );
        msgBox.setText( QString( "Invalid amount specified" ));
        msgBox.exec();
        return;
    }

    Template::add( name, amount, density, assay, molarMass, this->state());
    this->fillTemplates( db.templateList.last()->id());
}

/*
==========
setState
==========
*/
void Gui_Main::setState( const Template::State &state ) {
    this->m_state = state;

    if ( state == Template::Solid )
        this->ui->solidCheck->setChecked( true );
    else if ( state == Template::Liquid )
        this->ui->solidCheck->setChecked( false );
}

/*
==========
templateCombo->currentIndexChanged
==========
*/
void Gui_Main::on_templateCombo_currentIndexChanged( int index ) {
    if ( this->locked())
        return;

    if ( this->state() == Template::Solid )
        this->ui->massEdit->setFocus();
    else if ( this->state() == Template::Liquid )
        this->ui->volumeEdit->setFocus();

    this->curentTemplate = Template::fromId( this->ui->templateCombo->itemData( index, Qt::UserRole ).toInt());
    if ( this->curentTemplate != NULL ) {
        if ( this->curentTemplate->state() == Template::Solid )
            this->setMass( this->curentTemplate->amount());
        else if ( this->curentTemplate->state() == Template::Liquid )
            this->setVolume( this->curentTemplate->amount());

        this->setDensity( this->curentTemplate->density());
        this->setAssay( this->curentTemplate->assay() * 100.0f );
        this->setMolarMass( this->curentTemplate->molarMass());
        this->setState( this->curentTemplate->state());
    }
}

/*
==========
keepAboveAction->toggled
==========
*/
void Gui_Main::on_keepAboveAction_toggled( bool toggle ) {
    if ( toggle )
        this->setWindowFlags( this->windowFlags() | Qt::WindowStaysOnTopHint );
    else
        this->setWindowFlags( this->windowFlags() ^= Qt::WindowStaysOnTopHint );

    this->show();
}

/*
==========
densityEdit->textChanged
==========
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

/*
==========
molarEdit->textChanged
==========
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

/*
==========
assayEdit->textChanged
==========
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

/*
==========
removeAction->triggered
==========
*/
void Gui_Main::on_removeAction_triggered() {
    int state;
    QMessageBox msgBox;
    QSqlQuery query;
    Template *templatePtr;

    templatePtr = Template::fromId( this->ui->templateCombo->itemData( this->ui->templateCombo->currentIndex(), Qt::UserRole ).toInt());

    if ( templatePtr != NULL ) {
        msgBox.setText( this->tr( "Do you really want to remove '%1'?" ).arg( templatePtr->name()));
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
        msgBox.setDefaultButton( QMessageBox::Yes );
        msgBox.setIcon( QMessageBox::Warning );
        //msgBox.setWindowIcon( QIcon( ":/icons/template_remove.png" ));
        state = msgBox.exec();

        // check options
        switch ( state ) {
        case QMessageBox::Yes:
            // remove from memory and database
            db.templateList.removeOne( templatePtr );
            query.exec( QString( "delete from templates where id=%1" ).arg( templatePtr->id()));

            // refill list
            this->fillTemplates();
            break;

        case QMessageBox::No:
        default:
            return;
        }
    }
}

/*
==========
saveAction->triggered
==========
*/
void Gui_Main::on_saveAction_triggered() {
    double amount, molarMass, density, assay;

    if ( !this->molarMass( molarMass ) || !this->density( density ) || !this->assay( assay ))
        return;

    if ( this->state() == Template::Solid && !this->mass( amount ))
        return;

    if ( this->state() == Template::Liquid && !this->volume( amount ))
        return;

    this->curentTemplate->setAmount( amount );
    this->curentTemplate->setMolarMass( molarMass );
    this->curentTemplate->setAssay( assay );
    this->curentTemplate->setState( this->state());

    if ( this->state() == Template::Liquid )
        this->curentTemplate->setDensity( density );
}
