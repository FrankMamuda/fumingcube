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
#include "label.h"
#include "field.h"
#include "database.h"
#include <QSqlQuery>

/**
 * @brief Label::Label
 */
Label::Label() : Table( "label" ) {
    this->addField( PRIMARY_FIELD( ID ));
    this->addField( FIELD( Name,   String ));
    this->addField( FIELD( Colour, ByteArray ));
}

/**
 * @brief Label::add
 * @param name
 * @return
 */
Row Label::add( const QString &name, const QColor &colour ) {
    return Table::add( QVariantList() << Database_::null << name << QVariant( colour ).toByteArray());
}

/**
 * @brief Label::removeOrphanedEntries
 */
void Label::removeOrphanedEntries() {
    // NOTE: STUB
}

/**
 * @brief Label::populate
 */
void Label::populate() {
    this->add( this->tr( "Base" ),    QColor::fromRgb( 255,   0,   0, 32 ));
    this->add( this->tr( "Acid" ),    QColor::fromRgb( 0,   255,   0, 32 ));
    this->add( this->tr( "Solvent" ), QColor::fromRgb( 0,     0, 255, 32 ));
}
