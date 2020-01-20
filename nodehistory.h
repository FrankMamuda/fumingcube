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
#include <QTreeView>
#include "table.h"

/**
 * @brief The NodeHistory class
 */
class NodeHistory : public QObject {
    Q_OBJECT

public:
    NodeHistory( QTreeView *parent );
    ~NodeHistory();
    QTreeView *treeParent() const { return this->m_treeParent; }
    bool isEnabled() const { return this->m_enabled; }

public slots:
    void setEnabled( bool enabled = true ) { this->m_enabled = enabled; }
    void restoreNodeState();
    void saveHistory();
    void loadHistory();

private:
    QTreeView *m_treeParent;
    bool m_enabled = true;
    QList<Id> openNodes;
};
