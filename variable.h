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
#include <QMap>
#include <QString>
#include <QMetaMethod>
#include <QLoggingCategory>
#include <QWidget>
#include "variableentry.h"

/**
 * @brief The Variable_ namespace
 */
namespace Variable_ {
    const static QLoggingCategory Debug( "variable" );
}

//
// classes
//
class Widget;
class XMLTools;

/**
 * @brief The Variable class
 */
class Variable final : public QObject {
    Q_DISABLE_COPY( Variable )
    Q_OBJECT
    friend class XMLTools;

public:
    ~Variable() override;

    /**
     * @brief instance
     * @return
     */
    static Variable *instance() {
        static auto *instance( new Variable());
        return instance;
    }

    /**
     * @brief contains
     * @param key
     * @return
     */
    [[nodiscard]] bool contains( const QString &key ) const { return this->list.contains( key ); }

    /**
     * @brief value
     * @param key
     * @param defaultValue
     * @return
     */
    template<typename T>
    static T value( const QString &key, bool defaultValue = false ) {
        if ( !Variable::instance()->contains( key ))
            return QVariant().value<T>();
        if ( defaultValue ) return qvariant_cast<T>( Variable::instance()->list[key]->defaultValue());
        return qvariant_cast<T>( Variable::instance()->list[key]->value());
    }

    /**
     * @brief integer
     * @param key
     * @param defaultValue
     * @return
     */
    [[maybe_unused]] static int integer( const QString &key, bool defaultValue = false ) {
        return Variable::value<int>( key, defaultValue );
    }

    /**
     * @brief decimalValue
     * @param key
     * @param defaultValue
     * @return
     */
    [[maybe_unused]] static qreal decimalValue( const QString &key, bool defaultValue = false ) {
        return Variable::value<qreal>( key, defaultValue );
    }

    /**
     * @brief isEnabled
     * @param key
     * @param defaultValue
     * @return
     */
    static bool isEnabled( const QString &key, bool defaultValue = false ) {
        return Variable::value<bool>( key, defaultValue );
    }

    /**
     * @brief isDisabled
     * @param key
     * @param defaultValue
     * @return
     */
    static bool isDisabled( const QString &key, bool defaultValue = false ) {
        return !Variable::isEnabled( key, defaultValue );
    }

    /**
     * @brief string
     * @param key
     * @param defaultValue
     * @return
     */
    static QString string( const QString &key, bool defaultValue = false ) {
        return Variable::value<QString>( key, defaultValue );
    }

    /**
     * @brief compressedString
     * @param key
     * @param defaultValue
     * @return
     */
    static QString compressedString( const QString &key, bool defaultValue = false ) {
        return Variable::uncompressString( Variable::value<QString>( key, defaultValue ));
    }

    /**
     * @brief compressedByteArray
     * @param key
     * @param defaultValue
     * @return
     */
    static QByteArray compressedByteArray( const QString &key, bool defaultValue = false ) {
        return qUncompress(
                QByteArray::fromBase64( Variable::string( key, defaultValue ).toUtf8().constData()));
    }

    /**
     * @brief compressString
     * @param string
     * @return
     */
    [[nodiscard]]
    static QString compressString( const QString &string ) {
        return qCompress( QByteArray( string.toUtf8().constData())).toBase64().constData();
    }

    /**
     * @brief uncompressString
     * @param string
     * @return
     */
    [[nodiscard]]
    static QString uncompressString( const QString &string ) {
        if ( string.isEmpty()) return QString();
        return qUncompress( QByteArray::fromBase64( string.toUtf8().constData())).constData();
    }

    /**
     * @brief updateConnections
     * @param key
     * @param value
     */
    template<typename T>
    void updateConnections( const QString &key, const T &value ) {
        if ( Variable::instance()->slotList.contains( key )) {
            QPair<QObject *, int> slot;

            slot = Variable::instance()->slotList[key];
            slot.first->metaObject()->method( slot.second ).invoke( slot.first, Qt::QueuedConnection,
                                                                    Q_ARG( QVariant, value ));
        }
    }

