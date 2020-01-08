/*
 * Copyright (C) 2018-2020 Armands Aleksejevs
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
#include <QSqlRecord>
#include "table.h"
#include "database.h"
#include "field.h"
#include <QDebug>
#include <QSqlQuery>

/**
 * @brief Table::count
 * @return
 */
int Table::count() const {
    return Database::instance()->hasInitialised() ? this->rowCount() : 0;
}

/**
 * @brief Table::value
 * @param row
 * @param fieldId
 * @return
 */
QVariant Table::value( const Row &row, int fieldId ) const {
    if ( !this->isValid())
        return -1;

    const QModelIndex index( this->index( static_cast<int>( row ), fieldId ));
    if ( row == Row::Invalid || !index.isValid() || index.row() < 0 || index.row() >= this->count()) {
        qWarning( Database_::Debug ) << this->tr( "could not retrieve field \"%1\" value from table \"%2\"" )
                                        .arg( this->field( fieldId )->name())
                                        .arg( this->tableName()) << !index.isValid()
                                     << ( index.row() < 0 ) << ( index.row() >= this->count()) << row;
        return -1;
    }

    return QSqlTableModel::data( index );
}


/**
 * @brief Table::value this only works if table has a primary key
 * @param id
 * @param fieldId
 * @return
 */
QVariant Table::value( const Id &id, int fieldId ) const {
    if ( !this->isValid())
        return -1;

    if ( !this->hasPrimaryField())
        return -1;

    QSqlQuery query;
    query.exec( QString( "select %1, %2 from %3 where %1=%4" )
                .arg( this->fieldName( this->primaryField()->id()))
                .arg( this->fieldName( fieldId ))
                .arg( this->tableName())
                .arg( static_cast<int>( id )));

    return query.next() ? query.value( 1 ) : "";
}

/**
 * @brief Table::select
 * @return
 */
bool Table::select() {
    const bool result = QSqlTableModel::select();

    // fetch more
    while ( this->canFetchMore())
        this->fetchMore();

    return result;
}

/**
 * @brief Table::row
 * @param id
 * @return
 */
Row Table::row( const Id &id ) const {
    const QModelIndexList list( this->match( this->index( 0, 0 ), IDRole, static_cast<int>( id ), 1, Qt::MatchExactly ));
    return this->row( list.isEmpty() ? QModelIndex() : list.first());
}

/**
 * @brief Table::addConstraint
 * @param fields
 */
void Table::addUniqueConstraint( const QList<QSharedPointer<Field_>> &fields ) {
    this->constraints << fields;
}

/**
 * @brief Table::data
 * @param index
 * @param role
 * @return
 */
QVariant Table::data( const QModelIndex &index, int role ) const {
    if ( !Database::instance()->hasInitialised())
        return QVariant();

    if ( role == IDRole || role == Qt::UserRole ) {
        if ( !index.isValid())
            return static_cast<int>( Id::Invalid );

        return this->hasPrimaryField() ? this->value( static_cast<Row>( index.row()), this->primaryField()->id()).toInt() : -1;
    }

    return QSqlTableModel::data( index, role );
}

/**
 * @brief Table::setFilter
 * @param filter
 */
void Table::setFilter( const QString &filter ) {
    QSqlTableModel::setFilter( filter );
    this->select();
}

/**
 * @brief Table::fieldName
 * @param id
 * @return
 */
QString Table::fieldName( int id ) const {
    return this->field( id )->name();
}

/**
 * @brief Table_::fieldFromId
 * @param id
 * @return
 */
Field Table::field( int id ) const {
    if ( this->fields.contains( id ))
        return this->fields[id];

    return Field();
}

/**
 * @brief Table_::addField
 * @param id
 * @param fieldName
 * @param type
 * @param format
 * @param unique
 * @param autoValue
 */
void Table::addField( int id, const QString &fieldName, QVariant::Type type, const QString &format, bool unique, bool autoValue ) {
    if ( this->fields.contains( id ))
        return;

    Field field( new Field_( id, fieldName, type, format, unique, autoValue ));
    this->fields[id] = field;
    if ( field->isPrimary() && this->primaryField().isNull()) {
        this->m_primaryField = field;
        this->m_hasPrimary = true;
    }
}

/**
 * @brief Table::add
 * @param name
 */
