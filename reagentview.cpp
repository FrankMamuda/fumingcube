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
#include "variable.h"
#include "propertydock.h"
#include "reagentview.h"
#include "reagent.h"
#include "reagentdelegate.h"
#include "reagentdock.h"

/**
 * @brief ReagentView::ReagentView
 * @param parent
 */
ReagentView::ReagentView( QWidget *parent ) : QTreeView( parent ) {
    // set a model to treeView
    this->setModel( new SortFilterProxyModel( this ));

    // setup node history
    NodeHistory::instance()->setTreeParent( this );

    // setup model
    this->filterModel()->setSourceModel( this->reagentModel );
    this->filterModel()->setRecursiveFilteringEnabled( true );
    this->filterModel()->setSortCaseSensitivity( Qt::CaseInsensitive );
    this->filterModel()->setFilterCaseSensitivity( Qt::CaseInsensitive );
    this->setRootIndex( this->sourceModel()->invisibleRootItem()->index());

    // set html delegate
    this->delegate->setModel( this->filterModel());
    this->delegate->setParentView( this );
    this->setItemDelegate( this->delegate );

    // restore state
    NodeHistory::instance()->restoreNodeState();

    // make sure to change reagent on click
    ReagentView::connect( this, SIGNAL( clicked( const QModelIndex & )), this, SLOT( selectReagent( const QModelIndex & )));

    // setup timer
    this->resizeTimer.setSingleShot( true );
    QTimer::connect( &this->resizeTimer, &QTimer::timeout, this, [ this ]() {
        this->m_resizeInProgress = false;
        this->delegate->clearCache();

        // force update (triggered by sort)
        this->filterModel()->sort( 0, Qt::DescendingOrder );
        this->filterModel()->sort( 0, Qt::AscendingOrder );
    } );
}

/**
 * @brief ReagentView::~ReagentView
 */
ReagentView::~ReagentView() {
    QTimer::disconnect( &this->resizeTimer, &QTimer::timeout, this, nullptr );
    delete this->delegate;
    delete this->reagentModel;
}

/**
 * @brief ReagentView::updateView
 */
void ReagentView::updateView() {
    this->delegate->clearCache();
    NodeHistory::instance()->setEnabled( false );
    this->sourceModel()->setupModelData();

    // clear selection
    this->filterModel()->sort( 0, Qt::AscendingOrder );
    NodeHistory::instance()->restoreNodeState();
    this->restoreIndex();
}

/**
 * @brief ReagentView::selectReagent
 * @param index
 */
void ReagentView::selectReagent( const QModelIndex &filterIndex ) {
    const QModelIndex &index( this->filterModel()->mapToSource( filterIndex ));
    this->setCurrentIndex( filterIndex );
    this->selectionModel()->select( filterIndex, QItemSelectionModel::ClearAndSelect );

    // if reagent is invalid, display no properties
    if ( !index.isValid()) {
        Property::instance()->setFilter( "false" );
        Variable::setInteger( "reagentDock/selection", -1 );
        return;
    }

    // retrieve data from model
    const QStandardItem *item( this->itemFromIndex( index ));
    const int reagentId = item->data( ReagentModel::ID ).toInt();

    // store last selection in a variable
    Variable::setInteger( "reagentDock/selection", reagentId );

    // apply sql filter
    Property::instance()->setFilter( QString(
            "( %1=%2 and %1>-1 and %4 %6 ) or ( %1=%3 and %1>-1 and %4 %6 and %4 not in ( select %4 from %5 where ( %1=%2 and %4>-2 )))" )
                                             .arg( Property::instance()->fieldName( Property::ReagentId ),   // 1
                                                   QString::number( reagentId ),                             // 2
                                                   item->data( ReagentModel::ParentId ).toString(),          // 3
                                                   Property::instance()->fieldName( Property::TagId ),       // 4
                                                   Property::instance()->tableName(),                        // 5
                                                   PropertyDock::instance()->hiddenTags.join( ", " ).append(
                                                     " ) " ).prepend( "not in ( " ))    // 6
    );
    Property::instance()->sort( Property::TableOrder, Qt::AscendingOrder );
    Property::instance()->select();

    // resize the property view to fit contents
    PropertyDock::instance()->updateView();
}

