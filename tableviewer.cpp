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
#include "tableviewer.h"
#include "ui_tableviewer.h"
#include "tableproperty.h"
#include "reagent.h"
#include "property.h"
#include "htmlutils.h"
#include "tag.h"
#include "ghswidget.h"
#include "nfpawidget.h"
#include "reagentdelegate.h"
#include <QSqlQuery>
#include <QDebug>
#include <QScrollBar>
#include <QScreen>
#include <QLabel>
#include <QTimer>

/**
 * @brief TableViewer::TableViewer
 * @param parent
 */
TableViewer::TableViewer( QWidget *parent, const Id &tableId ) : QDialog( parent ), ui( new Ui::TableViewer ) {
    this->ui->setupUi( this );

    // abort if tableId is invalid
    if ( tableId == Id::Invalid )
        return;

    //
    // step one: get selected tags
    //
    QMap<Id, bool> settings;
    QList<Id> tagIds;
    Id tabId = Id::Invalid;

    // find all TableProperty entries matching tableId
    QSqlQuery query;
    query.exec( QString( "select %1, %2 from %3 where %4=%5 order by %6" )
                .arg( TableProperty::instance()->fieldName( TableProperty::TagId ),
                      TableProperty::instance()->fieldName( TableProperty::Tab ),
                      TableProperty::instance()->tableName(),
                      TableProperty::instance()->fieldName( TableProperty::TableId ),
                      QString::number( static_cast<int>( tableId )),
                      TableProperty::instance()->fieldName( TableProperty::TableOrder )
                      ));

    // store them in an unordered map (for now)
    while ( query.next()) {
        const Id id = query.value( 0 ).value<Id>();
        const bool tabbed = query.value( 1 ).toBool();

        settings[id] = tabbed;
        if ( !tabbed )
            tagIds << id;
        else
            tabId = id;
    }

    //
    // TODO: add switch to require certain columns for an item to be displayed
    //       also a flag to display:
    //           a) reagents (double click for batches?);
    //           b) reagents and batches
    //           c) batches only
    //
    // ONLY reagents for now

    //
    // now we should get unique entries of tabId for example:
    // tagName "Location", unique entries "C1", "C2", "C3" + "Unsorted" (with a null value)
    //
    // tabs will be displayed in a QTabBar
    // on tab change, select statement changes and table data is refreshed
    if ( tabId == Id::Invalid ) {
        this->ui->tabBar->hide();
    } else {
        this->ui->tabBar->setShape( QTabBar::RoundedSouth );

        QSqlQuery query( "..." );
        // TODO: implement this


        // 1) get count and name from sql
        // 2) append new tabs
        // 3) append 'unsorted' tab
        // if total count is zero, hide tabBar and do no filtering at all
    }

    //
    // step two: run an sql query that returns [reagentId] [propertyId_0] ... [propertyId_N]
    //
    QString statement( QString( "select %1.%2, " ).arg( Reagent::instance()->tableName(), Reagent::instance()->fieldName( Reagent::ID )));
    int c = 0;
    for ( c = 0; c < tagIds.count(); c++ )
        statement.append( QString( " ifnull( t%1.id, -1 ) as c%1" ).arg( QString::number( c )) + QString(( c < tagIds.count() - 1 ? "," : "" )));

    statement.append( " from " + Reagent::instance()->tableName());

    for ( c = 0; c < tagIds.count(); c++ ) {
        statement.append( QString( " left join "
                                   "( select %2, %3.%4 from %3 where %5=%8 ) "
                                   "as t%1 "
                                   "on t%1.%2=%6.%7" )
                          .arg( QString::number( c ), // 1
                                Property::instance()->fieldName( Property::ReagentId ), // 2
                                Property::instance()->tableName(), // 3
                                Property::instance()->fieldName( Property::ID ), // 4
                                Property::instance()->fieldName( Property::TagId ), // 5
                                Reagent::instance()->tableName(), // 6
                                Reagent::instance()->fieldName( Reagent::ID ), // 7
                                QString::number( static_cast<int>( tagIds.at( c ))) // 8
                                )
                          );
    }
    // TODO: add tab statement

    statement.append( " where " );
    for ( c = 0; c < tagIds.count(); c++ )
        statement.append( QString( "c%1" ).arg( QString::number( c )) + QString(( c < tagIds.count() - 1 ? "+" : "" )));
    statement.append( QString( "!=-%1" ).arg( QString::number( c )));

    //
    // step two: setup delegate early
    //

    // the rest of data is handled through the property delegate via special viewMode
    PropertyDelegate *delegate( new PropertyDelegate( this->ui->tableView ));
    delegate->setViewMode();

    // set property delegate to columns with properties (1+)
    this->ui->tableView->setSizeAdjustPolicy( QTableView::AdjustToContents );
    for ( c = 0; c < tagIds.count(); c++ )
        this->ui->tableView->setItemDelegateForColumn( c + 1, delegate );

    //
    // step four: visualize data, using QSqlQueryModel and QTreeView
    //
    this->model->setQuery( QSqlQuery( statement ));
    this->ui->tableView->setModel( this->model );

    // setup column names
    this->model->setHeaderData( 0, Qt::Horizontal, TableViewer::tr( "Reagent" ));
    for ( c = 0; c < tagIds.count(); c++ )
        this->model->setHeaderData( c + 1, Qt::Horizontal, QApplication::translate( "Tag", Tag::instance()->name( tagIds.at( c )).toUtf8().constData()));

    // setup index widgets (NFPA, GHS)
    // NOTE: similar to PropertyDock::setSpecialWidgets
    for ( int r = 0; r < this->model->rowCount(); r++ ) {
        for ( int c = 0; c < this->model->columnCount(); c++ ) {
            const QModelIndex index( this->model->index( r, c ));

            // handle first column (reagent name)
            if ( c == 0 ) {
                const Id reagentId = static_cast<Id>( index.data( Qt::DisplayRole ).value<Id>());
                if ( reagentId == Id::Invalid )
                    continue;

                // display name as a simple QLabel with html data
                QLabel *label( new QLabel( Reagent::instance()->name( reagentId )));
                label->setAutoFillBackground( true );
                label->setAttribute( Qt::WA_DeleteOnClose, true );
                this->ui->tableView->setIndexWidget( index, label );
                continue;
            }

            // handle the rest of columns, setting index widgets if needed
            const Id propertyId = static_cast<Id>( index.data( Qt::DisplayRole ).value<Id>());
            if ( propertyId == Id::Invalid )
                continue;
            const Id tagId = Property::instance()->tagId( propertyId );

            bool pixmap = false;
            if ( tagId != Id::Invalid )
                pixmap = Tag::instance()->type( tagId ) == Tag::Formula || tagId == PixmapTag;

            if ( tagId == Id::Invalid || pixmap )
                continue;

            const Tag::Types type = Tag::instance()->type( tagId );
            if ( type == Tag::NFPA || type == Tag::GHS ) {
                const QStringList parms( Property::instance()->propertyData( propertyId ).toString().split( " " ));
                QWidget *widget( this->ui->tableView->indexWidget( index ));

                if ( type == Tag::NFPA ) {
                    NFPAWidget *nfpa( new NFPAWidget( nullptr, parms ));
                    widget = nfpa;
                } else {
                    GHSWidget *ghs( new GHSWidget( nullptr, parms ));
                    widget = ghs;
                }

                // there aren't supposed to be updates (disimilar to PropertyDock::setSpecialWidgets)
                // therefore we can set the widget and forget about it
                //widget->setAttribute( Qt::WA_DeleteOnClose, true );

                // FIXME: dirty hack to fix alignment
                QWidget *container = new QWidget();
                container->setAttribute( Qt::WA_DeleteOnClose, true );
                QGridLayout *grid( new QGridLayout());
                grid->setSpacing(0);
                grid->setMargin(0);
                grid->addItem( new QSpacerItem( 1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding ), 0, 0, 1, 3 );
                grid->addItem( new QSpacerItem( 1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed ), 1, 0 );
                grid->addWidget( widget, 1, 1 );
                grid->addItem( new QSpacerItem( 1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed ), 1, 2 );
                grid->addItem( new QSpacerItem( 1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding ), 2, 0, 1, 3 );
                container->setLayout( grid );

                this->ui->tableView->setIndexWidget( index, container );
            }
        }
    }

    // resize contents
    this->ui->tableView->resizeColumnsToContents();
    this->ui->tableView->resizeRowsToContents();
}

/**
 * @brief TableViewer::~TableViewer
 */
TableViewer::~TableViewer() {
    delete this->model;
    delete this->ui;
}

/**
 * @brief TableViewer::showEvent
 * @param event
 */
void TableViewer::showEvent( QShowEvent *event ) {
    QDialog::showEvent( event );

    // FIXME: magic number
    int width = 48;
    for( int c = 0; c < this->model->columnCount(); c++ )
        width += this->ui->tableView->columnWidth( c );
    width += this->ui->tableView->verticalScrollBar()->width();

    this->resize( width, QApplication::primaryScreen()->availableSize().height() * 3 / 5 );
    this->move( QApplication::primaryScreen()->geometry().center().x() - this->width() / 2, QApplication::primaryScreen()->geometry().center().y() - this->height() / 2 );
}
