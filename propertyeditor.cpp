/*
 * Copyright (C) 2017-2018 Factory #12
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
#include <QMenu>
#include "charactermap.h"

/**
 * @brief PropertyEditor::PropertyEditor
 * @param parent
 */
PropertyEditor::PropertyEditor( QWidget *parent, Modes m ) : QMainWindow( parent ), ui( new Ui::PropertyEditor ), activeEditor( nullptr ), mode( m ), characterMap( new CharacterMap( this )) {
    const QFont font( "Times New Roman", 11 );

    // set up ui
    this->ui->setupUi( this );

    // load pictograms
    // TODO: better yet handle these automatically (just like NFPA widget)
    this->pictograms["Harmful"] = QIcon( ":/pictograms/harmful" );
    this->pictograms["Flammable"] = QIcon( ":/pictograms/flammable" );
    this->pictograms["Toxic"] = QIcon( ":/pictograms/toxic" );
    this->pictograms["Corrosive"] = QIcon( ":/pictograms/corrosive" );
    this->pictograms["Environment hazard"] = QIcon( ":/pictograms/environment" );
    this->pictograms["Health hazard"] = QIcon( ":/pictograms/health" );
    this->pictograms["Explosive"] = QIcon( ":/pictograms/explosive" );
    this->pictograms["Oxidizing"] = QIcon( ":/pictograms/oxidizing" );
    this->pictograms["Compressed gas"] = QIcon( ":/pictograms/compressed" );

    // set font toolbar below other buttons
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
        this->ui->actionGHS->setEnabled( enable );
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
        this->mergeFormat( qAsConst( format ));
    } );

    // italic text toggle lambda
    this->connect( this->ui->actionItalic, &QAction::triggered, [ this ] () {
        QTextCharFormat format;
        format.setFontItalic( this->ui->actionItalic->isChecked());
        this->mergeFormat( qAsConst( format ));
    } );

    // underlined text toggle lambda
    this->connect( this->ui->actionUnderlined, &QAction::triggered, [ this ] () {
        QTextCharFormat format;
        format.setFontUnderline( this->ui->actionUnderlined->isChecked());
        this->mergeFormat( qAsConst( format ));
    } );

    // subScript text toggle lambda
    this->ui->actionSubScript->setText( "\u25bc" );
    this->connect( this->ui->actionSubScript, &QAction::triggered, [ this ] () {
        QTextCharFormat format;
        format.setVerticalAlignment( !this->ui->actionSubScript->isChecked() ? QTextCharFormat::AlignNormal : QTextCharFormat::AlignSubScript );
        this->mergeFormat( qAsConst( format ));
    } );

    // superScript text toggle lambda
    this->ui->actionSuperScript->setText( "\u25b2" );
    this->connect( this->ui->actionSuperScript, &QAction::triggered, [ this ] () {
        QTextCharFormat format;
        format.setVerticalAlignment( !this->ui->actionSuperScript->isChecked() ? QTextCharFormat::AlignNormal : QTextCharFormat::AlignSuperScript );
        this->mergeFormat( qAsConst( format ));
    } );

    // font color picker lambda
    this->connect( this->ui->actionColour, &QAction::triggered, [ this ] () {
        QColor colour( QColorDialog::getColor( this->activeEditor->textColor(), this ));
        if ( !colour.isValid())
            return;

        QTextCharFormat format;
        format.setForeground( colour );
        this->mergeFormat( qAsConst( format ));
        this->colourChanged( colour );
    });

    // font selector lambda
    this->connect( this->ui->comboFont, QOverload<const QString &>::of( &QComboBox::activated ), [ this ]( const QString &fontName ) {
        QTextCharFormat format;
        format.setFontFamily( fontName );
        this->mergeFormat( qAsConst( format ));
    });

    // add font selector
    this->ui->fontToolBar->addWidget( this->ui->comboFont );

    // add font size comboBox
    this->ui->fontToolBar->addWidget( this->ui->comboSize );
    this->ui->comboSize->setEditable( true );

    // set up font sizes
    foreach ( const int &size, QFontDatabase::standardSizes())
        this->ui->comboSize->addItem( QString::number( size ));

    // set up size selector
    this->ui->comboSize->setCurrentIndex( QFontDatabase::standardSizes().indexOf( QApplication::font().pointSize()));
    this->connect( this->ui->comboSize, QOverload<const QString &>::of( &QComboBox::activated ), [ this ] ( const QString &pointSize ) {
        if ( pointSize.toFloat() > 0 ) {
            QTextCharFormat format;
            format.setFontPointSize( pointSize.toDouble());
            this->mergeFormat( qAsConst( format ));
        }
    } );

    // add character map action
    this->ui->actionCharacterMap->setText( "\u212b" );
    this->connect( this->ui->actionCharacterMap, &QAction::triggered, [ this ]() {
        this->characterMap->show();
    } );

    // add character map action
    this->connect( this->characterMap, &CharacterMap::characterSelected, [ this ]( const QString &character ) {
        this->activeEditor->insertPlainText( character );
    } );

    // set up image selector action
    this->connect( this->ui->actionImage, &QAction::triggered, [ this ]() {
        const QString fileName( QFileDialog::getOpenFileName( this, this->tr( "Open Image" ), "", this->tr( "Images (*.png *.jpg)" )));

        if ( fileName.isEmpty())
            return;

        // load image
        const QPixmap pixmap( fileName );
        if ( !pixmap.isNull())
            this->activeEditor->insertPixmap( pixmap );
    } );

    // set up image selector action
    this->connect( this->ui->actionGHS, &QAction::triggered, [ this ]() {
        QMenu *menu( new QMenu());

        foreach ( const QString &ghs, this->pictograms.keys()) {
            const QIcon icon( this->pictograms[ghs] );
            menu->addAction( icon, ghs, [ this, icon ]() {
                const QPixmap pixmap( icon.pixmap( this->GHSPictogramScale, this->GHSPictogramScale ));
                if ( !pixmap.isNull())
                    this->activeEditor->insertPixmap( pixmap );
            } );
        }

        menu->exec( QCursor::pos());
        menu->deleteLater();
    } );

    // disable title editor lambda
    this->connect( this->ui->comboCommon, QOverload<int>::of( &QComboBox::activated ), [ this ]( int index ) {
        this->ui->title->setEnabled( index == 0 );
    } );

    // resize title editor to minimum
    const QFontMetrics fm( this->ui->title->document()->defaultFont());
    const QMargins margins( this->ui->title->contentsMargins());
    this->ui->title->setFixedHeight( static_cast<int>( fm.lineSpacing() + margins.top() + margins.bottom() + ( this->ui->title->document()->documentMargin() + this->ui->title->frameWidth()) * 2 ));

    // connect button box
    this->connect( this->ui->buttonBox, &QDialogButtonBox::accepted, [ this ]() { this->close(); emit this->accepted( this->mode, this->ui->title->toHtml(), this->ui->value->toHtml()); } );
    this->connect( this->ui->buttonBox, &QDialogButtonBox::rejected, [ this ]() { this->close(); emit this->rejected(); } );

    // connect actionCleanHTML
    this->connect( this->ui->actionCleanHTML, &QAction::toggled, [ this ]( bool enable ) {
        this->ui->value->setCleanHTML( enable );
    } );
    this->ui->actionCleanHTML->setChecked( true );

    // make sure title editor gets plain HTML from clipboard
    this->ui->title->setCleanHTML( true );

    // for now
    this->ui->comboCommon->hide();
}

