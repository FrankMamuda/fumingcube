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
        // update the match list
        this->matches = this->view()->model()->setupModelData( filter );

        // if list empty, reset currentMatch and clear selection
        if ( this->matches.isEmpty()) {
            this->currentMatch = Id::Invalid;
            this->view()->selectReagent( QModelIndex());
            return;
        }

        // expand all nodes in search since we want to see all reagents and batches that match the search criteria
        if ( !filter.isEmpty())
            this->view()->expandAll();

        if ( this->currentMatch == Id::Invalid || !this->matches.contains( this->currentMatch )) {
            // do this if:
            //   current match has not been set (first search)
            //   current match differs from the current filter
            this->currentMatch = this->matches.last();
            this->view()->selectReagent( this->view()->indexFromId( this->currentMatch ));
        } else {
            // the previous item is the same, reselect it
            this->view()->selectReagent( this->view()->indexFromId( this->currentMatch ));
        }
    } );

    // implement going through found entries
    this->ui->searchEdit->connect( this->ui->searchEdit, &QLineEdit::returnPressed, [ this ]() {
        // no filter - hide the search box
        if ( this->ui->searchEdit->text().isEmpty()) {
            this->ui->searchEdit->hide();
            return;
        }

        if ( this->matches.count() >= 2 ) {
            // advance or wrap around the match list
            int pos = this->matches.indexOf( this->currentMatch ) + 1;
            if ( pos > this->matches.count() - 1 )
                pos = 0;

            // set the new item
            this->currentMatch = this->matches.at( pos );
            this->view()->selectReagent( this->view()->indexFromId( this->currentMatch ));
        } else {
            // if there is just one item left, hide the search box
            this->ui->searchEdit->hide();
        }
    } );

    // add keyboard shortcut to reagent filter
    this->shortcut = new QShortcut( QKeySequence( this->tr( "Ctrl+F", "Find" )), this );
    this->shortcut->connect( this->shortcut, &QShortcut::activated, [ this ]() {
        if ( !this->ui->searchEdit->isVisible())
            this->on_buttonFind_clicked();
        else {
            this->ui->searchEdit->setText( "" );
            this->ui->searchEdit->hide();
        }
    } );
}

/**
 * @brief ReagentDock::~ReagentDock
 */
ReagentDock::~ReagentDock() {
    delete this->shortcut;
    delete ui;
}

/**
 * @brief ReagentDock::checkForDuplicates
 * @param name
 * @param alias
 * @return
 */
