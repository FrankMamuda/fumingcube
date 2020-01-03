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
#include <QMap>
#include <QColor>
#include <QPalette>
#include <QStyleFactory>
#include <QStyle>

/**
 * @brief The Theme class
 */
class Theme : public QObject {
    Q_OBJECT

public:
    Theme( const QString &fileName = QString());
    QPalette palette() const;
    QColor syntaxColour( const QString &key ) const;
    bool isDark() const { return this->m_dark; }
    QStyle *style() const { return this->m_style; }

private slots:
    void readThemeFile( const QString &fileName );
    void initializeSyntaxColours();

private:
    QMap<QString, QColor> paletteMap;
    QMap<QString, QColor> syntaxMap;
    bool m_dark = false;
    QStyle *m_style = nullptr;
};
