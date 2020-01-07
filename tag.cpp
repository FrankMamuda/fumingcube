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

/*
 * includes
 */
#include "tag.h"
#include "field.h"
#include "database.h"

/**
 * @brief Tag::Tag
 */
Tag::Tag() : Table( "tag" ) {
    this->addField( PRIMARY_FIELD( ID ));
    this->addField( UNIQUE_FIELD( Name, String ));
    this->addField( FIELD( Type, Int ));
    this->addField( FIELD( Units, String ));
    this->addField( FIELD( MinValue, ByteArray ));
    this->addField( FIELD( MaxValue, ByteArray ));
    this->addField( FIELD( DefaultValue, ByteArray ));
    this->addField( FIELD( Precision, Int ));
    this->addField( FIELD( Function, String ));
    this->addField( FIELD( Scale, Double ));
    this->addField( FIELD( Script, ByteArray ));
    // right now 'Script' field holds a simple string list for PubChem parameters (heading, subheading, regExp)
    // however in the furture it is intended to fully offload this task to javascript and have a working
    // and scripted (not hardcoded) property extraction system from mutiple sources (PubChem, wiki, etc.)
    // qCompress will pronably used to store the tag, therefore it is a byte array
}

/**
 * @brief Tag::add
 * @param name
 * @return
 */
Row Tag::add( const QString &name, const Types &type, const QString &units, const QVariant &min, const QVariant &max, const QVariant &value, const int precision, const QString &function, const qreal scale , const QVariant &script ) {
    return Table::add( QVariantList() << Database_::null << name << static_cast<int>( type ) << units << min.toByteArray() << max.toByteArray() << value.toByteArray() << precision << function << scale << QByteArray( script.toStringList().join( ";" ).toUtf8().constData()));
}

/**
 * @brief Tag::removeOrphanedEntries
 */
void Tag::removeOrphanedEntries() {
    // NOTE: STUB
}

/**
 * @brief Tag::populate
 */
void Tag::populate() {
    this->add( this->tr( "Molar mass" ),           Real, this->tr( "&nbsp;g/mol" ),           1.0,      "",  18.0,  2, "molarMass",     1.00,
               QStringList() << "Molecular Weight" << QString() << "((?:\\d+,)?(?:\\d+.)?\\d+)(\\sg\\/mol)?" );
    this->add( this->tr( "Density" ),              Real, this->tr( "&nbsp;g/cm<sup>3</sup>" ), 0.001, 100.0,   1.0,  3, "density",       1.00,
               QStringList() << "Density" << QString() << "(\\d+(?:\\.\\d+)?)(?!\\)$)(\\sg\\/[\\w|\\d]+)?" );
    this->add( this->tr( "Assay" ),                Real, this->tr( "&percnt;" ),              0.0,   110.0, 100.0,  3, "assay",         0.01 );
    this->add( this->tr( "State" ),                State );
    this->add( this->tr( "Analysis id" ));
    this->add( this->tr( "Loss on drying" ),       Real, this->tr( "&percnt;" ),              0.0,   1000.0,  0.0,  2, "lossOnDrying",  0.01 );
    this->add( this->tr( "HPLC purity" ),          Real, this->tr( "&percnt;" ),              0.0,    110.0, 100.0, 2, "HPLC",          0.01 );
    this->add( this->tr( "GC purity" ),            Real, this->tr( "&percnt;" ),              0.0,    110.0, 100.0, 2, "GC",            0.01 );
    this->add( this->tr( "Related substances" ),   Real, this->tr( "&percnt;" ),              0.0,    110.0,   0.0, 2, "relatedSubst",  0.01 );
    this->add( this->tr( "Impurities" ),           Real, this->tr( "&percnt;" ),              0.0,    110.0,   0.0, 2, "impurities",    0.01 );
    this->add( this->tr( "Water content" ),        Real, this->tr( "&percnt;" ),              0.0,   1000.0,   0.0, 2, "waterContent",  0.01 );
    this->add( this->tr( "Concentration" ),        Real, this->tr( "&percnt;" ),              0.0,    110.0, 100.0, 5, "concentration", 0.01 );
    this->add( this->tr( "Boiling point" ),        Real, this->tr( "&deg;C" ),            -273.15,       "", 100.0, 1, "boilingPoint",  1.00,
               QStringList() << "Boiling Point" << QString() << "(-?(?:\\d+,)?(?:\\d+.)?\\d+\\s?(?!\\)$))(°?[CFK])?" );
    this->add( this->tr( "Melting point" ),        Real, this->tr( "&deg;C" ),            -273.15,       "",   0.0, 1, "meltingPoint",  1.00,
               QStringList() << "Melting Point" << QString() << "(-?(?:\\d+,)?(?:\\d+.)?\\d+\\s?(?!\\)$))(°?[CFK])?" );
    this->add( this->tr( "Flash point" ),          Real, this->tr( "&deg;C" ),            -273.15,       "",  50.0, 1, "flashPoint",    1.00,
               QStringList() << "Flash Point" << QString() << "(-?(?:\\d+,)?(?:\\d+.)?\\d+\\s?(?!\\)$))(°?[CFK])?)" );
    this->add( this->tr( "CAS number" ),           CAS,  "",                                   "",       "",    "", 0, "",              1.00,
               QStringList() << "CAS" << QString() << "(\\d+-\\d+-\\d+)" );
    this->add( this->tr( "Viscosity" ),            Real, this->tr( "&nbsp;mPa·s" ),            "",       "",  1.00, 3, "viscosity",     1.00 );
    this->add( this->tr( "Reftractive index" ),    Real, "",                                  1.0,     10.0,  1.00, 3, "",              1.00,
               QStringList() << "Refractive Index" << QString() << "([1234]\\.\\d+)" );
    this->add( this->tr( "GHS pictograms" ),       GHS,  "",                                    0,        0,     0, 0, "",                 0,
               QStringList() << "GHS Classification" << "Pictogram(s)" << "([Ee]xplosive|[Ff]lammable|[Oo]xidizing|[Cc]ompressed\\s[Gg]as|[Cc]orrosive|[Tt]oxic|[Hh]armful|[Hh]ealth\\s[Hh]azard|[Ee]nvironmental\\s[Hh]azard|[Ii]rritant)" << "1" );
    this->add( this->tr( "NFPA 704" ),             NFPA, "",                                    0,        0,     0, 0, "",                 0,
               QStringList() << "NFPA Hazard Classification" << "NFPA 704 Diamond" << "(\\d)(?:-(\\d))(?:-(\\d))" );
    this->add( this->tr( "Acidity (pKa)" ),        Real, "",                               -100.0,    100.0,  14.0, 2, "pKa"                 );
    this->add( this->tr( "Basicity (pKb)" ),       Real, "",                               -100.0,    100.0,   0.0, 2, "pKb"                 );
    this->add( this->tr( "Producer" ));
    this->add( this->tr( "Supplier" ));
    this->add( this->tr( "Structural formula" ),   Formula );
    this->add( this->tr( "Physical description" ), Text, "",                                    0,        0,     0, 0, "",                 0,
               QStringList() << "Physical Description" << "Physical Description" );
}
