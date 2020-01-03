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
#include "mainwindow.h"

/**
 * @brief SyntaxHighlighter::SyntaxHighlighter
 * @param parent
 */
SyntaxHighlighter::SyntaxHighlighter( QTextDocument *parent ) : QSyntaxHighlighter( parent ) {
    QSqlQuery query;

    query.exec( QString( "select %1, %2 from %3 where %2 not null" )
                .arg( Tag::instance()->fieldName( Tag::ID ))
                .arg( Tag::instance()->fieldName( Tag::Function ))
                .arg( Tag::instance()->tableName()));
    while ( query.next())
        this->keywords << qAsConst( query ).value( 1 ).toString();
}

/**
 * @brief SyntaxHighlighter::highlightBlock
 * @param text
 */
void SyntaxHighlighter::highlightBlock( const QString &text ) {
    const QColor number( MainWindow::instance()->theme()->syntaxColour( "Number" ));
    const QColor op( MainWindow::instance()->theme()->syntaxColour( "Operator" ));
    const QColor parenthesis( MainWindow::instance()->theme()->syntaxColour( "Parenthesis" ));
    const QColor keyword( MainWindow::instance()->theme()->syntaxColour( "Keyword" ));
    const QColor reference( MainWindow::instance()->theme()->syntaxColour( "Reference" ));
    const QColor error( MainWindow::instance()->theme()->syntaxColour( "Error" ));
    const QColor undefined( MainWindow::instance()->theme()->syntaxColour( "Undefined" ));
    const QColor string( MainWindow::instance()->theme()->syntaxColour( "String" ));

    /**
     * @brief The SyntaxHighlighterOption struct
     */
    struct SyntaxHighlighterOption {
        SyntaxHighlighterOption(
                    QString e,
                    QColor c,
                    bool b = false,
                    bool u = false,
                    bool i = false ) : expression( QRegularExpression( e )), colour( c ), bold( b ), underline( u ), italic( i ) {}
        QRegularExpression expression;
        QColor colour;
        bool bold;
        bool underline;
        bool italic;
    };

    // add types
    QList<SyntaxHighlighterOption> options = {
        { "\\w+",                           string                  },
        { "\\(|\\)",                        parenthesis,            },
        { "\\d+\\.?\\d*",                   number,     true        },
        { "\\/|\\*|\\+|\\-|\\=|\\,[^\\w]",  op                      },
        { "\".+?(?=\")\"",                  reference,  true        },
        { "\\w+Error.+",                    error,      true, true  },
        { "\\bundefined\\b(?!\")",          undefined               }
    };

    // add keywords
    foreach ( const QString &k, qAsConst( this->keywords ))
        options << SyntaxHighlighterOption( QString( "%1(?!\")" ).arg( k ), keyword );

    // set options
    foreach ( const SyntaxHighlighterOption &option, qAsConst( options )) {
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
}
