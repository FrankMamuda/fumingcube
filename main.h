/*
 * Copyright (C) 2017 Zvaigznu Planetarijs
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
#include <QList>
#include <QObject>

/**
 * @brief The GarbageMan class
 */
class GarbageMan final {
public:
    /**
     * @brief instance
     * @return
     */
    static GarbageMan *instance() { static GarbageMan *instance( new GarbageMan()); return instance; }
    GarbageMan( const GarbageMan & ) = delete;
    ~GarbageMan() = default;

    /**
     * @brief add adds pointers (singletons) to garbage collection list
     * @param object
     */
    void add( QObject *object ) {
        if ( !this->garbage.contains( object ))
            this->garbage << object;
    }

    /**
     * @brief clear deletes poiners in reverse order
     */
    void clear() {
        std::reverse( this->garbage.begin(), this->garbage.end());
        foreach ( QObject *object, this->garbage ) {
            if ( object != nullptr ) {
                delete object;
                object = nullptr;
            }
        }
        this->garbage.clear();
    }

private:
    explicit GarbageMan() = default;
    QList<QObject*> garbage;
};
