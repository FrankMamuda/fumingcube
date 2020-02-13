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
#include "label.h"
#include "labelset.h"
#include "reagent.h"
#include "labeldock.h"
#include "reagentdock.h"
#include "ui_labeldock.h"
#include "labeldialog.h"
#include <QDebug>
#include <QMenu>
#include <QSqlQuery>

/**
 * @brief LabelDock::LabelDock
 * @param parent
 */
LabelDock::LabelDock( QWidget *parent ) : DockWidget( parent ), ui( new Ui::LabelDock ) {
    this->ui->setupUi( this );
    this->ui->labelView->setModel( Label::instance());
    this->ui->labelView->setModelColumn( Label::Name );

    this->ui->allButton->connect( this->ui->allButton, &QPushButton::clicked, [ this ]() { this->ui->labelView->setCurrentIndex( QModelIndex()); } );
    this->ui->labelView->selectionModel()->connect( this->ui->labelView->selectionModel(), &QItemSelectionModel::selectionChanged, [ this ]( const QItemSelection &, const QItemSelection & ) {
        this->setFilter( this->ui->labelView->selectionModel()->selectedRows());
    } );
}

LabelDock::~LabelDock() {
    delete this->ui;
}


/**
 * @brief LabelDock::currentLabel
 * @return
 */
Id LabelDock::currentLabel() const {
    const QModelIndex index( this->ui->labelView->currentIndex());

    if ( !index.isValid())
        return Id::Invalid;

    return Label::instance()->id( Label::instance()->row( static_cast<int>( index.row())));
}

/**
 * @brief LabelDock::setFilter
 * @param list
 */
void LabelDock::setFilter( const QModelIndexList &list ) {
    // apply sql filter
    if ( list.isEmpty()) {
        Reagent::instance()->setFilter( "" );
        qDebug() << "select none";
    } else {
        QList<Id> labelIds;
        foreach ( const QModelIndex &index, list )
            labelIds << Label::instance()->id( Label::instance()->row( static_cast<int>( index.row())));


        QString whereStatement( QString( "select %1.%2 from %1 " ).arg( LabelSet::instance()->tableName()).arg( LabelSet::instance()->fieldName( LabelSet::ReagentId )));
        for ( int y = 0; y < labelIds.count(); y++ )
            whereStatement.append( QString( QString(( y == 0 ) ? "where" : "or" ) + " %1=%2 " ).arg( LabelSet::instance()->fieldName( LabelSet::LabelId )).arg( static_cast<int>( labelIds.at( y ))));

        /*QList<Id> reagentList;
        QSqlQuery query;
        query.exec( QString( "select distinct %1 from %2 where "
                             "%1 in ( %3 ) "
                             "or "
                             "%4 in ( %3 ) " )
                    .arg( Reagent::instance()->fieldName( Reagent::ID ))
                    .arg( Reagent::instance()->tableName())
                    .arg( whereStatement )
                    .arg( Reagent::instance()->fieldName( Reagent::ParentId )));
        while ( query.next()) {
            const Id id = query.value( 0 ).value<Id>();
            reagentList << id;
        }*/

        QString reagentStatement( QString( "select distinct %1 from %2 where "
                                     "%1 in ( %3 ) "
                                     "or "
                                     "%4 in ( %3 ) " )
                            .arg( Reagent::instance()->fieldName( Reagent::ID ))
                            .arg( Reagent::instance()->tableName())
                            .arg( whereStatement )
                            .arg( Reagent::instance()->fieldName( Reagent::ParentId )));

        // qDebug() << "REAGENTS" << reagentList;

        Reagent::instance()->setFilter( QString( "%1.%2 in ( %3 )" ).arg( Reagent::instance()->tableName()).arg( Reagent::instance()->fieldName( Reagent::ID )).arg( reagentStatement ));
    }
    Reagent::instance()->select();
    ReagentDock::instance()->view()->updateView();
}

/**
 * @brief LabelDock::on_labelView_customContextMenuRequested
 * @param pos
 */
void LabelDock::on_labelView_customContextMenuRequested( const QPoint &pos ) {
    if ( this->ui->labelView->selectionModel()->selectedRows().count() > 1 )
        return;

    QMenu menu;
    menu.addAction( QIcon::fromTheme( "add" ), this->tr( "Add new label" ), [ this ]() {
        LabelDialog ld( this );
        if ( ld.exec() == QDialog::Accepted ) {
            Label::instance()->add( ld.name(), ld.colour());
            ReagentDock::instance()->view()->updateView();
        }
    } );
    menu.addAction( QIcon::fromTheme( "remove" ), this->tr( "Remove label" ), [ this ]() {
        if ( !this->ui->labelView->currentIndex().isValid())
            return;

        const Row row = Label::instance()->row( this->ui->labelView->currentIndex());
        if ( row != Row::Invalid )
            Label::instance()->remove( row );

        // remove orphans
        LabelSet::instance()->removeOrphanedEntries();
    } );
    menu.addAction( QIcon::fromTheme( "edit" ), this->tr( "Edit label" ), [ this ]() {
        if ( !this->ui->labelView->currentIndex().isValid())
            return;

        const Row row = Label::instance()->row( this->ui->labelView->currentIndex());
        if ( row != Row::Invalid ) {
            LabelDialog ld( this );
            ld.setName( Label::instance()->name( row ));
            ld.setColour( Label::instance()->colour( row ));

            if ( ld.exec() == QDialog::Accepted ) {
                Label::instance()->setName( row, ld.name());
                Label::instance()->setColour( row, ld.colour());
                ReagentDock::instance()->view()->updateView();
            }
        }
    } );

    menu.exec( this->mapToGlobal( pos ));
}
