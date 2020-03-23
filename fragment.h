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
#include <QMainWindow>

/*
 * classes
 */
class ExtractionDialog;

/**
 * @brief The Fragment class
 */
class Fragment : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY( Fragment )

public:
    explicit Fragment( QWidget *parent = nullptr ) : QMainWindow( parent ) {}

    // disable move
    Fragment( Fragment&& ) = delete;
    Fragment& operator=( Fragment&& ) = delete;

    /**
     * @brief host
     * @return
     */
    [[nodiscard]] ExtractionDialog *host() const { return this->m_host; }

public slots:
    /**
     * @brief setHost
     * @param host
     */
    void setHost( ExtractionDialog *host ) { this->m_host = host; }

private:
    ExtractionDialog *m_host;
};
