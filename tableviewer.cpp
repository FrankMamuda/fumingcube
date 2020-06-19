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
#include "tableentry.h"
#include <QSqlQuery>
#include <QDebug>
#include <QScrollBar>
#include <QScreen>
#include <QLabel>
#include <QTimer>

//
// TODO: add an option to export data
// TODO: add an option to dynamically sort data
// FIXME: remove empty categories
//

/**
 * @brief TableViewer::TableViewer
 * @param parent
 */
TableViewer::TableViewer( QWidget *parent, const Id &tableId ) : QDialog( parent ), ui( new Ui::TableViewer ), tableId( tableId ) {
    this->ui->setupUi( this );

    // abort if tableId is invalid
    if ( tableId == Id::Invalid )
        return;

    //
    // step one: get selected tags
    //
    QList<Id> tagIds;
    Id tabId = Id::Invalid;

    // name column uses ReagentDelegate
    this->reagentDelegate = new ReagentDelegate();
    this->reagentDelegate->setViewMode();
    this->ui->tableView->setItemDelegateForColumn( 0, this->reagentDelegate );

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

    // store selected tagIds them in a list
    // also store the selected tabId for use in a horizonatal tab filter
    while ( query.next()) {
        const Id id = query.value( 0 ).value<Id>();
        const bool tabbed = query.value( 1 ).toBool();

        if ( !tabbed )
            tagIds << id;
        else
            tabId = id;
    }

    // retrieve corresponding unique properties where tagId=tabId
    // for example:
    //     tagName: "Location"
    //     unique entries: "C1", "C2", "C3" + "Unsorted" (when values are missing)
    //
    // unique entries (or categories) are displayed as tabs in the QTabBar
    // on tab change, select statement changes and table data is refreshed

    // property data is handled through the property delegate via special viewMode
    // NOTE: code is reused in PropertyDock and here
    this->propertyDelegate = new PropertyDelegate( this->ui->tableView );
    this->propertyDelegate->setViewMode();

    if ( tabId == Id::Invalid ) {
        // if we do not have a tabId set, there is no need for filtering
        // just populate the table
        this->ui->tabBar->hide();
        this->populateTable( tagIds, this->filter());
    } else {
        // setup tabBar
        this->ui->tabBar->setShape( QTabBar::RoundedSouth );

        // get unique categories to use as tabs
        QStringList categories;
        QSqlQuery query( QString( "select %1, count( %1 ) from %2 where %3=%4 group by %1 order by %1" ).arg(
                             Property::instance()->fieldName( Property::PropertyData ), // 1
                             Property::instance()->tableName(), // 2
                             Property::instance()->fieldName( Property::TagId ), // 3
                             QString::number( static_cast<int>( tabId )) // 4
                             ));

        // store them in a list
        while ( query.next())
            categories.append( query.value( 0 ).toString());

        if ( categories.count() > 1 ) {
            // add corresponding tabs
            for ( const QString &category : qAsConst( categories ))
                this->ui->tabBar->addTab( category );

            // append 'unsorted' tab (for entries withou values)
            this->ui->tabBar->addTab( TableViewer::tr( "Unsorted" ));

            // connect tabBar for updates (click on a tab triggers setFilter which in turn repopulates the table)
            QTabBar::connect( this->ui->tabBar, &QTabBar::currentChanged, this, [ this, tabId, tagIds ] { this->setFilter( tabId, tagIds ); } );

            // populate with the first category
            this->setFilter( tabId, tagIds );
        } else {
            // a malformed filter may return zero categories
            // in this case populate the table without filtering it
            this->ui->tabBar->hide();
            this->populateTable( tagIds, this->filter());
        }
    }

    // set property delegate to columns with properties (1+)
    this->ui->tableView->setSizeAdjustPolicy( QTableView::AdjustToContents );
    for ( int c = 0; c < tagIds.count(); c++ )
        this->ui->tableView->setItemDelegateForColumn( c + 1, this->propertyDelegate );
}


/**
 * @brief TableViewer::~TableViewer
 */
