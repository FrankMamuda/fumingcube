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
#include <QPaintEvent>
#include <QWidget>

/**
 * @brief The Character namespace
 */
namespace CharacterNamespace {
static const int GridSize = 16;
#ifndef Q_CC_MSVC
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
#else
static const QString SpecialChars( "\xc2\xb0;\xce\x91;\xce\x92;\xce\x93;\xce\x94;\xce\x95;\xce\x96;\xce\x97;\xce\x98;\xce\x99;\xce\x9a;\xce\x99;\xce\x9c;\xce\x9d;\xce\x9f;\xce\xa0;\xce\xa1;\xce\xa3;\xce\xa4;\xce\xa5;\xce\xa6;\xce\xa7;\xce\xa8;\xce\xa9;\xce\xb1;\xce\xb2;\xce\xb3;\xce\xb4;\xce\xb5;\xce\xb6;\xce\xb7;\xce\xb8;\xce\xb9;\xce\xba;\xce\xbb;\xce\xbc;\xce\xbd;\xce\xbf;\xcf\x80;\xcf\x81;\xcf\x82;\xcf\x83;\xcf\x84;\xcf\x85;\xcf\x86;\xcf\x87;\xcf\x88;\xcf\x89;\xe2\x86\x90;\xe2\x86\x91;\xe2\x86\x92;\xe2\x86\x93;\xe2\x86\x94;\xe2\x86\x9a;\xe2\x86\x9b;\xe2\x87\x84;\xe2\x87\x90;\xe2\x87\x92;\xe2\x84\xab;\xc2\xb7;\xe2\x98\xa0;\xe2\x98\xa2;\xe2\x98\xa3;\xe2\x9a\x9b;\xc2\xb1;\xe2\x88\x93;\xe2\x88\x9a;\xc3\x97;\xc3\xb7;\xe2\x89\x83;\xe2\x89\x84;\xe2\x89\x85;\xe2\x89\x86;\xe2\x89\x87;\xe2\x89\x88;\xe2\x89\x89;\xe2\x89\xa0;\xe2\x89\xa1;\xe2\x88\x9e;\xe2\x88\xab;\xe2\x88\x87;\xe2\x88\x85;\x24;\xe2\x82\xac;\xe2\x80\xb0;\xe2\x9a\xa0;\xe2\x96\xba;\xe2\x97\x84;\xe2\x96\xb2;\xe2\x96\xbc;\xe2\x8f\xaa;\xe2\x8f\xa9;\xe2\x8f\xab;\xe2\x98\x85;\xe2\x98\x86;\xe2\x9c\x93;\xe2\x9c\x97;\xe2\x89\xa4;\xe2\x89\xa5;\xe2\x89\xaa;" );
#endif
};


/**
 * @brief The CharacterMap class
 */
class CharacterMap : public QWidget {
    Q_OBJECT

public:
    explicit CharacterMap( QWidget *parent = nullptr );
    ~CharacterMap() override { this->characters.clear(); }

protected:
    void paintEvent( QPaintEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;

signals:
    void characterSelected( const QString &character );

private:
    QStringList characters;
    QPair<int,int> blockId;
};
