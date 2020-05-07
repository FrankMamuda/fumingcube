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
#include "propertydock.h"
#include "listutils.h"
#include <QDebug>
#include <QMenu>
#include <QPainter>
#include <QSqlQuery>

/**
 * @brief LabelDock::LabelDock
 * @param parent
 */
LabelDock::LabelDock( QWidget *parent ) : DockWidget( parent ), ui( new Ui::LabelDock ) {
    this->ui->setupUi( this );
    this->ui->labelView->setModel( Label::instance());
    this->ui->labelView->setModelColumn( Label::Name );

    // TODO: deleteme
    this->ui->labelView->setItemDelegate( new LabelDelegate());

    QPushButton::connect( this->ui->allButton, &QPushButton::clicked, [ this ]() {
        this->ui->labelView->selectionModel()->blockSignals( true );
        this->ui->labelView->reset();
        this->ui->labelView->selectionModel()->blockSignals( false );

        LabelDock::setFilter( QModelIndexList());
    } );
    QItemSelectionModel::connect( this->ui->labelView->selectionModel(),
                                  &QItemSelectionModel::selectionChanged,
                                  [ this ]( const QItemSelection &, const QItemSelection & ) {
                                      LabelDock::setFilter( this->ui->labelView->selectionModel()->selectedRows());
                                  } );

    Variable::instance()->bind( "labelDock/selectedRows", this, SLOT( changed()));
}

/**
 * @brief LabelDock::~LabelDock
 */
LabelDock::~LabelDock() {
    delete this->ui;
}

/**
 * @brief LabelDock::currentLabel
 * @return
 */
Id LabelDock::currentLabel() const {
    const QList<Row> list( ListUtils::toNumericList<Row>( Variable::string( "labelDock/selectedRows" ).split( ";" )));
    if ( list.count() != 1 )
        return Id::Invalid;

    return Label::instance()->id( list.first());
}

/**
 * @brief LabelDock::setFilter
 * @param list
 */
void LabelDock::setFilter( const QModelIndexList &list ) {
    // apply sql filter
    if ( list.isEmpty()) {
        // don't set filter twice
        if ( Reagent::instance()->filter().isEmpty())
            return;

        Reagent::instance()->setFilter( "" );
        Variable::setString( "labelDock/selectedRows", "" );
    } else {
        QList<Id> labelIds;
        QStringList rows;

        for ( const QModelIndex &index : list ) {
            labelIds << Label::instance()->id( Label::instance()->row( static_cast<int>( index.row())));
            rows << QString::number( index.row());
        }

        QString whereStatement( QString( "select %1.%2 from %1 " )
                                .arg( LabelSet::instance()->tableName(),
                                      LabelSet::instance()->fieldName( LabelSet::ReagentId )));
        for ( int y = 0; y < labelIds.count(); y++ )
            whereStatement.append( QString( QString(( y == 0 ) ? "where" : "or" ) + " %1=%2 " )
                                   .arg( LabelSet::instance()->fieldName( LabelSet::LabelId ),
                                         QString::number( static_cast<int>( labelIds.at( y )))));

        QString reagentStatement( QString( "select distinct %1 from %2 where "
                                           "%1 in ( %3 ) "
                                           "or "
                                           "%4 in ( %3 ) " )
                                          .arg( Reagent::instance()->fieldName( Reagent::ID ),
                                                Reagent::instance()->tableName(),
                                                whereStatement,
                                                Reagent::instance()->fieldName( Reagent::ParentId )));

        const QString filter( QString( "%1.%2 in ( %3 )" )
                              .arg( Reagent::instance()->tableName(),
                                    Reagent::instance()->fieldName( Reagent::ID ),
                                    reagentStatement ));

        // don't set filter twice
        if ( !QString::compare( Reagent::instance()->filter(), filter ))
            return;

        Reagent::instance()->setFilter( filter );

        // store variable
        Variable::setString( "labelDock/selectedRows", rows.join( ";" ));

    }
    Reagent::instance()->select();
    ReagentDock::instance()->view()->updateView();
    PropertyDock::instance()->updateView();
}

/**
 * @brief LabelDock::showEvent
 * @param event
 */
void LabelDock::showEvent( QShowEvent *event ) {
    DockWidget::showEvent( event );

    // restore filter
    // TODO: why not store Label actual filter as a string
    //       since I cannot figure out how to select rows
    const QList<int> rows( ListUtils::toNumericList<int>( Variable::string( "labelDock/selectedRows" ).split( ";" )));
    if ( rows.count() == 1 ) {
        if ( rows.first() == -1 ) {
            this->on_noButton_clicked();
            return;
        }
    }

    if ( !rows.isEmpty()) {
        QModelIndexList list;

        for ( const int row : rows ) {
            if ( row == -1 )
                continue;

            list << Label::instance()->index( row, 0 );
        }

        this->setFilter( list );
    }
}

/**
 * @brief LabelDock::on_labelView_customContextMenuRequested
 * @param pos
 */
void LabelDock::on_labelView_customContextMenuRequested( const QPoint &pos ) {
    if ( this->ui->labelView->selectionModel()->selectedRows().count() > 1 )
        return;

    QMenu menu;
    menu.addAction( QIcon::fromTheme( "add" ), LabelDock::tr( "Add new label" ), [ this ]() {
        LabelDialog ld( this );
        if ( ld.exec() == QDialog::Accepted ) {
            Label::instance()->add( ld.name(), ld.colour());
            ReagentDock::instance()->view()->updateView();
        }
    } );
    menu.addAction( QIcon::fromTheme( "remove" ), LabelDock::tr( "Remove label" ), [ this ]() {
        if ( !this->ui->labelView->currentIndex().isValid())
            return;

        const Row row = Label::instance()->row( this->ui->labelView->currentIndex());
        if ( row != Row::Invalid )
            Label::instance()->remove( row );

        // remove orphans
        LabelSet::instance()->removeOrphanedEntries();
    } );
    menu.addAction( QIcon::fromTheme( "edit" ), LabelDock::tr( "Edit label" ), [ this ]() {
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

/**
 * @brief LabelDock::on_noButton_clicked
 */
void LabelDock::on_noButton_clicked() {
    Reagent::instance()->setFilter( QString( "%1 not in ( select %2 from %3 )" )
                                    .arg( Reagent::instance()->fieldName( Reagent::ID ))
                                    .arg( LabelSet::instance()->fieldName( LabelSet::ReagentId ))
                                    .arg( LabelSet::instance()->tableName())
                                    );

    this->ui->labelView->selectionModel()->blockSignals( true );
    this->ui->labelView->reset();
    this->ui->labelView->selectionModel()->blockSignals( false );

    Reagent::instance()->select();
    ReagentDock::instance()->view()->updateView();
    PropertyDock::instance()->updateView();
    Variable::setString( "labelDock/selectedRows", "-1" );
}

/**
 * @brief LabelDock::changed
 */
void LabelDock::changed() {
    this->ui->labelView->repaint();
}

/**
 * @brief LabelDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void LabelDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    // NOTE: basically we don't want to see an extra selection highlight here
    //       currently active labels are stored in a variable and selection is
    //       applied via Label::data Qt::BackgroundRole

    // remove selection
    QStyleOptionViewItem optionNoSelection( option );
    if ( option.state & QStyle::State_Selected )
        optionNoSelection.state &= ~QStyle::State_Selected;

    // paint as usual
    QStyledItemDelegate::paint( painter, qAsConst( optionNoSelection ), index );
}
