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

#include <QSqlQuery>

/**
 * @brief Tag::Tag
 */
Tag::Tag() : Table( "tag" ) {
    this->addField( PRIMARY_FIELD( ID ) );
    this->addField( UNIQUE_FIELD( Name, String ) );
    this->addField( FIELD( Type, Int ) );
    this->addField( FIELD( Units, String ) );
    this->addField( FIELD( MinValue, ByteArray ) );
    this->addField( FIELD( MaxValue, ByteArray ) );
    this->addField( FIELD( DefaultValue, ByteArray ) );
    this->addField( FIELD( Precision, Int ) );
    this->addField( FIELD( Function, String ) );
    this->addField( FIELD( Scale, Double ) );
    this->addField( FIELD( Script, ByteArray ) );
    // right now 'Script' field holds a simple string list for PubChem parameters (heading, subheading, regExp)
    // however in the future it is intended to fully offload this task to javascript and have a working
    // and scripted (not hardcoded) property extraction system from multiple sources (PubChem, wiki, etc.)
    // qCompress will probably used to store the tag, therefore it is a byte array
}

/**
 * @brief Tag::add
 * @param name
 * @return
 */
Row Tag::add( const QString &name, const Types &type, const QString &units, const QVariant &min, const QVariant &max,
              const QVariant &value, const int precision, const QString &function, const qreal scale,
              const QVariant &script ) {
    return Table::add(
            QVariantList() << Database_::null << name << static_cast<int>( type ) << units << min.toByteArray()
                           << max.toByteArray() << value.toByteArray() << precision << function << scale
                << QByteArray( script.toStringList().join( ";" ).toUtf8().constData()));
}

/**
 * @brief Tag::getFunctionList
 * @return
 */
QStringList Tag::getFunctionList() const {
    // NOTE: this is not very expensive so there is no need to optimize this yet
    QStringList functions;
    QSqlQuery query;
    query.exec( QString( "select %1, %2 from %3 where %2 not null" )
                        .arg( Tag::instance()->fieldName( Tag::ID ),
                              Tag::instance()->fieldName( Tag::Function ),
                              Tag::instance()->tableName()));
    while ( query.next()) {
        const QString functionName( query.value( 1 ).toString());
        if ( !functionName.isEmpty())
            functions << functionName;
    }

    return functions;
}

/**
 * @brief Tag::removeOrphanedEntries
 */
void Tag::removeOrphanedEntries() {
    // NOTE: tags do not have foreign ids, therefore they cannot be orphaned
}

/**
 * @brief Tag::populate
 */
