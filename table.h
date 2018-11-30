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
#include <QSqlRelationalTableModel>
#include <QSharedPointer>
#include <QSqlRecord>

//
// classes
//
class Field_;
class QDebug;

/**
 * namespace Database
 */
namespace Database_ {
#ifdef Q_CC_MSVC
static constexpr const char *TimeFormat = "hh:mm";
#else
static constexpr const char __attribute__((unused)) *TimeFormat = "hh:mm";
#endif
}

/**
 * @brief The Id enum strong-typed id
 */
enum class Id : int { Invalid = -1 };
QDebug operator<<( QDebug debug, const Id &id );

/**
 * @brief The Row enum strong-typed row
 */
enum class Row : int { Invalid = -1 };
QDebug operator<<( QDebug debug, const Row &row );

/**
 * @brief The Table_ class
 */
class Table : public QSqlTableModel {
    Q_OBJECT
    Q_ENUMS( Roles )
    Q_DISABLE_COPY( Table )
    friend class Database_N;

public:
    enum Roles {
        IDRole = Qt::UserRole
    };

    Table( const QString &tableName = QString()) { this->setTable( tableName ); }
    virtual ~Table() { this->setValid( false ); this->clear(); }
    bool isValid() const { return this->m_valid; }
    bool hasPrimaryField() const { return this->m_hasPrimary; }
    int count() const;
    QVariant value( const Row &row, int fieldId ) const;
    bool contains( int fieldId, const QVariant &value ) const { return this->contains( this->field( fieldId ), value ); }
    virtual bool select() override;
    QSharedPointer<Field_> primaryField() const { return this->m_primaryField; }
    virtual QVariant data( const QModelIndex &index, int role ) const override;
    virtual void setFilter( const QString &filter ) override;
    QString fieldName( int id ) const;
    Row row( const int index ) const { if ( index < 0 || index >= this->count()) return Row::Invalid; return static_cast<Row>( index ); }
    Row row( const QModelIndex &index ) const { if ( index.row() < 0 || index.row() >= this->count() || index.model() != this ) return Row::Invalid; return static_cast<Row>( index.row()); }
    QModelIndex find( const Id &id ) const;
    Row row( const Id &id ) const;

public slots:
    void setValid( bool valid = true ) { this->m_valid = valid; }
    void addField( int id, const QString &fieldName = QString(), QVariant::Type type = QVariant::Invalid, const QString &format = QString( "text" ), bool unique = false, bool autoValue = false );
    Row add( const QVariantList &arguments );
    void remove( const Row &row );
    void setValue( const Row &row, int fieldId, const QVariant &value );

protected:
    QMap<int, QSharedPointer<Field_>> fields;
    QSharedPointer<Field_> field( int id ) const;
    bool contains( const QSharedPointer<Field_> &field, const QVariant &value ) const;

private:
    bool m_valid = false;
    bool m_hasPrimary = false;
    QSharedPointer<Field_> m_primaryField;
};

// declare enums
Q_DECLARE_METATYPE( Table::Roles )
Q_DECLARE_METATYPE( Id )
Q_DECLARE_METATYPE( Row )
