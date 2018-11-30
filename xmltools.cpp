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

//
// includes
//
#include <QDebug>
#include "xmltools.h"
#include "variable.h"
#include <QBuffer>
#include <QDataStream>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QXmlStreamWriter>
#include "main.h"

/**
 * @brief XMLTools::XMLTools
 * @param parent
 */
XMLTools::XMLTools( QObject *parent ) : QObject( parent ) {
    // add to garbage collector
    GarbageMan::instance()->add( this );
}

/**
 * @brief XMLTools::write
 * @param mode
 */
void XMLTools::write() {
    QString path, savedData, newData;
    QDir configDir( XML::ConfigPath );

    if ( !configDir.exists())
        configDir.mkpath( configDir.absolutePath());

    // set path
    path = configDir.absolutePath() + "/" + XML::Variables;

    // read xml file and create buffer
    QFile xmlFile( path );
    QBuffer xmlBuffer;
    xmlBuffer.open( QBuffer::WriteOnly | QBuffer::Text | QBuffer::Truncate );

    // create stream
    QXmlStreamWriter stream( &xmlBuffer );
    stream.setAutoFormatting( true );
    stream.writeStartDocument();
    stream.writeStartElement( "configuration" );
    stream.writeAttribute( "version", "3" );

    // switch mode
    foreach ( const QSharedPointer<Var> &var, qAsConst( Variable::instance()->list )) {
        if ( var->key().isEmpty() || var->flags() & Var::Flag::NoSave )
            continue;

        stream.writeEmptyElement( "variable" );
        stream.writeAttribute( "key", var->key());

        if ( !var->value().canConvert<QString>()) {
            QByteArray array;
            QBuffer buffer(&array);

            buffer.open( QIODevice::WriteOnly );
            QDataStream out( &buffer );

            out << var->value();
            buffer.close();

            stream.writeAttribute( "binary", QString( array.toBase64()));
        } else {
            stream.writeAttribute( "value", var->value().toString());
        }
    }

    // end config element
    stream.writeEndElement();

    // end document
    stream.writeEndDocument();

    // close buffer
    xmlBuffer.close();

    // read existing config from file
    if ( xmlFile.open( QFile::ReadOnly | QIODevice::Text )) {
        savedData = xmlFile.readAll();
        xmlFile.close();
    }

    // read new config from buffer
    if ( xmlBuffer.open( QFile::ReadOnly | QIODevice::Text )) {
        newData = xmlBuffer.readAll();
        xmlBuffer.close();
    }

    // compare data
    if ( QString::compare( savedData, newData )) {
        // write out as binary (not QIODevice::Text) to avoid CR line endings
        if ( !xmlFile.open( QFile::WriteOnly | QFile::Truncate )) {
            qCritical() << "could not open configuration file" << path;
            return;
        }

        xmlFile.write( newData.toUtf8().replace( "\r", "" ));
    }

    // close file
    xmlFile.close();
}

/**
 * @brief XMLTools::read
 * @param mode
 * @param object
 */
void XMLTools::read() {
    QString path;
    QDomDocument document;
    QDomNode node, childNode;
    QDomElement element, childElement;
    QDir configDir( XML::ConfigPath );

    if ( !configDir.exists())
        configDir.mkpath( configDir.absolutePath());

    // set path
    path = configDir.absolutePath() + "/" + XML::Variables;

    // load xml file
    QFile xmlFile( path );

    if ( !xmlFile.exists() || !xmlFile.open( QFile::ReadOnly | QFile::Text )) {
        qCritical() << this->tr( "no configuration file found" );
        return;
    }

    document.setContent( &xmlFile );
    node = document.documentElement().firstChild();

    while ( !node.isNull()) {
        element = node.toElement();

        if ( !element.isNull()) {
            if ( !QString::compare( element.tagName(), "variable" )) {
                QString key;
                QVariant value;

                childNode = element.firstChild();
                key = element.attribute( "key" );

                if ( element.hasAttribute( "binary" )) {
                    QByteArray array;
                    array = QByteArray::fromBase64( element.attribute( "binary" ).toUtf8().constData());
                    QBuffer buffer( &array );
                    buffer.open( QIODevice::ReadOnly );
                    QDataStream in( &buffer );
                    in >> value;
                } else
                    value = element.attribute( "value" );

                if ( Variable::instance()->contains( key ))
                    Variable::instance()->setValue( key, value, true );
            }
        }
        node = node.nextSibling();
    }

    document.clear();
    xmlFile.close();
}
