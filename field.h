/*
 * Copyright (C) 2018 Factory #12
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
#include <QSqlField>

/**
 * @brief The Field class
 */
class Field_ : public QSqlField {
public:
    Field_( int id = 0, const QString &fieldName = QString(), QVariant::Type type = QVariant::Invalid, const QString &format = QString( "text" ), bool unique = false, bool autoValue = false ) : QSqlField( fieldName, type ), m_id( id ), m_unique( unique ), m_format( format ) { this->setAutoValue( autoValue ); }
    bool isUnique() const { return this->m_unique; }
    QString format() const { return m_format; }
    int id() const { return this->m_id; }
    bool isPrimary() { return this->isAutoValue() && this->type() == QVariant::UInt && this->format().contains( "primary key" ); }

private:
    int m_id;
    bool m_unique;
    QString m_format;
};

/**
 * @brief Field a shared pointer to the field class
 */
typedef QSharedPointer<Field_> Field;
