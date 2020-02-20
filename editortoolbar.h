/*
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
#include "textedit.h"
#include <QTextEdit>
#include <QToolBar>

/**
 * @brief The EditorToolbar class
 */
class EditorToolbar : public QToolBar {
    Q_OBJECT

public:
    explicit EditorToolbar( QWidget *parent = nullptr, TextEdit *editor = nullptr );

    /**
     * @brief editor
     * @return
     */
    [[nodiscard]] TextEdit *editor() const { return this->m_editor; }

    /**
     * @brief The Feature enum
     */
    enum Feature {
        NoFeatures              = 0x0,
        Font                    = 0x01,
        Colour                  = 0x02,
        Image                   = 0x04,
        VerticalAlignment       = 0x08,
        CharacterMap            = 0x10,
        GHS                     = 0x20,
        CleanHTML               = 0x40
    };

    Q_DECLARE_FLAGS( Features, Feature )
    Q_FLAG( Features )

    /**
     * @brief features
     * @return
     */
    Features features() { return this->m_features; }

public slots:
    void setEditor( TextEdit *editor );
    void installFeature( const Feature &feature );

private slots:
    void colourChanged( const QColor &colour );
    void formatChanged( const QTextCharFormat &format );
    void mergeFormat( const QTextCharFormat &format );

private:
    QAction *actionSubScript = nullptr;
    QAction *actionSuperScript = nullptr;
    QAction *actionBold = nullptr;
    QAction *actionItalic = nullptr;
    QAction *actionUnderlined = nullptr;
    QAction *actionColour = nullptr;
    QAction *actionImage = nullptr;
    QAction *actionGHS = nullptr;
    QAction *actionCharacterMap = nullptr;
    QAction *actionCleanHTML = nullptr;

    TextEdit *m_editor = nullptr;
    Features m_features;
};
