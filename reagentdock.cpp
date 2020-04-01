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
#include "reagentdock.h"
#include "ui_reagentdock.h"
#include <QInputDialog>
#include <QMenu>
#include <QClipboard>
#include <QMoveEvent>
#include <QSqlQuery>
#include <QMessageBox>
#include <QTextEdit>
#include <QPainter>
#include <QDesktopServices>
#include "reagent.h"
#include "property.h"
#include "variable.h"
#include "propertydock.h"
#include "reagentdialog.h"
#include "extractiondialog.h"
#include "label.h"
#include "labelset.h"
#include "htmlutils.h"
#include "textutils.h"
#include "searchengine.h"

/**
 * @brief ReagentDock::ReagentDock
 * @param parent
 */
ReagentDock::ReagentDock( QWidget *parent ) : DockWidget( parent ), ui( new Ui::ReagentDock ) {
    // set up ui
    this->ui->setupUi( this );

    // disable the remove button (and enable it only when a reagent is clicked upon)
    this->ui->removeButton->setEnabled( false );
    QItemSelectionModel::connect( this->view()->selectionModel(), &QItemSelectionModel::currentChanged,
                                             [ this ]( const QModelIndex &current, const QModelIndex & ) {
                                                 this->ui->removeButton->setEnabled( current.isValid());
                                                 this->ui->editButton->setEnabled( current.isValid());
                                                 emit this->currentIndexChanged( current );
                                             } );
    // hide searchBox
    this->ui->searchEdit->hide();

    // implement search
    QLineEdit::connect( this->ui->searchEdit, &QLineEdit::textChanged, [ this ]( const QString &filter ) {
        this->view()->filterModel()->setFilterFixedString( filter );

        if ( !filter.isEmpty())
            this->view()->expandAll();

        // previous selection is unavailable
        const QModelIndex &previous( this->view()->selectionModel()->currentIndex());
        const QModelIndexList list( this->view()->filterModel()->match(
                previous.isValid() ? previous : this->view()->filterModel()->index( 0, 0 ), Qt::DisplayRole, filter, 2,
                Qt::MatchContains | Qt::MatchRecursive | Qt::MatchWrap ));
        if ( !previous.isValid() && !list.isEmpty()) {
            const QModelIndex nextIndex( list.first());
            this->view()->selectionModel()->select( list.first(), QItemSelectionModel::ClearAndSelect );
            this->view()->scrollTo( nextIndex );
            return;
        }
    } );

    // implement going through found entries
    QLineEdit::connect( this->ui->searchEdit, &QLineEdit::returnPressed, [ this ]() {
        const QString &filter( this->ui->searchEdit->text());

        // no filter - hide the search box
        if ( filter.isEmpty()) {
            this->on_buttonFind_clicked();
            return;
        }

        // previous selection is unavailable
        const QModelIndexList selectedIndexes( this->view()->selectionModel()->selectedIndexes());
        const QModelIndex &previousIndex( selectedIndexes.isEmpty() ? QModelIndex() : selectedIndexes.first());
        const QModelIndexList list(
                this->view()->filterModel()->match( this->view()->filterModel()->index( 0, 0 ), Qt::DisplayRole, filter,
                                                    -1, Qt::MatchContains | Qt::MatchRecursive | Qt::MatchWrap ));

        // if we have more than two reagents, iterate over them
        if ( list.count() < 2 ) {
            // if there is only one, close the search
            if ( list.count() == 1 )
                this->on_buttonFind_clicked();

            return;
        }

        const int previous = list.indexOf( previousIndex );
        const int next = ( previous >= list.count() - 1 ) ? 0 : previous + 1;
        const QModelIndex nextIndex( list.at( next ));
        this->view()->selectionModel()->select( nextIndex, QItemSelectionModel::ClearAndSelect );
        this->view()->scrollTo( nextIndex );
    } );

    // add keyboard shortcut to reagent filter
    this->shortcut = new QShortcut( QKeySequence( ReagentDock::tr( "Ctrl+F", "Find" )), this );
    QShortcut::connect( this->shortcut, &QShortcut::activated, [ this ]() {
        this->on_buttonFind_clicked();
    } );
}

