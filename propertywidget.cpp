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
#include "imageutils.h"

/**
 * @brief PropertyWidget::PropertyWidget
 * @param parent
 * @param values
 * @param tagId
 */
PropertyWidget::PropertyWidget( QWidget *parent, const QList<QStringList> &values, const Id &tagId ) : QWidget(
        parent ), m_tagId( tagId ) {
    if ( this->tagId() == Id::Invalid )
        return;

    const Tag::Types &type = Tag::instance()->type( tagId );
    this->layout->setContentsMargins( 0, 0, 0, 0 );
    this->layout->setSpacing( 0 );

    if ( values.isEmpty())
        return;

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
            this->label = new QLabel( this->displayValues.first());
            this->label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
            this->label->setWordWrap( true );
            this->layout->addWidget( this->label );
            break;

        case Tag::NFPA:
            this->nfpa = new NFPAWidget( nullptr, propertyValues.first());
            this->layout->addWidget( this->nfpa );
            this->nfpa->setScale( 16 );
            break;

        case Tag::GHS:
            this->ghs = new GHSWidget( nullptr, PropertyWidget::parseGHS( this->propertyValues.first()));
            this->ghs->setLinear();
            this->layout->addWidget( this->ghs );
            break;

        case Tag::Formula:
        case Tag::State:
        case Tag::NoType:
            return;
    }

    this->m_position = 0;

    if ( this->displayValues.count() >= 2 ) {
        this->left = new QToolButton();
        this->right = new QToolButton();
        this->left->setIcon( QIcon::fromTheme( "left" ));
        this->right->setIcon( QIcon::fromTheme( "right" ));
        this->left->setIconSize( QSize( 8, 16 ));
        this->right->setIconSize( QSize( 8, 16 ));
        this->layout->addWidget( this->left );
        this->layout->addWidget( this->right );
        QToolButton::connect( this->left, &QToolButton::pressed, [ this, type ]() {
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
        QToolButton::connect( this->right, &QToolButton::pressed, [ this, type ]() {
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
 * @brief PropertyWidget::PropertyWidget
 * @param parent
 * @param pixmap
 * @param tagId
 */
PropertyWidget::PropertyWidget( QWidget *parent, const QPixmap &pixmap ) : QWidget( parent ), m_pixmap( pixmap ) {
    if ( pixmap.isNull())
        return;

    const bool darkMode = Variable::isEnabled( "darkMode" );
    this->label = new QLabel();
    this->label->setPixmap( darkMode ? ImageUtils::invertPixmap( ImageUtils::autoCropPixmap( pixmap )) : pixmap );
    this->label->setFixedSize( pixmap.width(), pixmap.height());
    this->layout->addWidget( label );
    this->setLayout( this->layout );
}

/**
 * @brief PropertyWidget::~PropertyWidget
 */
PropertyWidget::~PropertyWidget() {
    if ( this->left != nullptr ) PropertyWidget::disconnect( this->left, &QToolButton::pressed, this, nullptr );
    if ( this->right != nullptr ) PropertyWidget::disconnect( this->right, &QToolButton::pressed, this, nullptr );

    delete this->ghs;
    delete this->nfpa;
    delete this->label;
    delete this->left;
    delete this->right;
    delete this->layout;
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
            Property::instance()->add( QString(), this->tagId(), this->propertyValues[this->position()].join( " " ),
                                       id );
            break;

        case Tag::GHS:
            Property::instance()->add( QString(), this->tagId(),
                                       PropertyWidget::parseGHS( this->propertyValues[this->position()] ).join( " " ),
                                       id );
            break;

        case Tag::Formula:
        case Tag::State:
        case Tag::NoType:
            return;
    }
}
