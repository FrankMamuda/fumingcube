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

//
// includes
//
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "entry.h"

/**
 * @brief Entry::setValue
 * @param name
 * @param value
 */
void Entry::setValue( const QString &valueName, const QVariant &value ) {
    QSqlQuery query;
    QVariant update( value );

    // make sure we don't perform useless updates
    if ( this->record().value( valueName ) == update )
        return;

    // update counters
    emit this->changed();

    // store local value
    this->m_record.setValue( valueName, update );

    // check for strings (should be wrapped in quotes)
    if ( update.type() == QVariant::String )
        update.setValue( QString( "%1" ).arg( update.toString()));

    // update database value
    if ( !this->table().isNull()) {
        query.prepare( QString( "update %1 set %2 = :value where id = :id" ).arg( this->table()).arg( valueName ));
        query.bindValue( ":value", update.toString());
        query.bindValue( ":id", this->record().value( "id" ).toInt());

        if ( !query.exec())
            qCritical() << this->tr( "could not store value, reason - '%1'" ).arg( query.lastError().text());
    }
}