/**
 * @brief ReagentDock::~ReagentDock
 */
ReagentDock::~ReagentDock() {
    delete this->shortcut;
    delete this->ui;
}

/**
 * @brief ReagentDock::checkForDuplicates
 * @param name
 * @param reference
 * @return
 */
bool ReagentDock::checkForDuplicates( const QString &name, const QString &reference, const Id reagentId ) const {
    //
    //
    // LET ME THINK
    //
    // reagent names and references can be the same, BUT there cannot be duplicate n/r for other reagents
    //
    // so when we add a reagent, we check for names/references in existing reagents
    // and when we edit, we do the same, omitting the reagentId
    //
    //

    QSqlQuery query;
    if ( reagentId == Id::Invalid ) {
        // reagent does not exist yet
        query.exec( QString( "select %1, %2 from %3 where %4=%5" )
                            .arg( Reagent::instance()->fieldName( Reagent::Name ),
                                  Reagent::instance()->fieldName( Reagent::Reference ),
                                  Reagent::instance()->tableName(),
                                  Reagent::instance()->fieldName( Reagent::ParentId ),
                                  QString::number( static_cast<int>( Id::Invalid )))
        );
    } else {
        // reagent does exist, we're just renaming it
        query.exec( QString( "select %1, %2, %6 from %3 where %4=%5 and %6!=%7" )
                            .arg( Reagent::instance()->fieldName( Reagent::Name ),
                                  Reagent::instance()->fieldName( Reagent::Reference ),
                                  Reagent::instance()->tableName(),
                                  Reagent::instance()->fieldName( Reagent::ParentId ),
                                  QString::number( static_cast<int>( Id::Invalid )),
                                  Reagent::instance()->fieldName( Reagent::ID ),
                                  QString::number( static_cast<int>( reagentId )))
        );
    }

    // check plainText names
    while ( query.next()) {
        const QString name_( HTMLUtils::convertToPlainText( query.value( 0 ).toString()));
        const QString reference_( HTMLUtils::convertToPlainText( query.value( 1 ).toString()));

        if ( !QString::compare( name, name_, Qt::CaseInsensitive ) ||
             !QString::compare( name, reference_, Qt::CaseInsensitive ) ||
             !QString::compare( reference, reference_, Qt::CaseInsensitive ) ||
             !QString::compare( reference, name_, Qt::CaseInsensitive )) {
            QMessageBox::warning( ReagentDock::instance(), ReagentDock::tr( "Cannot add or rename reagent" ),
                                  ReagentDock::tr( "Name or reference already exists" ));
            return false;
        }
    }

    return true;
}

/**
 * @brief ReagentDock::checkBatchDuplicates
 * @param name
 * @param parentId
 * @return
 */
bool ReagentDock::checkBatchForDuplicates( const QString &name, const Id parentId ) const {
    QSqlQuery query;
    query.exec( QString( "select %4 from %1 where %2=%3 and %4='%5'" )
                        .arg( Reagent::instance()->tableName(),
                              Reagent::instance()->fieldName( Reagent::ParentId ),
                              QString::number( static_cast<int>( parentId )),
                              Reagent::instance()->fieldName( Reagent::Name ),
                              name )
    );

    if ( query.next()) {
        QMessageBox::warning( ReagentDock::instance(), ReagentDock::tr( "Cannot add or rename batch" ),
                              ReagentDock::tr( "Batch '%1' already exists for this reagent" ).arg( name ));
        return false;
    }

    return true;
}

/**
 * @brief ReagentDock::view
 * @return
 */
ReagentView *ReagentDock::view() const {
    return this->ui->reagentView;
}

/**
 * @brief ReagentDock::buildMenu
 * @param context
 * @return
 */
