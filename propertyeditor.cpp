/*
 * Copyright (C) 2017-2018 Factory #12
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
#include "propertyeditor.h"
#include "ui_propertyeditor.h"
#include <QPainter>
#include <QDebug>
#include <QColorDialog>
#include <QBitmap>
#include <QMenu>
#include <QStandardItem>
#include <QListWidgetItem>
#include "charactermap.h"

//
// TODO: store a reference to the font in HTML style=\" font-family:'##FONT##'; font-size:9pt; font-weight:400;
//       to be replaced on every system with native font
//

/**
 * @brief PropertyEditor::eventFilter
 * @param watched
 * @param event
 * @return
 */
bool PropertyEditor::eventFilter( QObject *object, QEvent *event ) {
    if ( object == this->ui->name ) {
        if ( event->type() == QEvent::KeyPress ) {
            const QKeyEvent *keyEvent( dynamic_cast<QKeyEvent *>( event ));

            if ( keyEvent->key() == Qt::Key_Tab ) {
                this->ui->value->setFocus();
                return true;
            }
        }
    }

    return QDialog::eventFilter( object, event );
}

/**
 * @brief PropertyEditor::name
 * @return
 */
QString PropertyEditor::name() const {
    return this->ui->name->toHtml();
}

/**
 * @brief PropertyEditor::value
 * @return
 */
QString PropertyEditor::value() const {
    return this->ui->value->toHtml();
}

/**
 * @brief PropertyEditor::PropertyEditor
 * @param parent
 */
PropertyEditor::PropertyEditor( QWidget *parent, Modes mode, const QString &name, const QString &value ) : QDialog(
        parent ), ui( new Ui::PropertyEditor ), mode( mode ) {
    // set up ui
    this->ui->setupUi( this );
    this->ui->mainWindow->setWindowFlags( Qt::Widget );

    // setup name editor and toolbar
    this->ui->name->installEventFilter( this );
    this->ui->name->setSimpleEditor( true );
    this->ui->name->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->name->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->nameToolBar->setEditor( this->ui->name );
    this->ui->nameToolBar->installFeature( EditorToolbar::Font );
    this->ui->nameToolBar->installFeature( EditorToolbar::VerticalAlignment );

    // set default font for property value editor
    const QListWidget w;
    const QFont font( QApplication::font( &w ));
    this->ui->value->setFont( font );
    this->ui->valueToolBar->setEditor( this->ui->value );
    this->ui->valueToolBar->installFeature( EditorToolbar::Font );
    this->ui->valueToolBar->installFeature( EditorToolbar::VerticalAlignment );
    this->ui->valueToolBar->installFeature( EditorToolbar::CharacterMap );
    this->ui->valueToolBar->installFeature( EditorToolbar::Image );
    this->ui->valueToolBar->installFeature( EditorToolbar::GHS );
    this->ui->valueToolBar->installFeature( EditorToolbar::Colour );
    this->ui->valueToolBar->installFeature( EditorToolbar::CleanHTML );

    // actions performed upon entering property name editor
    PropertyEditor::connect( this->ui->name, &TextEdit::entered, [ this ]() {
        this->ui->nameToolBar->show();
        this->ui->valueToolBar->hide();
    } );

    // actions performed upon entering property value editor
    PropertyEditor::connect( this->ui->value, &TextEdit::entered, [ this ]() {
        this->ui->valueToolBar->show();
        this->ui->nameToolBar->hide();
    } );

    // focus on the name editor to begin with
    this->ui->name->setFocus();

    // setup view
    if ( mode == Add ) {
        this->setText( Name, "" );
        this->setText( Value, "" );

        this->ui->name->setDisabled( false );
    } else if ( mode == Edit ) {
        this->ui->name->setDisabled( true );

        if ( !name.isEmpty())
            this->setText( Name, name );

        if ( !value.isEmpty())
            this->setText( Value, value );
    }
}

/**
 * @brief PropertyEditor::~PropertyEditor
 */
PropertyEditor::~PropertyEditor() {
    // disconnects
    PropertyEditor::disconnect( this->ui->name, &TextEdit::entered, this, nullptr );
    PropertyEditor::disconnect( this->ui->value, &TextEdit::entered, this, nullptr );

    delete this->ui;
}

/**
 * @brief PropertyEditor::setText
 * @param fileName
 * @return
 */
void PropertyEditor::setText( Editors editor, const QString &text ) {
    switch ( editor ) {
        case Name:
            this->ui->name->setHtml( text );
            break;

        case Value:
            this->ui->value->setHtml( text );
            break;

        case NoEditor:
            break;
    }
}

/**
 * @brief PropertyEditor::showEvent
 * @param event
 */
void PropertyEditor::showEvent( QShowEvent *event ) {
    QDialog::showEvent( event );
    this->ui->name->setMaximumHeight( this->ui->lineEdit->height());
    this->ui->lineEdit->hide();
    this->ui->nameToolBar->setMinimumHeight( this->ui->valueToolBar->height());
}


