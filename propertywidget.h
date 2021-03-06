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
#include "table.h"
#include <QToolButton>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <QLabel>
#include "nfpawidget.h"
#include "ghswidget.h"

/**
 * @brief The PropertyValueWidget class
 */
class PropertyWidget : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY( PropertyWidget )

public:
    explicit PropertyWidget( QWidget *parent = nullptr, const QList<QStringList> &values = QList<QStringList>(),
                             const Id &tagId = Id::Invalid );
    explicit PropertyWidget( QWidget *parent = nullptr, const QPixmap &pixmap = QPixmap());

    // disable move
    PropertyWidget( PropertyWidget&& ) = delete;
    PropertyWidget& operator=( PropertyWidget&& ) = delete;

    ~PropertyWidget() override;

    /**
     * @brief position
     * @return
     */
    [[nodiscard]] int position() const { return this->m_position; }

    /**
     * @brief tagId
     * @return
     */
    [[nodiscard]] Id tagId() const { return this->m_tagId; }

    /**
     * @brief parseGHS
     * @param list
     * @return
     */
    static QStringList parseGHS( const QStringList &list ) {
        QStringList parms;
        for ( const QString &parm : list ) {
            if ( parm.contains( QRegularExpression( "[Ee]xplosive" )))
                parms << "GHS01";
            if ( parm.contains( QRegularExpression( "[Ff]lammable" )))
                parms << "GHS02";
            if ( parm.contains( QRegularExpression( "[Oo]xidizing" )))
                parms << "GHS03";
            if ( parm.contains( QRegularExpression( "[Cc]ompressed\\s[Gg]as" )))
                parms << "GHS04";
            if ( parm.contains( QRegularExpression( "[Cc]orrosive" )))
                parms << "GHS05";
            if ( parm.contains( QRegularExpression( "[Tt]oxic" )))
                parms << "GHS06";
            if ( parm.contains( QRegularExpression( "[Hh]armful" )) ||
                 parm.contains( QRegularExpression( "[Ii]rritant" )))
                parms << "GHS07";
            if ( parm.contains( QRegularExpression( "[Hh]ealth\\s[Hh]azard" )))
                parms << "GHS08";
            if ( parm.contains( QRegularExpression( "[Ee]nvironmental\\s[Hh]azard" )))
                parms << "GHS09";
        }
        return qAsConst( parms );
    }

    /**
     * @brief pixmap
     * @return
     */
    [[nodiscard]] QPixmap pixmap() const { return this->m_pixmap; }

public slots:
    void add( const Id &id );

private:
    int m_position = -1;
    QMap<int, QString> displayValues;
    QMap<int, QStringList> propertyValues;
    QLabel *label = nullptr;
    QToolButton *left = nullptr;
    QToolButton *right = nullptr;
    QHBoxLayout *layout = new QHBoxLayout();
    NFPAWidget *nfpa = nullptr;
    GHSWidget *ghs = nullptr;
    QPixmap m_pixmap;

    Id m_tagId = Id::Invalid;
};
