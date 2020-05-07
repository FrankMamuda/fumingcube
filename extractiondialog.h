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
#include <QDialog>
#include <QStackedWidget>
#include "table.h"

/*
 * classes
 */
class SearchFragment;
class StructureFragment;
class PropertyFragment;
class FragmentNavigation;
class Fragment;

/**
 * @brief The Ui namespace
 */
namespace Ui {
    class ExtractionDialog;
}

/**
 * @brief The ExtractionDialog class
 */
class ExtractionDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY( ExtractionDialog )

public:

    /**
     * @brief The Modes enum
     */
    enum Modes {
        NoMode = -1,
        SearchMode,
        ExistingMode
    };
    Q_ENUM( Modes )

    // ExistingMode constructor
    explicit ExtractionDialog( QWidget *parent = nullptr, const Id &reagentId = Id::Invalid, const Id &batchId = Id::Invalid );

    // SearchMode constructor
    //ExtractionDialog( QWidget *parent = nullptr, const int id = 0 );

    // disable move
    ExtractionDialog( ExtractionDialog&& ) = delete;
    ExtractionDialog& operator=( ExtractionDialog&& ) = delete;

    ~ExtractionDialog() override;

    /**
     * @brief reagentId
     * @return
     */
    [[nodiscard]] Id reagentId() const { return this->m_reagentId; }

    /**
     * @brief batchId
     * @return
     */
    [[nodiscard]] Id batchId() const { return this->m_batchId; }
    [[nodiscard]] SearchFragment *searchFragment() const;
    [[nodiscard]] StructureFragment *structureFragment() const;
    [[nodiscard]] PropertyFragment *propertyFragment() const;
    [[nodiscard]] QStackedWidget *fragmentHost() const;
    [[nodiscard]] FragmentNavigation *fragmentNavigation() const;

    /**
     * @brief mode
     * @return
     */
    [[nodiscard]] Modes mode() const { return this->m_mode; }

public slots:
    void setCurrentFragment( Fragment *fragment );
    void setFragmentEnabled( Fragment *fragment, bool enabled = true );
    void setStatusMessage( const QString &message = QString());
    void setErrorMessage( const QString &message = QString());
    void clearStatusMessage();
    void setReagentId( const Id &reagentId ) { this->m_reagentId = reagentId; }

private:
    Ui::ExtractionDialog *ui;
    Id m_reagentId = Id::Invalid;
    Id m_batchId = Id::Invalid;
    Modes m_mode = SearchMode;

    SearchFragment *m_searchFragment;
    StructureFragment *m_structureFragment;
    PropertyFragment *m_propertyFragment;
};
