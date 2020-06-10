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

    // FIXME: hide arrows for now
    //        order not implemented yet
    this->ui->upButton->hide();
    this->ui->downButton->hide();

    /*
     * saveState lambda
     */
    auto saveState = [ this ]( const Id &id ) {
        if ( id == Id::Invalid )
            return;

        for ( int y = 0; y < this->ui->tagWidget->rowCount(); y++ ) {
            const Id tagId = this->ui->tagWidget->item( y, 0 )->data( Qt::UserRole ).value<Id>();
            if ( tagId == Id::Invalid )
                continue;

            auto usedCheck = qobject_cast<QCheckBox*>( this->ui->tagWidget->cellWidget( y, 1 ));
            auto tabCheck = qobject_cast<QCheckBox*>( this->ui->tagWidget->cellWidget( y, 2 ));
            if ( usedCheck == nullptr || tabCheck == nullptr )
                continue;

            if ( !usedCheck->isChecked())
                continue;

            //qDebug() << "save" << id << tagId << tabCheck->isChecked();
            TableProperty::instance()->add( id, tagId, tabCheck->isChecked());
        }
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



                // NOTE: a proper way would be to check the modified values, but the easiest way is to
                //       delete all entries and re-add anew
                const Id id = TableEntry::instance()->id( row );
                if ( id == Id::Invalid )
                    return;

                QSqlQuery().exec( QString( "delete from %1 where %2=%3 )" )
                                          .arg( TableProperty::instance()->tableName(),
                                                TableProperty::instance()->fieldName( TableProperty::TableId ),
                                                QString::number( static_cast<int>( id ))));
                saveState( id );
            } else if ( this->mode() == Add ) {
                const Row row = TableEntry::instance()->add( this->ui->nameEdit->text());
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
    this->ui->tagWidget->clearContents();
    this->ui->tagWidget->setRowCount( Tag::instance()->count());

    QMap<Id, bool> settings;

    const QModelIndex index( this->ui->tableView->currentIndex());
    if ( index.isValid() && this->mode() == Edit ) {
        const Row row = TableEntry::instance()->row( index );
        if ( row != Row::Invalid ) {
            const Id id = TableEntry::instance()->id( row );
            if ( id != Id::Invalid ) {
                QSqlQuery query;
                query.exec( QString( "select %1, %2 from %3 where %4=%5" )
                            .arg( TableProperty::instance()->fieldName( TableProperty::TagId ),
                                  TableProperty::instance()->fieldName( TableProperty::Tab ),
                                  TableProperty::instance()->tableName(),
                                  TableProperty::instance()->fieldName( TableProperty::TableId ),
                                  QString::number( static_cast<int>( id ))));

                while ( query.next())
                    settings[query.value( 0 ).value<Id>()] = query.value( 1 ).toBool();
            }
        }
    }

    qDebug() << "restore" << settings;

    for ( int y = 0; y < Tag::instance()->count(); y++ ) {
        const Row tagRow = Tag::instance()->row( y );
        const Id tagId = Tag::instance()->id( tagRow );
        if ( tagId == Id::Invalid )
            continue;

        auto item = new QTableWidgetItem( Tag::instance()->name( tagRow ));
        item->setData( Qt::UserRole, static_cast<int>( tagId ));
        this->ui->tagWidget->setItem( y, 0, item );

        auto checkUsed = new QCheckBox();
        checkUsed->setStyleSheet( "margin-left:50%; margin-right:50%;" );
        checkUsed->setChecked( settings.contains( tagId ));
        this->ui->tagWidget->setCellWidget( y, 1, checkUsed );

        auto checkTabbed = new QCheckBox();
        checkTabbed->setStyleSheet( "margin-left:50%; margin-right:50%;" );
        if ( settings.contains( tagId ))
            checkTabbed->setChecked( settings[tagId] );

        if ( !checkUsed->isChecked())
            checkTabbed->setDisabled( true );

        QCheckBox::connect( checkUsed, &QCheckBox::toggled, this, [ checkTabbed ]( bool check ) {
            checkTabbed->setEnabled( check );
        } );

        this->ui->tagWidget->setCellWidget( y, 2, checkTabbed );
    }

    this->ui->tagWidget->resizeColumnsToContents();
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
