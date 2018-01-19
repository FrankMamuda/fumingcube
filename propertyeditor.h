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

#pragma once

//
// includes
//
#include <QMainWindow>
#include <QTextCharFormat>

//
// classes
//
class TextEdit;
class CharacterMap;

/**
 * @brief The Ui namespace
 */
namespace Ui {
class PropertyEditor;
}

/**
 * @brief The PropertyEditor class
 */
class PropertyEditor : public QMainWindow {
    Q_OBJECT
    Q_ENUMS( Editors )
    Q_ENUMS( Modes )
    Q_DISABLE_COPY( PropertyEditor )

public:
    enum Editors {
        NoEditor = -1,
        Title,
        Value
    };

    enum Modes {
        NoMode = -1,
        Add,
        Edit
    };

    explicit PropertyEditor( QWidget *parent = nullptr, Modes mode = NoMode );
    ~PropertyEditor();

public slots:
    void open( Modes mode, const QString &title = QString::null, const QString &value = QString::null );

signals:
    void accepted( Modes mode, const QString &title, const QString &value );
    void rejected();

private slots:
    void mergeFormat( const QTextCharFormat &format );
    void fontChanged( const QFont &font );
    void colourChanged( const QColor &colour );
    void setText( Editors editor, const QString &text );

private:
    Ui::PropertyEditor *ui;
    TextEdit *activeEditor;
    Modes mode;
    CharacterMap *characterMap;
};