QMenu *ReagentDock::buildMenu( bool context ) {
    auto *menu( new QMenu());

    auto *addMenu( menu->addMenu( ReagentDock::tr( "Add" )));
    addMenu->setIcon( QIcon::fromTheme( "add" ));
    addMenu->addAction( ReagentDock::tr( "Add new reagent" ), this, [ this ]() { this->addReagent( Id::Invalid ); } )->setIcon(
            QIcon::fromTheme( "reagent" ));

    menu->addAction( ReagentDock::tr( "Edit" ), this, [ this ]() { this->on_editButton_clicked(); } )->setIcon(
            QIcon::fromTheme( "edit" ));

    menu->addAction( ReagentDock::tr( "Remove" ), this, [ this ]() { this->on_removeButton_clicked(); } )->setIcon(
            QIcon::fromTheme( "remove" ));

    auto *visMenu( menu->addMenu( ReagentDock::tr( "Visibility" )));
    visMenu->setIcon( QIcon::fromTheme( "show" ));

    const QModelIndex index( this->view()->filterModel()->mapToSource( context ? this->view()->indexAt( this->view()->mapFromGlobal( QCursor::pos())) : this->view()->currentIndex()));
    if ( index.isValid()) {
        const QStandardItem *item( this->view()->itemFromIndex( index ));
        const auto parentId = item->data( ReagentModel::ParentId ).value<Id>();
        const QString name(( parentId == Id::Invalid ) ? item->text() : item->parent()->text());
        const QString batchName( TextUtils::elidedString( item->text()));

        addMenu->addAction( ReagentDock::tr( "Add new batch to reagent \"%1\"" ).arg( TextUtils::elidedString( name )), this,
                         [ this, parentId, item ]() { this->addReagent(
                                    ( parentId == Id::Invalid ) ?
                                        static_cast<Id>( item->data( ReagentModel::ID ).toInt())
                                      : parentId ); } )->setIcon( QIcon::fromTheme( "add" ));

        // deprecation
        if  ( parentId != Id::Invalid ) {
            const Id id = item->data( ReagentModel::ID ).value<Id>();
            if ( !NodeHistory::instance()->isDeperecated( id )) {
                visMenu->addAction( ReagentDock::tr( "Deprecate \"%1\"" ).arg( batchName ), this, [ this, id ]() {
                    NodeHistory::instance()->deprecate( id );
                    this->view()->updateView();
                } )->setIcon( QIcon::fromTheme( "remove" ));
            } else {
                visMenu->addAction( ReagentDock::tr( "Restore \"%1\"" ).arg( batchName ), this, [ this, id ]() {
                    NodeHistory::instance()->restore( id );
                    this->view()->updateView();
                } )->setIcon( QIcon::fromTheme( "show" ));
            }
        }

        if ( context ) {
            menu->addAction( ReagentDock::tr( "Copy name" ), this,
                             [ item ]() { QGuiApplication::clipboard()->setText( item->text()); } )->setIcon(
                    QIcon::fromTheme( "copy" ));

            if ( parentId == Id::Invalid ) {
                QMenu *labels( menu->addMenu( ReagentDock::tr( "Labels" )));
                labels->setIcon( QIcon::fromTheme( "label" ));
                for ( int y = 0; y < Label::instance()->count(); y++ ) {
                    const auto row = static_cast<Row>( y );
                    const Id menuLabelId = Label::instance()->id( row );
                    const auto reagentId = item->data( ReagentModel::ID ).value<Id>();

                    const QList<Id> labelIds( Reagent::instance()->labelIds( Reagent::instance()->row( reagentId )));
                    bool hasLabel = false;
                    for ( const Id &labelId : labelIds ) {
                        if ( labelId == menuLabelId ) {
                            hasLabel = true;
                            break;
                        }
                    }

                    QAction *action(
                            labels->addAction( QIcon( Label::instance()->pixmap( Label::instance()->colour( row ))),
                                               QApplication::translate( "Label", Label::instance()->name( row ).toUtf8().constData(), nullptr ),
                                               [ item, menuLabelId, reagentId, hasLabel ]() {
                                                   if ( hasLabel ) {
                                                       LabelSet::instance()->remove( menuLabelId, reagentId );
                                                       LabelSet::instance()->removeOrphanedEntries();
                                                   } else
                                                       LabelSet::instance()->add( menuLabelId, reagentId );

                                                   // force icon reset without resetting the model
                                                   //const_cast<QStandardItem*>( item )->setIcon( QIcon());
                                                   // force pixmap reset without resetting the model
                                                   const_cast<QStandardItem *>( item )->setData( QPixmap(),
                                                                                                 ReagentModel::Pixmap );
                                               } ));
                    action->setCheckable( true );
                    if ( hasLabel )
                        action->setChecked( true );
                }
            }
        }

        if ( context ) {
            const auto id = item->data( ReagentModel::ID ).value<Id>();
            const QString reagentName( HTMLUtils::convertToPlainText( Reagent::instance()->name(
                    Reagent::instance()->row( parentId == Id::Invalid ? id : parentId ))));

            // search online            
            SearchEngineManager::instance()->populateMenu( menu->addMenu( ReagentDock::tr( "Search online" )), reagentName );

            // hide reagent
            visMenu->addAction( ReagentDock::tr( R"(Hide "%1")" ).arg( TextUtils::elidedString( item->text())), [ this, id ]() {
                NodeHistory::instance()->hide( id );
                this->view()->updateView();
            } )->setIcon( QIcon::fromTheme( "hide" ));
        }
    }

    if ( NodeHistory::instance()->hiddenCount() > 0 && context ) {
        visMenu->addAction( PropertyDock::tr( "Show all reagents" ), this, []() {
            NodeHistory::instance()->clearHiddenNodes();
            ReagentDock::instance()->view()->updateView();
        } )->setIcon( QIcon::fromTheme( "show" ));
    }

    if ( context ) {
        visMenu->addAction( ReagentDock::tr( "Collapse all" ), []() {
            ReagentDock::instance()->view()->collapseAll();
        } );
    }

    menu->setAttribute( Qt::WA_DeleteOnClose, true );
    return menu;
}

