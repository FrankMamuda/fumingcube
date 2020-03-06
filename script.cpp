/*
 * Copyright (C) 2019 Factory #12
 * Copyright (C) 2019-2020 Armands Aleksejevs
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
#include "main.h"
#include "mainwindow.h"
#include "property.h"
#include "reagent.h"
#include "script.h"
#include "tag.h"
#include <QRegularExpression>
#include <QSqlQuery>
#include "htmlutils.h"
#include "variable.h"

/**
 * @brief Script::Script
 */
Script::Script() {
    // add to garbage collector
    // FIXME: this is broken
    //         pure virtual method called
    //GarbageMan::instance()->add( this );

    // add database related tables to the engine
    this->engine.globalObject().setProperty( "JS", this->engine.newQObject( this ));
}

/**
 * @brief Script::evaluate
 * @param script
 * @return
 */
QJSValue Script::evaluate( const QString &script ) {
    QJSValue result;

    // pre-process script to ensure no javascript keywords are used
    const QRegularExpression keywords(
            "\\b(abstract|arguments|await|boolean|break|byte|case|catch|char|class|const|continue|debugger|default|delete|do|double|else|enum|eval|export|extends|false|final|finally|float|for|function|goto|if|implements|import|in|instanceof|int|interface|let|long|native|new|null|package|private|protected|public|return|short|static|super|switch|synchronized|this|throw|throws|transient|true|try|typeof|var|void|volatile|while|with|yield|Array|Date|eval|function|hasOwnProperty|Infinity|isFinite|isNaN|isPrototypeOf|length|Math|NaN|name|Number|Object|prototype|String|toString|undefined|valueOf|alert|all|anchor|anchors|area|assign|blur|button|checkbox|clearInterval|clearTimeout|clientInformation|close|closed|confirm|constructor|crypto|decodeURI|decodeURIComponent|defaultStatus|document|element|elements|embed|embeds|encodeURI|encodeURIComponent|escape|event|fileUpload|focus|form|forms|frame|innerHeight|innerWidth|layer|layers|link|location|mimeTypes|navigate|navigator|frames|frameRate|hidden|history|image|images|offscreenBuffering|open|opener|option|outerHeight|outerWidth|packages|pageXOffset|pageYOffset|parent|parseFloat|parseInt|password|pkcs11|plugin|prompt|propertyIsEnum|radio|reset|screenX|screenY|scroll|secure|select|self|setInterval|setTimeout|status|submit|taint|text|textarea|top|unescape|untaint|window)\\b" );
    const QRegularExpressionMatch match( keywords.match( script ));
    if ( match.hasMatch())
        result = this->engine.newErrorObject( QJSValue::SyntaxError,
                                              Script::tr( "keyword \"%1\" is not allowed" ).arg( match.captured( 1 )));

    // unfortunately we have to do this every time unless we start caching tag functions
    // which also is painful, since we have to track each add/edit/remove
    // performance is a non-issue, so this can remain as-is for now
    // the other option (mapping globalObject properties via wrapper function is
    // also not the preferred way, since we also have to track tag updates)
    if ( !result.isError()) {
        const QStringList functions( Tag::instance()->getFunctionList());

        // do replacement magic:
        //  1) replace proto-functions with JS.getProperty( functionName, args, .. )
        //  2) replace comma decimal separator with a dot
        //  3) simplify string to remove trailing whitespace and newline
        const QString processed( QString( script ).replace(
                QRegularExpression( QString( R"((%1)\s*\(\s*\")" ).arg( functions.join( "|" ))),
                R"(JS.getProperty( "\1", ")" ).replace(
                QRegularExpression( R"((\d+),(\d+)(?=(?:[^"]|"[^"]*")*$))" ), "\\1.\\2" ).replace(
                QRegularExpression( R"((?<!")\b(ans)\b(?!"))" ), "JS.ans()" ).simplified());

        // evaluate script
        result = this->engine.evaluate( processed );

        // do some rounding up to avoid ugly numbers
        if ( result.isNumber())
            result = QString::number( result.toNumber(), 'g', 12 ).toDouble();
    }

    return result;
}

/**
 * @brief Script::ans
 * @return
 */
QJSValue Script::ans() {
    const QString answer( Variable::string( "calculator/ans" ));

    if ( answer.isEmpty()) {
        this->engine.throwError( QJSValue::EvalError, Script::tr( "answer is empty" ));
        return QJSValue();
    }

    return answer;
}

/**
 * @brief Script::getProperty
 * @param functionName
 * @param reference
 * @return
 */
QJSValue Script::getProperty( const QString &functionName, const QString &reference ) {
    // validate reagent reference
    if ( reference.isEmpty()) {
        this->engine.throwError( QJSValue::SyntaxError,
                                 Script::tr( "expected an argument for function \"%1\"" ).arg( functionName ));
        return QJSValue();
    }

    return this->getPropertyInternal( functionName, reference );
}

/**
 * @brief Script::getProperty
 * @param functionName
 * @param reference
 * @param batchName
 * @return
 */
QJSValue Script::getProperty( const QString &functionName, const QString &reference, const QString &batchName ) {
    // validate batchName
    if ( batchName.isEmpty()) {
        this->engine.throwError( QJSValue::SyntaxError,
                                 Script::tr( "expected arguments for function \"%1\"" ).arg( functionName ));
        return QJSValue();
    }

    return this->getPropertyInternal( functionName, reference, batchName );
}

/**
 * @brief Script::getPropertyInternal
 * @param functionName
 * @param reference
 * @param batchName
 * @return
 */
