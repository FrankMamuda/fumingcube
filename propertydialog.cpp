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
#include "propertydialog.h"
#include "propertydock.h"
#include "propertyeditor.h"
#include "reagent.h"
#include "tag.h"
#include "ui_propertydialog.h"
#include <QDebug>
#include <QRegularExpressionValidator>

/**
 * @brief PropertyDialog::PropertyDialog
 * @param tag
 * @param parent
 */
PropertyDialog::PropertyDialog( QWidget *parent, const Id &tagId, const QString &defaultValue ) : QDialog( parent ), ui( new Ui::PropertyDialog ), tag( tagId ) {
    this->ui->setupUi( this );

    const Row row = Tag::instance()->row( tagId );
    if ( row == Row::Invalid )
        return;

    const Tag::Types type = Tag::instance()->type( row );
    switch ( type ) {
    case Tag::Integer:
    case Tag::Real:
    {

        const QRegularExpression pattern( type == Tag::Integer ? "-?\\d+" : "-?\\d*(?:[,|\\.]\\d*)?" );
        QRegularExpressionValidator *validator( new QRegularExpressionValidator( pattern, this ));
        this->ui->textEdit->setValidator( validator );
        this->ui->textEdit->setText( defaultValue.isEmpty() ? Tag::instance()->defaultValue( row ).toString() : defaultValue );
        this->ui->textEdit->setAlignment( Qt::AlignRight );
        this->ui->textEdit->connect( this->ui->textEdit, &QLineEdit::textChanged, [ this, row, type ]( const QString &text ) {
            bool ok;

            if ( type == Tag::Integer ) {
                const int value = text.toInt( &ok );
                const int min = Tag::instance()->min( row ).toInt();
                const int max = Tag::instance()->max( row ).toInt();

                if ( value < min )
                    ok = false;
                if ( value > max )
                    ok = false;
            } else {
                const qreal value = QString( text ).replace( ",", "." ).toDouble( &ok );
                const qreal min = Tag::instance()->min( row ).toReal();
                const qreal max = Tag::instance()->max( row ).toReal();

                if ( value < min )
                    ok = false;
                if ( value > max )
                    ok = false;
            }

            this->ui->textEdit->setStyleSheet( ok ? "QLineEdit {}" : "QLineEdit { background-color: #ff9999; }" );
            this->ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( ok );
        } );
    }
        break;

    case Tag::CAS:
    {
        this->ui->textEdit->connect( this->ui->textEdit, &QLineEdit::textChanged, [ this ]( const QString &text ) {
            const QRegularExpression pattern( "\\d{2,7}-\\d{2}-\\d{1}" );

            QString simplified( text.simplified());
            int pos = 0;
            this->ui->textEdit->setStyleSheet(( QRegularExpressionValidator( pattern ).validate( simplified, pos ) == QValidator::Acceptable ) ? "QLineEdit {}" : "QLineEdit { background-color: #ff9999; }" );
        } );
    }
        break;

    case Tag::Text:
        break;

    default:
        qDebug() << "BAD TYPE";
        return;
    }

    this->setWindowTitle( this->tr( "Add '%1'" ).arg( Tag::instance()->name( row )));
    this->ui->nameLabel->setText( Tag::instance()->name( row ));
    this->ui->unitsLabel->setText( Tag::instance()->units( row ));
    this->ui->unitsLabel->setTextFormat( Qt::RichText );

    // focus on input dialog
    this->ui->textEdit->setFocus();
}

/**
 * @brief PropertyDialog::~PropertyDialog
 */
PropertyDialog::~PropertyDialog() {
    delete this->ui;
}

/**
 * @brief PropertyDialog::value
 * @return
 */
QVariant PropertyDialog::value() const {
    const Row row = Tag::instance()->row( this->tag );
    if ( row == Row::Invalid )
        return QVariant();

    switch ( Tag::instance()->type( row )) {
    case Tag::Text:
        return this->ui->textEdit->text();

    case Tag::CAS:
        return this->ui->textEdit->text().simplified();

    case Tag::Real:
        return this->ui->textEdit->text().replace( ",", "." ).toDouble();

    case Tag::Integer:
        return this->ui->textEdit->text().toInt();

    default:
        ;
    }
    return QVariant();
}

/**
 * @brief PropertyDialog::on_advancedButton_clicked
 */
void PropertyDialog::on_advancedButton_clicked() {
    this->done( PropertyDialog::Advanced );
}
