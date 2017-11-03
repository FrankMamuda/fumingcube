/*
 * Copyright (C) 2017 Factory #12
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

//
// includes
//
#include <QWidget>
#include "ui_templatewidget.h"
#include "template.h"

/**
 * @brief The Ui namespace
 */
namespace Ui {
class TemplateWidget;
}

/**
 * @brief The TemplateWidget class
 */
class TemplateWidget : public QWidget {
    Q_OBJECT

public:
    explicit TemplateWidget( QWidget *parent = nullptr );
    ~TemplateWidget();
    QString name() const { return this->ui->nameEdit->text(); }
    Template::State state() const { return static_cast<Template::State>( this->ui->stateCombo->currentIndex()); }
    double amount() const { return this->ui->amountSpin->value(); }
    double density() const { return this->ui->densitySpin->value(); }
    double molarMass() const { return this->ui->molarMassSpin->value(); }
    double assay() const { return this->ui->assaySpin->value(); }

public slots:
    void setDefault() { this->ui->nameEdit->setDisabled( true ); this->ui->nameEdit->setText( "<default>" ); }

signals:
    void nameChanged( const QString &name );

private slots:
    void changeState( int state );

private:
    Ui::TemplateWidget *ui;
};