QJSValue
Script::getPropertyInternal( const QString &functionName, const QString &reference, const QString &batchName ) {
    // validate functionName
    if ( functionName.isEmpty()) {
        this->engine.throwError( QJSValue::SyntaxError, Script::tr( "function name expected" ));
        return QJSValue();
    }

    // get propertyId
    const Id propertyId = this->getPropertyId( functionName );
    if ( propertyId == Id::Invalid ) {
        this->engine.throwError( QJSValue::ReferenceError,
                                 Script::tr( "function \"%1\" is not defined" ).arg( functionName ));
        return QJSValue();
    }

    // get reagentId
    const Id reagentId = this->getReagentId( reference );
    if ( reagentId == Id::Invalid ) {
        this->engine.throwError( QJSValue::ReferenceError,
                                 Script::tr( "reagent \"%1\" is not defined" ).arg( reference ));
        return QJSValue();
    }

    QVariant propertyValue;

    // get batchId
    if ( !batchName.isEmpty()) {
        const Id batchId = this->getReagentId( batchName, reagentId );
        if ( batchId == Id::Invalid ) {
            this->engine.throwError( QJSValue::ReferenceError,
                                     Script::tr( "batch \"%1\" is not defined" ).arg( batchName ));
            return QJSValue();
        }
        propertyValue = this->getPropertyValue( propertyId, batchId, reagentId );
    } else {
        propertyValue = this->getPropertyValue( propertyId, reagentId );
    }

    // get property value
    if ( propertyValue.isNull()) {
        this->engine.throwError( QJSValue::TypeError,
                                 Script::tr( R"(property "%1" is not defined for "%2")" )
                                 .arg( functionName,
                                       batchName.isEmpty() ? reference : batchName ));
        return QJSValue();
    }

    // make sure it is a valid number
    bool ok;
    const qreal value = propertyValue.toReal( &ok );
    if ( !ok ) {
        this->engine.throwError( QJSValue::TypeError,
                                 Script::tr( "%1( %2 ) does not evaluate to a valid number" )
                                 .arg( functionName,
                                       reference ) +
                                       ( batchName.isEmpty() ? "" : QString( ", %1" ).arg( batchName )));
        return QJSValue();
    }

    // return scaled (for example assay is % or 0.01) value
    return value * Tag::instance()->scale( propertyId );
}

/**
 * @brief Script::getPropertyId
 * @param name
 * @return
 */
Id Script::getPropertyId( const QString &name ) const {
    QSqlQuery query;
    query.exec( QString( "select %1 from %2 where %3='%4'" )
                        .arg( Tag::instance()->fieldName( Tag::ID ),
                              Tag::instance()->tableName(),
                              Tag::instance()->fieldName( Tag::Function ),
                              name )
    );
    return query.next() ? query.value( 0 ).value<Id>() : Id::Invalid;
}

/**
 * @brief Script::getReagentId
 * @param reference
 * @param parentId
 * @return
 */
Id Script::getReagentId( const QString &reference, const Id &parentId ) const {
    // first pass (no rich text)
    QSqlQuery query;
    query.exec( QString( "select %1 from %2 where ( %3='%4' or %5='%4' ) and ( %6=%7 )" )
                        .arg( Reagent::instance()->fieldName( Reagent::ID ),
                              Reagent::instance()->tableName(),
                              Reagent::instance()->fieldName( Reagent::Reference ),
                              reference,
                              Reagent::instance()->fieldName( Reagent::Name ),
                              Reagent::instance()->fieldName( Reagent::ParentId ),
                              QString::number( static_cast<int>( parentId )))
    );

    if ( query.next())
        return query.value( 0 ).value<Id>();

    //
    // second pass (handles rich text)
    //
    // get all names and reference
    query.exec( QString( "select %1, %2, %3 from %4 where %5=%6" )
                        .arg( Reagent::instance()->fieldName( Reagent::ID ),
                              Reagent::instance()->fieldName( Reagent::Name ),
                              Reagent::instance()->fieldName( Reagent::Reference ),
                              Reagent::instance()->tableName(),
                              Reagent::instance()->fieldName( Reagent::ParentId ),
                              QString::number( static_cast<int>( parentId )))
    );

    // build a map of plainText names as keys and ids as values
    QMap<QString, Id> map;
    while ( query.next()) {
        map[HTMLUtils::convertToPlainText( query.value( 1 ).toString())] = query.value( 0 ).value<Id>();

        const QString ref( HTMLUtils::convertToPlainText( query.value( 2 ).toString()));
        if ( !ref.isEmpty())
            map[ref] = query.value( 0 ).value<Id>();
    }

    // compare plainText names
    if ( map.contains( reference ))
        return map[reference];

    // still nothing? return failure!
    return Id::Invalid;
}

/**
 * @brief Script::getPropertyValue
 * @param tagId
 * @param reagentId
 * @return
 */
QVariant Script::getPropertyValue( const Id &tagId, const Id &reagentId, const Id &parentId ) const {
    QSqlQuery query;
    query.exec( QString( "select %1 from %2 where ( %3=%4 and %5=%6 ) "
                         "or ( %3=%4 and %5=%7 and ( select count(*) from %2 where ( %3=%4 and %5=%6 )) = 0 )" )
                        .arg( Property::instance()->fieldName( Property::PropertyData ),// 1
                              Property::instance()->tableName(),                        // 2
                              Property::instance()->fieldName( Property::TagId ),       // 3
                              QString::number( static_cast<int>( tagId )),              // 4
                              Property::instance()->fieldName( Property::ReagentId ),   // 5
                              QString::number( static_cast<int>( reagentId )),          // 6
                              QString::number( static_cast<int>( parentId )))           // 7
    );

    return query.next() ? query.value( 0 ) : QVariant();
}
