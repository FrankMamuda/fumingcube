/*
 * Copyright (C) 2019 Factory #12
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
#include "mainwindow.h"
#include "reagent.h"
#include "script.h"
#include "tag.h"
#include <QDebug>
#include <QRegularExpression>
#include <QSqlQuery>

/**
 * @brief Script::Script
 */
Script::Script() {
    // add database related tables to the engine
    this->engine.globalObject().setProperty( "JS", this->engine.newQObject( this ));
    // TODO: IMPLEMENT ans (history), Avogadro constant, etc.
}

/**
 * @brief Script::evaluate
 * @param script
 * @return
 */
QJSValue Script::evaluate( const QString &script ) {
    QJSValue result;

    // pre-process script to ensure no javascript keywords are used
    const QRegularExpression keywords( "\\b(abstract|arguments|await|boolean|break|byte|case|catch|char|class|const|continue|debugger|default|delete|do|double|else|enum|eval|export|extends|false|final|finally|float|for|function|goto|if|implements|import|in|instanceof|int|interface|let|long|native|new|null|package|private|protected|public|return|short|static|super|switch|synchronized|this|throw|throws|transient|true|try|typeof|var|void|volatile|while|with|yield|Array|Date|eval|function|hasOwnProperty|Infinity|isFinite|isNaN|isPrototypeOf|length|Math|NaN|name|Number|Object|prototype|String|toString|undefined|valueOf|alert|all|anchor|anchors|area|assign|blur|button|checkbox|clearInterval|clearTimeout|clientInformation|close|closed|confirm|constructor|crypto|decodeURI|decodeURIComponent|defaultStatus|document|element|elements|embed|embeds|encodeURI|encodeURIComponent|escape|event|fileUpload|focus|form|forms|frame|innerHeight|innerWidth|layer|layers|link|location|mimeTypes|navigate|navigator|frames|frameRate|hidden|history|image|images|offscreenBuffering|open|opener|option|outerHeight|outerWidth|packages|pageXOffset|pageYOffset|parent|parseFloat|parseInt|password|pkcs11|plugin|prompt|propertyIsEnum|radio|reset|screenX|screenY|scroll|secure|select|self|setInterval|setTimeout|status|submit|taint|text|textarea|top|unescape|untaint|window)\\b" );
    const QRegularExpressionMatch match( keywords.match( script ));
    if ( match.hasMatch())
        result = this->engine.newErrorObject( QJSValue::SyntaxError, this->tr( "keyword \"%1\" is not allowed" ).arg( match.captured( 1 )));

    // unfortunately we have to do this every time unless we start caching tag functions
    // which also is painful, since we have to track each add/edit/remove
    // performance is a non-issue, so this can remain as-is for now
    // the other option (mapping globalObject properties via wrapper function is
    // also not the preferred way, since we also have to track tag updates)
    if ( !result.isError()) {
        QStringList functions;
        QSqlQuery query;
        query.exec( QString( "select %1 from %2 where %1 not null" ).arg( Tag::instance()->fieldName( Tag::Function )) .arg( Tag::instance()->tableName()));
        while ( query.next())
            functions << query.value( 0 ).toString();

        // do replacement magic:
        //  1) replace proto-functions with JS.getProperty( functionName, args, .. )
        //  2) replace comma decimal separator with a dot
        //  3) simplify string to remove trailing whitespace and newline
        const QString processed( QString( script ).replace( QRegularExpression( QString( "(%1)\\s*\\(").arg( functions.join( "|" ))), "JS.getProperty( \"\\1\"," ).replace( QRegularExpression("(\\d+),(\\d+)"),"\\1.\\2" ).simplified());

        // evalute script
        result = this->engine.evaluate( processed );
    }

    return result;
}

/**
 * @brief Script::getProperty
 * @param tag
 * @return
 */
QJSValue Script::getProperty( const QString &functionName, const QString &reagentAlias ) {
    // validate reagentAlias
    if ( reagentAlias.isEmpty()) {
        this->engine.throwError( QJSValue::SyntaxError, this->tr( "expected an argument for function \"%1\"" ).arg( functionName ));
        return QJSValue();
    }

    return this->getPropertyInternal( functionName, reagentAlias );
}

/**
 * @brief Script::getProperty
 * @param tag
 * @param alias
 * @param batch
 * @return
 */
QJSValue Script::getProperty( const QString &functionName, const QString &reagentAlias, const QString &batchName ) {
    // validate batchName
    if ( batchName.isEmpty()) {
        this->engine.throwError( QJSValue::SyntaxError, this->tr( "expected arguments for function \"%1\"" ).arg( functionName ));
        return QJSValue();
    }

    return this->getPropertyInternal( functionName, reagentAlias, batchName );
}

