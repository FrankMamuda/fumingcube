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
#include <QJSEngine>

/**
 * @brief The System class
 */
class System : public QObject {
    Q_OBJECT

public:
    System();    
    Q_INVOKABLE void print( const QString &message );
    Q_INVOKABLE void replaceGreeting();
    Q_INVOKABLE void dbInfo();
    Q_INVOKABLE void clearCommandHistory();
    Q_INVOKABLE void printVariableValue( const QString &key );
};
