/*
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
#include "syntaxhighlighter.h"
#include <QRegularExpression>
#include "tag.h"
#include "variable.h"
#include <QSqlQuery>
#include <QApplication>
#include <utility>
#include "mainwindow.h"

/**
 * @brief SyntaxHighlighter::SyntaxHighlighter
 * @param parent
 */
SyntaxHighlighter::SyntaxHighlighter( QTextDocument *parent ) : QSyntaxHighlighter( parent ) {
    QSqlQuery query;

    query.exec( QString( "select %1, %2 from %3 where %2 not null" )
                        .arg( Tag::instance()->fieldName( Tag::ID ),
                              Tag::instance()->fieldName( Tag::Function ),
                              Tag::instance()->tableName()));
    while ( query.next())
        this->keywords << qAsConst( query ).value( 1 ).toString();
}

/**
 * @brief SyntaxHighlighter::highlightBlock
 * @param text
 */
void SyntaxHighlighter::highlightBlock( const QString &text ) {
    const Theme *theme( MainWindow::instance()->calcTheme());
    const QColor number( theme->syntaxColour( "Number" ));
    const QColor op( theme->syntaxColour( "Operator" ));
    const QColor comment( theme->syntaxColour( "Comment" ));
    const QColor parenthesis( theme->syntaxColour( "Parenthesis" ));
    const QColor keyword( theme->syntaxColour( "Keyword" ));
    const QColor reference( theme->syntaxColour( "Reference" ));
    const QColor error( theme->syntaxColour( "Error" ));
    const QColor undefined( theme->syntaxColour( "Undefined" ));
    const QColor string( theme->syntaxColour( "String" ));

    /**
     * @brief The SyntaxHighlighterOption struct
     */
    struct SyntaxHighlighterOption {
        SyntaxHighlighterOption(
                const QString &e,
                QColor c,
                bool b = false,
                bool u = false,
                bool i = false ) : expression( QRegularExpression( e )), colour( std::move( c )), bold( b ),
                                   underline( u ),
                                   italic( i ) {}
        QRegularExpression expression;
        QColor colour;
        bool bold;
        bool underline;
        bool italic;
    };

    // add types
    QList<SyntaxHighlighterOption> options = {
            { "\\w+",                          string },
            { "\\(|\\)",                       parenthesis, },
            { R"(\d+(\.|\,)?\d*)",             number,    true },
            { R"(\/|\*|\+|\-|\=|\,[^\w])",     op },
            { "\".+?(?=\")\"",                 reference, true },
            { "\\w+Error.+",                   error,     true, true },
            { R"(\bundefined\b(?!"))",         undefined },
            { "\\/\\/.+",                      comment }
    };

    // add keywords
    for ( const QString &k : qAsConst( this->keywords ))
        options << SyntaxHighlighterOption( QString( "%1(?!\")" ).arg( k ), keyword );

    // set options
    for ( const SyntaxHighlighterOption &option : qAsConst( options )) {
        QTextCharFormat format;

        if ( option.bold )
            format.setFontWeight( QFont::Bold );

        format.setFontItalic( option.italic );
        format.setFontUnderline( option.underline );
        format.setForeground( option.colour );

        QRegularExpressionMatchIterator matchIterator( option.expression.globalMatch( text ));
        while ( matchIterator.hasNext()) {
            const QRegularExpressionMatch match( matchIterator.next());
            this->setFormat( match.capturedStart(), match.capturedLength(), qAsConst( format ));
        }
    }

    // handle multiline comments
    const QRegularExpression startExpression( "/\\*" );
    const QRegularExpression endExpression( "\\*/" );

    this->setCurrentBlockState( 0 );
    int startIndex = 0;
    if ( this->previousBlockState() != 1 )
        startIndex = text.indexOf( startExpression );

    while ( startIndex >= 0 ) {
        QRegularExpressionMatch endMatch;
        const int endIndex = text.indexOf( endExpression, startIndex, &endMatch );
        int commentLength;
        if ( endIndex == -1 ) {
            this->setCurrentBlockState( 1 );
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        this->setFormat( startIndex, commentLength, comment );
        startIndex = text.indexOf( startExpression, startIndex + commentLength );
    }
}
