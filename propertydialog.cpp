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
#include "propertydialog.h"
#include "propertydock.h"
#include "reagent.h"
#include "tag.h"
#include "ui_propertydialog.h"
#include <QRegularExpressionValidator>

/**
 * @brief PropertyDialog::PropertyDialog
 * @param tag
 * @param parent
 */
PropertyDialog::PropertyDialog( QWidget *parent, const Id &tagId, const QString &defaultValue ) : QDialog( parent ),
                                                                                                  ui( new Ui::PropertyDialog ),
                                                                                                  tag( tagId ) {
    this->ui->setupUi( this );

    if ( tagId == Id::Invalid )
        return;

    const Tag::Types type = Tag::instance()->type( tagId );
    const bool darkMode = Variable::isEnabled( "darkMode" );

    switch ( type ) {
        case Tag::Integer:
        case Tag::Real: {

            const QRegularExpression pattern( type == Tag::Integer ? "\\s*-?\\d+" : R"(-?\d*(?:[,|\.]\d*)?\s*)" );
            auto *validator( new QRegularExpressionValidator( pattern, this ));
            this->ui->textEdit->setValidator( validator );
            this->ui->textEdit->setText(
                    defaultValue.isEmpty() ? Tag::instance()->defaultValue( tagId ).toString() : defaultValue );
            this->ui->textEdit->setAlignment( Qt::AlignRight );
            QLineEdit::connect( this->ui->textEdit, &QLineEdit::textChanged,
                                [ this, tagId, type, darkMode ]( const QString &text ) {
                                    bool ok;

                                    const QString minStr( Tag::instance()->minValue( tagId ).toString());
                                    const QString maxStr( Tag::instance()->maxValue( tagId ).toString());

                                    if ( type == Tag::Integer ) {
                                        const int value = text.toInt( &ok );
                                        const int min = minStr.isEmpty() ? std::numeric_limits<int>::min()
                                                                         : Tag::instance()->minValue( tagId ).toInt();
                                        const int max = maxStr.isEmpty() ? std::numeric_limits<int>::max()
                                                                         : Tag::instance()->maxValue( tagId ).toInt();

                                        if ( value < min )
                                            ok = false;
                                        if ( value > max )
                                            ok = false;
                                    } else {
                                        const qreal value = QString( text ).replace( ",", "." ).toDouble( &ok );
                                        const qreal min = minStr.isEmpty() ? std::numeric_limits<qreal>::min()
                                                                           : Tag::instance()->minValue(
                                                        tagId ).toDouble();
                                        const qreal max = maxStr.isEmpty() ? std::numeric_limits<qreal>::max()
                                                                           : Tag::instance()->maxValue(
                                                        tagId ).toDouble();

                                        if ( value < min )
                                            ok = false;
                                        if ( value > max )
                                            ok = false;
                                    }

                                    if ( darkMode )
                                        this->ui->textEdit->setStyleSheet( ok ? "QLineEdit {}"
                                                                              : "QLineEdit { background-color: #ff5555; color: white; }" );
                                    else
                                        this->ui->textEdit->setStyleSheet(
                                                ok ? "QLineEdit {}" : "QLineEdit { background-color: #ff9999; }" );
                                    this->ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( ok );
                                } );
        }
            break;

        case Tag::CAS: {
            this->ui->textEdit->setText(
                    defaultValue.isEmpty() ? Tag::instance()->defaultValue( tagId ).toString() : defaultValue );
            QLineEdit::connect( this->ui->textEdit, &QLineEdit::textChanged, [ this, darkMode ]( const QString &text ) {
                const QRegularExpression pattern( R"(\s*\d{2,7}-\d{2}-\d{1}\s*)" );

                QString simplified( text.simplified());
                int pos = 0;

                const bool acceptable = ( QRegularExpressionValidator( pattern ).validate( simplified, pos ) ==
                                          QValidator::Acceptable );
                if ( darkMode )
                    this->ui->textEdit->setStyleSheet(
                            acceptable ? "QLineEdit {}" : "QLineEdit { background-color: #ff5555; color: white; }" );
                else
                    this->ui->textEdit->setStyleSheet(
                            acceptable ? "QLineEdit {}" : "QLineEdit { background-color: #ff9999; }" );
            } );
        }
            break;

        case Tag::Text:
            this->ui->textEditMultiLine->setPlainText(
                    defaultValue.isEmpty() ? Tag::instance()->defaultValue( tagId ).toString() : defaultValue );
            break;

        case Tag::State: {
            bool ok;
            int index = QString( defaultValue.isEmpty() ? "0" : defaultValue ).toInt( &ok );
            if ( !ok )
                index = 0;

            this->ui->stateCombo->setCurrentIndex( qAsConst( index ));
        }
            break;

        default:
            qDebug() << "BAD TYPE";
            return;
    }

    this->setWindowTitle(( defaultValue.isEmpty() ? PropertyDialog::tr( "Add" ) : PropertyDialog::tr( "Edit" )) +
                         QString( " '%1'" ).arg( Tag::instance()->name( tagId )));
    this->ui->nameLabel->setText( Tag::instance()->name( tagId ));
    this->ui->unitsLabel->setText( Tag::instance()->units( tagId ));
    this->ui->unitsLabel->setTextFormat( Qt::RichText );

    if ( !defaultValue.isEmpty() && type != Tag::Text )
        this->ui->advancedButton->hide();

    // focus on input dialog
    if ( type == Tag::Text ) {
        this->ui->stackedWidget->setCurrentIndex( 1 );
        this->ui->textEditMultiLine->setFocus();
        this->adjustSize();
    } else if ( type == Tag::State ) {
        this->ui->stackedWidget->setCurrentIndex( 2 );
        this->ui->stateCombo->setFocus();
    } else {
        this->ui->stackedWidget->setCurrentIndex( 0 );
        this->ui->textEdit->setFocus();
    }

    this->ui->textEditMultiLine->resize( this->ui->textEdit->size());
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
    if ( this->tag == Id::Invalid )
        return QVariant();

    switch ( Tag::instance()->type( this->tag )) {
        case Tag::Text:
            return this->ui->textEditMultiLine->toPlainText();

        case Tag::CAS:
            return this->ui->textEdit->text().simplified();

        case Tag::Real:
            return this->ui->textEdit->text().replace( ",", "." ).toDouble();

        case Tag::Integer:
            return this->ui->textEdit->text().toInt();

        case Tag::State:
            return this->ui->stateCombo->currentIndex();

        default:;
    }
    return QVariant();
}

/**
 * @brief PropertyDialog::on_advancedButton_clicked
 */
void PropertyDialog::on_advancedButton_clicked() {
    this->done( PropertyDialog::Advanced );
}
