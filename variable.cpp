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

/*
 * includes
 */
#include "variable.h"
#include "widget.h"
#include <QCheckBox>
#include <QComboBox>
#include <QAction>

/**
 * @brief Variable::Variable
 */
Variable::Variable() {
    // update widgets on variable change
    Variable::connect( this, &Variable::valueChanged, [ this ]( const QString &key ) {
        const QList<Widget*> list( this->boundVariables.values( key ));
        for ( Widget *widget : list ) {
            auto var( Variable::value<QVariant>( key ));

            if ( widget->value() != var )
                widget->setValue( var );
        }
    } );

    // update variable and sibling widgets on widget change
    Variable::connect( this, &Variable::widgetChanged,
                   [ this ]( const QString &key, Widget *widget, const QVariant &value ) {
                       const QList<Widget*> list( this->boundVariables.values( key ));
                       for ( Widget *boundWidget : list ) {
                           if ( boundWidget == widget ) {
                               Variable::setValue( key, value );
                           } else {
                               boundWidget->setValue( value );
                           }
                       }
                   } );
}

/**
 * @brief Variable::~Variable
 */
Variable::~Variable() {
    this->disconnect( this, SIGNAL( widgetChanged( QString, Widget *, QVariant )));
    this->disconnect( this, SIGNAL( valueChanged( QString )));
}

/**
 * @brief Variable::bind
 * @param key
 * @param receiver
 * @param method
 */
void Variable::bind( const QString &key, const QObject *receiver, const char *method ) {
    if ( key.isEmpty())
        return;

    // check if method is a slot
    const int code = (( static_cast<int>( *method ) - '0' ) & 0x3 );
    if ( code != 1 )
        return;

    // create an object/method pair and add pair to slotList
    ++method;
    this->slotList[key] = qMakePair( const_cast<QObject *>( receiver ), receiver->metaObject()->indexOfSlot( QMetaObject::normalizedSignature( qPrintable( method ))));
}

/**
 * @brief Variable::bind
 * @param key
 * @param object
 * @return
 */
QString Variable::bind( const QString &key, QObject *object ) {
    auto *boundWidget( new Widget( object ));

    boundWidget->setValue( Variable::value<QVariant>( key ));
    Variable::connect( boundWidget, &Widget::changed, this, [ this, key, boundWidget ]( const QVariant &value ) {
        emit this->widgetChanged( key, boundWidget, value );
    } );
    this->boundVariables.insert( key, boundWidget );

    return key;
}

/**
 * @brief Variable::unbind
 * @param key
 * @param object
 */
void Variable::unbind( const QString &key, QObject *object ) {
    if ( this->slotList.contains( key ))
        this->slotList.remove( key );

    if ( this->boundVariables.contains( key )) {
        QList<Widget *> widgetList( this->boundVariables.values( key ));

        if ( object == nullptr ) {
            qDeleteAll( widgetList );
            this->boundVariables.remove( key );
            return;
        }

        const QList<Widget*> list( this->boundVariables.values( key ));
        for ( Widget *compare : list ) {
            if ( compare->widget == object ) {
                this->boundVariables.remove( key, compare );
                delete compare;
            }
        }
    }
}