/**
 * @brief ReagentView::restoreIndex
 */
void ReagentView::restoreIndex() {
    // get id list from variable
    const auto id( Variable::value<Id>( "reagentDock/selection" ));
    if ( id != Id::Invalid ) {
        const Id parentId( Reagent::instance()->parentId( id ));
        if ( parentId != Id::Invalid )
            this->expand( this->filterModel()->mapFromSource( this->indexFromId( parentId )));

        const QModelIndex index( this->filterModel()->mapFromSource( this->indexFromId( id )));
        this->selectReagent( index );

        // NOTE: this must be delayed
        QTimer::singleShot( 100, [ this ]() {
            this->scrollTo( this->currentIndex(), ScrollHint::PositionAtCenter );
        } );

        return;
    }

    this->selectReagent();
}

/**
 * @brief ReagentView::keyReleaseEvent
 * @param event
 */
void ReagentView::keyReleaseEvent( QKeyEvent *event ) {
    if ( event->key() == Qt::Key_Down || event->key() == Qt::Key_Up ) {
        const QModelIndex &index( this->selectionModel()->currentIndex());
        if ( index.isValid())
            this->selectReagent( index );
    }

    QTreeView::keyReleaseEvent( event );
}

/**
 * @brief ReagentView::mouseReleaseEvent
 * @param event
 */
void ReagentView::mouseReleaseEvent( QMouseEvent *event ) {
    if ( event->button() == Qt::RightButton ) {
        const QModelIndex index( this->indexAt( event->pos()));

        if ( index.isValid())
            this->selectReagent( index );
    }

    QTreeView::mouseReleaseEvent( event );
}

/**
 * @brief ReagentView::resizeEvent
 * @param event
 */
void ReagentView::resizeEvent( QResizeEvent *event ) {

    this->m_resizeInProgress = true;
    this->resizeTimer.start( 128 );

    QTreeView::resizeEvent( event );
}

/**
 * @brief SortFilterProxyModel::lessThan
 * @param left
 * @param right
 * @return
 */
bool SortFilterProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const {
    const QStandardItem *leftItem( qobject_cast<ReagentModel *>( this->sourceModel())->itemFromIndex( left ));
    const QStandardItem *rightItem( qobject_cast<ReagentModel *>( this->sourceModel())->itemFromIndex( right ));

    if ( leftItem->data( ReagentModel::ParentId ).value<Id>() == Id::Invalid && rightItem->data( ReagentModel::ParentId ).value<Id>() == Id::Invalid )
        return QSortFilterProxyModel::lessThan( left, right );

    const QDateTime leftDate( leftItem->data( ReagentModel::DateTime ).toDateTime());
    const QDateTime rightDate( rightItem->data( ReagentModel::DateTime ).toDateTime());

    if ( leftDate == rightDate )
        return QSortFilterProxyModel::lessThan( left, right );

    return leftDate < rightDate;
}

/**
 * @brief SortFilterProxyModel::filterAcceptsRow
 * @param sourceRow
 * @param sourceParent
 * @return
 */
bool SortFilterProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const {
    if ( QSortFilterProxyModel::filterAcceptsRow( sourceRow, sourceParent ))
        return true;

    QModelIndex parent( sourceParent );
    while ( parent.isValid()) {
        if ( QSortFilterProxyModel::filterAcceptsRow( parent.row(), parent.parent()))
            return true;

        parent = parent.parent();
    }

    QModelIndex index( this->sourceModel()->index( sourceRow, 0, sourceParent ));
    if ( !index.isValid())
        return true;

    return false;
}
