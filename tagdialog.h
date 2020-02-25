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
#include <QDialog>
#include <QTextCharFormat>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class TagDialog;
}

/**
 * @brief The TagDialog class
 */
class TagDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY( TagDialog )

public:
    explicit TagDialog( QWidget *parent = nullptr );

    // disable move
    TagDialog( TagDialog&& ) = delete;
    TagDialog& operator=( TagDialog&& ) = delete;

    ~TagDialog() override;

    /**
     * @brief The Modes enum
     */
    enum Modes {
        NoMode = -1,
        Add,
        Edit
    };
    Q_ENUM( Modes )

    /**
     * @brief mode
     * @return
     */
    [[nodiscard]] Modes mode() const { return this->m_mode; }
    static QString captureBody( const QString &input );

private slots:
    void on_actionAdd_triggered();
    void on_actionRemove_triggered();
    void on_actionEdit_triggered();
    void clear();

    /**
     * @brief setMode
     * @param mode
     */
    void setMode( const TagDialog::Modes &mode = NoMode ) { this->m_mode = mode; }

protected:
    bool eventFilter( QObject *object, QEvent *event ) override;

private:
    Ui::TagDialog *ui;
    Modes m_mode = NoMode;
};

