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
#include <QPaintEvent>
#include <QWidget>

/**
 * @brief The Character namespace
 */
namespace CharacterNamespace {
static const int GridSize = 16;
static const QString SpecialChars( "\u00b0;"
                                   "\u0391;\u0392;\u0393;\u0394;\u0395;\u0396;\u0397;\u0398;\u0399;\u039a;\u0399;\u039c;\u039d;\u039f;"
                                   "\u03a0;\u03a1;\u03a3;\u03a4;\u03a5;\u03a6;\u03a7;\u03a8;\u03a9;"
                                   "\u03b1;\u03b2;\u03b3;\u03b4;\u03b5;\u03b6;\u03b7;\u03b8;\u03b9;\u03ba;\u03bb;\u03bc;\u03bd;\u03bf;"
                                   "\u03c0;\u03c1;\u03c2;\u03c3;\u03c4;\u03c5;\u03c6;\u03c7;\u03c8;\u03c9;"
                                   "\u2190;\u2191;\u2192;\u2193;\u2194;\u219a;\u219b;\u21c4;\u21d0;\u21d2;"
                                   "\u212b;\u00b7;"
                                   "\u2620;\u2622;\u2623;\u269b;"
                                   "\u00b1;\u2213;\u221a;\u00d7;\u00f7;\u2243;\u2244;\u2245;\u2246;\u2247;\u2248;\u2249;\u2260;\u2261;\u221e;\u222b;\u2207;\u2205;\u0024;\u20ac;\u2030;\u26a0;\u25ba;\u25c4;\u25b2;\u25bc;\u23ea;\u23e9;\u23eb;"
                                   "\u2605;\u2606;\u2713;\u2717;\u2264;\u2265;" );//\u226a;\u226b" );
};

/**
 * @brief The CharacterMap class
 */
class CharacterMap : public QWidget {
    Q_OBJECT

public:
    explicit CharacterMap( QWidget *parent = nullptr );
    ~CharacterMap() { this->characters.clear(); }

protected:
    void paintEvent( QPaintEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );

signals:
    void characterSelected( const QString &character );

private:
    QStringList characters;
    QPair<int,int> blockId;
};