/**
 * @brief ReagentDock::addReagent
 * @param parentId
 */
Id ReagentDock::addReagent( const Id &parentId, const QString &reagentName, const int cid ) {
    QString name;
    QString reference;
    bool ok;

    //
    // NOTE:
    //  reagents can only have unique names and references
    //  batches can be named anything and do not have references at all
    //
    QList<Id> labels;
    if ( parentId != Id::Invalid ) {
        ReagentDialog rd( this, reagentName, reagentName, ReagentDialog::BatchMode );
        ok = ( rd.exec() == QDialog::Accepted );
        name = HTMLUtils::convertToPlainText( rd.name());

        if ( ok ) {
            if ( !this->checkBatchForDuplicates( qAsConst( name ), parentId ))
                return Id::Invalid;
        }
    } else {
        ReagentDialog rd( this, reagentName, reagentName, ( cid == 0 ? ReagentDialog::AddMode : ReagentDialog::SearchMode ));
        ok = ( rd.exec() == QDialog::Accepted );
        name = rd.name();
        reference = rd.reference();

        if ( ok ) {
            if ( !this->checkForDuplicates( qAsConst( name ), qAsConst( reference )))
                return Id::Invalid;

            labels = rd.labels;
        }
    }

    if ( ok ) {
        if ( !name.isEmpty()) {
            // save filter
            // NOTE: why? because when we add a reagent when filtered the row will always be invalid
            //       due to non-existent label sets
            //       this way filter is temporarily removed and restore after addition
            const QString oldFilter( Reagent::instance()->filter());
            if ( !Reagent::instance()->filter().isEmpty())
                Reagent::instance()->setFilter( "" );

            const Row row = Reagent::instance()->add( qAsConst( name ), qAsConst( reference ), parentId );
            if ( row == Row::Invalid )
                return Id::Invalid;

            // get reagentId
            const Id reagentId = Reagent::instance()->id( row );

            // add labels if any
            //qDebug() << "got labels" << labels;
            for ( const Id &id : qAsConst( labels )) {
                if ( id == Id::Invalid )
                    continue;

                LabelSet::instance()->add( id, reagentId );
            }

            // ... and add to treeView without resetting the model
            this->view()->sourceModel()->add( reagentId );

            // expand parent reagent
            if ( parentId != Id::Invalid )
                this->view()->expand(
                        this->view()->filterModel()->mapFromSource( this->view()->indexFromId( parentId )));

            // select the newly added reagent or batch
            this->view()->selectReagent(
                        this->view()->filterModel()->mapFromSource( this->view()->indexFromId( reagentId )));

            // open extraction dialog if this feature is enabled
            if ( Variable::isEnabled( "fetchPropertiesOnAddition" ) && parentId == Id::Invalid && cid == 0 ) {
                ExtractionDialog ed( this, reagentId ); // cid );
                ed.exec();
                PropertyDock::instance()->updateView();
            }

            // restore filter
            if ( !oldFilter.isEmpty()) {
                Reagent::instance()->setFilter( oldFilter );
                this->view()->updateView();
            }

            return reagentId;
        } else {
            QMessageBox::warning( this, ReagentDock::tr( "Cannot add reagent" ),
                                  ( parentId != Id::Invalid ? ReagentDock::tr( "Batch" ) : ReagentDock::tr( "Reagent" )) +
                                  ReagentDock::tr( " name is empty" ));
        }
    }

    return Id::Invalid;
}

