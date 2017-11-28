/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

/*
 * Copyright (C) 2017 Factory #12
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

//
// includes
//
#include "propertyeditor.h"
#include "textedit.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QToolBar>
#include <QTextCodec>
#include <QApplication>
#include <QPainter>

/**
 * @brief PropertyEditor::PropertyEditor
 * @param parent
 */
PropertyEditor::PropertyEditor( QWidget *parent ) : QMainWindow( parent ), textEdit( new TextEdit( this )), toolBar( this->addToolBar( this->tr( "Text format" ))), textFont( QFont( "Times New Roman" )), characterMap( new CharacterMap( this )) {
    // formatChanged lambda
    auto formatChanged = [ this ]( const QTextCharFormat &format ) {
        this->fontChanged( format.font());
        this->colourChanged( format.foreground().color());
        this->alignmentChanged( format.verticalAlignment());
    };
    this->connect( this->textEdit, &TextEdit::currentCharFormatChanged, formatChanged );

    // set up ui
    this->setCentralWidget( this->textEdit );
    this->toolBar->setMovable( false );
    this->toolBar->setFloatable( false );
    this->setupToolBarActions();
    this->textFont.setPointSize( 12 );
    this->textEdit->setFont( this->textFont );
    this->fontChanged( this->textEdit->font());
    this->colourChanged( this->textEdit->textColor());
    this->textEdit->setFocus();

    // connect character map
    this->connect( this->characterMap, &CharacterMap::characterSelected, [ this ]( const QString &character ) {
        this->textEdit->insertPlainText( character );
    } );

    //
    this->setWindowTitle( this->tr( "Property editor" ));
}

/**
 * @brief PropertyEditor::setupToolBarActions
 */
void PropertyEditor::setupToolBarActions() {
    QFont font( this->textFont );

    // set up font
    font.setPointSize( static_cast<int>( font.pointSize() * 1.25 ));

    // addAction lambda
    auto addAction = [ this ]( const QString &name, const QString &toolTip, const QString &icon, const QString &fallBack = QString::null ) {
        QAction *action( this->toolBar->addAction( QIcon::fromTheme( icon, QIcon( fallBack )), name ));
        action->setCheckable( true );
        action->setToolTip( toolTip );
        return action;
    };

    // bold
    this->actions[Bold] = addAction( "B", this->tr( "Set bold font" ), "format-text-bold" );
    QFont boldFont( font );
    boldFont.setBold( true );
    this->actions[Bold]->setFont( boldFont );
    this->actions[Bold]->connect( this->actions[Bold], &QAction::triggered, [ this ] () {
        QTextCharFormat fmt;
        fmt.setFontWeight( this->actions[Bold]->isChecked() ? QFont::Bold : QFont::Normal );
        this->mergeFormat( fmt );
    } );

    // italic
    this->actions[Italic] = addAction( "I", this->tr( "Set italic font" ), "format-text-italic" );
    QFont italicFont( font );
    italicFont.setItalic( true );
    this->actions[Italic]->setFont( italicFont );
    this->actions[Italic]->connect( this->actions[Italic], &QAction::triggered, [ this ] () {
        QTextCharFormat fmt;
        fmt.setFontItalic( this->actions[Italic]->isChecked());
        this->mergeFormat( fmt );
    } );

    // underlined
    this->actions[UnderLine] = addAction( "U", this->tr( "Set underlined font" ), "format-text-underline" );
    QFont underlineFont( font );
    underlineFont.setUnderline( true );
    this->actions[UnderLine]->setFont( underlineFont );
    this->actions[UnderLine]->connect( this->actions[UnderLine], &QAction::triggered, [ this ] () {
        QTextCharFormat fmt;
        fmt.setFontUnderline( this->actions[UnderLine]->isChecked());
        this->mergeFormat( fmt );
    } );

    // subscript
    this->actions[SubScript] = addAction( "\u25bc", this->tr( "Set subscript" ), "format-text-subscript" );
    this->actions[SubScript]->connect( this->actions[SubScript], &QAction::triggered, [ this ] () {
        QTextCharFormat fmt;
        fmt.setVerticalAlignment( !this->actions[SubScript]->isChecked() ? QTextCharFormat::AlignNormal : QTextCharFormat::AlignSubScript );
        this->mergeFormat( fmt );
    } );

    // superscript
    this->actions[SuperScript] = addAction( "\u25b2", this->tr( "Set superscript" ), "format-text-superscript" );
    this->actions[SuperScript]->connect( this->actions[SuperScript], &QAction::triggered, [ this ] () {
        QTextCharFormat fmt;
        fmt.setVerticalAlignment( !this->actions[SuperScript]->isChecked() ? QTextCharFormat::AlignNormal : QTextCharFormat::AlignSuperScript );
        this->mergeFormat( fmt );
    } );

    // font color
    this->actions[Colour] = this->toolBar->addAction( QPixmap(), "", [ this ]() {
        QColor colour( QColorDialog::getColor( this->textEdit->textColor(), this ));
        if ( !colour.isValid())
            return;

        QTextCharFormat fmt;
        fmt.setForeground( colour );
        this->mergeFormat( fmt );
        this->colourChanged( colour );
    });
    this->actions[Colour]->setToolTip( this->tr( "Set text colour" ));
    this->toolBar->addAction( this->actions[Colour] );

    // font
    this->comboFont = new QFontComboBox( this->toolBar );
    this->connect( this->comboFont, static_cast< void( QComboBox::* )( const QString & )>( &QComboBox::activated ), [ this ]( const QString &f ) {
        QTextCharFormat fmt;
        fmt.setFontFamily( f );
        this->mergeFormat( fmt );
    });
    this->toolBar->addWidget( this->comboFont );

    // font size
    this->comboSize = new QComboBox( this->toolBar );
    this->toolBar->addWidget( this->comboSize );
    this->comboSize->setEditable( true );

    foreach ( int size, QFontDatabase::standardSizes())
        this->comboSize->addItem( QString::number( size ));

    // set up size selector
    this->comboSize->setCurrentIndex( QFontDatabase::standardSizes().indexOf( QApplication::font().pointSize()));
    this->connect( this->comboSize, static_cast< void( QComboBox::* )( const QString & )>( &QComboBox::activated ), [ this ] ( const QString &pointSize ) {
        if ( pointSize.toFloat() > 0 ) {
            QTextCharFormat fmt;
            fmt.setFontPointSize( pointSize.toDouble());
            this->mergeFormat( fmt );
        }
    } );

    // character map
    this->actions[CharMap] = this->toolBar->addAction( "\u212b", [ this ]() {
        this->characterMap->show();
    } );
    font.setPointSize( static_cast<int>( font.pointSize() * 0.8 ));
    this->actions[CharMap]->setFont( font );
    this->actions[CharMap]->setToolTip( this->tr( "Insert special char" ));

    // image
    this->actions[Image] = this->toolBar->addAction( "Image", [ this ]() {
        QString fileName( QFileDialog::getOpenFileName( this, this->tr( "Open Image"), "", this->tr( "Images (*.png *.jpg)" )));
        QPixmap pixmap( fileName );

        if ( fileName.isEmpty())
            return;

        // load image
        if ( pixmap.load( fileName ))
            this->textEdit->insertImage( pixmap );
    } );
    font.setPointSize( static_cast<int>( font.pointSize() * 0.8 ));
    this->actions[Image]->setFont( font );
    this->actions[Image]->setToolTip( this->tr( "Insert image" ));
}

