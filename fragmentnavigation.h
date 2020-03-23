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
#include <QStackedWidget>
#include <QToolBar>

/**
 * @brief The FragmentNavigation class
 */
class FragmentNavigation : public QToolBar {
    Q_OBJECT
    Q_DISABLE_COPY( FragmentNavigation )

public:
    explicit FragmentNavigation( QWidget *parent = nullptr );
    ~FragmentNavigation();

    // disable move
    FragmentNavigation( FragmentNavigation&& ) = delete;
    FragmentNavigation& operator=( FragmentNavigation&& ) = delete;

    /**
     * @brief fragmentHost
     * @return
     */
    [[nodiscard]] QStackedWidget *fragmentHost() const { return this->m_fragmentHost; }
    [[nodiscard]] QAction *addFragment( const QString &name, const QIcon &icon, QWidget *widget );

public slots:
    /**
     * @brief setFragmentHost
     * @param fragmentHost
     */
    void setFragmentHost( QStackedWidget *fragmentHost ) { this->m_fragmentHost = fragmentHost; }
    void setCurrentFragment( QAction *action );

private:
    QStackedWidget *m_fragmentHost = nullptr;
    QMap<QAction*, QWidget*> map;
};
