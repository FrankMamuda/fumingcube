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
#include "tableentry.h"
#include "tabledialog.h"
#include "ui_tabledialog.h"
#include "tag.h"
#include "tableproperty.h"
#include "tableviewer.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QCheckBox>

//
// NOTE: this shares a lot of code with TagDialog class
//       might be a potential candidate for code reuse
//

// TODO: abort with empty names

/**
 * @brief TableDialog::TableDialog
 * @param parent
 */
TableDialog::TableDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::TableDialog ) {
    this->ui->setupUi( this );
    this->ui->widget->setWindowFlags( Qt::Widget );
    this->ui->tableView->setModel( TableEntry::instance());
    this->ui->tableView->setModelColumn( TableEntry::Name );
    this->ui->dockWidget->close();
    this->ui->dockWidget->installEventFilter( this );

    // make sure edit/view button is visible only when one item is selected
    // make sure remove button is invisible when no items are selected
    QItemSelectionModel::connect( this->ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [ this ]( const QItemSelection &, const QItemSelection & ) {
        const QModelIndexList list( this->ui->tableView->selectionModel()->selectedRows());
        this->ui->actionEdit->setDisabled( list.count() == 0 || list.count() > 1 );
        this->ui->actionRemove->setDisabled( list.count() == 0 );
        this->ui->actionView->setDisabled( list.count() == 0 || list.count() > 1 );
    } );

    const QPixmap pixmap( QIcon::fromTheme( "info" ).pixmap( 16, 16 ));
    this->ui->infoLabel->setPixmap( pixmap );

    /*
     * saveState lambda
     */
    auto saveState = [ this ]( const Id &id ) {
        if ( id == Id::Invalid )
            return;

        int order = 0;
        for ( int y = 0; y < this->ui->selectedWidget->count(); y++ ) {
            const Id tagId = this->ui->selectedWidget->item( y )->data( Qt::UserRole ).value<Id>();
            if ( tagId == Id::Invalid )
                continue;

            TableProperty::instance()->add( id, tagId, false, order );
            order++;
        }

        const QVariant currentData( this->ui->tabBox->currentData());
        if ( !currentData.isValid() || currentData.isNull())
            return;

        const Id tabTagId = currentData.value<Id>();
        if ( tabTagId == Id::Invalid )
            return;

        TableProperty::instance()->add( id, tabTagId, true );
    };

    // handle save button when edit dock is open
    QDialogButtonBox::connect( this->ui->buttonBox, &QDialogButtonBox::clicked, this, [ this, saveState ]( QAbstractButton *button ) {
        const QDialogButtonBox::ButtonRole role = this->ui->buttonBox->buttonRole( button );

        if ( role == QDialogButtonBox::AcceptRole ) {
            if ( this->mode() == Edit ) {
                const QModelIndex index( this->ui->tableView->currentIndex());
                if ( !index.isValid())
                    return;

                const Row row = TableEntry::instance()->row( index );
                if ( row == Row::Invalid )
                    return;

                TableEntry::instance()->setName( row, this->ui->nameEdit->text());
                TableEntry::instance()->setMode( row, static_cast<TableEntry::Modes>( this->ui->modeCombo->currentIndex()));

                // NOTE: a proper way would be to check the modified values, but the easiest way is to
                //       delete all entries and re-add anew
                const Id id = TableEntry::instance()->id( row );
                if ( id == Id::Invalid )
                    return;

                QSqlQuery().exec( QString( "delete from %1 where %2=%3" )
                                          .arg( TableProperty::instance()->tableName(),
                                                TableProperty::instance()->fieldName( TableProperty::TableId ),
                                                QString::number( static_cast<int>( id ))));
                saveState( id );
            } else if ( this->mode() == Add ) {
                const Row row = TableEntry::instance()->add( this->ui->nameEdit->text(), static_cast<TableEntry::Modes>( this->ui->modeCombo->currentIndex()));
                if ( row == Row::Invalid )
                    return;

                const Id id = TableEntry::instance()->id( row );
                saveState( id );
            }
        }

        this->ui->dockWidget->close();
        this->clear();
    } );
}

/**
 * @brief TableDialog::~TableDialog
 */
TableDialog::~TableDialog() {
    delete this->ui;
    // TODO: disconnects
}

/**
 * @brief TableDialog::on_actionAdd_triggered
 */
void TableDialog::on_actionAdd_triggered() {
    this->ui->dockWidget->show();
    //this->clear();
    this->setMode( Add );
    this->populate();
}

/**
 * @brief TableDialog::on_actionEdit_triggered
 */
void TableDialog::on_actionEdit_triggered() {
    const QModelIndex index( this->ui->tableView->currentIndex());
    if ( !index.isValid())
        return;

    const Row row = TableEntry::instance()->row( index );
    if ( row == Row::Invalid )
        return;

    this->ui->dockWidget->show();
    this->setMode( Edit );

    this->ui->nameEdit->setText( TableEntry::instance()->name( row ));
    this->ui->modeCombo->setCurrentIndex( static_cast<int>( TableEntry::instance()->mode( row )));

    //
    // TODO/STUB: populate tags
    //
    this->populate();
}

/**
 * @brief TableDialog::on_actionRemove_triggered
 */
