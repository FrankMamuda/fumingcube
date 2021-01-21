/*
 * Copyright (C) 2018-2020 Armands Aleksejevs
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
#include <QFileInfo>
#include <QLoggingCategory>
#include <QSharedPointer>
#include <QSqlTableModel>

//
// classes
//
class Table;

/**
 * @brief The Database_ class
 */
namespace Database_ {
    const static QLoggingCategory Debug( "database" );
    const static constexpr int null = 0;
    [[maybe_unused]] static const constexpr int API = 1;
}

/**
 * @brief The Database class
 */
class Database final : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY( Database )

public:
    // disable move
    Database( Database&& ) = delete;
    Database& operator=( Database&& ) = delete;

    /**
     * @brief instance
     * @return
     */
    static Database *instance() {
        static auto *instance( new Database());
        return instance;
    }
    ~Database() override;
    bool add( Table *table );

    /**
     * @brief hasInitialised
     * @return
     */
    [[nodiscard]] bool hasInitialised() const { return this->m_initialised; }

public slots:
    void removeOrphanedEntries();

private:
    explicit Database( QObject *parent = nullptr );
    bool testPath( const QString &path );

    /**
     * @brief setInitialised
     * @param initialised
     */
    void setInitialised( bool initialised = true ) { this->m_initialised = initialised; }
    QMap<QString, Table *> tables;
    bool m_initialised = false;
};