TableViewer::~TableViewer() {
    delete this->reagentDelegate;
    delete this->propertyDelegate;
    delete this->model;
    //delete this->filterModel;
    delete this->ui;
}

/**
 * @brief TableViewer::showEvent
 * @param event
 */
void TableViewer::showEvent( QShowEvent *event ) {
    QDialog::showEvent( event );

    // get optimal dialog width
    // FIXME: magic number
    int width = 48;
    for( int c = 0; c < this->model->columnCount(); c++ )
        width += this->ui->tableView->columnWidth( c );
    width += this->ui->tableView->verticalScrollBar()->width();

    // get optimal dialog height
    // FIXME: magic number
    int height = 0;
    for( int r = 0; r < this->model->rowCount(); r++ )
        height += this->ui->tableView->rowHeight( r );

    width = qMax( width, 256 );
    height = qMax( height, 256 );

    // adjust dialog size
    const QSize size( QApplication::primaryScreen()->availableSize());
    this->resize( qMin( width, size.width() * 3 / 5 ), qMin( height, size.height() * 3 / 5 ));

    // reposition dialog in the middle of the screen
    this->move( QApplication::primaryScreen()->geometry().center().x() - this->width() / 2, QApplication::primaryScreen()->geometry().center().y() - this->height() / 2 );

    // resize contents
    this->ui->tableView->resizeColumnsToContents();
    this->ui->tableView->resizeRowsToContents();
}

/**
 * @brief TableViewer::populateTable
 */
