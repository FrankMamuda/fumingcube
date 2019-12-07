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
#include "dockwidget.h"
#include "propertydelegate.h"
#include <QDockWidget>
#include "table.h"
#include "propertyeditor.h"

/**
 * @brief The Ui namespace
 */
namespace Ui {
class PropertyDock;
}

/**
 * @brief The PropertyDock class
 */
class PropertyDock final : public DockWidget {
    Q_OBJECT
    Q_DISABLE_COPY( PropertyDock )

public:
    static PropertyDock *instance() { static PropertyDock *reagentDock( new PropertyDock()); return reagentDock; }
    ~PropertyDock();
    int sectionSize( int column ) const;

public slots:
    void updateView();
    void clearDocumentCache();
    void setSpecialWidgets();

private slots:
    void on_addPropButton_clicked();
    void on_propertyView_customContextMenuRequested( const QPoint &pos );
    void on_removePropButton_clicked();
    void on_editPropButton_clicked();
    void addProperty( const QString &name, const QVariant &value, const Id &reagentId, const Id &tagId = Id::Invalid );

    void on_propertyView_doubleClicked(const QModelIndex &index);

private:
    explicit PropertyDock( QWidget *parent = nullptr );
    Ui::PropertyDock *ui;
    QModelIndex reagentIndex;
    QPair<QString, QVariant> getPropertyValue(const Id &reagentId, const Id &tagId = Id::Invalid, const Id &propertyId = Id::Invalid ) const;
};
