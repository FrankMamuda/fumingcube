/*
 * Copyright (C) 2018-2019 Armands Aleksejevs
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

/*
 * includes
 */
#include <QSqlField>
#include <utility>

/**
 * @brief The Field class
 */
class Field_ : public QSqlField {
public:
    /**
     * @brief Field_
     * @param id
     * @param fieldName
     * @param type
     * @param format
     * @param unique
     * @param autoValue
     */
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    //explicit Field_( int id = 0, const QString &fieldName = QString(), QVariant::Type type = QVariant::Invalid,
    //                 QString format = QString( "text" ), bool unique = false, bool autoValue = false ) : QSqlField(
     //                                                                                                        fieldName, type ), m_id( id ), m_unique( unique ), m_format( std::move( format )) { this->setAutoValue( autoValue ); }
    explicit Field_( int id = 0, const QString &fieldName = QString(), QMetaType::Type type = QMetaType::UnknownType,
                     QString format = QString( "text" ), bool unique = false, bool autoValue = false ) : QSqlField(
                                                                                                             fieldName, static_cast<QVariant::Type>( type )), m_id( id ), m_unique( unique ), m_format( std::move( format )) { this->setAutoValue( autoValue ); }
#else
    explicit Field_( int id = 0, const QString &fieldName = QString(), QMetaType::Type type = QMetaType::UnknownType,
                     QString format = QString( "text" ), bool unique = false, bool autoValue = false ) : QSqlField(
                                                                                                             fieldName, QMetaType( type )), m_id( id ), m_unique( unique ), m_format( std::move( format )) { this->setAutoValue( autoValue ); }
#endif

    /**
     * @brief isUnique
     * @return
     */
    [[nodiscard]] bool isUnique() const { return this->m_unique; }

    /**
     * @brief format
     * @return
     */
    [[nodiscard]] QString format() const { return m_format; }

    /**
     * @brief id
     * @return
     */
    [[nodiscard]] int id() const { return this->m_id; }

    /**
     * @brief isPrimary
     * @return
     */
    bool isPrimary() { return this->isAutoValue() && this->format().contains( "primary key" ); }

private:
    int m_id;
    bool m_unique;
    QString m_format;
};

/**
 * @brief Field a shared pointer to the field class
 */
using Field = QSharedPointer<Field_>;
