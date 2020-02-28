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
#include "htmlutils.h"
#include <QRegularExpression>

/**
 * @brief HTMLUtils::captureBody
 * @param input
 * @return
 */
QString HTMLUtils::captureBody( const QString &html ) {
    // TODO: add extended options to strip parts of html (strip images, strip tables, etc.)
    //       eventually merge with simplify and convertToPlainText
    const QString text( QString( html ).remove( "\n" ));
    const QRegularExpression re( "<body.+?(?=<p)<p.+?(?=>)>(.+?)(?=<\\/p)" );
    const QRegularExpressionMatch match = re.match( text );
    return ( match.hasMatch()) ? match.captured( 1 ).remove( "<br>" ).remove( "<br />" ).remove( "<br/>" ) : text;
}

/**
 * @brief HTMLUtils::convertToPlainText
 * @param input
 * @return
 */
QString HTMLUtils::convertToPlainText( const QString &html ) {
    // NOTE/TODO: since all plainText ops are routed through this method
    //            we could reduce overhead by writing more efficient code
    return QTextEdit( html ).toPlainText();
}

/**
 * @brief HTMLUtils::simplify
 * @param html
 * @return
 */
QString HTMLUtils::simplify( const QString &html ) {
    // TODO: don't strip tables as well?
    const QRegularExpression re( "((?:<\\/?(?:table|a|td|tr|tbody|li|ul|img).*?[>])|(?:<!--\\w+-->))" );
    return QString( html ).replace( re, "" );
}
