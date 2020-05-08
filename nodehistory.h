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
    Q_DISABLE_COPY( NodeHistory )
    friend class ReagentDock;

public:
    // disable move
    NodeHistory( NodeHistory&& ) = delete;
    NodeHistory& operator=( NodeHistory&& ) = delete;

    ~NodeHistory() override;

    /**
     * @brief treeParent
     * @return
     */
    [[nodiscard]] QTreeView *treeParent() const { return this->m_treeParent; }

    /**
     * @brief isEnabled
     * @return
     */
    [[nodiscard]] bool isEnabled() const { return this->m_enabled; }

    /**
     * @brief isHidden
     * @param id
     * @return
     */
    [[nodiscard]] bool isHidden( const Id& id ) const { return this->hiddenNodes.contains( id ); }

    /**
     * @brief isDeperecated
     * @param id
     * @return
     */
    [[nodiscard]] bool isDeperecated( const Id& id ) const { return this->deprecatedNodes.contains( id ); }


    /**
     * @brief hiddenCount
     * @return
     */
    [[nodiscard]] int hiddenCount() const { return this->hiddenNodes.count(); }

    /**
     * @brief instance
     * @return
     */
    static NodeHistory *instance() {
        static auto *instance( new NodeHistory());
        return instance;
    }

public slots:
    /**
     * @brief setEnabled
     * @param enabled
     */
    void setEnabled( bool enabled = true ) { this->m_enabled = enabled; }
    void restoreNodeState();
    void saveHistory();
    void loadHistory();
    void setTreeParent( QTreeView *parent );
    void removeFromHistory( const Id &id );

    /**
     * @brief hide
     * @param id
     */
    void hide( const Id& id ) { if ( !this->hiddenNodes.contains( id )) this->hiddenNodes << id; }

    /**
     * @brief deprecate
     * @param id
     */
    void deprecate( const Id& id ) { if ( !this->deprecatedNodes.contains( id )) this->deprecatedNodes << id; }

    /**
     * @brief restore
     * @param id
     */
    void restore( const Id& id ) { if ( this->deprecatedNodes.contains( id )) this->deprecatedNodes.removeAll( id ); }

    /**
     * @brief clearHiddenNodes
     */
    void clearHiddenNodes() { this->hiddenNodes.clear(); }    

private:
    explicit NodeHistory();

    QTreeView *m_treeParent;
    bool m_enabled = true;
    QList<Id> openNodes;
    QList<Id> hiddenNodes;
    QList<Id> deprecatedNodes;
};