void TableDialog::on_actionRemove_triggered() {
    const QModelIndexList indexes( this->ui->tableView->selectionModel()->selectedRows());
    auto removeTables = []( const QModelIndexList &indexList ) {
        QList<Id> idList;

        // must build an id list, because indexes/rows change on removal
        for ( const QModelIndex &index : indexList ) {
            if ( !index.isValid())
                continue;

            const Row row = TableEntry::instance()->row( index );
            if ( row == Row::Invalid )
                return;

            idList << TableEntry::instance()->id( row );
        }

        for ( const Id &id : qAsConst( idList ))
            TableEntry::instance()->remove( TableEntry::instance()->row( id ));

        // remove all table property orphans
        TableProperty::instance()->removeOrphanedEntries();
    };

    if ( indexes.count() == 1 ) {
        const QModelIndex index( this->ui->tableView->currentIndex());
        if ( !index.isValid())
            return;

        const Row row = TableEntry::instance()->row( index );
        if ( row == Row::Invalid )
            return;

        if ( QMessageBox::warning( this, TableDialog::tr( "Confirm removal" ), TableDialog::tr( "Remove '%1' table?" ).arg( TableEntry::instance()->name( row )), QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
            removeTables( QModelIndexList() << this->ui->tableView->currentIndex());
    } else if ( indexes.count() > 1 ) {
        if ( QMessageBox::warning( this, TableDialog::tr( "Confirm removal" ), TableDialog::tr( "Remove %1 tables?" ).arg( indexes.count()), QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
            removeTables( indexes );
    }
}

/**
 * @brief TableDialog::on_actionView_triggered
 */
void TableDialog::on_actionView_triggered() {
    const QModelIndex index( this->ui->tableView->currentIndex());
    if ( !index.isValid())
        return;

    const Row row = TableEntry::instance()->row( index );
    if ( row == Row::Invalid )
        return;

    const Id id = TableEntry::instance()->id( row );
    TableViewer( this, id ).exec();
}

/**
 * @brief TableDialog::populate
 */
void TableDialog::populate() {
    // get previous settings (selected tags) from the database (EDIT mode)
    QMap<Id, bool> settings;
    QList<Id> orderedList;
    const QModelIndex index( this->ui->tableView->currentIndex());
    if ( index.isValid() && this->mode() == Edit ) {
        const Row row = TableEntry::instance()->row( index );
        if ( row != Row::Invalid ) {
            const Id tableId = TableEntry::instance()->id( row );
            if ( tableId != Id::Invalid ) {

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
                    settings[id] = query.value( 1 ).toBool();
                    orderedList << id;
                }
            }
        }
    }

    // populate tag tabBox
    this->ui->tabBox->clear();
    this->ui->tabBox->addItem( TableDialog::tr( "None" ), -1 );

    // populate available tag table
    this->ui->tagWidget->clear();
    QMap<Id, int> comboIds;
    for ( int y = 0; y < Tag::instance()->count(); y++ ) {
        const Row tagRow = Tag::instance()->row( y );
        const Id tagId = Tag::instance()->id( tagRow );
        const QString tagName( Tag::instance()->name( tagRow )); // TODO: i18n
        const Tag::Types type( Tag::instance()->type( tagRow ));

        if ( tagId == Id::Invalid )
            continue;

        if ( type == Tag::Text || type == Tag::Integer || type == Tag::Real || type == Tag::Date ) {
            this->ui->tabBox->addItem( tagName, static_cast<int>( tagId ));
            comboIds[tagId] = this->ui->tabBox->count() - 1;
        }
        if ( settings.contains( tagId ))
            continue;

        // create a simple item and store tableId
        QListWidgetItem *item( new QListWidgetItem( tagName ));
        item->setData( Qt::UserRole, static_cast<int>( tagId ));
        this->ui->tagWidget->addItem( item );
    }

    // populate selected tag table
    this->ui->selectedWidget->clear();
    for ( const Id tagId : orderedList ) {
        if ( settings[tagId] && comboIds.contains( tagId )) {
            const int index = comboIds[tagId];
            this->ui->tabBox->setCurrentIndex( index );

            continue;
        }

        // create a simple item and store tableId
        QListWidgetItem *item( new QListWidgetItem( Tag::instance()->name( tagId )));
        item->setData( Qt::UserRole, static_cast<int>( tagId ));
        this->ui->selectedWidget->addItem( item );
    }
}

/**
 * @brief TableDialog::clear
 */
void TableDialog::clear() {
    this->ui->nameEdit->clear();

    // TODO: clear table
    this->setMode();
}

/**
 * @brief TableDialog::eventFilter
 * @param object
 * @param event
 * @return
 */
bool TableDialog::eventFilter( QObject *object, QEvent *event ) {
    if ( object == this->ui->dockWidget && ( event->type() == QEvent::Close || event->type() == QEvent::Show )) {
        this->ui->primaryToolBar->setEnabled( event->type() == QEvent::Close );
        this->ui->closeButton->setEnabled( event->type() == QEvent::Close );
        this->ui->tableView->setEnabled( event->type() == QEvent::Close );
        return true;
    }

    return QDialog::eventFilter( object, event );
}

/**
 * @brief TableDialog::on_tableView_doubleClicked
 * @param index
 */
void TableDialog::on_tableView_doubleClicked( const QModelIndex & ) {
    this->on_actionView_triggered();
}

/**
 * @brief TableDialog::on_tagWidget_itemDoubleClicked
 * @param item
 */
void TableDialog::on_tagWidget_itemDoubleClicked( QListWidgetItem *item ) {
    QListWidgetItem *taken( this->ui->tagWidget->takeItem( this->ui->tagWidget->row( item )));
    this->ui->selectedWidget->addItem( taken );
}

/**
 * @brief TableDialog::on_selectedWidget_itemDoubleClicked
 * @param item
 */
void TableDialog::on_selectedWidget_itemDoubleClicked( QListWidgetItem *item ) {
    QListWidgetItem *taken( this->ui->selectedWidget->takeItem( this->ui->selectedWidget->row( item )));
    this->ui->tagWidget->addItem( taken );
}
