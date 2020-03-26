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
#include "tagdialog.h"
#include "ui_tagdialog.h"
#include "tag.h"
#include <QMessageBox>
#include <QKeyEvent>
#include "htmlutils.h"
#include "property.h"

/**
 * @brief TagDialog::TagDialog
 * @param parent
 */
TagDialog::TagDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::TagDialog ) {
    this->ui->setupUi( this );
    this->ui->widget->setWindowFlags( Qt::Widget );

    // setup defaults
    this->ui->unitsEdit->setFixedHeight( this->ui->nameEdit->height());
    this->ui->tagView->setModel( Tag::instance());
    this->ui->tagView->setModelColumn( Tag::Name );
    this->ui->dockWidget->close();
    this->ui->dockWidget->installEventFilter( this );
    this->ui->unitsEdit->installEventFilter( this );
    this->ui->styleToolbar->hide();
    this->ui->styleToolbar->installFeature( EditorToolbar::VerticalAlignment );
    this->ui->styleToolbar->installFeature( EditorToolbar::CharacterMap );
    this->ui->styleToolbar->installFeature( EditorToolbar::Font );
    this->ui->styleToolbar->setEditor( this->ui->unitsEdit );
    this->ui->unitsEdit->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->unitsEdit->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->unitsEdit->setSimpleEditor( true );

    // set style toolbar below other buttons
    this->ui->widget->insertToolBarBreak( this->ui->styleToolbar );

    const QList<QWidget*> widgets( QList<QWidget*>() <<
                                   this->ui->unitsEdit <<
                                   this->ui->minEdit <<
                                   this->ui->maxEdit <<
                                   this->ui->valueEdit <<
                                   this->ui->precisionSpin <<
                                   this->ui->functionEdit <<
                                   this->ui->scaleSpin );

    // widget enabler/disabler
    auto widgetTest = [ this, widgets ]( int index ) {
        for ( QWidget *widget : widgets )
            widget->setDisabled( true );

        if ( index >= Tag::Text && index <= Tag::Real )
            this->ui->valueEdit->setEnabled( true );

        switch ( index ) {
        case Tag::NoType:
        case Tag::State:
            return;

        case Tag::PubChemId:
        case Tag::Integer:
        case Tag::Real:
            this->ui->nameEdit->setEnabled( true );
            this->ui->minEdit->setEnabled( true );
            this->ui->maxEdit->setEnabled( true );
            this->ui->scaleSpin->setEnabled( true );
            this->ui->valueEdit->setEnabled( true );
            this->ui->functionEdit->setEnabled( true );
            this->ui->unitsEdit->setEnabled( true );
            break;

        case Tag::GHS:
        case Tag::NFPA:
        case Tag::Text:
        case Tag::Formula:
            break;
        }

        if ( index == Tag::Real )
            this->ui->precisionSpin->setEnabled( true );
    };
    QComboBox::connect( this->ui->typeCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), widgetTest );
    widgetTest( this->ui->typeCombo->currentIndex());

    // make sure edit button is visible only when one item is selected
    // make sure remove button is invisible when no items are selected
    QItemSelectionModel::connect( this->ui->tagView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [ this ]( const QItemSelection &, const QItemSelection & ) {
        const QModelIndexList list( this->ui->tagView->selectionModel()->selectedRows());
        this->ui->actionEdit->setDisabled( list.count() == 0 || list.count() > 1 );
        this->ui->actionRemove->setDisabled( list.count() == 0 );
    } );

    // disable buttons by default
    this->ui->actionEdit->setDisabled( true );
    this->ui->actionRemove->setDisabled( true );

    // handle save button when edit dock is open
    QDialogButtonBox::connect( this->ui->buttonBox, &QDialogButtonBox::clicked, [ this ]( QAbstractButton *button ) {
        const QDialogButtonBox::ButtonRole role = this->ui->buttonBox->buttonRole( button );

        if ( role == QDialogButtonBox::AcceptRole ) {
            const QModelIndex index( this->ui->tagView->currentIndex());
            if ( !index.isValid())
                return;

            const Row row = Tag::instance()->row( index );
            if ( row == Row::Invalid )
                return;

            if ( this->mode() == Edit ) {
                Tag::instance()->setType( row, static_cast<Tag::Types>( this->ui->typeCombo->currentIndex()));
                Tag::instance()->setName( row, this->ui->nameEdit->text());
                Tag::instance()->setUnits( row, HTMLUtils::captureBody( this->ui->unitsEdit->toHtml()));
                Tag::instance()->setMinValue( row, this->ui->minEdit->text());
                Tag::instance()->setMaxValue( row, this->ui->maxEdit->text());
                Tag::instance()->setDefaultValue( row, this->ui->valueEdit->text());
                Tag::instance()->setPrecision( row, this->ui->precisionSpin->value());
                Tag::instance()->setFunction( row, this->ui->functionEdit->text());
                Tag::instance()->setScale( row, this->ui->scaleSpin->value());
                Tag::instance()->setScript( row, this->ui->patternEdit->text());

            } else if ( this->mode() == Add ) {
                Tag::instance()->add(
                            this->ui->nameEdit->text(),
                            static_cast<Tag::Types>( this->ui->typeCombo->currentIndex()),
                            HTMLUtils::captureBody( this->ui->unitsEdit->toHtml()),
                            this->ui->minEdit->text(),
                            this->ui->maxEdit->text(),
                            this->ui->valueEdit->text(),
                            this->ui->precisionSpin->value(),
                            this->ui->functionEdit->text(),
                            this->ui->scaleSpin->value(),
                            this->ui->patternEdit->text()
                            );
            }
        }

        this->ui->dockWidget->close();
        this->clear();
    } );
}

