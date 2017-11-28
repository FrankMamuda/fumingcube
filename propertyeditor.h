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

#pragma once

//
// includes
//
#include "charactermap.h"
#include <QFontComboBox>
#include <QMainWindow>
#include <QTextCharFormat>

//
// class
//
class TextEdit;

/**
 * @brief The PropertyEditor class
 */
class PropertyEditor : public QMainWindow {
    Q_OBJECT
    Q_ENUMS( Actions )

public:
    enum Actions {
        Bold = 0,
        UnderLine,
        Italic,
        SubScript,
        SuperScript,
        Colour,
        CharMap,
        Image
    };
    PropertyEditor( QWidget *parent = 0 );
    ~PropertyEditor() { delete this->characterMap; }
    void setText( const QString &text );

private slots:
    void setupToolBarActions();
    void mergeFormat( const QTextCharFormat &format );
    void fontChanged( const QFont &font );
    void colourChanged( const QColor &colour );
    void alignmentChanged( QTextCharFormat::VerticalAlignment alignment );

protected:
    void dropEvent( QDropEvent *event ) { event->accept(); }

private:
    QMap<Actions, QAction*> actions;
    QFontComboBox *comboFont;
    QComboBox *comboSize;
    TextEdit *textEdit;
    QToolBar *toolBar;
    QFont textFont;
    CharacterMap *characterMap;
};
