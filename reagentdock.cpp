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
#include "reagent.h"
#include "property.h"
#include "tag.h"
#include "variable.h"
#include "propertydock.h"
#include "reagentdialog.h"
#include "extractiondialog.h"
#include "label.h"
#include "labelset.h"

/**
 * @brief ReagentDock::ReagentDock
 * @param parent
 */
ReagentDock::ReagentDock( QWidget *parent ) : DockWidget( parent ), ui( new Ui::ReagentDock ) {
    // set up ui
    this->ui->setupUi( this );

    // disable the remove button (and enable it only when a reagent is clicked upon)
    this->ui->removeButton->setEnabled( false );
    this->view()->selectionModel()->connect( this->view()->selectionModel(), &QItemSelectionModel::currentChanged, [ this ]( const QModelIndex &current, const QModelIndex & ) {
        this->ui->removeButton->setEnabled( current.isValid());
        this->ui->editButton->setEnabled( current.isValid());
        emit this->currentIndexChanged( current );
    } );

    // hide searchBox
    this->ui->searchEdit->hide();

    // implement search
    this->ui->searchEdit->connect( this->ui->searchEdit, &QLineEdit::textChanged, [ this ]( const QString &filter ) {
        this->view()->filterModel()->setFilterFixedString( filter );

        if ( !filter.isEmpty())
            this->view()->expandAll();

        // previous selection is unavailable
        const QModelIndex &previous( this->view()->selectionModel()->currentIndex());
        const QModelIndexList list( this->view()->filterModel()->match( previous.isValid() ? previous : this->view()->filterModel()->index( 0, 0 ), Qt::DisplayRole, filter, 2, Qt::MatchContains | Qt::MatchRecursive | Qt::MatchWrap ));
        if ( !previous.isValid() && !list.isEmpty()) {
            const QModelIndex nextIndex( list.first());
            this->view()->selectionModel()->select( list.first(), QItemSelectionModel::ClearAndSelect );
            this->view()->scrollTo( nextIndex );
            return;
        }
    } );

    // implement going through found entries
    this->ui->searchEdit->connect( this->ui->searchEdit, &QLineEdit::returnPressed, [ this ]() {
        const QString &filter( this->ui->searchEdit->text());

        // no filter - hide the search box
        if ( filter.isEmpty()) {
            this->on_buttonFind_clicked();
            return;
        }

        // previous selection is unavailable
        const QModelIndexList selectedIndexes( this->view()->selectionModel()->selectedIndexes());
        const QModelIndex &previousIndex( selectedIndexes.isEmpty() ? QModelIndex() : selectedIndexes.first());
        const QModelIndexList list( this->view()->filterModel()->match( this->view()->filterModel()->index( 0, 0 ), Qt::DisplayRole, filter, -1, Qt::MatchContains | Qt::MatchRecursive | Qt::MatchWrap ));

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
    this->shortcut = new QShortcut( QKeySequence( this->tr( "Ctrl+F", "Find" )), this );
    this->shortcut->connect( this->shortcut, &QShortcut::activated, [ this ]() {
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
 * @param alias
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
                    .arg( Reagent::instance()->fieldName( Reagent::Name ))
                    .arg( Reagent::instance()->fieldName( Reagent::Alias ))
                    .arg( Reagent::instance()->tableName())
                    .arg( Reagent::instance()->fieldName( Reagent::ParentId ))
                    .arg( static_cast<int>( Id::Invalid ))
                    );
    } else {
        // reagent does exist, we're just renaming it
        query.exec( QString( "select %1, %2, %6 from %3 where %4=%5 and %6!=%7" )
                    .arg( Reagent::instance()->fieldName( Reagent::Name ))
                    .arg( Reagent::instance()->fieldName( Reagent::Alias ))
                    .arg( Reagent::instance()->tableName())
                    .arg( Reagent::instance()->fieldName( Reagent::ParentId ))
                    .arg( static_cast<int>( Id::Invalid ))
                    .arg( Reagent::instance()->fieldName( Reagent::ID ))
                    .arg( static_cast<int>( reagentId ))
                    );
    }

    // check plainText names
    while ( query.next()) {
        const QString name_( QTextEdit( query.value( 0 ).toString()).toPlainText());
        const QString reference_( QTextEdit( query.value( 1 ).toString()).toPlainText());

        if ( !QString::compare( name, name_, Qt::CaseInsensitive ) || !QString::compare( name, reference_, Qt::CaseInsensitive ) ||
             !QString::compare( reference, reference_, Qt::CaseInsensitive ) || !QString::compare( reference, name_, Qt::CaseInsensitive )) {
            QMessageBox::warning( ReagentDock::instance(), this->tr( "Cannot add or rename reagent" ), this->tr( "Name or reference already exists" ));
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
                .arg( Reagent::instance()->tableName())
                .arg( Reagent::instance()->fieldName( Reagent::ParentId ))
                .arg( static_cast<int>( parentId ))
                .arg( Reagent::instance()->fieldName( Reagent::Name ))
                .arg( name )
                );

    if ( query.next()) {
        QMessageBox::warning( ReagentDock::instance(), this->tr( "Cannot add or rename batch" ), this->tr( "Batch '%1' already exists for this reagent" ).arg( name ));
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
    QMenu *menu( new QMenu());

    auto addReagent = [ this ]( const Id &parentId ) {
        QString name, alias;
        bool ok;

        //
        // NOTE:
        //  reagents can only have unique names and aliases
        //  batches can be named anything and do not have aliases at all
        //
        QList<Id> labels;
        if ( parentId != Id::Invalid ) {
            name = QInputDialog::getText( this, this->tr( "Add batch" ), this->tr( "Name:" ), QLineEdit::Normal, QString(), &ok );

            if ( !this->checkBatchForDuplicates( qAsConst( name ), parentId ))
                return;
        } else {
            ReagentDialog rd( this );
            ok = ( rd.exec() == QDialog::Accepted );
            name = rd.name();
            alias = rd.reference();

            if ( ok ) {
                if ( !this->checkForDuplicates( qAsConst( name ), qAsConst( alias )))
                    return;

                labels = rd.labels;
            }
        }

        if ( ok ) {
            if ( !name.isEmpty()) {
                // save filter
                // NOTE: why? because when we add a reagent when filtered the row will always be invalid
                //       due to non-existant label sets
                //       this way filter is temporarely removed and restore after addition
                const QString oldFilter( Reagent::instance()->filter());
                if ( !Reagent::instance()->filter().isEmpty())
                    Reagent::instance()->setFilter( "" );

                const Row row = Reagent::instance()->add( qAsConst( name ), qAsConst( alias ), parentId );
                if ( row == Row::Invalid )
                    return;

                // get reagentId
                const Id reagentId = Reagent::instance()->id( row );

                // add labels if any
                //qDebug() << "got labels" << labels;
                foreach ( const Id &id, labels ) {
                    if ( id == Id::Invalid )
                        continue;

                    LabelSet::instance()->add( id, reagentId );
                }

                // ... and add to treeView without resetting the model
                this->view()->model()->add( reagentId );

                // expand parent reagent
                if ( parentId != Id::Invalid )
                    this->view()->expand( this->view()->filterModel()->mapFromSource( this->view()->indexFromId( parentId )));

                // select the newly added reagent or batch
                this->view()->selectReagent( this->view()->filterModel()->mapFromSource( this->view()->indexFromId( reagentId )));

                // open extraction dialog if this feature is enabled
                if ( Variable::instance()->isEnabled( "fetchPropertiesOnAddition" ) && parentId == Id::Invalid ) {
                    ExtractionDialog ed( this, reagentId );
                    ed.exec();
                    PropertyDock::instance()->updateView();
                }

                // restore filter
                if ( !oldFilter.isEmpty()) {
                    Reagent::instance()->setFilter( oldFilter );
                    this->view()->updateView();
                }
            } else {
                QMessageBox::warning( this,  this->tr( "Cannot add reagent" ), ( parentId != Id::Invalid ?  this->tr( "Batch" ) : this->tr( "Reagent" )) + this->tr( " name is empty" ));
                return;
            }
        }
    };

    menu->addAction( this->tr( "Add new reagent" ), std::bind( addReagent, Id::Invalid ))->setIcon( QIcon::fromTheme( "reagent" ));

    const QModelIndex index( this->view()->filterModel()->mapToSource( this->view()->currentIndex()));
    if ( index.isValid()) {
        const QStandardItem *item( this->view()->itemFromIndex( index ));
        const Id parentId = item->data( ReagentModel::ParentId ).value<Id>();
        const QString name(( parentId == Id::Invalid ) ? item->text() : item->parent()->text());

        menu->addAction( this->tr( "Add new batch to reagent \"%1\"" ).arg( name ), std::bind( addReagent, ( parentId == Id::Invalid ) ? static_cast<Id>( item->data( ReagentModel::ID ).toInt()) : parentId  ))->setIcon( QIcon::fromTheme( "add" ));

        if ( context ) {
            menu->addAction( this->tr( "Copy name" ), [ item ]() { QGuiApplication::clipboard()->setText( item->text()); } )->setIcon( QIcon::fromTheme( "copy" ));

            if ( parentId == Id::Invalid ) {
                QMenu *labels( menu->addMenu( this->tr( "Labels" )));
                labels->setIcon( QIcon::fromTheme( "label" ));
                for ( int y = 0; y < Label::instance()->count(); y++ ) {
                    const Row row = static_cast<Row>( y );
                    const Id menuLabelId = Label::instance()->id( row );
                    const Id reagentId = item->data( ReagentModel::ID ).value<Id>();

                    const QList<Id> labelIds( Reagent::instance()->labelIds( Reagent::instance()->row( reagentId )));
                    bool hasLabel = false;
                    foreach ( const Id &labelId, labelIds ) {
                        if ( labelId == menuLabelId ) {
                            hasLabel = true;
                            break;
                        }
                    }

                    QAction *action( labels->addAction( QIcon( Label::instance()->pixmap( Label::instance()->colour( row ))), Label::instance()->name( row ), [ item, menuLabelId, reagentId, hasLabel ]() {
                        if ( hasLabel ) {
                            LabelSet::instance()->remove( menuLabelId, reagentId );
                            LabelSet::instance()->removeOrphanedEntries();
                        } else
                            LabelSet::instance()->add( menuLabelId, reagentId );

                        // force icon reset without resetting the model
                        //const_cast<QStandardItem*>( item )->setIcon( QIcon());
                        // force pixmap reset without resetting the model
                        const_cast<QStandardItem*>( item )->setData( QPixmap(), ReagentModel::Pixmap );
                    } ));
                    action->setCheckable( true );
                    if ( hasLabel )
                        action->setChecked( true );
                }
            }
        }
    }

    menu->setAttribute( Qt::WA_DeleteOnClose, true );
    return menu;
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
        const Id reagentId = item->data( ReagentModel::ID ).value<Id>();
        const Row reagentRow = Reagent::instance()->row( reagentId );
        if ( reagentRow == Row::Invalid )
            return;

        // remove batches
        if ( Reagent::instance()->parentId( reagentRow ) == Id::Invalid ) {
            const QList<Row>children( Reagent::instance()->children( reagentRow ));
            foreach ( const Row &batchRow, children )
                Reagent::instance()->remove( batchRow );
        }

        // remove reagent
        Reagent::instance()->remove( reagentRow );

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
        menu.addAction( this->tr( "Remove %1 selected reagents and their batches" ).arg( list.count()), [ this, list, removeReagentsAndBatches ]() {
            QModelIndexList sourceList;
            foreach ( const QModelIndex &filter, list ) {
                const QModelIndex &index( this->view()->filterModel()->mapToSource( filter ));
                sourceList << index;

                const QStandardItem *item( this->view()->itemFromIndex( index ));
                if ( item != nullptr )
                    removeReagentsAndBatches( item );
            }

            // remove items without resetting model
            this->view()->model()->remove( qAsConst( sourceList ));
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
        const Id parentId = item->data( ReagentModel::ParentId ).value<Id>();
        menu.addAction( this->tr( parentId == Id::Invalid ?
                                      "Remove reagent '%1' and its batches" :
                                      "Remove batch '%1'"
                                      ).arg( item->text()), [ this, item, index, parentId, removeReagentsAndBatches ]() {
            removeReagentsAndBatches( item );

            // remove items without resetting model
            this->view()->model()->remove( index );

            // reselect parent reagent item if any
            if ( parentId != Id::Invalid )
                this->view()->selectReagent( this->view()->filterModel()->mapFromSource( this->view()->indexFromId( parentId )));
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
    this->ui->searchEdit->setVisible( !this->ui->searchEdit->isVisible() );

    if ( this->ui->searchEdit->isVisible()) {
        // disable node history
        this->view()->nodeHistory()->setEnabled( false );

        // focus on the searchBox
        this->ui->searchEdit->setFocus();

        // select the first reagent
        this->view()->selectionModel()->clearSelection();
        this->view()->selectionModel()->select( this->view()->filterModel()->index( 0, 0 ), QItemSelectionModel::Select );
    } else {
        const QModelIndexList list( this->view()->selectionModel()->selectedIndexes());
        if ( !list.isEmpty())
            Variable::instance()->setValue( "reagentDock/selection", static_cast<int>( this->view()->idFromIndex( this->view()->filterModel()->mapToSource( list.first()))));

        this->ui->searchEdit->clear();
        this->view()->nodeHistory()->setEnabled( true );
        this->view()->nodeHistory()->restoreNodeState();
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
    const QStandardItem *item( this->view()->itemFromIndex( index ));
    const Id reagentId = item->data( ReagentModel::ID ).value<Id>();
    if ( reagentId == Id::Invalid )
        return;

    const Row reagentRow = Reagent::instance()->row( reagentId );
    if ( reagentRow == Row::Invalid )
        return;

    const Id parentId = item->data( ReagentModel::ParentId ).value<Id>();
    const QString previousName( Reagent::instance()->name( reagentId ));
    const QString previousAlias( Reagent::instance()->alias( reagentId ));

    bool ok;
    if ( parentId != Id::Invalid ) {
        const QString name( QInputDialog::getText( this, this->tr( "Rename batch" ), this->tr( "Name:" ), QLineEdit::Normal, previousName, &ok ));

        if ( !name.isEmpty()) {
            if ( !this->checkBatchForDuplicates( name, parentId ))
                return;

            Reagent::instance()->setName( reagentRow, name );

            // rename without resetting the model
            const QString generatedName( ReagentModel::generateName( name ));
            QStandardItem* modelItem = const_cast<QStandardItem*>( item );
            modelItem->setText( QTextEdit( generatedName ).toPlainText());
            modelItem->setData( generatedName, ReagentModel::HTML );
        }
    } else {
        ReagentDialog rd( this, previousName, previousAlias, ReagentDialog::EditMode );
        ok = ( rd.exec() == QDialog::Accepted );
        const QString name( rd.name());
        const QString alias( rd.reference());

        if ( !this->checkForDuplicates( name, alias, reagentId ) || name.isEmpty() || alias.isEmpty() || !ok )
            return;

        Reagent::instance()->setName( reagentRow, name );
        Reagent::instance()->setAlias( reagentRow, alias );

        // rename without resetting the model
        const QString generatedName( ReagentModel::generateName( name, alias ));
        QStandardItem* modelItem = const_cast<QStandardItem*>( item );
        modelItem->setText( QTextEdit( generatedName ).toPlainText());
        modelItem->setData( generatedName, ReagentModel::HTML );
    }
}