/**
 * @brief TagDialog::~TagDialog
 */
TagDialog::~TagDialog() {
    delete this->ui;
}

/**
 * @brief TagDialog::on_actionAdd_triggered
 */
void TagDialog::on_actionAdd_triggered() {
    this->ui->dockWidget->show();
    this->clear();
    this->setMode( Add );
}

/**
 * @brief TagDialog::on_actionRemove_triggered
 */
void TagDialog::on_actionRemove_triggered() {
    const QModelIndexList indexes( this->ui->tagView->selectionModel()->selectedRows());
    auto removeTags = []( const QModelIndexList &indexList ) {
        QList<Id> idList;

        // must build an id list, because indexes/rows change on removal
        for ( const QModelIndex &index : indexList ) {
            if ( !index.isValid())
                continue;

            const Row row = Tag::instance()->row( index );
            if ( row == Row::Invalid )
                return;

            idList << Tag::instance()->id( row );
        }

        for ( const Id &id : qAsConst( idList ))
            Tag::instance()->remove( Tag::instance()->row( id ));

        // remove all property orphans
        Property::instance()->removeOrphanedEntries();
    };

    if ( indexes.count() == 1 ) {
        const QModelIndex index( this->ui->tagView->currentIndex());
        if ( !index.isValid())
            return;

        const Row row = Tag::instance()->row( index );
        if ( row == Row::Invalid )
            return;

        if ( QMessageBox::warning( this, TagDialog::tr( "Confirm removal" ), TagDialog::tr( "Remove '%1' tag?\nThis will irreversibly remove all associated properties." ).arg( Tag::instance()->name( row )), QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
            removeTags( QModelIndexList() << this->ui->tagView->currentIndex());
    } else if ( indexes.count() > 1 ) {
        if ( QMessageBox::warning( this, TagDialog::tr( "Confirm removal" ), TagDialog::tr( "Remove %1 tags?\nThis will irreversibly remove all associated properties." ).arg( indexes.count()), QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
            removeTags( indexes );
    }
}

/**
 * @brief TagDialog::on_actionEdit_triggered
 */
void TagDialog::on_actionEdit_triggered() {
    const QModelIndex index( this->ui->tagView->currentIndex());
    if ( !index.isValid())
        return;

    const Row row = Tag::instance()->row( index );
    if ( row == Row::Invalid )
        return;

    this->ui->dockWidget->show();
    this->setMode( Edit );

    this->ui->typeCombo->setCurrentIndex( static_cast<int>( Tag::instance()->type( row )));
    this->ui->nameEdit->setText( Tag::instance()->name( row ));
    this->ui->unitsEdit->setHtml( HTMLUtils::captureBody( Tag::instance()->units( row )).replace( " ","&nbsp;" ));
    this->ui->minEdit->setText( Tag::instance()->minValue( row ).toString());
    this->ui->maxEdit->setText( Tag::instance()->maxValue( row ).toString());
    this->ui->valueEdit->setText( Tag::instance()->defaultValue( row ).toString());
    this->ui->precisionSpin->setValue( Tag::instance()->precision( row ));
    this->ui->functionEdit->setText( Tag::instance()->function( row ));
    this->ui->scaleSpin->setValue( Tag::instance()->scale( row ));
    this->ui->patternEdit->setText( Tag::instance()->script( row ).toString());
}

/**
 * @brief TagDialog::clear
 */
void TagDialog::clear() {
    this->ui->nameEdit->clear();
    this->ui->unitsEdit->clear();
    this->ui->minEdit->clear();
    this->ui->maxEdit->clear();
    this->ui->valueEdit->clear();
    this->ui->precisionSpin->setValue( 2 );
    this->ui->functionEdit->clear();
    this->ui->scaleSpin->setValue( 1.0 );
    this->ui->typeCombo->setCurrentIndex( 0 );
    this->ui->patternEdit->clear();

    this->setMode();
}

/**
 * @brief TagDialog::eventFilter
 * @param object
 * @param event
 * @return
 */
bool TagDialog::eventFilter( QObject *object, QEvent *event ) {
    if ( object == this->ui->dockWidget && ( event->type() == QEvent::Close || event->type() == QEvent::Show )) {
        this->ui->primaryToolBar->setEnabled( event->type() == QEvent::Close );
        this->ui->closeButton->setEnabled( event->type() == QEvent::Close );
        this->ui->tagView->setEnabled( event->type() == QEvent::Close );
        return true;
    }

    if ( object == this->ui->unitsEdit ) {
        if ( event->type() == QEvent::KeyPress ) {
            const QKeyEvent *keyEvent( dynamic_cast<QKeyEvent*>( event ));

            if ( keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Return )
                return true;
        } else if ( event->type() == QEvent::FocusIn ) {
            this->ui->styleToolbar->show();
        } else if ( event->type() == QEvent::FocusOut ) {
            this->ui->styleToolbar->hide();
        }
    }

    return QDialog::eventFilter( object, event );
}

