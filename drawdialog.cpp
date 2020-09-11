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
#include "drawdialog.h"
#include "imageutils.h"
#include "mainwindow.h"
#include "ui_drawdialog.h"
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QClipboard>

/**
 * @brief DrawDialog::DrawDialog
 * @param parent
 */
DrawDialog::DrawDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::DrawDialog ) {
    this->ui->setupUi( this );
    this->ui->contents->setWindowFlags( Qt::Widget );

    const QColor baseColour( QApplication::palette().color( QPalette::Window ));
    QString colour( QString( "rgb( %1, %2, %3 )" ).arg( baseColour.red()).arg( baseColour.green()).arg( baseColour.blue()));

    QFile file( ":/chemDoodle/index.html" );
    if ( file.open( QFile::ReadOnly )) {
        QString buffer( file.readAll());
        const QString path( QDir::currentPath() + "/ChemDoodleWeb" );

        buffer.replace( "[_PATH]", path );
        buffer.replace( "[_COLOUR]", colour );

        file.close();
        this->ui->webView->setHtml( buffer, path );
    }

    // setup timer
    this->resizeTimer.setSingleShot( true );
    QTimer::connect( &this->resizeTimer, &QTimer::timeout, this, [ this ]() {
        this->m_resizeInProgress = false;
        this->ui->webView->page()->runJavaScript( "if ( typeof( sketcher) != \"undefined\" ) { sketcher.resize( window.innerWidth, window.innerHeight - 76 ); sketcher.repaint(); }" );
    } );
}

/**
 * @brief DrawDialog::~DrawDialog
 */
DrawDialog::~DrawDialog() {
    QTimer::disconnect( &this->resizeTimer, &QTimer::timeout, this, nullptr );

    delete this->ui;
}

/**
 * @brief DrawDialog::resizeEvent
 */
void DrawDialog::resizeEvent( QResizeEvent *event ) {
    this->resizeTimer.start( 200 );
    this->m_resizeInProgress = true;
    QDialog::resizeEvent( event );
}

/**
 * @brief DrawDialog::on_scriptButton_clicked
 */
void DrawDialog::on_scriptButton_clicked() {
    QFile file( ":/chemDoodle/export.js" );
    QString buffer;
    if ( file.open( QFile::ReadOnly )) {
        buffer = file.readAll();
        file.close();
    }

    this->ui->webView->page()->runJavaScript( buffer, [ this ]( const QVariant &v ) {
        QRegularExpression reg( "data:image\\/png;base64,(.+)" );
        const QRegularExpressionMatch match( reg.match( v.toString()));
        if ( match.hasMatch()) {
            QString captured( match.captured( 1 ));

            QByteArray array( QByteArray::fromBase64( QByteArray( captured.toUtf8().constData())));

            QPixmap pixmap;
            pixmap.loadFromData( array );

            ImageUtils iu( MainWindow::instance(), ImageUtils::EditMode );
            iu.setImage( pixmap.toImage());

            this->close();

            iu.exec();
        }
    }
    );
}

/**
 * @brief DrawDialog::on_copyButton_clicked
 */
void DrawDialog::on_copyButton_clicked() {
    /*QFile file( ":/chemDoodle/clipboard.js" );
    QString buffer;
    if ( file.open( QFile::ReadOnly )) {
        buffer = file.readAll();
        file.close();
    }

    this->ui->webView->page()->runJavaScript( buffer, []( const QVariant &v ) {
        QRegularExpression reg( "data:image\\/png;base64,(.+)" );
        const QRegularExpressionMatch match( reg.match( v.toString()));
        if ( match.hasMatch()) {
            QString captured( match.captured( 1 ));

            QByteArray array( QByteArray::fromBase64( QByteArray( captured.toUtf8().constData())));

            QPixmap pixmap;
            pixmap.loadFromData( array );

            QApplication::clipboard()->setPixmap( pixmap, QClipboard::Clipboard );
        }
    }
    );*/
}
