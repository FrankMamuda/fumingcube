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
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QMap>
#include "table.h"

/**
 * @brief The PropertyDelegate class
 */
class PropertyDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    /**
     * @brief The TextFlag enum
     */
    enum TextFlag {
        NoFlags      = 0x0,
        Batch        = 0x01,
        Duplicate    = 0x02,
        Override     = 0x04
    };

    Q_DECLARE_FLAGS( TextFlags, TextFlag )
    Q_FLAG( TextFlags )

    explicit PropertyDelegate( QObject *parent = nullptr ) : QStyledItemDelegate( parent ) {}
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    [[nodiscard]] QSize sizeHint( const QStyleOptionViewItem &item, const QModelIndex &index ) const override;
    mutable QMap<QModelIndex, QTextDocument *> cache;

    /**
     * @brief viewMode
     * @return
     */
    [[nodiscard]] bool viewMode() const { return this->m_viewMode; }

public slots:
    /**
     * @brief clearCache
     */
    void clearCache() {
        if ( this->cache.isEmpty())
            return;

        qDeleteAll( this->cache );
        this->cache.clear();
    }

    /**
     * @brief setViewMode
     * @param viewMode
     */
    void setViewMode( bool viewMode = true ) {
        this->m_viewMode = viewMode;
    }

private slots:
    void setupDocument( const QModelIndex &index, const QFont &font ) const;
    void setupPixmapDocument( const QModelIndex &index, QTextDocument *document, const QByteArray &data, bool isFormula = false ) const;
    void setupTextDocument( const QModelIndex &index, QTextDocument *document, const QString &text, const TextFlags &flags, const QFont &font ) const;
    void finializeDocument( const QModelIndex &index, QTextDocument *document ) const;
    void setTextFlags( TextFlags &flags, const Id &tagId, const Row &propertyRow ) const;

private:
    mutable QMap<QString, QSize> sizeCache;
    bool m_viewMode = false;
};
