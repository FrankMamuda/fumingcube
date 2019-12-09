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

/*
 * includes
 */
#include "propertywidget.h"
#include "tag.h"
#include "property.h"

/**
 * @brief PropertyWidget::PropertyWidget
 * @param parent
 * @param values
 * @param tagId
 */
PropertyWidget::PropertyWidget( QWidget *parent, const QList<QStringList> &values, const Id &tagId ) : QWidget( parent ) {
    this->m_tagId = tagId;
    if ( this->tagId() == Id::Invalid )
        return;

    const Tag::Types &type = Tag::instance()->type( tagId );

    this->left->setIcon( QIcon::fromTheme( "left" ));
    this->right->setIcon( QIcon::fromTheme( "right" ));
    this->left->setIconSize( QSize( 8, 16 ));
    this->right->setIconSize( QSize( 8, 16 ));
    this->layout->setContentsMargins( 0, 0, 0, 0 );
    this->layout->setSpacing( 0 );


    if ( values.isEmpty())
        return;

    this->ghs->setLinear();

    int index = 0;
    for ( int y = 0; y < values.count(); y++ ) {
        QStringList valueList( values.at( y ));
        if ( valueList.count() < 2 )
            continue;

        this->displayValues[index] = valueList.takeFirst();
        this->propertyValues[index] = valueList;
        index++;
    }

    if ( displayValues.isEmpty() || propertyValues.isEmpty())
        return;

    switch ( type ) {
    case Tag::Text:
    case Tag::Integer:
    case Tag::Real:
    case Tag::CAS:
        this->label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
        this->label->setWordWrap( true );
        this->label->setText( this->displayValues.first());
        this->layout->addWidget( this->label );
        break;

    case Tag::NFPA:
        this->layout->addWidget( this->nfpa );
        this->nfpa->setScale( 16 );
        this->nfpa->update( propertyValues.first());
        break;

    case Tag::GHS:
        this->layout->addWidget( this->ghs );
        this->ghs->update( PropertyWidget::parseGHS( this->propertyValues.first()));
        break;

    case Tag::Formula:
    case Tag::State:
    case Tag::NoType:
        return;
    }

    this->m_position = 0;

    if ( this->displayValues.count() >= 2 ) {
        this->layout->addWidget( this->left );
        this->layout->addWidget( this->right );
        this->left->connect( this->left, &QToolButton::pressed, [ this, type ]() {
            if ( this->position() == -1 )
                return;

            if ( this->position() - 1 >= 0 )
                this->m_position--;
            else
                this->m_position = this->displayValues.count() - 1;

            switch ( type ) {
            case Tag::Text:
            case Tag::Integer:
            case Tag::Real:
            case Tag::CAS:
                this->label->setText( this->displayValues[this->position()] );
                break;

            case Tag::NFPA:
                this->nfpa->update( this->propertyValues[this->position()] );
                break;

            case Tag::GHS:
                this->ghs->update( PropertyWidget::parseGHS( this->propertyValues[this->position()] ));
                break;

            case Tag::Formula:
            case Tag::State:
            case Tag::NoType:
                return;
            }
        } );
        this->right->connect( this->right, &QToolButton::pressed, [ this, type ]() {
            if ( this->position() == -1 )
                return;

            if ( this->position() + 1 < this->displayValues.count())
                this->m_position++;
            else
                this->m_position = 0;

            switch ( type ) {
            case Tag::Text:
            case Tag::Integer:
            case Tag::Real:
            case Tag::CAS:
                this->label->setText( this->displayValues[this->position()] );
                break;

            case Tag::NFPA:
                this->nfpa->update( this->propertyValues[this->position()] );
                break;

            case Tag::GHS:
                this->ghs->update( PropertyWidget::parseGHS( this->propertyValues[this->position()] ));
                break;

            case Tag::Formula:
            case Tag::State:
            case Tag::NoType:
                return;
            }
        } );
    }

    this->setLayout( this->layout );
}

/**
 * @brief PropertyWidget::add
 * @param id
 */
void PropertyWidget::add( const Id &id ) {
    if ( id == Id::Invalid )
        return;

    switch ( Tag::instance()->type( this->tagId())) {
    case Tag::Text:
    case Tag::Integer:
    case Tag::Real:
    case Tag::CAS:
        Property::instance()->add( QString(), this->tagId(), this->propertyValues[this->position()].first(), id );
        break;

    case Tag::NFPA:
        Property::instance()->add( QString(), this->tagId(), this->propertyValues[this->position()].join( " " ), id );
        break;

    case Tag::GHS:
        Property::instance()->add( QString(), this->tagId(), PropertyWidget::parseGHS( this->propertyValues[this->position()] ).join( " " ), id );
        break;

    case Tag::Formula:
    case Tag::State:
    case Tag::NoType:
        return;
    }
}