bool ReagentDock::checkForDuplicates(const QString &name, const QString &alias, const Id reagentId ) const {
    QSqlQuery query;

    // check alias for duplicates
    query.exec( QString( "select %1 from %2 where %1='%3' and %4!=%5" )
                .arg( Reagent::instance()->fieldName( Reagent::Alias ))
                .arg( Reagent::instance()->tableName())
                .arg( alias )
                .arg( Reagent::instance()->fieldName( Reagent::ID ))
                .arg( static_cast<int>( reagentId )));
    if ( query.next()) {
        QMessageBox::warning( ReagentDock::instance(), reagentId != Id::Invalid ? this->tr( "Cannot rename reagent" ) : this->tr( "Cannot add reagent" ), this->tr( "Alias '%1' already exists" ).arg( alias ));
        return false;
    }

    // check name for duplicates
    query.exec( QString( "select %1 from %2 where %1='%3' and %4!=%5" )
                .arg( Reagent::instance()->fieldName( Reagent::Name ))
                .arg( Reagent::instance()->tableName())
                .arg( name )
                .arg( Reagent::instance()->fieldName( Reagent::ID ))
                .arg( static_cast<int>( reagentId )));
    if ( query.next()) {
        QMessageBox::warning( ReagentDock::instance(), reagentId != Id::Invalid ? this->tr( "Cannot rename reagent" ) : this->tr( "Cannot add reagent" ), this->tr( "Reagent '%1' already exists" ).arg( name ));
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
 * @brief ReagentDock::on_reagentView_customContextMenuRequested
 * @param pos
 */
void ReagentDock::on_reagentView_customContextMenuRequested( const QPoint &pos ) {
    QMenu menu;

    auto addReagent = [ this ]( const Id &parentId ) {
        QString name, alias;
        bool ok;

        //
        // NOTE:
        //  reagents can only have unique names and aliases
        //  batches can be named anything and do not have aliases at all
        //
        if ( parentId != Id::Invalid ) {
            name = QInputDialog::getText( this, this->tr( "Add batch" ), this->tr( "Name:" ), QLineEdit::Normal, QString(), &ok );
        } else {
            ReagentDialog rd( this );
            ok = ( rd.exec() == QDialog::Accepted );
            name = rd.name();
            alias = rd.alias();

            if ( !this->checkForDuplicates( qAsConst( name ), qAsConst( alias )))
                return;
        }

        if ( ok ) {
            if ( !name.isEmpty()) {
                const Row row = Reagent::instance()->add( qAsConst( name ), qAsConst( alias ), parentId );
                if ( row == Row::Invalid )
                    return;

                // get reagentId and add to treeView without resetting the model
                const Id reagentId = Reagent::instance()->id( row );
                this->view()->model()->add( reagentId );

                // expand parent reagent
                if ( parentId != Id::Invalid )
                    this->view()->expand( this->view()->indexFromId( parentId ));

                // select the newly added reagent or batch
                this->view()->selectReagent( this->view()->indexFromId( reagentId ));

                // open extraction dialog if this feature is enabled
                if ( Variable::instance()->isEnabled( "fetchPropertiesOnAddition" ) && parentId == Id::Invalid ) {
                    ExtractionDialog ed( this, reagentId );
                    ed.exec();
                    PropertyDock::instance()->updateView();
                }
            } else {
                QMessageBox::warning( this,  this->tr( "Cannot add reagent" ), ( parentId != Id::Invalid ?  this->tr( "Batch" ) : this->tr( "Reagent" )) + this->tr( " name is empty" ));
                return;
            }
        }
    };

    menu.addAction( this->tr( "Add new reagent" ), std::bind( addReagent, Id::Invalid ));

    const QModelIndex index( this->view()->currentIndex());
    if ( index.isValid()) {
        const QStandardItem *item( this->view()->itemFromIndex( index ));
        const Id parentId = item->data( ReagentModel::ParentId ).value<Id>();
        const QString name(( parentId == Id::Invalid ) ? item->text() : item->parent()->text());

        menu.addAction( this->tr( "Add new batch to reagent \"%1\"" ).arg( name ), std::bind( addReagent, ( parentId == Id::Invalid ) ? static_cast<Id>( item->data( ReagentModel::ID ).toInt()) : parentId  ));
        menu.addAction( this->tr( "Copy name" ), [ item ]() { QGuiApplication::clipboard()->setText( item->text()); } );

        if ( parentId == Id::Invalid ) {
            QMenu *labels( menu.addMenu( this->tr( "Labels" )));
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
                    const_cast<QStandardItem*>( item )->setIcon( QIcon());
                } ));
                action->setCheckable( true );
                if ( hasLabel )
                    action->setChecked( true );
            }
        }
    }

    menu.exec( this->mapToGlobal( pos ));
}

/**
 * @brief ReagentDock::on_addButton_clicked
 */
void ReagentDock::on_addButton_clicked() {
    this->on_reagentView_customContextMenuRequested( this->ui->addButton->pos());
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
            foreach ( const QModelIndex &index, list ) {
                const QStandardItem *item( this->view()->itemFromIndex( index ));
                if ( item != nullptr )
                    removeReagentsAndBatches( item );
            }

            // remove items without resetting model
            this->view()->model()->remove( list );
        } );
    } else {
        // remove just one entry
        // get current index
        const QModelIndex index( this->view()->currentIndex());
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
                this->view()->selectReagent( this->view()->indexFromId( parentId ));
        } );
    }

    // display menu
    menu.exec( this->mapToGlobal( this->ui->removeButton->pos()));
}

/**
 * @brief ReagentDock::on_buttonFind_clicked
 */
void ReagentDock::on_buttonFind_clicked() {
    const bool visible = this->ui->searchEdit->isVisible();

    this->view()->nodeHistory()->setEnabled( visible );
    this->ui->searchEdit->setVisible( !visible );
    if ( !visible ) {
        this->ui->searchEdit->setFocus();
    } else {
        this->view()->updateView();
    }
}

/**
 * @brief ReagentDock::on_editButton_clicked
 */
void ReagentDock::on_editButton_clicked() {
    // get current index
    const QModelIndex index( this->view()->currentIndex());
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
            Reagent::instance()->setName( reagentRow, name );

            // rename without resetting the model
            const_cast<QStandardItem*>( item )->setText( ReagentModel::generateName( name ));
        }
    } else {
        ReagentDialog rd( this, previousName, previousAlias );
        ok = ( rd.exec() == QDialog::Accepted );
        const QString name( rd.name());
        const QString alias( rd.alias());

        if ( !this->checkForDuplicates( name, alias, reagentId ) || name.isEmpty() || alias.isEmpty())
            return;

        Reagent::instance()->setName( reagentRow, name );
        Reagent::instance()->setAlias( reagentRow, alias );

        // rename without resetting the model
        const_cast<QStandardItem*>( item )->setText( ReagentModel::generateName( name, alias ));
    }
}
