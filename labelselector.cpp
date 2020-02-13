/*
 * Copyright (C) 2020 Armands Aleksejevs
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
#include "labelselector.h"
#include "ui_labelselector.h"
#include "label.h"
#include "labelset.h"
#include "reagent.h"

/**
 * @brief LabelSelector::LabelSelector
 * @param parent
 */
LabelSelector::LabelSelector( QWidget *parent , const QList<Id> &selected ) : QDialog( parent ), labelIds( selected ), ui( new Ui::LabelSelector ) {
    this->ui->setupUi( this) ;
    this->setWindowFlag( Qt::Tool );

    NoCloseMenu *labels = new NoCloseMenu();
    for ( int y = 0; y < Label::instance()->count(); y++ ) {
        const Row row = static_cast<Row>( y );
        const Id id = Label::instance()->id( row );
        QAction *action( labels->addAction( QIcon( Label::instance()->pixmap( Label::instance()->colour( row ))), Label::instance()->name( row )));
        action->setCheckable( true );

        if ( this->labelIds.contains( id ))
            action->setChecked( true );

        action->connect( action, &QAction::toggled, [ this, id ]( bool toggle ) {
            if ( toggle )
                this->labelIds << id;
            else
                this->labelIds.removeAll( id );
        } );
    }

    this->ui->labelLayout->addWidget( labels );
    this->adjustSize();
    this->ui->buttonBox->setStandardButtons( QDialogButtonBox::Ok );
}

/**
 * @brief LabelSelector::~LabelSelector
 */
LabelSelector::~LabelSelector() {
    delete this->ui;
}

/**
 * @brief NoCloseMenu::NoCloseMenu
 * @param parent
 */
NoCloseMenu::NoCloseMenu( QWidget *parent ) : QMenu( parent ) {
    this->setStyleSheet( "QMenu { border: 0px; }" );
    this->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
    this->setWindowFlags( Qt::Widget );
}
