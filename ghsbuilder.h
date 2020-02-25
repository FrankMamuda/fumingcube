/*
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
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QToolButton>

/**
 * @brief The GHSBuilder class
 */
class GHSBuilder : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY( GHSBuilder )

public:
    explicit GHSBuilder( QWidget *parent = nullptr, const QStringList &parameters = QStringList());

    // disable move
    GHSBuilder( GHSBuilder&& ) = delete;
    GHSBuilder& operator=( GHSBuilder&& ) = delete;

    ~GHSBuilder() override;

    /**
     * @brief parameters
     * @return
     */
    [[nodiscard]] QStringList parameters() const;

private:
    QList<QToolButton *> buttons;
    QGridLayout *grid = new QGridLayout();
    QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
};
