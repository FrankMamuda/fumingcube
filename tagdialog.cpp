/*
 * Copyright (C) 2017-2018 Factory #12
 * Copyright (C) 2013-2019 Armands Aleksejevs
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
#include <QDebug>

/**
 * @brief TagDialog::TagDialog
 * @param parent
 */
TagDialog::TagDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::TagDialog ) {
    this->ui->setupUi( this );
    this->ui->widget->setWindowFlags( Qt::Widget );
    this->ui->unitsEdit->setFixedHeight( this->ui->nameEdit->height());
    this->ui->tagView->setModel( Tag::instance());
    this->ui->tagView->setModelColumn( Tag::Name );
    this->ui->dockWidget->close();
    this->ui->dockWidget->installEventFilter( this );

    const QList<QWidget*> widgets( QList<QWidget*>() <<
                                   this->ui->unitsEdit <<
                                   this->ui->minEdit <<
                                   this->ui->maxEdit <<
                                   this->ui->valueEdit <<
                                   this->ui->precisionSpin <<
                                   this->ui->functionEdit <<
                                   this->ui->scaleSpin );
    auto widgetText = [ this, widgets ]( int index ) {
        foreach ( QWidget *widget, widgets )
            widget->setDisabled( true );

        if ( index >= Tag::Text && index <= Tag::Real )
            this->ui->valueEdit->setEnabled( true );

        switch ( index ) {
        case Tag::NoType:
        case Tag::State:
            return;

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
            break;
        }

        if ( index == Tag::Real )
            this->ui->precisionSpin->setEnabled( true );
    };
    this->ui->typeCombo->connect( this->ui->typeCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), widgetText );
    widgetText( this->ui->typeCombo->currentIndex());

    this->ui->buttonBox->connect( this->ui->buttonBox, &QDialogButtonBox::clicked, [ this ]( QAbstractButton *button ) {
        qDebug() << "clicked on" << this->ui->buttonBox->buttonRole( button );
        this->ui->dockWidget->close();

        // CLEAR EVERYTHING AFTER addition/edit
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

    // CLEAR EVERYTHING

}

/**
 * @brief TagDialog::on_actionRemove_triggered
 */
void TagDialog::on_actionRemove_triggered() {

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
    this->ui->typeCombo->setCurrentIndex( static_cast<int>( Tag::instance()->type( row )));

    this->ui->nameEdit->setText( Tag::instance()->name( row ));
    this->ui->unitsEdit->setHtml( Tag::instance()->units( row ));
    this->ui->minEdit->setText( Tag::instance()->min( row ).toString());
    this->ui->maxEdit->setText( Tag::instance()->max( row ).toString());
    this->ui->valueEdit->setText( Tag::instance()->defaultValue( row ).toString());
    this->ui->precisionSpin->setValue( Tag::instance()->precison( row ));
    this->ui->functionEdit->setText( Tag::instance()->function( row ));
    this->ui->scaleSpin->setValue( Tag::instance()->scale( row ));
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

    return QDialog::eventFilter( object, event );
}

