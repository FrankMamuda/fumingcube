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

#pragma once

/*
 * includes
 */
#include <QJSEngine>
#include <QTime>
#include "system.h"
#include "table.h"

/**
 * @brief The Script_ namespace
 */
namespace Script_ {
    [[maybe_unused]] const static unsigned int API = 1;
}

/**
 * @brief The Script class
 */
class Script final : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY( Script )

public:
    // disable move
    Script( Script&& ) = delete;
    Script& operator=( Script&& ) = delete;

    /**
     * @brief instance
     * @return
     */
    static Script *instance() {
        static auto *instance = new Script();
        return instance;
    }

    /**
     * @brief ~Script
     */
    ~Script() override {
        delete this->system;
    }

    [[nodiscard]] QJSValue evaluate( const QString &script );
    Q_INVOKABLE QJSValue ans();
    Q_INVOKABLE QJSValue getProperty( const QString &functionName, const QString &reference );
    Q_INVOKABLE QJSValue getProperty( const QString &functionName, const QString &reference, const QString &batchName );
    [[nodiscard]] QJSValue
    getPropertyInternal( const QString &functionName, const QString &reference, const QString &batchName = QString());
    [[nodiscard]] Id getPropertyId( const QString &name ) const;
    [[nodiscard]] Id getReagentId( const QString &reference, const Id &parentId = Id::Invalid ) const;
    [[nodiscard]] QVariant getPropertyValue( const Id &tagId, const Id &reagentId, const Id &parentId = Id::Invalid ) const;
    [[nodiscard]] Q_INVOKABLE QJSValue round( qreal value, int precision = 2 );
    [[nodiscard]] Q_INVOKABLE QJSValue floor( qreal value );
    [[nodiscard]] Q_INVOKABLE QJSValue ceil( qreal value );
    [[nodiscard]] Q_INVOKABLE QJSValue abs( qreal value );

private:
    explicit Script();
    QJSEngine engine;
    System *system = new System();
};
