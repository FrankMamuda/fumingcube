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
#include "tag.h"
#include "tagselectiondialog.h"
#include "ui_tagselectiondialog.h"
#include "listutils.h"
#include "variable.h"

/**
 * @brief TagSelectionDialog::TagSelectionDialog
 * @param parent
 */
TagSelectionDialog::TagSelectionDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::TagSelectionDialog ) {
    // setup ui
    this->ui->setupUi( this );
    this->ui->tagDialogContents->setWindowFlags( Qt::Widget );

    // get selected tags from a variable
    this->selectedTags = ListUtils::toNumericList<Id>( Variable::value<QStringList>( "propertyFragment/selectedTags" ));

    // if none are selected, select all
    bool selectAll = this->selectedTags.isEmpty();

    // go through tags
    for ( int y = 0; y < Tag::instance()->count(); y++ ) {
        const auto row = static_cast<Row>( y );

        // ignore tags without extraction script
        const QString script( Tag::instance()->script( row ).toString());
        const Tag::Types type( Tag::instance()->type( row ));
        if ( script.isEmpty() && ( type != Tag::PubChemId && type != Tag::Formula ))
            continue;

        // get tag id for storage
        const Id id( Tag::instance()->id( row ));

        // make list widget item (tag name checkbox)
        QListWidgetItem *item( new QListWidgetItem());
        QCheckBox *box( new QCheckBox( Tag::instance()->name( row )));
        box->setCheckable( true );
        box->setChecked( selectAll || this->selectedTags.contains( id ));

        // add item to list
        this->ui->tagList->insertItem( y, item );
        this->ui->tagList->setItemWidget( item, box );

        // store box/id combos in a map
        this->map[id] = box;
    }
}

/**
 * @brief TagSelectionDialog::~TagSelectionDialog
 */
TagSelectionDialog::~TagSelectionDialog() {
    // reset tags
    this->selectedTags.clear();

    // go through widget items and construct a new list value for the variable
    const QList<Id> keys( this->map.keys());
    for ( const Id &id : keys ) {
        QCheckBox *box( this->map[id] );
        if ( !box->isChecked())
            continue;

        this->selectedTags << id;
    }

    // store list
    Variable::setValue( "propertyFragment/selectedTags", ListUtils::toStringList<Id>( qAsConst( this->selectedTags )));

    // get rid of ui
    delete this->ui;
}

/**
 * @brief TagSelectionDialog::on_actionSelectAll_triggered
 */
void TagSelectionDialog::on_actionSelectAll_triggered() {
    const QList<QCheckBox*> boxes( this->map.values());
    for ( QCheckBox *box : boxes )
        box->setChecked( true );
}

/**
 * @brief TagSelectionDialog::on_actionDeselectAll_triggered
 */
void TagSelectionDialog::on_actionDeselectAll_triggered() {
    const QList<QCheckBox*> boxes( this->map.values());
    for ( QCheckBox *box : boxes )
        box->setChecked( false );
}
