/*
 * Copyright (C) 2018 Factory #12
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

//
// includes
//
#include "table.h"

/**
 * @brief The TemplateTable namespace
 */
namespace TemplateTable {
const static QString Name( "templates" );
}

/**
 * @brief The Template class
 */
class Template final : public Table {
    Q_OBJECT
    Q_ENUMS( Fields )
    Q_ENUMS( State )
    Q_DISABLE_COPY( Template )

public:
    enum Fields {
        NoField = -1,
        ID,
        Name,
        Amount,
        Density,
        Assay,
        MolarMass,
        ChemState,
        Reagent,

        // count
        Count
    };

    /**
     * @brief instance
     * @return
     */
    static Template *instance() { static Template *instance = new Template(); return instance; }
    virtual ~Template() {}

    enum State {
        Solid = 0,
        Liquid
    };

    Row add( const QString &name, const double amount, const double density, const double assay, const double molarMass, const State &state, const Id &reagentId );
    Id id( const Row &row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }
    QString name( const Row &row ) const { return this->value( row, Name ).toString(); }
    qreal amount( const Row &row ) const { return this->value( row, Amount ).toDouble(); }
    qreal density( const Row &row ) const { return this->value( row, Density ).toDouble(); }
    qreal assay( const Row &row ) const { return this->value( row, Assay ).toDouble(); }
    qreal molarMass( const Row &row ) const { return this->value( row, MolarMass ).toDouble(); }
    State state( const Row &row ) const { return static_cast<State>( this->value( row, ChemState ).toInt()); }
    Id reagentId( const Row &row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }

public slots:
    void setName( const Row &row, const QString &name ) { this->setValue( row, Name, name ); }
    void setAmount( const Row &row, const qreal &amount ) { this->setValue( row, Amount, amount ); }
    void setDensity( const Row &row, const qreal &density ) { this->setValue( row, Density, density ); }
    void setAssay( const Row &row, const qreal &assay ) { this->setValue( row, Assay, assay ); }
    void setMolarMass( const Row &row, const qreal &molarMass ) { this->setValue( row, MolarMass, molarMass ); }
    void setState( const Row &row, const State &state ) { this->setValue( row, ChemState, state ); }
    void removeOrphanedEntries() override;

private:
    explicit Template();
};

// declare enums
Q_DECLARE_METATYPE( Template::Fields )
