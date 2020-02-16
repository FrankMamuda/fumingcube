/*
 * Copyright (C) 2017-2018 Factory #12
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
#include <QDialog>
#include <QTextCharFormat>

//
// classes
//
class TextEdit;
class CharacterMap;

/**
 * @brief The Ui namespace
 */
namespace Ui {
class PropertyEditor;
}

/**
 * @brief The PropertyEditor class
 */
class PropertyEditor : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY( PropertyEditor )

public:
    enum Editors {
        NoEditor = -1,
        Name,
        Value
    };
    Q_ENUM( Editors )

    enum Modes {
        NoMode = -1,
        Add,
        Edit
    };
    Q_ENUM( Modes )

    explicit PropertyEditor( QWidget *parent = nullptr, Modes mode = Modes::Add, const QString &name = QString(), const QString &value = QString());
    ~PropertyEditor() override;
    bool eventFilter( QObject *watched, QEvent *event ) override;
    QString name() const;
    QString value() const;

private slots:
    void setText( Editors editor, const QString &text );

protected:
    void showEvent( QShowEvent *event ) override;

private:
    Ui::PropertyEditor *ui;
    Modes mode;
};
