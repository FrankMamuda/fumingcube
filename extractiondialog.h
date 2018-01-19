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

#pragma once

//
// includes
//
#include <QDialog>

//
// classes
//
class ExtractionModel;

/**
 * @brief The Ui namespace
 */
namespace Ui {
class ExtractionDialog;
static const QString PatternWiki( "<tr>\n?<td>\n?(?:(?:<.+?(?=>)>)(?:<.+?(?=>)>)?)?(.*?(?=<\\/(?:a|td|span)))(?:(?:<\\/.+?(?=>)>)?(?:<\\/.+?(?=>)>)?)?\n?<\\/td>\n?<td>(.+)<\\/td>\n?<\\/tr>" );
}

/**
 * @brief The ExtractionDialog class
 */
class ExtractionDialog : public QDialog {
    Q_OBJECT
    Q_PROPERTY( int templateId READ templateId WRITE setTemplateId )

public:
    explicit ExtractionDialog( QWidget *parent = nullptr );
    ~ExtractionDialog();
    int templateId() const { return this->m_templateId; }

public slots:
    void setTemplateId( int id = -1 ) { this->m_templateId = id; }

private:
    Ui::ExtractionDialog *ui;
    ExtractionModel *model;
    QStringList properties;
    QStringList values;
    int m_templateId;
};
