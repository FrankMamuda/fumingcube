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
#include "ui_propertyeditor.h"
#include <QPainter>
#include <QDebug>
#include <QColorDialog>
#include <QFileDialog>
#include <QBitmap>

/**
 * @brief PropertyEditor::PropertyEditor
 * @param parent
 */
PropertyEditor::PropertyEditor( QWidget *parent, Modes m ) : QMainWindow( parent ), ui( new Ui::PropertyEditor ), activeEditor( nullptr ), mode( m ) {
    QFont font( "Times New Roman", 12 );

    // set up ui
    this->ui->setupUi( this );

    // set font toolbar below othher buttons
    this->insertToolBarBreak( this->ui->fontToolBar );

    /**
     * @brief formatChanged (lambda) changes ui elements (buttons, font selector, etc.) to match text format
     * @param format
     */
    auto formatChanged = [ this ]( const QTextCharFormat &format ) {
        this->fontChanged( format.font());
        this->colourChanged( format.foreground().color());
        this->ui->actionSubScript->setChecked( format.verticalAlignment() == QTextCharFormat::AlignSubScript );
        this->ui->actionSuperScript->setChecked( format.verticalAlignment() == QTextCharFormat::AlignSuperScript );
    };

    /**
     * @brief activeFormatChanged (lambda) changes ui elements according to active editor
     */
    auto activeFormatChanged = [ this, formatChanged ]() { formatChanged( this->activeEditor->currentCharFormat()); };

    // make sure to change ui elements on active editor switch
    this->connect( this->ui->title, &TextEdit::currentCharFormatChanged, formatChanged );
    this->connect( this->ui->value, &TextEdit::currentCharFormatChanged, formatChanged );

    // set default font for property value editor
    this->ui->value->setFont( font );

    /**
     * @brief flipFlop (lambda) toggles ui element visibility
     * @param enable
     */
    auto flipFlop = [ this ]( bool enable ) {
        this->ui->comboFont->setEnabled( enable );
        this->ui->comboSize->setEnabled( enable );
        this->ui->actionImage->setEnabled( enable );
        this->ui->actionColour->setEnabled( enable );
    };

    // actions performed upon entering property title editor
    this->connect( this->ui->title, &TextEdit::entered, [ this, activeFormatChanged, flipFlop, font ]() {
        this->activeEditor = this->ui->title;
        activeFormatChanged();
        flipFlop( false );

        // must always use default font (disallow other fonts)
        this->ui->title->setFont( font );
        this->fontChanged( this->ui->title->currentFont());
    } );

    // actions performed upon entering property value editor
    this->connect( this->ui->value, &TextEdit::entered, [ this, activeFormatChanged, flipFlop ]() {
        this->activeEditor = this->ui->value;
        activeFormatChanged();
        flipFlop( true );
    } );

    // focus on the title editor to begin with
    this->ui->title->setFocus();

    // bold text toggle lambda
    this->connect( this->ui->actionBold, &QAction::triggered, [ this ] () {
        QTextCharFormat format;
        format.setFontWeight( this->ui->actionBold->isChecked() ? QFont::Bold : QFont::Normal );
        this->mergeFormat( format );
    } );

    // italic text toggle lambda
    this->connect( this->ui->actionItalic, &QAction::triggered, [ this ] () {
        QTextCharFormat format;
        format.setFontItalic( this->ui->actionItalic->isChecked());
        this->mergeFormat( format );
    } );

    // underlined text toggle lambda
    this->connect( this->ui->actionUnderlined, &QAction::triggered, [ this ] () {
        QTextCharFormat format;
        format.setFontUnderline( this->ui->actionUnderlined->isChecked());
        this->mergeFormat( format );
    } );

    // subScript text toggle lambda
    this->ui->actionSubScript->setText( "\u25bc" );
    this->connect( this->ui->actionSubScript, &QAction::triggered, [ this ] () {
        QTextCharFormat format;
        format.setVerticalAlignment( !this->ui->actionSubScript->isChecked() ? QTextCharFormat::AlignNormal : QTextCharFormat::AlignSubScript );
        this->mergeFormat( format );
    } );

    // superScript text toggle lambda
    this->ui->actionSuperScript->setText( "\u25b2" );
    this->connect( this->ui->actionSuperScript, &QAction::triggered, [ this ] () {
        QTextCharFormat format;
        format.setVerticalAlignment( !this->ui->actionSuperScript->isChecked() ? QTextCharFormat::AlignNormal : QTextCharFormat::AlignSuperScript );
        this->mergeFormat( format );
    } );

    // font color picker lambda
    this->connect( this->ui->actionColour, &QAction::triggered, [ this ] () {
        QColor colour( QColorDialog::getColor( this->activeEditor->textColor(), this ));
        if ( !colour.isValid())
            return;

        QTextCharFormat format;
        format.setForeground( colour );
        this->mergeFormat( format );
        this->colourChanged( colour );
    });

    // font selector lambda
    this->connect( this->ui->comboFont, static_cast< void( QComboBox::* )( const QString & )>( &QComboBox::activated ), [ this ]( const QString &fontName ) {
        QTextCharFormat format;
        format.setFontFamily( fontName );
        this->mergeFormat( format );
    });

    // add font selector
    this->ui->fontToolBar->addWidget( this->ui->comboFont );

    // add font size comboBox
    this->ui->fontToolBar->addWidget( this->ui->comboSize );
    this->ui->comboSize->setEditable( true );

    // set up font sizes
    foreach ( int size, QFontDatabase::standardSizes())
        this->ui->comboSize->addItem( QString::number( size ));

    // set up size selector
    this->ui->comboSize->setCurrentIndex( QFontDatabase::standardSizes().indexOf( QApplication::font().pointSize()));
    this->connect( this->ui->comboSize, static_cast< void( QComboBox::* )( const QString & )>( &QComboBox::activated ), [ this ] ( const QString &pointSize ) {
        if ( pointSize.toFloat() > 0 ) {
            QTextCharFormat format;
            format.setFontPointSize( pointSize.toDouble());
            this->mergeFormat( format );
        }
    } );

    // add character map action
    this->ui->actionCharacterMap->setText( "\u212b" );
    this->connect( this->ui->actionCharacterMap, &QAction::triggered, [ this ]() {
        //this->characterMap->show();
    } );

    // set up image selector action
    this->connect( this->ui->actionImage, &QAction::triggered, [ this ]() {
        QString fileName( QFileDialog::getOpenFileName( this, this->tr( "Open Image" ), "", this->tr( "Images (*.png *.jpg)" )));
        QPixmap pixmap( fileName );

        if ( fileName.isEmpty())
            return;

        // load image
        if ( pixmap.load( fileName ))
            this->activeEditor->insertImage( pixmap );
    } );

    // disable title editor lambda
    this->connect( this->ui->comboCommon, static_cast< void( QComboBox::* )( int )>( &QComboBox::activated ), [ this ]( int index ) {
        this->ui->title->setEnabled( index == 0 );
    } );

    // resize title editor to minimum
    QFontMetrics fm( this->ui->title->document()->defaultFont());
    QMargins margins( this->ui->title->contentsMargins());
    this->ui->title->setFixedHeight( static_cast<int>( fm.lineSpacing() + margins.top() + margins.bottom() + ( this->ui->title->document()->documentMargin() + this->ui->title->frameWidth()) * 2 ));

    // connect button box
    this->connect( this->ui->buttonBox, &QDialogButtonBox::accepted, [ this ]() { this->close(); emit this->accepted( this->mode, this->ui->title->toHtml(), this->ui->value->toHtml()); } );
    this->connect( this->ui->buttonBox, &QDialogButtonBox::rejected, [ this ]() { this->close(); emit this->rejected(); } );

    // make sure title editor gets plainText from clipboard
    this->ui->title->setPastePlainText( true );
}

