/*
 * Copyright (C) 2019 Armands Aleksejevs
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
#include "nfpabuilder.h"
#include "ui_nfpabuilder.h"

/**
 * @brief NFPABuilder::NFPABuilder
 * @param parent
 */
NFPABuilder::NFPABuilder( QWidget *parent, const QStringList &parameters ) : QDialog( parent ), ui( new Ui::NFPABuilder ) {
    this->ui->setupUi( this );

    qDebug() << parameters;

    if ( parameters.count() >= 3 ) {
        this->ui->nfpaWidget->update( parameters );
        this->ui->healthSlider->setValue( parameters.at( 0 ).toInt());
        this->ui->flameSlider->setValue( parameters.at( 1 ).toInt());
        this->ui->reactSlider->setValue( parameters.at( 2 ).toInt());

        if ( parameters.count() == 4 ) {
            const QString hazard( parameters.at( 3 ));
            if ( hazard.isEmpty())
                this->ui->hazardCombo->setCurrentIndex( 0 );
            else if ( !QString::compare( hazard, "OX" ))
                this->ui->hazardCombo->setCurrentIndex( 1 );
            else if ( !QString::compare( hazard, "W" ))
                this->ui->hazardCombo->setCurrentIndex( 2 );
            else if ( !QString::compare( hazard, "SA" ))
                this->ui->hazardCombo->setCurrentIndex( 3 );
            else {
                this->ui->hazardCombo->setCurrentIndex( 4 );
                this->ui->customHazard->setText( hazard );
                this->ui->customHazard->setEnabled( true );
            }
        }
    } else {
        this->updateNFPA();
    }

    this->ui->flameSlider->connect( this->ui->flameSlider, SIGNAL( valueChanged( int )), this, SLOT( updateNFPA()));
    this->ui->healthSlider->connect( this->ui->healthSlider, SIGNAL( valueChanged( int )), this, SLOT( updateNFPA()));
    this->ui->reactSlider->connect( this->ui->reactSlider, SIGNAL( valueChanged( int )), this, SLOT( updateNFPA()));
    this->ui->hazardCombo->connect( this->ui->hazardCombo, SIGNAL( currentIndexChanged( int )), this, SLOT( updateNFPA()));
    this->ui->customHazard->connect( this->ui->customHazard, SIGNAL( textChanged( QString )), this, SLOT( updateNFPA()));

}

/**
 * @brief NFPABuilder::~NFPABuilder
 */
NFPABuilder::~NFPABuilder() {
    delete this->ui;
}

/**
 * @brief NFPABuilder::parameters
 * @return
 */
QStringList NFPABuilder::parameters() const {
    return this->ui->nfpaWidget->parameters();
}

/**
 * @brief NFPABuilder::updateNFPA
 */
void NFPABuilder::updateNFPA() {
    const QRegularExpression reProp( "(\\d)\\s(\\d)\\s(\\d)(?:.+?(?=(OX|W|SA)))?" );

    if ( this->ui->hazardCombo->currentIndex() != 4 )
        this->ui->customHazard->setEnabled( false );

    QString hazard;
    switch ( this->ui->hazardCombo->currentIndex()) {
    case 1:
        hazard = "OX";
        break;

    case 2:
        hazard = "W";
        break;

    case 3:
        hazard = "SA";
        break;

    case 4:
        if ( !this->ui->customHazard->isEnabled())
             this->ui->customHazard->setEnabled( true );

        hazard = this->ui->customHazard->text();
        break;

    default:
        break;
    }

    this->ui->nfpaWidget->update( QString( "%1 %2 %3 %4" )
                                  .arg( this->ui->healthSlider->value())
                                  .arg( this->ui->flameSlider->value())
                                  .arg( this->ui->reactSlider->value())
                                  .arg( qAsConst( hazard )).split( " " )
                                  );
}
