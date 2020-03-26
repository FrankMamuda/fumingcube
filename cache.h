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

#pragma once

/*
 * includes
 */
#include <QObject>
#include <QVariant>

/**
 * @brief The Cache class
 */
class Cache : public QObject {
    Q_OBJECT
    friend class SearchFragment;

public:
    constexpr static const char *FormulaContext = "formula";
    constexpr static const char *IUPACContext = "iupac";
    constexpr static const char *DataContext = "data";
    constexpr static const char *IdMapContext = "id";

    /**
     * @brief The Types enum
     */
    enum Types {
        NoType = -1,
    };

    Q_ENUM( Types )

    /**
     * @brief instance
     * @return
     */
    static Cache *instance() {
        static auto *instance( new Cache());
        return instance;
    }

    /**
     * @brief path
     * @return
     */
    [[nodiscard]] QString path() const { return this->m_path; }
    [[nodiscard]] QVariant data( const Types &type, const QString &key ) const;
    [[nodiscard]] static QString checksum( const QByteArray &array );
    [[nodiscard]] bool contains( const QString &context, const QString &key ) const;
    [[nodiscard]] QByteArray getData( const QString &context, const QString &key, bool compressed = false ) const;
    bool insert( const QString &context, const QString &key, const QByteArray &data, bool compress = false );
    void clear( const QString &context, const QString &key );
    [[nodiscard]] QString contextPath( const QString &context, const QString &key = QString()) const;
    [[nodiscard]] static bool validate( const QString &context, const QString &key );

public slots:
    void readReagentCache();
    void writeReagentCache();


private:
    explicit Cache();
    QString m_path;

    QMultiMap<QString, int> nameIdMap;
    QMultiMap<int, QString> idNameMap;

    constexpr static const int Version = 1;
};
