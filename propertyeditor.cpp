/*
 * Copyright (C) 2017-2018 Factory #12
 * Copyright (C) 2019 Armands Aleksejevs
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
#include <QFileDialog>
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
            const QKeyEvent *keyEvent( static_cast<QKeyEvent*>( event ));

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
PropertyEditor::PropertyEditor( QWidget *parent, Modes mode, const QString &name, const QString &value ) : QDialog( parent ), ui( new Ui::PropertyEditor ), activeEditor( nullptr ), mode( mode ), characterMap( new CharacterMap( this )) {
    // set up ui
    this->ui->setupUi( this );
    this->ui->mainWindow->setWindowFlags( Qt::Widget );

    QListWidget w;
    const QFont font( QApplication::font( &w ));

    // we don't currently need this
    this->ui->fontToolBar->hide();

    // fix tab issues
    this->ui->name->installEventFilter( this );
    this->ui->name->setSimpleEditor( true );

    // load pictograms for manual addition
    //   although these are handled automatically (just like NFPA widget)
    this->pictograms["Harmful"]            = QIcon( ":/pictograms/GHS07" );
    this->pictograms["Flammable"]          = QIcon( ":/pictograms/GHS02" );
    this->pictograms["Toxic"]              = QIcon( ":/pictograms/GHS06" );
    this->pictograms["Corrosive"]          = QIcon( ":/pictograms/GHS05" );
    this->pictograms["Environment hazard"] = QIcon( ":/pictograms/GHS09" );
    this->pictograms["Health hazard"]      = QIcon( ":/pictograms/GHS08" );
    this->pictograms["Explosive"]          = QIcon( ":/pictograms/GHS01" );
    this->pictograms["Oxidizing"]          = QIcon( ":/pictograms/GHS03" );
    this->pictograms["Compressed gas"]     = QIcon( ":/pictograms/GHS04" );

    // set font toolbar below other buttons
    this->ui->mainWindow->insertToolBarBreak( this->ui->fontToolBar );

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
    this->connect( this->ui->name, &TextEdit::currentCharFormatChanged, formatChanged );
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

    // actions performed upon entering property name editor
    this->connect( this->ui->name, &TextEdit::entered, [ this, activeFormatChanged, flipFlop, font ]() {
        this->activeEditor = this->ui->name;
        activeFormatChanged();
        flipFlop( false );

        // must always use default font (disallow other fonts)
        this->ui->name->setFont( font );
        this->fontChanged( this->ui->name->currentFont());
    } );

    // actions performed upon entering property value editor
    this->connect( this->ui->value, &TextEdit::entered, [ this, activeFormatChanged, flipFlop ]() {
        this->activeEditor = this->ui->value;
        activeFormatChanged();
        flipFlop( true );
    } );

    // focus on the name editor to begin with
    this->ui->name->setFocus();

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
#ifdef Q_CC_MSVC
    this->ui->actionSubScript->setText( "\xe2\x96\xbc" );
#else
    this->ui->actionSubScript->setText( "\u25bc" );
#endif
    this->connect( this->ui->actionSubScript, &QAction::triggered, [ this ] () {
        QTextCharFormat format;
        format.setVerticalAlignment( !this->ui->actionSubScript->isChecked() ? QTextCharFormat::AlignNormal : QTextCharFormat::AlignSubScript );
        this->mergeFormat( qAsConst( format ));
    } );

    // superScript text toggle lambda
#ifdef Q_CC_MSVC
    this->ui->actionSuperScript->setText( "\xe2\x96\xb2" );
#else
    this->ui->actionSuperScript->setText( "\u25b2" );
#endif
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
#ifdef Q_CC_MSVC
    this->ui->actionCharacterMap->setText( "\xe2\x84\xab" );
#else
    this->ui->actionCharacterMap->setText( "\u212b" );
#endif
    this->connect( this->ui->actionCharacterMap, &QAction::triggered, [ this ]() {

        // TODO: remove from private member?
        this->characterMap->exec();
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

    // resize name editor to minimum
    //const QFontMetrics fm( this->ui->name->document()->defaultFont());
    //const QMargins margins( this->ui->name->contentsMargins());
    //this->ui->name->setFixedHeight( static_cast<int>( fm.lineSpacing() + margins.top() + margins.bottom() + ( this->ui->name->document()->documentMargin() + this->ui->name->frameWidth()) * 2 ));
    // connect actionCleanHTML
    this->connect( this->ui->actionCleanHTML, &QAction::toggled, [ this ]( bool enable ) {
        this->ui->value->setCleanHTML( enable );
    } );
    this->ui->actionCleanHTML->setChecked( true );

    // make sure name editor gets plain HTML from clipboard
    this->ui->name->setCleanHTML( true );

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
    this->disconnect( this->ui->name, &TextEdit::currentCharFormatChanged, this, nullptr );
    this->disconnect( this->ui->value, &TextEdit::currentCharFormatChanged, this, nullptr );
    this->disconnect( this->ui->name, &TextEdit::entered, this, nullptr );
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