/**
 * @brief PropertyEditor::~PropertyEditor
 */
PropertyEditor::~PropertyEditor() {
    // disconnects
    delete ui;
}

/**
 * @brief PropertyEditor::setText
 * @param fileName
 * @return
 */
void PropertyEditor::setText( Editors editor, const QString &text ) {
    switch ( editor ) {
    case Title:
        this->ui->title->setHtml( text );
        break;

    case Value:
        this->ui->value->setHtml( text );
        break;

    case NoEditor:
        break;
    }
}

/**
 * @brief PropertyEditor::open
 * @param mode
 * @param title
 * @param value
 */
void PropertyEditor::open( PropertyEditor::Modes mode, const QString &title, const QString &value ) {
    this->mode = mode;

    if ( mode == Add ) {
        this->setText( Title, "" );
        this->setText( Value, "" );

        this->ui->comboCommon->setDisabled( false );
        this->ui->title->setDisabled( false );
    } else if ( mode == Edit ) {
        this->ui->comboCommon->setDisabled( true );
        this->ui->title->setDisabled( true );

        if ( !title.isEmpty())
            this->setText( Title, title );

        if ( !value.isEmpty())
            this->setText( Value, value );
    }

    this->show();
}

/**
 * @brief PropertyEditor::mergeFormat
 * @param format
 */
void PropertyEditor::mergeFormat( const QTextCharFormat &format ) {
    QTextCursor cursor( this->activeEditor->textCursor());

    if ( !cursor.hasSelection())
        cursor.select( QTextCursor::WordUnderCursor );

    cursor.mergeCharFormat( format );
    this->activeEditor->mergeCurrentCharFormat( format );
}

/**
 * @brief PropertyEditor::fontChanged
 * @param font
 */
void PropertyEditor::fontChanged( const QFont &font ) {
    this->ui->comboFont->setCurrentIndex( this->ui->comboFont->findText( QFontInfo( font ).family()));
    this->ui->comboSize->setCurrentIndex( this->ui->comboSize->findText( QString::number( font.pointSize())));
    this->ui->actionBold->setChecked( font.bold());
    this->ui->actionItalic->setChecked( font.italic());
    this->ui->actionUnderlined->setChecked( font.underline());
}

/**
 * @brief PropertyEditor::colourChanged
 * @param colour
 */
void PropertyEditor::colourChanged( const QColor &colour ) {
    QPixmap pixmap( ":/icons/colour" );
    QBitmap mask( pixmap.createMaskFromColor( Qt::transparent, Qt::MaskInColor ));

    pixmap.fill( colour );
    pixmap.setMask( mask );

    this->ui->actionColour->setIcon( QIcon( pixmap ));
}
