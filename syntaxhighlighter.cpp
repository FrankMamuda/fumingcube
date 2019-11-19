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
#include <QSqlQuery>

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
        this->keywords << query.value( 1 ).toString();
}

/**
 * @brief SyntaxHighlighter::highlightBlock
 * @param text
 */
void SyntaxHighlighter::highlightBlock( const QString &text ) {
    {
        QTextCharFormat myClassFormat;
        myClassFormat.setFontWeight( QFont::Bold );
        myClassFormat.setForeground( QColor::fromRgb( 0, 0, 128 ));

        QRegularExpression expression("\\d");
        QRegularExpressionMatchIterator i = expression.globalMatch(text);
        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            this->setFormat(match.capturedStart(), match.capturedLength(), myClassFormat);
        }
    }

    {
        QTextCharFormat myClassFormat;
        //myClassFormat.setFontWeight(QFont::Bold);
        myClassFormat.setForeground(Qt::black);

        QRegularExpression expression("\\/|\\*|\\+|\\-");
        QRegularExpressionMatchIterator i = expression.globalMatch(text);
        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            setFormat(match.capturedStart(), match.capturedLength(), myClassFormat);
        }
    }

    {
        QTextCharFormat myClassFormat;
        //myClassFormat.setFontWeight(QFont::Bold);
        myClassFormat.setForeground( QColor::fromRgb( 0, 103, 124 ));

        for (const QString &keyword : qAsConst( this->keywords )) {
            QRegularExpressionMatchIterator matchIterator = QRegularExpression( keyword ).globalMatch( text );
            while ( matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), myClassFormat );
            }
        }
    }

    {
        QTextCharFormat myClassFormat;
        myClassFormat.setFontWeight( QFont::Bold );
        myClassFormat.setForeground( QColor::fromRgb( 0, 128, 0 ));

        QRegularExpression expression("\"[\\w\\s]+\"");
        QRegularExpressionMatchIterator i = expression.globalMatch(text);
        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            setFormat(match.capturedStart(), match.capturedLength(), myClassFormat);
        }
    }

    {
        QTextCharFormat myClassFormat;
        myClassFormat.setFontWeight( QFont::Bold );
        myClassFormat.setForeground( Qt::red );

        QRegularExpression expression("\\w+Error.+");
        QRegularExpressionMatchIterator i = expression.globalMatch(text);
        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            setFormat(match.capturedStart(), match.capturedLength(), myClassFormat);
        }
    }

    {
        QTextCharFormat myClassFormat;
        myClassFormat.setFontWeight( QFont::Bold );
        myClassFormat.setForeground( Qt::red );

        QRegularExpression expression("\\bReference\\s'.+'\\s.+");
        QRegularExpressionMatchIterator i = expression.globalMatch(text);
        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            setFormat(match.capturedStart(), match.capturedLength(), myClassFormat);
        }
    }
    {
        QTextCharFormat myClassFormat;
        //myClassFormat.setFontWeight( QFont::Bold );
        myClassFormat.setForeground( Qt::darkYellow );

        QRegularExpression expression("\\bundefined\\b");
        QRegularExpressionMatchIterator i = expression.globalMatch(text);
        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            setFormat(match.capturedStart(), match.capturedLength(), myClassFormat);
        }
    }
}