/**
 * @brief Script::getPropertyInternal
 * @param tag
 * @param alias
 * @param batchName
 * @return
 */
QJSValue Script::getPropertyInternal( const QString &functionName, const QString &reagentAlias, const QString &batchName ) {
    // validate functionName
    if ( functionName.isEmpty()) {
        this->engine.throwError( QJSValue::SyntaxError, this->tr( "function name expected" ));
        return QJSValue();
    }

    // get propertyId
    const Id propertyId = this->getPropertyId( functionName );
    if ( propertyId == Id::Invalid ) {
        this->engine.throwError( QJSValue::ReferenceError, this->tr( "function \"%1\" is not defined" ).arg( functionName ));
        return QJSValue();
    }
    const Row propertyRow = Tag::instance()->row( propertyId );

    // get reagentId
    const Id reagentId = this->getReagentId( reagentAlias );
    if ( reagentId == Id::Invalid ) {
        this->engine.throwError( QJSValue::ReferenceError, this->tr( "reagent \"%1\" is not defined" ).arg( reagentAlias ));
        return QJSValue();
    }

    QVariant propertyValue;

    // get batchId
    if ( !batchName.isEmpty()) {
        const Id batchId = this->getReagentId( batchName, reagentId );
        if ( batchId == Id::Invalid ) {
            this->engine.throwError( QJSValue::ReferenceError, this->tr( "batch \"%1\" is not defined" ).arg( batchName ));
            return QJSValue();
        }
        propertyValue = this->getPropertyValue( propertyId, batchId, reagentId );
    } else {
        propertyValue = this->getPropertyValue( propertyId, reagentId );
    }

    // get property value
    if ( propertyValue.isNull()) {
        this->engine.throwError( QJSValue::TypeError, this->tr( "property \"%1\" is not defined for \"%2\"" ).arg( functionName ).arg( batchName.isEmpty() ? reagentAlias : batchName ));
        return QJSValue();
    }

    // make sure it is a valid number
    bool ok;
    const qreal value = propertyValue.toReal( &ok );
    if ( !ok ) {
        this->engine.throwError( QJSValue::TypeError, this->tr( "%1( %2 ) does not evaluate to a valid number" ).arg( functionName ).arg( reagentAlias ) + ( batchName.isEmpty() ? "" : QString( ", %1" ).arg( batchName )));
        return QJSValue();
    }

    // return scaled (for example assay is % or 0.01) value
    return value * Tag::instance()->scale( propertyRow );
}

/**
 * @brief Script::getPropertyId
 * @param name
 * @return
 */
Id Script::getPropertyId( const QString &name ) const {
    QSqlQuery query;
    query.exec( QString( "select %1 from %2 where %3='%4'" )
                .arg( Tag::instance()->fieldName( Tag::ID ))
                .arg( Tag::instance()->tableName())
                .arg( Tag::instance()->fieldName( Tag::Function ))
                .arg( name )
                );
    return query.next() ? static_cast<Id>( query.value( 0 ).toInt()) : Id::Invalid;
}

/**
 * @brief Script::getReagentId
 * @param alias
 * @param parentId
 * @return
 */
Id Script::getReagentId( const QString &alias, const Id &parentId ) const {
    QSqlQuery query;
    query.exec( QString( "select %1 from %2 where ( %3='%4' or %5='%4' ) and ( %6=%7 )" )
                .arg( Reagent::instance()->fieldName( Reagent::ID ))
                .arg( Reagent::instance()->tableName())
                .arg( Reagent::instance()->fieldName( Reagent::Alias ))
                .arg( alias )
                .arg( Reagent::instance()->fieldName( Reagent::Name ))
                .arg( Reagent::instance()->fieldName( Reagent::ParentID ))
                .arg( static_cast<int>( parentId ))
                );

    return query.next() ? static_cast<Id>( query.value( 0 ).toInt()) : Id::Invalid;
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
                .arg( Property::instance()->fieldName( Property::Value ))       // 1
                .arg( Property::instance()->tableName())                        // 2
                .arg( Property::instance()->fieldName( Property::TagID ))       // 3
                .arg( static_cast<int>( tagId ))                                // 4
                .arg( Property::instance()->fieldName( Property::ReagentID ))   // 5
                .arg( static_cast<int>( reagentId ))                            // 6
                .arg( static_cast<int>( parentId ))                             // 7
                );

    return query.next() ? query.value( 0 ) : QVariant();
}