void TableViewer::populateTable( const QList<Id> tagIds, const QString &filter ) {
    this->reagentDelegate->clearCache();
    this->propertyDelegate->clearCache();

    //
    // step one: run an sql query that returns [reagentId] [propertyId_0] ... [propertyId_N]
    //
    QString statement( QString( "with base_table as ( select %1.%2 as rid, " ).arg( Reagent::instance()->tableName(), Reagent::instance()->fieldName( Reagent::ID )));
    int c = 0;
    for ( c = 0; c < tagIds.count(); c++ )
        statement.append( QString( " ifnull( t%1.id, -1 ) as c%1" ).arg( QString::number( c )) + QString(( c < tagIds.count() - 1 ? "," : "" )));

    statement.append( QString( ", %1.%2 as pid " ).arg( Reagent::instance()->tableName(), Reagent::instance()->fieldName( Reagent::ParentId )));
    statement.append( " from " + Reagent::instance()->tableName());

    for ( c = 0; c < tagIds.count(); c++ ) {
        statement.append( QString( " left join "
                                   "( select %2, %3.%4 from %3 where %5=%6 ) "
                                   "as t%1 "
                                   "on t%1.%2=rid " )
                          .arg( QString::number( c ), // 1
                                Property::instance()->fieldName( Property::ReagentId ), // 2
                                Property::instance()->tableName(), // 3
                                Property::instance()->fieldName( Property::ID ), // 4
                                Property::instance()->fieldName( Property::TagId ), // 5
                                QString::number( static_cast<int>( tagIds.at( c ))) // 6
                                )
                          );
    }

    statement.append( " where " + filter );
    for ( c = 0; c < tagIds.count(); c++ )
        statement.append( QString( "c%1" ).arg( QString::number( c )) + QString(( c < tagIds.count() - 1 ? "+" : "" )));
    statement.append( QString( "!=-%1" ).arg( QString::number( c )));

    const TableEntry::Modes mode = TableEntry::instance()->mode( this->tableId );
    if ( mode == TableEntry::Reagents )
        statement.append( QString( " and %1.%2=-1 " ).arg( Reagent::instance()->tableName(), Reagent::instance()->fieldName( Reagent::ParentId )));

    const QString orderClause( QString( " order by case when %1.%3=-1 then %1.%4 else %1.%3 end, %1.%4, %1.%2" )
                               .arg( Reagent::instance()->tableName(), // 1
                                     Reagent::instance()->fieldName( Reagent::Name ), // 2 NOTE: html names are not sorted correctly
                                     Reagent::instance()->fieldName( Reagent::ParentId ), // 3
                                     Reagent::instance()->fieldName( Reagent::ID ) // 4
                               ));

    statement.append( orderClause );

    QStringList tags;
    QStringList minusOnes;
    for ( c = 0; c < tagIds.count(); c++ ) {
        tags <<  QString( "c%1" ).arg( QString::number( c ));
        minusOnes << "-1";
    }

    statement.append( QString( " ) select rid, %1 from base_table " ).arg( tags.join( ", " )));
    statement.append( QString( "union select %2.%3 as rid_, %1 from %2 where " ).arg( minusOnes.join( ", " ), Reagent::instance()->tableName(), Reagent::instance()->fieldName( Reagent::ID )));
    statement.append( "rid_ in ( select pid from base_table ) and rid_ not in ( select rid from base_table )" );

    // DEBUG:
    // qDebug() << statement;

    // select name, id, parentId from reagent order by name, case when parentId=-1 then id else parentId end

    //
    // step two: visualize data, using QSqlQueryModel and QTableView
    //
    this->model->setQuery( QSqlQuery( statement ));
    //this->filterModel->setSourceModel( this->model );
    //this->ui->tableView->setModel( this->filterModel );
    this->ui->tableView->setModel( this->model );
    this->ui->tableView->setSizeAdjustPolicy( QTableView::AdjustToContents );

    // setup column names
    this->model->setHeaderData( 0, Qt::Horizontal, TableViewer::tr( "Reagent" ));
    for ( c = 0; c < tagIds.count(); c++ )
        this->model->setHeaderData( c + 1, Qt::Horizontal, QApplication::translate( "Tag", Tag::instance()->name( tagIds.at( c )).toUtf8().constData()));

    // setup index widgets (NFPA, GHS)
    // NOTE: similar to PropertyDock::
    for ( int r = 0; r < this->model->rowCount(); r++ ) {
        for ( int c = 1; c < this->model->columnCount(); c++ ) {
            const QModelIndex index( this->model->index( r, c ));

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

    // resize to contents
    this->ui->tableView->resizeColumnsToContents();
    this->ui->tableView->resizeRowsToContents();
}

/**
 * @brief TableViewer::setFilter
 */
void TableViewer::setFilter( const Id tabId, const QList<Id> tagList ) {
    // clear filter just in case
    this->m_filter.clear();

    // no tabs - no filter
    if ( this->ui->tabBar->currentIndex() < 0 )
        return;

    // get tab name which is also the category filter
    const QString text( this->ui->tabBar->currentIndex() == this->ui->tabBar->count() - 1 ? "" : this->ui->tabBar->tabText( this->ui->tabBar->currentIndex()));

    // [filter0] [filter1] .. [filterN] [Unsorted]
    // if text is empty - it indicates that we have selected the last or 'Unsorted' tab which uses a simplified query
    if ( text.isEmpty())
        this->m_filter = QString( " %1.%2 not in ( select %3 from %4 where %5=%6 ) and " ).arg(
                    Reagent::instance()->tableName(), // 1
                    Reagent::instance()->fieldName( Reagent::ID ), // 2
                    Property::instance()->fieldName( Property::ReagentId ), // 3
                    Property::instance()->tableName(), // 4
                    Property::instance()->fieldName( Property::TagId ), // 5
                    QString::number( static_cast<int>( tabId ))
                    );
    else
        this->m_filter = QString( " %1.%2 in ( select %3 from %4 where %5=%6 and cast( %7 as text )='%8' ) and " ).arg(
                    Reagent::instance()->tableName(), // 1
                    Reagent::instance()->fieldName( Reagent::ID ), // 2
                    Property::instance()->fieldName( Property::ReagentId ), // 3
                    Property::instance()->tableName(), // 4
                    Property::instance()->fieldName( Property::TagId ), // 5
                    QString::number( static_cast<int>( tabId )), // 6
                    Property::instance()->fieldName( Property::PropertyData ), // 7
                    text // 8
                    );

    // repopulate table with filtering enabled
    this->populateTable( tagList, this->filter());
}
