/*
 * Copyright (C) 2017-2018 Factory #12
 * Copyright (C) 2019 Armands Aleksejevs
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
#include <QPaintEvent>
#include <QDialog>

/**
 * @brief The Character namespace
 */
namespace CharacterNamespace {
    static const int GridSize = 16;
    static const QList<QChar> SpecialChars {
        // FIXME
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            /* degree, angstrom */
            0x00b0, 0x212b,
            /* uppercase greek */
            0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039a, 0x0399, 0x039c, 0x039d,
            0x039f,
            0x03a0, 0x03a1, 0x03a3, 0x03a4, 0x03a5, 0x03a6, 0x03a7, 0x03a8, 0x03a9,
            /* lowercase greek */
            0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5, 0x03b6, 0x03b7, 0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd,
            0x03bf,
            0x03c0, 0x03c1, 0x03c2, 0x03c3, 0x03c4, 0x03c5, 0x03c6, 0x03c7, 0x03c8, 0x03c9,
            /* arrows */
            0x2190, 0x2191, 0x2192, 0x2193, 0x2194, 0x219a, 0x219b, 0x21c4, 0x21d0, 0x21d2,
            0x25ba, 0x25c4, 0x25b2, 0x25bc,
            /* dot */
            0x00b7,
            /* dollar, euro, permille */
            0x00b1, 0x2213, 0x221a,
            /* various math symbols */
            0x00f7, 0x2243, 0x2244, 0x2245, 0x2246, 0x2247, 0x2248, 0x2249, 0x2260, 0x2261, 0x221e, 0x222b, 0x2207,
            0x2205,
            0x2264, 0x2265,
            /* misc */
            0x0024, 0x20ac, 0x2030, 0x26a0,
            0x2605, 0x2606, 0x2713, 0x2717
#endif
    };
}

/**
 * @brief The CharacterMap class
 */
class CharacterMap : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY( CharacterMap )

public:
    explicit CharacterMap( QWidget *parent = nullptr );

    /**
     * @brief ~CharacterMap
     */
    ~CharacterMap() override { this->characters.clear(); }

    // disable move
    CharacterMap( CharacterMap&& ) = delete;
    CharacterMap& operator=( CharacterMap&& ) = delete;

protected:
    void paintEvent( QPaintEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;

signals:
    void characterSelected( const QString &character );

private:
    QList<QChar> characters;
    QPair<int, int> blockId;
};