/**
 * @brief PropertyEditor::setText
 * @param fileName
 * @return
 */
void PropertyEditor::setText( const QString &text ) {
    this->textEdit->setHtml( text );
}

/**
 * @brief PropertyEditor::mergeFormat
 * @param format
 */
void PropertyEditor::mergeFormat( const QTextCharFormat &format ) {
    QTextCursor cursor( this->textEdit->textCursor());

    if ( !cursor.hasSelection())
        cursor.select( QTextCursor::WordUnderCursor );

    cursor.mergeCharFormat( format );
    this->textEdit->mergeCurrentCharFormat( format );
}

/**
 * @brief PropertyEditor::fontChanged
 * @param font
 */
void PropertyEditor::fontChanged( const QFont &font ) {
    this->comboFont->setCurrentIndex( this->comboFont->findText( QFontInfo( font ).family()));
    this->comboSize->setCurrentIndex( this->comboSize->findText( QString::number( font.pointSize())));
    this->actions[Bold]->setChecked( font.bold());
    this->actions[Italic]->setChecked( font.italic());
    this->actions[UnderLine]->setChecked( font.underline());
}

/**
 * @brief PropertyEditor::colourChanged
 * @param colour
 */
void PropertyEditor::colourChanged( const QColor &colour ) {
    QPixmap icon( 16, 16 );
    icon.fill( Qt::transparent );

    QPainter painter( &icon );
    painter.setRenderHint( QPainter::HighQualityAntialiasing );
    painter.setBrush( colour );
    painter.setPen( colour );
    painter.drawEllipse( 1, 1, 14, 14 );
    this->actions[Colour]->setIcon( icon );
}

/**
 * @brief PropertyEditor::alignmentChanged
 * @param alinment
 */
void PropertyEditor::alignmentChanged( QTextCharFormat::VerticalAlignment alignment ) {
    this->actions[SubScript]->setChecked( alignment == QTextCharFormat::AlignSubScript );
    this->actions[SuperScript]->setChecked( alignment == QTextCharFormat::AlignSuperScript );
}
