/*
 * Copyright (C) 2017-2018 Factory #12
 * Copyright (C) 2013-2019 Armands Aleksejevs
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
#include <QSharedPointer>
#include <QVariant>
#include <utility>

/**
 * @brief The Var class
 */
class Var final {
public:
    /**
     * @brief The Flag enum
     */
    enum class Flag {
        NoFlags = 0x0,
        ReadOnly = 0x1,
        NoSave = 0x2,
        Hidden = 0x4
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * @brief Var
     * @param key
     * @param defaultValue
     * @param flags
     */
    explicit Var( QString key = QString(), const QVariant &defaultValue = QVariant(),
                  Flags flags = Flag::NoFlags ) : m_key( std::move( key )), m_value( defaultValue ), m_defaultValue( defaultValue ),
                                                  m_flags( flags ) {}
    virtual ~Var() = default;

    /**
     * @brief key
     * @return
     */
    [[nodiscard]] QString key() const { return this->m_key; }

    /**
     * @brief flags
     * @return
     */
    [[nodiscard]] Flags flags() const { return this->m_flags; }

    /**
     * @brief value
     * @return
     */
    [[nodiscard]] virtual QVariant value() const { return this->m_value; }

    /**
     * @brief defaultValue
     * @return
     */
    [[nodiscard]] QVariant defaultValue() const { return this->m_defaultValue; }

    /**
     * @brief setValue
     * @param value
     */
    virtual void setValue( const QVariant &value ) { m_value = value; }

    // copy op
    Var &operator=( const Var & ) = default;
    Var( const Var & ) = default;

    /**
     * @brief copy
     * @return
     */
    [[nodiscard]] virtual QSharedPointer<Var> copy() const { return QSharedPointer<Var>( new Var( *this )); }

private:
    QString m_key;
    QVariant m_value;
    QVariant m_defaultValue;
    Flags m_flags;
};

// declare flags
Q_DECLARE_OPERATORS_FOR_FLAGS( Var::Flags )