    /**
     * @brief validate
     * @param var
     * @return
     */
    [[nodiscard]] static QVariant validate( const QVariant &value ) {
        QVariant var( value );
        if ( var.type() == QVariant::String ) {
            if ( !QString::compare( var.toString(), "true" )) {
                var = true;
            } else if ( !QString::compare( var.toString(), "false" )) {
                var = false;
            }
        }
        return var;
    }

    /**
     * @brief setValue
     * @param key
     * @param value
     * @param initial
     */
    template<typename T>
    static void setValue( const QString &key, const T &value, bool initial = false ) {
        QVariant var( Variable::validate( value ));

        if ( initial ) {
            // initial read from configuration file
            Variable::instance()->list[key]->setValue( var );
        } else {
            QVariant currentValue;

            if ( !Variable::instance()->contains( key ))
                return;

            currentValue = Variable::instance()->list[key]->value();

            // any subsequent value changes emit a valueChanged signal
            if ( value != currentValue ) {
                Variable::instance()->list[key]->setValue( var );
                emit Variable::instance()->valueChanged( key );
                Variable::instance()->updateConnections( key, var );
            }
        }
    }

    /**
     * @brief add
     * @param key
     * @param value
     * @param flags
     */
    template<typename T>
    static void add( const QString &key, const T &value, Var::Flags flags = Var::Flag::NoFlags ) {
        Variable::add<Var, T>( key, value, flags );
    }

    /**
     * @brief add
     * @param key
     * @param value
     * @param flags
     */
    template<class Container, typename T>
    static void add( const QString &key, const T &value, Var::Flags flags = Var::Flag::NoFlags ) {
        const QVariant var( Variable::validate( value ));

        if ( !Variable::instance()->list.contains( key ) && !key.isEmpty())
            Variable::instance()->list[key] = Container( key, var, flags ).copy();
    }

public slots:
    /**
     * @brief setInteger
     * @param key
     * @param value
     */
    static void setInteger( const QString &key, int value ) { Variable::setValue<int>( key, value ); }

    /**
     * @brief setDecimalValue
     * @param key
     * @param value
     */
    static void setDecimalValue( const QString &key, qreal value ) { Variable::setValue<qreal>( key, value ); }

    /**
     * @brief setCompressedString
     * @param key
     * @param string
     */
    static void setCompressedString( const QString &key, const QString &string ) {
        Variable::setValue<QString>( key, Variable::compressString( string ));
    }

    /**
     * @brief setCompressedByteArray
     * @param key
     * @param byteArray
     */
    static void setCompressedByteArray( const QString &key, const QByteArray &byteArray ) {
        Variable::setValue<QString>( key, qCompress( byteArray ).toBase64().constData());
    }

    /**
     * @brief setEnabled
     * @param key
     * @param value
     */
    static void setEnabled( const QString &key, bool value ) { Variable::setValue<bool>( key, value ); }

    /**
     * @brief enable
     * @param key
     */
    static void enable( const QString &key ) { Variable::setValue<bool>( key, true ); }

    /**
     * @brief disable
     * @param key
     */
    static void disable( const QString &key ) { Variable::setValue<bool>( key, false ); }

    /**
     * @brief setString
     * @param key
     * @param string
     */
    static void setString( const QString &key, const QString &string ) {
        Variable::setValue<QString>( key, string );
    }

    /**
     * @brief reset
     * @param key
     */
    static void reset( const QString &key ) {
        if ( Variable::instance()->contains( key ))
            Variable::setValue<QVariant>( key, Variable::value<QVariant>( key, true ));
    }
    void bind( const QString &key, const QObject *receiver, const char *method );
    QString bind( const QString &key, QObject *object );

    /**
     * @brief bind
     * @param key
     * @param widget
     * @return
     */
    QString bind( const QString &key, QWidget *widget ) { return this->bind( key, qobject_cast<QObject *>( widget )); }
    void unbind( const QString &key, QObject *object = nullptr );

    /**
     * @brief update
     * @param key
     */
    void update( const QString &key ) { emit this->valueChanged( key ); }

signals:
    void valueChanged( const QString &key );
    void widgetChanged( const QString &key, Widget *widget, const QVariant &value );

private:
    explicit Variable();
    QMap<QString, QSharedPointer<Var>> list;
    QMultiMap<QString, Widget *> boundVariables;
    QMap<QString, QPair<QObject *, int> > slotList;
};
