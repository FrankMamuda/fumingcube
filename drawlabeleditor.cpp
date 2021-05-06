/*
 * Copyright (C) 2021 Armands Aleksejevs
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
#include "charactermap.h"
#include "drawlabeleditor.h"
#include "ui_drawlabeleditor.h"
#include "htmlutils.h"
#include <QRegularExpression>
#include <QTimer>

//
// TODO:
//  alt->enter
//  add charge actions
//  rename window title... currently 'Add'
//

/**
 * @brief DrawLabelEditor::DrawLabelEditor
 * @param parent
 */
DrawLabelEditor::DrawLabelEditor( QWidget *parent, const QString &label )
    : QDialog( parent ), ui( new Ui::DrawLabelEditor ) {
    this->ui->setupUi( this );
    this->ui->mainWindow->setWindowFlags( Qt::Widget );

    // setup editor toolbar
    this->ui->editorToolBar->installFeature( EditorToolbar::Font );
    this->ui->editorToolBar->installFeature( EditorToolbar::VerticalAlignment );
    this->ui->editorToolBar->installFeature( EditorToolbar::CharacterMap );
    this->ui->editorToolBar->installFeature( EditorToolbar::HorizonatalAlignment );
    this->ui->editorToolBar->setEditor( this->ui->labelEdit );


    // focus on the label editor to begin with
    this->ui->labelEdit->setFocus();

    if ( !label.isEmpty())
        this->ui->labelEdit->insertHtml( QString( "<span style=\"margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; text-indent:0px; font-family:'Times New Roman'; font-size:12pt;\">%1</span>" ).arg( label ));
}

/**
 * @brief DrawLabelEditor::~DrawLabelEditor
 */
DrawLabelEditor::~DrawLabelEditor() {
    delete this->ui;
}

/**
 * @brief DrawLabelEditor::label
 * @return
 */
QString DrawLabelEditor::label() const {
    // special kind of html stripping magic
    QString html( this->ui->labelEdit->toHtml().remove( "\n" ));

    // capture body
    const QRegularExpression re( R"(<body.+?>(.+)(?:<\/body>))" );
    const QRegularExpressionMatch match = re.match( html );
    html = match.hasMatch() ? match.captured( 1 ) : html;

    // strip font family and sizes, qt related stuff
    const QStringList expressions( QStringList() << R"(font-family:.+?;)" << R"(font-size:.+?;)" << R"(-qt-paragraph-type:.+?;)" << R"(-qt-block-indent:.+?;)" << R"(text-indent.+?;)" );
    for ( const QString &expression : expressions )
        html.remove( QRegularExpression( expression ));

    // might be better just to replace with sup and sub
    html.replace( QRegularExpression( R"(vertical-align:\s*sub)" ), "vertical-align:sub;font-size:smaller" );
    html.replace( QRegularExpression( R"(vertical-align:\s*super)" ), "vertical-align:super;font-size:smaller" );

    return HTMLUtils::toPlainText( html ).simplified().isEmpty() ? "" : qAsConst( html );
}

/**
 * @brief DrawLabelEditor::showEvent
 * @param event
 */
void DrawLabelEditor::showEvent( QShowEvent *event ) {
    QDialog::showEvent( event );

    this->ui->labelEdit->document()->setDocumentMargin( 2 );
    this->ui->labelEdit->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->labelEdit->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->ui->labelEdit->setCleanHTML( true );

    // force widget to resize to minimum
    QApplication::processEvents();
    QTimer::singleShot( 0, this, [ this ]() {
        this->resize( this->width(), this->minimumSizeHint().height());
    } );
}
