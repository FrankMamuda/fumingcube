/*
 * Copyright (C) 2020 Armands Aleksejevs
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
#include "charactermap.h"
#include "editortoolbar.h"
#include "ghspictograms.h"
#include <QBitmap>
#include <QColorDialog>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QDebug>
#include <QFileDialog>
#include <QMenu>

/**
 * @brief EditorToolbar::EditorToolbar
 */
EditorToolbar::EditorToolbar( QWidget *parent, TextEdit *editor ) : QToolBar( parent ) {
    this->setEditor( editor );
}

/**
 * @brief EditorToolbar::setEditor
 * @param editor
 */
void EditorToolbar::setEditor( TextEdit *editor ) {
    this->m_editor = editor;
    if ( this->editor() == nullptr )
        return;


    // make sure to change ui elements on active editor switch
    this->editor()->connect( this->editor(), SIGNAL( currentCharFormatChanged( QTextCharFormat )), this, SLOT( formatChanged( QTextCharFormat )) );
    this->formatChanged( this->editor()->currentCharFormat());
}

/**
 * @brief EditorToolbar::installFeature
 * @param feature
 */
void EditorToolbar::installFeature( const EditorToolbar::Feature &feature ) {
    this->m_features.setFlag( feature );

    switch ( feature ) {
    case Font:
    {
        // bold text toggle lambda
        this->actionBold = this->addAction( "B" );
        this->actionBold->setCheckable( true );
        QFont boldFont( this->actionBold->font());
        boldFont.setBold( true );
        this->actionBold->setFont( boldFont );
        this->connect( this->actionBold, &QAction::triggered, [ this ] () {
            QTextCharFormat format;
            format.setFontWeight( this->actionBold->isChecked() ? QFont::Bold : QFont::Normal );
            this->mergeFormat( qAsConst( format ));
        } );

        // italic text toggle
        this->actionItalic = this->addAction( "I" );
        this->actionItalic->setCheckable( true );
        QFont italicFont( this->actionItalic->font());
        italicFont.setItalic( true );
        this->actionItalic->setFont( italicFont );
        this->connect( this->actionItalic, &QAction::triggered, [ this ] () {
            QTextCharFormat format;
            format.setFontItalic( this->actionItalic->isChecked());
            this->mergeFormat( qAsConst( format ));
        } );

        // underlined text
        this->actionUnderlined = this->addAction( "U" );
        this->actionUnderlined->setCheckable( true );
        QFont underlinedFont( this->actionUnderlined->font());
        underlinedFont.setUnderline( true );
        this->actionUnderlined->setFont( underlinedFont );
        this->connect( this->actionUnderlined, &QAction::triggered, [ this ] () {
            QTextCharFormat format;
            format.setFontUnderline( this->actionUnderlined->isChecked());
            this->mergeFormat( qAsConst( format ));
        } );

    }
        break;

    case VerticalAlignment:
        // subscript
        this->actionSubScript = this->addAction( QString( "x" ) + QChar( 0x2082 ));
        this->actionSubScript->setCheckable( true );
        this->connect( this->actionSubScript, &QAction::triggered, [ this ] () {
            QTextCharFormat format;
            format.setVerticalAlignment( !this->actionSubScript->isChecked() ? QTextCharFormat::AlignNormal : QTextCharFormat::AlignSubScript );
            this->mergeFormat( qAsConst( format ));
        } );

        // superscript
        this->actionSuperScript = this->addAction( QString( "x" ) + QChar( 0x00B2 ));
        this->actionSuperScript->setCheckable( true );
        this->connect( this->actionSuperScript, &QAction::triggered, [ this ] () {
            QTextCharFormat format;
            format.setVerticalAlignment( !this->actionSuperScript->isChecked() ? QTextCharFormat::AlignNormal : QTextCharFormat::AlignSuperScript );
            this->mergeFormat( qAsConst( format ));
        } );
        break;

    case Colour:
        // font color picker
        this->actionColour = this->addAction( "" );
        this->actionColour->setIcon( QIcon::fromTheme( "colour" ).pixmap( 16, 16 ));
        this->connect( this->actionColour, &QAction::triggered, [ this ] () {
            QColor colour( QColorDialog::getColor( this->editor()->textColor(), this ));
            if ( !colour.isValid())
                return;

            QTextCharFormat format;
            format.setForeground( colour );
            this->mergeFormat( qAsConst( format ));
            this->colourChanged( colour );
        } );
        break;

    case CharacterMap:
        // add character map action
        this->actionCharacterMap = this->addAction( QChar( 0x212b ));
        this->connect( this->actionCharacterMap, &QAction::triggered, [ this ]() {
            class CharacterMap cm( this );

            // add character map action
            this->connect( &cm, &CharacterMap::characterSelected, [ this ]( const QString &character ) {
                this->editor()->insertPlainText( character );
            } );

            cm.exec();
        } );
        break;

    case Image:
        // set up image selector action
        this->actionImage = this->addAction( "" );
        this->actionImage->setIcon( QIcon::fromTheme( "image" ));
        this->connect( this->actionImage, &QAction::triggered, [ this ]() {
            const QString fileName( QFileDialog::getOpenFileName( this, this->tr( "Open Image" ), "", this->tr( "Images (*.png *.jpg)" )));

            if ( fileName.isEmpty())
                return;

            // load image
            const QPixmap pixmap( fileName );
            if ( !pixmap.isNull())
                this->editor()->insertPixmap( pixmap );
        } );
        break;

    case GHS:
        // set up GHS selector action
        this->actionGHS = this->addAction( "" );
        this->actionGHS->setIcon( QIcon::fromTheme( "hazard" ));
        this->connect( this->actionGHS, &QAction::triggered, [ this ]() {
            QMenu *menu( new QMenu());

            foreach ( const QString &key, GHSHazards::Hazards.keys()) {
                const QIcon icon( GHSPictograms::icon( key ));
                menu->addAction( icon, GHSHazards::Hazards[key], [ this, key ]() {
                    const QPixmap pixmap( GHSPictograms::pixmap( key, 48 ));
                    if ( !pixmap.isNull())
                        this->editor()->insertPixmap( pixmap );
                } );
            }

            menu->exec( QCursor::pos());
            menu->deleteLater();
        } );
        break;

    case CleanHTML:
        this->actionCleanHTML = this->addAction( "" );
        this->actionCleanHTML->setIcon( QIcon::fromTheme( "clean" ));
        this->connect( this->actionCleanHTML, &QAction::toggled, [ this ]( bool enable ) {
            this->editor()->setCleanHTML( enable );
        } );

        if ( this->editor() != nullptr )
            this->actionCleanHTML->setChecked( true );

        break;

    case NoFeatures:
        break;
    }
}