Row Table::add( const QVariantList &arguments ) {
    int y;

    if ( !this->isValid())
        return Row::Invalid;

    if ( this->fields.count() != arguments.count())
        qCCritical( Database_::Debug ) << this->tr( "argument count mismatch - %1, required - %2" ).arg( arguments.count()).arg( this->fields.count());

    // insert empty row
    const int row = this->count();
    this->beginInsertRows( QModelIndex(), this->count(), this->count());
    if ( !this->insertRow( this->count())) {
        qCCritical( Database_::Debug ) << this->tr( "cannot insert row into table \"%1\"" ).arg( this->tableName());
        this->endInsertRows();
        return Row::Invalid;
    }

    // prepare statement
    for ( y = 0; y < arguments.count(); y++ ) {
        Field field;
        QVariant argument;

        // get field and argument
        field = this->fields[y];
        argument = arguments.at( y );

        // compare types
        if ( field->type() != argument.type()) {
            qCCritical( Database_::Debug ) << this->tr( "incompatible field type - %1 for argument %2 (%3), required - %4" ).arg( argument.type()).arg( y ).arg( field->format()).arg( field->type());
            this->revert();
            this->endInsertRows();
            return Row::Invalid;
        }

        // check for unique fields
        if ( field->isUnique() && !field->isAutoValue()) {
            if ( this->contains( field, argument )) {
                qCWarning( Database_::Debug )
                        << this->tr( "table already has a unique field \"%1\" with value - \"%2\", aborting addition" )
                           .arg( field->name()).arg( argument.toString());
                this->revert();
                this->endInsertRows();
                return Row::Invalid;
            }
        }

        // set data if field is not primary
        if ( !field->isPrimary())
            this->setData( this->index( row, y ), argument );
        else
            this->m_hasPrimary = true;
    }

    this->submit();
    this->endInsertRows();
    this->select();
    return this->row( row );
}

/**
 * @brief Table::prepare
 * @return
 */
QSqlQuery Table::prepare() const {
    if ( !this->isValid())
        return QSqlQuery();

    // prepare statement
    QString statement( "insert or ignore into " + this->tableName() + " (" );
    QString values;
    for ( int y = 0; y < this->fields.count(); y++ ) {
        const Field &field( this->fields[y] );
        if ( field->isPrimary())
            continue;

        const bool last = ( y == this->fields.count() - 1 );

        values.append( " :_" + field->name() + + ( last ? " )" : "," ));
        statement.append( " " + field->name() + ( last ? " ) values(" + values : "," ));
    }
    QSqlQuery query;
    query.prepare( statement );

    return query;
}

/**
 * @brief Table::bind
 * @param query
 * @param arguments
 */
void Table::bind( QSqlQuery &query, const QVariantList &arguments ) {
    if ( !this->isValid())
        return;

    const int numFields = this->fields.count() - ( this->hasPrimaryField() ? 1 : 0 );
    if ( numFields != arguments.count())
        qCCritical( Database_::Debug ) << this->tr( "argument count mismatch - %1, required - %2" ).arg( arguments.count()).arg( this->fields.count());

    // prepare statement
    int y = 0;
    foreach ( const Field &field, this->fields ) {
        if ( field->isPrimary())
            continue;

        const QVariant argument( arguments.at( y ));

        // compare types
        if ( field->type() != argument.type()) {
            qCCritical( Database_::Debug ) << this->tr( "incompatible field type - %1 for argument %2 (%3), required - %4" )
                                              .arg( argument.type()).arg( y ).arg( field->format()).arg( field->type());
            return;
        }

        // bind value
        query.bindValue( ":_" + field->name(), argument );

        y++;
    }
}

/**
 * @brief Table::remove
 * @param row
 */
void Table::remove( const Row &row ) {
    if ( !this->isValid() || row == Row::Invalid )
        return;

    this->removeRow( static_cast<int>( row ));
    this->select();
}

/**
 * @brief Table::setValue
 * @param row
 * @param fieldId
 */
void Table::setValue( const Row &row, int fieldId, const QVariant &value ) {
    if ( !this->isValid() || row == Row::Invalid )
        return;

    this->setData( this->index( static_cast<int>( row ), fieldId ), value );
    this->submit();
}

/**
 * @brief Table::contains
 * @param field
 * @param value
 * @return
 */
bool Table::contains( const QSharedPointer<Field_> &field, const QVariant &value ) const {
    int y;

    if ( !this->isValid())
        return false;

    for ( y = 0; y < this->count(); y++ ) {
        if ( this->value( this->row( y ), field->id()) == value )
            return true;
    }
    return false;
}

/**
 * @brief operator <<
 * @param debug
 * @param id
 * @return
 */
QDebug operator<<( QDebug debug, const Id &id ) {
    QDebugStateSaver saver( debug );
    debug.nospace() << "id:" << static_cast<int>( id );

    return debug;
}

/**
 * @brief operator <<
 * @param debug
 * @param row
 * @return
 */
QDebug operator<<( QDebug debug, const Row &row ) {
    QDebugStateSaver saver( debug );
    debug.nospace() << "row:" << static_cast<int>( row );

    return debug;
}