/**
 * @brief ReagentDock::on_reagentView_customContextMenuRequested
 * @param pos
 */
void ReagentDock::on_reagentView_customContextMenuRequested( const QPoint &pos ) {
    QMenu *menu( this->buildMenu( true ));
    menu->exec( this->mapToGlobal( pos ));
}

/**
 * @brief ReagentDock::on_addButton_clicked
 */
void ReagentDock::on_addButton_clicked() {
    QMenu *menu( this->buildMenu( false ));
    menu->exec( this->mapToGlobal( this->ui->addButton->pos()));
}

/**
 * @brief ReagentDock::on_removeButton_clicked
 */
void ReagentDock::on_removeButton_clicked() {
    /**
     * removeReagentsAndBatches lambda
     */
    auto removeReagentsAndBatches = [ this ]( const QStandardItem *item ) {
        const auto reagentId = item->data( ReagentModel::ID ).value<Id>();
        const Row reagentRow = Reagent::instance()->row( reagentId );
        if ( reagentRow == Row::Invalid )
            return;

        // remove batches
        if ( Reagent::instance()->parentId( reagentRow ) == Id::Invalid ) {
            const QList<Row> children( Reagent::instance()->children( reagentRow ));
            for ( const Row &batchRow : children ) {
                Reagent::instance()->remove( batchRow );

                // remove from depcrecated nodes
                NodeHistory::instance()->deprecatedNodes.removeOne( Reagent::instance()->id( batchRow ));
            }
        }

        // remove reagent
        Reagent::instance()->remove( reagentRow );

        // remove from open and closed nodes (we don't need dangling reagents)
        NodeHistory::instance()->openNodes.removeOne( reagentId );
        NodeHistory::instance()->hiddenNodes.removeOne( reagentId );

        // remove orphans just in case
        Reagent::instance()->removeOrphanedEntries();
        Property::instance()->removeOrphanedEntries();

        // clear selection
        this->view()->selectReagent();
    };

    QMenu menu;
    const QModelIndexList list( this->view()->selectionModel()->selectedRows());

    // remove reagents and batches (list)
    if ( list.count() > 1 ) {
        menu.addAction( ReagentDock::tr( "Remove %1 selected reagents and their batches" ).arg( list.count()),
                        this, [ this, list, removeReagentsAndBatches ]() {
            QModelIndexList sourceList;
            for ( const QModelIndex &filter : list ) {
                const QModelIndex &index( this->view()->filterModel()->mapToSource( filter ));
                sourceList << index;

                const QStandardItem *item( this->view()->itemFromIndex( index ));
                if ( item != nullptr )
                    removeReagentsAndBatches( item );
            }

            // remove items without resetting model
            this->view()->sourceModel()->remove( qAsConst( sourceList ));
        } )->setIcon( QIcon::fromTheme( "remove" ));
    } else {
        // remove just one entry
        // get current index
        const QModelIndex index( this->view()->filterModel()->mapToSource( this->view()->currentIndex()));
        if ( !index.isValid())
            return;

        // get current item
        // remove reagent and batches
        const QStandardItem *item( this->view()->itemFromIndex( index ));
        const auto parentId = item->data( ReagentModel::ParentId ).value<Id>();
        menu.addAction( ReagentDock::tr( parentId == Id::Invalid ?
                                             "Remove reagent '%1' and its batches" :
                                             "Remove batch '%1'"
                                             ).arg( TextUtils::elidedString( item->text())), this, [ this, item, index, parentId, removeReagentsAndBatches ]() {
            removeReagentsAndBatches( item );

            // remove items without resetting model
            this->view()->sourceModel()->remove( index );

            // reselect parent reagent item if any
            if ( parentId != Id::Invalid )
                this->view()->selectReagent(
                        this->view()->filterModel()->mapFromSource( this->view()->indexFromId( parentId )));
        } );
    }

    // display menu
    menu.exec( this->mapToGlobal( this->ui->removeButton->pos()));
}

