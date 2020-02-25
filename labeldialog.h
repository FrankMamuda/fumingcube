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
#include <QDialog>

/**
 * @brief The Ui namespace
 */
namespace Ui {
    class LabelDialog;
}

/**
 * @brief The LabelDialog class
 */
class LabelDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY( LabelDialog )

public:
    explicit LabelDialog( QWidget *parent = nullptr );

    // disable move
    LabelDialog( LabelDialog&& ) = delete;
    LabelDialog& operator=( LabelDialog&& ) = delete;

    ~LabelDialog() override;

    [[nodiscard]] QString name() const;

    /**
     * @brief colour
     * @return
     */
    [[nodiscard]] QColor colour() const { return this->m_colour; }

public slots:
    void setName( const QString &name );
    void setColour( const QColor &colour );

private:
    Ui::LabelDialog *ui;
    QColor m_colour;
};