void Tag::populate() {
    this->add( Tag::tr( "Molar mass" ), Real, Tag::tr( "&nbsp;g/mol" ), 1.0, "", 18.0, 2, "molarMass", 1.00,
               QStringList() << "Molecular Weight" << QString() << R"(((?:\d+,)?(?:\d+.)?\d+)(\sg\/mol)?)" );
    this->add( Tag::tr( "Density" ), Real, Tag::tr( "&nbsp;g/cm<sup>3</sup>" ), 0.001, 100.0, 1.0, 3, "density", 1.00,
               QStringList() << "Density" << QString() << R"((\d+(?:\.\d+)?)(?!\)$)(\sg\/[\w|\d]+)?)" );
    this->add( Tag::tr( "Assay" ), Real, Tag::tr( "&percnt;" ), 0.0, 110.0, 100.0, 3, "assay", 0.01 );
    this->add( Tag::tr( "State" ), State );
    this->add( Tag::tr( "Analysis id" ));
    this->add( Tag::tr( "Loss on drying" ), Real, Tag::tr( "&percnt;" ), 0.0, 1000.0, 0.0, 2, "lossOnDrying", 0.01 );
    this->add( Tag::tr( "HPLC purity" ), Real, Tag::tr( "&percnt;" ), 0.0, 110.0, 100.0, 2, "HPLC", 0.01 );
    this->add( Tag::tr( "GC purity" ), Real, Tag::tr( "&percnt;" ), 0.0, 110.0, 100.0, 2, "GC", 0.01 );
    this->add( Tag::tr( "Related substances" ), Real, Tag::tr( "&percnt;" ), 0.0, 110.0, 0.0, 2, "relatedSubst",
               0.01 );
    this->add( Tag::tr( "Impurities" ), Real, Tag::tr( "&percnt;" ), 0.0, 110.0, 0.0, 2, "impurities", 0.01 );
    this->add( Tag::tr( "Water content" ), Real, Tag::tr( "&percnt;" ), 0.0, 1000.0, 0.0, 2, "waterContent", 0.01 );
    this->add( Tag::tr( "Concentration" ), Real, Tag::tr( "&percnt;" ), 0.0, 110.0, 100.0, 5, "concentration", 0.01 );
    this->add( Tag::tr( "Boiling point" ), Real, Tag::tr( "&deg;C" ), -273.15, "", 100.0, 1, "boilingPoint", 1.00,
               QStringList() << "Boiling Point" << QString() << R"((-?(?:\d+,)?(?:\d+.)?\d+\s?(?!\)$))(°?[CFK])?)");
    this->add( Tag::tr( "Melting point" ), Real, Tag::tr( "&deg;C" ), -273.15, "", 0.0, 1, "meltingPoint", 1.00,
               QStringList() << "Melting Point" << QString() << R"((-?(?:\d+,)?(?:\d+.)?\d+\s?(?!\)$))(°?[CFK])?)");
    this->add( Tag::tr( "Flash point" ), Real, Tag::tr( "&deg;C" ), -273.15, "", 50.0, 1, "flashPoint", 1.00,
               QStringList() << "Flash Point" << QString() << R"((-?(?:\d+,)?(?:\d+.)?\d+\s?(?!\)$))(°?[CFK])?))");
    this->add( Tag::tr( "CAS number" ), CAS, "", "", "", "", 0, "", 1.00,
               QStringList() << "CAS" << QString() << R"((\d+-\d+-\d+))" );
    this->add( Tag::tr( "Viscosity" ), Real, Tag::tr( "&nbsp;mPa·s" ), "", "", 1.00, 3, "viscosity", 1.00 );
    this->add( Tag::tr( "Refractive index" ), Real, "", 1.0, 10.0, 1.00, 3, "", 1.00,
               QStringList() << "Refractive Index" << QString() << "([1234]\\.\\d+)" );
    this->add( Tag::tr( "GHS pictograms" ), GHS, "", 0, 0, 0, 0, "", 0,
               QStringList() << "GHS Classification" << "Pictogram(s)"
                             << R"(([Ee]xplosive|[Ff]lammable|[Oo]xidizing|[Cc]ompressed\s[Gg]as|[Cc]orrosive|[Tt]oxic|[Hh]armful|[Hh]ealth\s[Hh]azard|[Ee]nvironmental\s[Hh]azard|[Ii]rritant))"
                             << "1" );
    this->add( Tag::tr( "NFPA 704" ), NFPA, "", 0, 0, 0, 0, "", 0,
               QStringList() << "NFPA Hazard Classification" << "NFPA 704 Diamond" << R"((\d)(?:-(\d))(?:-(\d)))" );
    this->add( Tag::tr( "Acidity (pKa)" ), Real, "", -100.0, 100.0, 14.0, 2, "pKa" );
    this->add( Tag::tr( "Basicity (pKb)" ), Real, "", -100.0, 100.0, 0.0, 2, "pKb" );
    this->add( Tag::tr( "Producer" ));
    this->add( Tag::tr( "Supplier" ));
    this->add( Tag::tr( "Structural formula" ), Formula );
    this->add( Tag::tr( "Physical description" ), Text, "", 0, 0, 0, 0, "", 0,
               QStringList() << "Physical Description" << "Physical Description" );
    this->add( Tag::tr( "Solubility" ), Text, "", 0, 0, 0, 0, "", 0,
               QStringList() << "Solubility" << "Solubility" );
    this->add( Tag::tr( "Synonyms" ), Text, "", 0, 0, 0, 0, "", 0,
               QStringList() << "MeSH Entry Terms" << "MeSH Entry Terms" );
    this->add( Tag::tr( "IUPAC Name" ), Text, "", 0, 0, 0, 0, "", 0,
               QStringList() << "IUPAC Name" << "IUPAC Name" );
}
