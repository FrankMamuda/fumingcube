/*
 * Copyright (C) 2017-2018 Factory #12
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
#include "networkmanager.h"

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
    Q_PROPERTY( QString name READ name NOTIFY nameChanged )
    Q_PROPERTY( qreal assay READ assay )
    Q_PROPERTY( qreal amount READ amount )
    Q_PROPERTY( qreal density READ density )
    Q_PROPERTY( qreal molarMass READ molarMass )
    Q_PROPERTY( Template::State state READ state WRITE setState )
    Q_ENUMS( Properties )

public:
    explicit TemplateWidget( QWidget *parent = nullptr, Template *t = nullptr );
    ~TemplateWidget();
    QString name() const { return this->ui->nameEdit->text(); }
    Template::State state() const { return static_cast<Template::State>( this->ui->stateCombo->currentIndex()); }
    qreal amount() const { return this->ui->amountEdit->scaledValue(); }
    qreal density() const { return this->ui->densityEdit->scaledValue(); }
    qreal molarMass() const { return this->ui->molarMassEdit->scaledValue(); }
    qreal assay() const { return this->ui->assayEdit->scaledValue(); }

    enum Properties {
        NoProperty = -1,
        Density,
        MolarMass
    };

public slots:
    void setDefault() { this->ui->nameEdit->setDisabled( true ); this->ui->nameEdit->setText( "<default>" ); }
    int save( int id );
    void requestFinished( const QString &url, NetworkManager::Type type, const QVariant &userData, const QByteArray &data );

signals:
    void nameChanged( const QString &name );

private slots:
    void setState( Template::State state );

private:
    Ui::TemplateWidget *ui;
    Template *templ;
};