/**
 * @brief EditorToolbar::colourChanged
 * @param colour
 */
void EditorToolbar::colourChanged( const QColor &colour ) {
    QPixmap pixmap( QIcon::fromTheme( "colour" ).pixmap( 16, 16 ));
    const QBitmap mask( pixmap.createMaskFromColor( Qt::transparent, Qt::MaskInColor ));

    pixmap.fill( colour );
    pixmap.setMask( mask );

    this->actionColour->setIcon( QIcon( qAsConst( pixmap )));
}

/**
 * @brief EditorToolbar::formatChanged
 * @param format
 */
void EditorToolbar::formatChanged( const QTextCharFormat &format ) {
    if ( this->features().testFlag( Font )) {
        this->actionBold->setChecked( format.font().bold());
        this->actionItalic->setChecked( format.font().italic());
        this->actionUnderlined->setChecked( format.font().underline());
    }

    if ( this->features().testFlag( Colour ))
        this->colourChanged( format.foreground().color());

    if ( this->features().testFlag( VerticalAlignment )) {
        this->actionSubScript->setChecked( format.verticalAlignment() == QTextCharFormat::AlignSubScript );
        this->actionSuperScript->setChecked( format.verticalAlignment() == QTextCharFormat::AlignSuperScript );
    }
}

/**
 * @brief EditorToolbar::mergeFormat
 * @param format
 */
void EditorToolbar::mergeFormat(const QTextCharFormat &format) {
    if ( this->editor() == nullptr )
        return;

    QTextCursor cursor( this->editor()->textCursor());

    if ( !cursor.hasSelection())
        cursor.select( QTextCursor::WordUnderCursor );

    cursor.mergeCharFormat( format );
    this->editor()->mergeCurrentCharFormat( format );
}