/**
 * @brief PropertyEditor::~PropertyEditor
 */
PropertyEditor::~PropertyEditor() {
    // disconnects
    this->disconnect( this->ui->title, &TextEdit::currentCharFormatChanged, this, nullptr );
    this->disconnect( this->ui->value, &TextEdit::currentCharFormatChanged, this, nullptr );
    this->disconnect( this->ui->title, &TextEdit::entered, this, nullptr );
    this->disconnect( this->ui->value, &TextEdit::entered, this, nullptr );
    this->disconnect( this->ui->actionBold, &QAction::triggered, this, nullptr );
    this->disconnect( this->ui->actionItalic, &QAction::triggered, this, nullptr );
    this->disconnect( this->ui->actionUnderlined, &QAction::triggered, this, nullptr );
    this->disconnect( this->ui->actionSubScript, &QAction::triggered, this, nullptr );
    this->disconnect( this->ui->actionSuperScript, &QAction::triggered, this, nullptr );
    this->disconnect( this->ui->actionColour, &QAction::triggered, this, nullptr );
    this->disconnect( this->ui->comboFont, QOverload<const QString &>::of( &QComboBox::activated ), this, nullptr );
    this->disconnect( this->ui->comboSize, QOverload<const QString &>::of( &QComboBox::activated ), this, nullptr );
    this->disconnect( this->ui->actionCharacterMap, &QAction::triggered, this, nullptr );
    this->disconnect( this->characterMap, &CharacterMap::characterSelected, this, nullptr );
    this->disconnect( this->ui->actionImage, &QAction::triggered, this, nullptr );
    this->disconnect( this->ui->actionGHS, &QAction::triggered, this, nullptr );
    this->disconnect( this->ui->comboCommon, QOverload<int>::of( &QComboBox::activated ), this, nullptr );
    this->disconnect( this->ui->buttonBox, &QDialogButtonBox::accepted, this, nullptr );
    this->disconnect( this->ui->buttonBox, &QDialogButtonBox::rejected, this, nullptr );
    this->disconnect( this->ui->actionCleanHTML, &QAction::toggled, this, nullptr );

    delete this->characterMap;
    delete this->ui;
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
    const QBitmap mask( pixmap.createMaskFromColor( Qt::transparent, Qt::MaskInColor ));

    pixmap.fill( colour );
    pixmap.setMask( mask );

    this->ui->actionColour->setIcon( QIcon( qAsConst( pixmap )));
}