/**
 * @brief ReagentDock::on_buttonFind_clicked
 */
void ReagentDock::on_buttonFind_clicked() {
    // toggle searchBox visibility
    this->ui->searchEdit->setVisible( !this->ui->searchEdit->isVisible());

    if ( this->ui->searchEdit->isVisible()) {
        // disable node history
        NodeHistory::instance()->setEnabled( false );

        // focus on the searchBox
        this->ui->searchEdit->setFocus();

        // select the first reagent
        this->view()->selectionModel()->clearSelection();
        this->view()->selectionModel()->select( this->view()->filterModel()->index( 0, 0 ),
                                                QItemSelectionModel::Select );
    } else {
        const QModelIndexList list( this->view()->selectionModel()->selectedIndexes());
        if ( !list.isEmpty())
            Variable::setValue( "reagentDock/selection", static_cast<int>( this->view()->idFromIndex(
                    this->view()->filterModel()->mapToSource( list.first()))));

        this->ui->searchEdit->clear();
        NodeHistory::instance()->setEnabled( true );
        NodeHistory::instance()->restoreNodeState();
        this->view()->restoreIndex();
    }
}

/**
 * @brief ReagentDock::on_editButton_clicked
 */
void ReagentDock::on_editButton_clicked() {
    // get current index
    const QModelIndex index( this->view()->filterModel()->mapToSource( this->view()->currentIndex()));
    if ( !index.isValid())
        return;

    // get current item
    QStandardItem *item( this->view()->itemFromIndex( index ));
    const auto reagentId = item->data( ReagentModel::ID ).value<Id>();
    if ( reagentId == Id::Invalid )
        return;

    const Row reagentRow = Reagent::instance()->row( reagentId );
    if ( reagentRow == Row::Invalid )
        return;

    const auto parentId = item->data( ReagentModel::ParentId ).value<Id>();
    const QString previousName( Reagent::instance()->name( reagentId ));
    const QString previousReference( Reagent::instance()->reference( reagentId ));

    bool ok;
    if ( parentId != Id::Invalid ) {
        ReagentDialog rd( this, previousName, "", ReagentDialog::BatchEditMode );
        ok = ( rd.exec() == QDialog::Accepted );
        const QString name( HTMLUtils::convertToPlainText( rd.name()));

        if ( !name.isEmpty() && ok ) {
            if ( !this->checkBatchForDuplicates( name, parentId ))
                return;

            Reagent::instance()->setName( reagentRow, name );

            // rename without resetting the model
            const QString generatedName( ReagentModel::generateName( name ));
            item->setText( HTMLUtils::convertToPlainText( generatedName ));
            item->setData( generatedName, ReagentModel::HTML );
        }
    } else {
        ReagentDialog rd( this, previousName, previousReference, ReagentDialog::EditMode );
        ok = ( rd.exec() == QDialog::Accepted );
        const QString name( rd.name());
        const QString reference( rd.reference());

        if ( !this->checkForDuplicates( name, reference, reagentId ) || name.isEmpty() || reference.isEmpty() || !ok )
            return;

        Reagent::instance()->setName( reagentRow, name );
        Reagent::instance()->setReference( reagentRow, reference );

        // rename without resetting the model
        const QString generatedName( ReagentModel::generateName( name, reference ));
        item->setText( HTMLUtils::convertToPlainText( generatedName ));
        item->setData( generatedName, ReagentModel::HTML );
    }
}
