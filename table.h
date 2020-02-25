/*
 * Copyright (C) 2018-2019 Armands Aleksejevs
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
#include <QSqlRelationalTableModel>
#include <QSharedPointer>
#include <QSqlRecord>

//
// classes
//
class Field_;
class QDebug;
class QSqlQuery;

/**
 * @brief The Id enum strong-typed id
 */
enum class Id : int {
    Invalid = -1
};
QDebug operator<<( QDebug debug, const Id &id );
using _Id = Id;

Q_DECLARE_METATYPE( Id )

/**
 * @brief The Row enum strong-typed row
 */
enum class Row : int {
    Invalid = -1
};
QDebug operator<<( QDebug debug, const Row &row );

Q_DECLARE_METATYPE( Row )

/*
 * FIELD macro generates a lowercase fieldName from field index (enum)
 */
static const QMap<QVariant::Type, QString> _fieldTypes {{ QVariant::Int,       "integer" },
                                                        { QVariant::Double,    "real" },
                                                        { QVariant::String,    "text" },
                                                        { QVariant::Color,     "colour" },
                                                        { QVariant::ByteArray, "blob" }};
#define FIELD( fieldId, type ) fieldId, QString( #fieldId ).replace( 0, 1, QString( #fieldId ).at( 0 ).toLower()), QVariant::type, _fieldTypes[QVariant::type]
#define UNIQUE_FIELD( fieldId, type ) fieldId, QString( #fieldId ).replace( 0, 1, QString( #fieldId ).at( 0 ).toLower()), QVariant::type, _fieldTypes[QVariant::type], true
#define PRIMARY_FIELD( fieldId ) fieldId, QString( #fieldId ).toLower(), QVariant::Int, "integer primary key", true, true
#define FIELD_GETTER( type, fieldId, name ) public:  type name( const Row &row ) const { return this->value( row, fieldId ).value<type>(); } type name( const Id &id ) const { return this->value( id, fieldId ).value<type>(); }
#define FIELD_SETTER( type, fieldId ) public slots: void set##fieldId( const Row &row, const type &variable ) { this->setValue( row, fieldId, QVariant::fromValue( variable )); }
#define INITIALIZE_FIELD( type, fieldId, name ) FIELD_GETTER( type, fieldId, name ) FIELD_SETTER( type, fieldId )

/**
 * @brief The Table_ class
 */
class Table : public QSqlTableModel {
    Q_OBJECT
    Q_DISABLE_COPY( Table )
    friend class Database;

public:
    // disable move
    Table( Table&& ) = delete;
    Table& operator=( Table&& ) = delete;

    enum Roles {
        IDRole = Qt::UserRole
    };
    Q_ENUM( Roles )

    explicit Table( const QString &tableName = QString()) { this->setTable( tableName ); }
    ~Table() override {
        this->setValid( false );
        this->clear();
    }

    /**
     * @brief isValid
     * @return
     */
    [[nodiscard]] bool isValid() const { return this->m_valid; }

    /**
     * @brief hasPrimaryField
     * @return
     */
    [[nodiscard]] bool hasPrimaryField() const { return this->m_hasPrimary; }
    Q_INVOKABLE [[nodiscard]] int count() const;

    [[nodiscard]] virtual QVariant value( const Row &row, int fieldId ) const;
    [[nodiscard]] virtual QVariant value( const Id &id, int fieldId ) const;

    /**
     * @brief contains
     * @param fieldId
     * @param value
     * @return
     */
    [[nodiscard]] bool contains( int fieldId, const QVariant &value ) const {
        return this->contains( this->field( fieldId ), value );
    }
    bool select() override;

    /**
     * @brief primaryField
     * @return
     */
    [[nodiscard]] QSharedPointer<Field_> primaryField() const { return this->m_primaryField; }
    [[nodiscard]] QVariant data( const QModelIndex &index, int role ) const override;
    void setFilter( const QString &filter ) override;
    [[nodiscard]] QString fieldName( int id ) const;

    /**
     * @brief row
     * @param index
     * @return
     */
    [[nodiscard]] Row row( const int index ) const {
        if ( index < 0 || index >= this->count()) return Row::Invalid;
        return static_cast<Row>( index );
    }

    /**
     * @brief row
     * @param index
     * @return
     */
    [[nodiscard]] Row row( const QModelIndex &index ) const {
        if ( index.row() < 0 || index.row() >= this->count() || index.model() != this )return Row::Invalid;
        return static_cast<Row>( index.row());
    }
    [[nodiscard]] Row row( const Id &id ) const;

    [[maybe_unused]] void addUniqueConstraint( const QList<QSharedPointer<Field_>> &constrainedFields );
    [[maybe_unused]][[nodiscard]] QSqlQuery prepare() const;
    [[maybe_unused]] void bind( QSqlQuery &query, const QVariantList &arguments );

public slots:
    /**
     * @brief setValid
     * @param valid
     */
    void setValid( bool valid = true ) { this->m_valid = valid; }
    void addField( int id, const QString &fieldName = QString(), QVariant::Type type = QVariant::Invalid,
                   const QString &format = QString( "text" ), bool unique = false, bool autoValue = false );
    Row add( const QVariantList &arguments );
    void remove( const Row &row );
    void setValue( const Row &row, int fieldId, const QVariant &value );

    /**
     * @brief removeOrphanedEntries
     */
    virtual void removeOrphanedEntries() {}

protected:
    QMap<int, QSharedPointer<Field_>> fields;
    [[nodiscard]] QSharedPointer<Field_> field( int id ) const;
    [[nodiscard]] bool contains( const QSharedPointer<Field_> &field, const QVariant &value ) const;

private:
    bool m_valid = false;
    bool m_hasPrimary = false;
    QSharedPointer<Field_> m_primaryField;
    QList<QList<QSharedPointer<Field_>>> constraints;
};

// declare enums
Q_DECLARE_METATYPE( Table::Roles )
