/*
 * Copyright (C) 2020 Armands Aleksejevs
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
#include "tableviewer.h"
#include "ui_tableviewer.h"
#include "tableproperty.h"
#include "reagent.h"
#include "property.h"
#include "htmlutils.h"
#include "tag.h"
#include "ghswidget.h"
#include "nfpawidget.h"
#include <QSqlQuery>
#include <QDebug>
#include <QLabel>

/**
 * @brief TableViewer::TableViewer
 * @param parent
 */
TableViewer::TableViewer( QWidget *parent, const Id &tableId ) : QDialog( parent ), ui( new Ui::TableViewer ) {
    this->ui->setupUi( this );

    if ( tableId == Id::Invalid )
        return;

    // step one: get selected tags
    QMap<Id, bool> settings;
    QStringList tags;

    QSqlQuery query;
    query.exec( QString( "select %1, %2 from %3 where %4=%5" )
                .arg( TableProperty::instance()->fieldName( TableProperty::TagId ),
                      TableProperty::instance()->fieldName( TableProperty::Tab ),
                      TableProperty::instance()->tableName(),
                      TableProperty::instance()->fieldName( TableProperty::TableId ),
                      QString::number( static_cast<int>( tableId ))));

    while ( query.next()) {
        settings[query.value( 0 ).value<Id>()] = query.value( 1 ).toBool();
        tags << query.value( 0 ).toString();
    }


    //
    // TODO: add switch to require certain columns for an item to be displayed
    //       also a flag to display:
    //           a) reagents (double click for batches?);
    //           b) reagents and batches
    //           c) batches only
    //
    // step two: get items
    this->ui->tableWidget->setColumnCount( settings.count() + 1 );

    // ONLY reagents for now
    QList<Id> reagents;
    query.exec( QString( "select %1 from %2 where %3!=-1" )
                .arg( Reagent::instance()->fieldName( Reagent::ID ),
                      Reagent::instance()->tableName(),
                      Reagent::instance()->fieldName( Reagent::ParentId )));

    while ( query.next())
        reagents << query.value( 0 ).value<Id>();

    // step three: fetch properties
    // MAP <REAGENT, TAG, VALUE>


    QMap<Id, QMap<Id, QByteArray>> propertyData;
    query.exec( QString( "select %1, %2, %3, %4 from %5 where %3 in ( %6 )" )
                .arg( Property::instance()->fieldName( Property::ID ),
                      Property::instance()->fieldName( Property::ReagentId ),
                      Property::instance()->fieldName( Property::TagId ),
                      Property::instance()->fieldName( Property::PropertyData ),
                      Property::instance()->tableName(),
                      tags.join( ", " ))
                );

    qDebug() << QString( "select %1, %2, %3, %4 from %5 where %3 in ( %6 )" )
                .arg( Property::instance()->fieldName( Property::ID ),
                      Property::instance()->fieldName( Property::ReagentId ),
                      Property::instance()->fieldName( Property::TagId ),
                      Property::instance()->fieldName( Property::PropertyData ),
                      Property::instance()->tableName(),
                      tags.join( ", " ));

    while ( query.next())
        propertyData[query.value( 1 ).value<Id>()][query.value( 2 ).value<Id>()] = query.value( 3 ).toByteArray();

    // step four: populate table
    this->ui->tableWidget->setRowCount( propertyData.keys().count());


    // terrible implementation, but it works for now
    // TODO: use delegates, dummy models and tableview instead of tablewidget
    int y = 0;
    for ( const Id &reagentId : propertyData.keys()) {
        //this->ui->tableWidget->setItem( y, 0, new QTableWidgetItem( HTMLUtils::convertToPlainText( Reagent::instance()->name( reagentId ))));
        //qDebug() << HTMLUtils::convertToPlainText( Reagent::instance()->name( reagentId )) << reagentId;
        this->ui->tableWidget->setCellWidget( y, 0, new QLabel( Reagent::instance()->name( reagentId )));

        for ( const Id &tagId : propertyData[reagentId].keys()) {
            const int column = settings.keys().indexOf( tagId );
            //this->ui->tableWidget->setItem( y, column + 1, new QTableWidgetItem( HTMLUtils::convertToPlainText( QString::fromUtf8( propertyData[reagentId][tagId].constData()))));

           /* NoType = -1,
            Text,
            Integer,
            Real,
            GHS,
            NFPA,
            CAS,
            State,
            Formula,
            PubChemId,
            Date*/

            const Tag::Types tagType = Tag::instance()->type( tagId );
            QLabel *label( new QLabel());
            if ( tagId == PixmapTag || tagType == Tag::Formula ) {
                //qDebug() << "READ FROM DATA";

                QPixmap pixmap;
                //qDebug() << QString::fromUtf8( propertyData[reagentId][tagId] ).left( 5 );
                if ( pixmap.loadFromData( propertyData[reagentId][tagId] ))
                    label->setPixmap( pixmap );

                this->ui->tableWidget->setCellWidget( y, column + 1, label );
            } else {
                switch ( tagType ) {
                case Tag::Integer:
                case Tag::Real:
                case Tag::Formula:
                case Tag::PubChemId:
                case Tag::CAS:
                    label->setText( QString::fromUtf8( propertyData[reagentId][tagId].constData()));
                    label->setAlignment( Qt::AlignCenter );
                    this->ui->tableWidget->setCellWidget( y, column + 1, label );
                    break;

                case Tag::GHS:
                {
                    GHSWidget *ghs( new GHSWidget( nullptr, QString::fromUtf8( propertyData[reagentId][tagId].constData()).split( " " )));
                    this->ui->tableWidget->setCellWidget( y, column + 1, ghs );
                }
                    break;

                case Tag::NFPA:
                {
                    NFPAWidget *nfpa( new NFPAWidget( nullptr, QString::fromUtf8( propertyData[reagentId][tagId].constData()).split( " " )));
                    this->ui->tableWidget->setCellWidget( y, column + 1, nfpa );
                }
                    break;

                default:
                    ;
                }
            }

        }

        y++;
    }

    QStringList tagNames( QStringList() << TableViewer::tr( "Reagent" ));
    for ( const Id &id : settings.keys())
        tagNames << Tag::instance()->name( id );

    this->ui->tableWidget->setHorizontalHeaderLabels( tagNames );
    this->ui->tableWidget->resizeColumnsToContents();
    this->ui->tableWidget->resizeRowsToContents();

}

/**
 * @brief TableViewer::~TableViewer
 */
TableViewer::~TableViewer() {
    delete this->ui;
}
