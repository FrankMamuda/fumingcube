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
#include "networkmanager.h"
#include "ui_drawdialog.h"
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QClipboard>
#include <QMessageBox>

/**
 * @brief DrawDialog::DrawDialog
 * @param parent
 */
DrawDialog::DrawDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::DrawDialog ) {
    this->ui->setupUi( this );
    this->ui->contents->setWindowFlags( Qt::Widget );

    QDir dir( QDir::currentPath() + "/chemDoodle/" );
    if ( !dir.exists()) {
        dir.mkpath( QDir::currentPath() + "/chemDoodle/" );

        if ( !dir.exists()) {
            QMessageBox::critical( this, DrawDialog::tr( "Error" ), DrawDialog::tr( "Something went wrong.\nTry again later." ));
            QMetaObject::invokeMethod( this, "close", Qt::QueuedConnection );
        }
    }

    // setup finished connection to the NetworkManager
    NetworkManager::connect( NetworkManager::instance(), &NetworkManager::finished, this, [ this ]( const QString &, NetworkManager::Types type, const QVariant &, const QByteArray &data ) {
        switch ( type ) {
        case NetworkManager::CDDemoPage:
        {
            qDebug() << "network->CDDemoPage";
            //this->loadComponent();

            const QString page( data );

            QRegularExpression reJSQ( R"(href=\"(.+uis\/jquery.+?\.css))" );
            const QRegularExpressionMatch matchJSQ( reJSQ.match( page ));
            QRegularExpression reCDW( R"(src=\"(.+ChemDoodleWeb.js))" );
            const QRegularExpressionMatch matchCDW( reCDW.match( page ));
            QRegularExpression reUI( R"(src=\"(.+uis\/ChemDoodleWeb-uis.js))" );
            const QRegularExpressionMatch matchUI( reUI.match( page ));

            if ( matchJSQ.hasMatch() && matchCDW.hasMatch() && matchUI.hasMatch()) {
                qDebug() << "success" << matchJSQ.captured( 1 ) << matchCDW.captured( 1 ) << matchUI.captured( 1 );

                NetworkManager::instance()->execute( "https://web.chemdoodle.com/" + matchJSQ.captured( 1 ), NetworkManager::CDjQueryCSS );
                NetworkManager::instance()->execute( "https://web.chemdoodle.com/" + matchCDW.captured( 1 ), NetworkManager::CDScript );
                NetworkManager::instance()->execute( "https://web.chemdoodle.com/" + matchUI.captured( 1 ), NetworkManager::CDUIScript );

            } else {
                QMessageBox::critical( this, DrawDialog::tr( "Error" ), DrawDialog::tr( "Something went wrong.\nTry again later." ));
                QMetaObject::invokeMethod( this, "close", Qt::QueuedConnection );
                return;
            }

        }
            break;


        case NetworkManager::CDScript:
        {
            if ( !data.isEmpty()) {
                QFile file( QDir::currentPath() + "/chemDoodle/ChemDoodleWeb.js" );
                if ( file.open( QIODevice::WriteOnly )) {
                    file.write( data );
                    file.close();
                    this->script = true;
                }
            }
        }
            break;


        case NetworkManager::CDUIScript:
        {
            if ( !data.isEmpty()) {
                QFile file( QDir::currentPath() + "/chemDoodle/ChemDoodleWeb-uis.js" );
                if ( file.open( QIODevice::WriteOnly )) {
                    file.write( data );
                    file.close();
                    this->scriptUI = true;
                }
            }
        }
            break;


        case NetworkManager::CDjQueryCSS:
        {
            if ( !data.isEmpty()) {
                QFile file( QDir::currentPath() + "/chemDoodle/jquery-ui.css" );
                if ( file.open( QIODevice::WriteOnly )) {
                    file.write( data );
                    file.close();
                    this->jQuery = true;
                }
            }
        }
            break;

        default:
            ;
        }

        if ( this->script && this->scriptUI && this->jQuery )
            this->loadComponent();
    } );

    // setup error connection to the NetworkManager
    NetworkManager::connect( NetworkManager::instance(), &NetworkManager::error, this, [ this ]( const QString &, NetworkManager::Types type, const QVariant &, const QString & ) {
        switch ( type ) {
        case NetworkManager::CDDemoPage:
        case NetworkManager::CDScript:
        case NetworkManager::CDUIScript:
        case NetworkManager::CDjQueryCSS:
            QMessageBox::critical( this, DrawDialog::tr( "Error" ), DrawDialog::tr( "Something went wrong.\nTry again later." ));
            QMetaObject::invokeMethod( this, "close", Qt::QueuedConnection );
            return;

        default:
            ;
        }
    } );

    if ( !QFileInfo( QDir::currentPath() + "/chemDoodle/ChemDoodleWeb.js" ).exists() || !QFileInfo( QDir::currentPath() + "/chemDoodle/ChemDoodleWeb-uis.js" ).exists() || !QFileInfo( QDir::currentPath() + "/chemDoodle/jquery-ui.css" ).exists()) {
        if ( QMessageBox::question( this, DrawDialog::tr( "Install draw tool component" ), DrawDialog::tr( "To use the draw tool, ChemDoodle Web Components must be downloaded.\nContinue with the download?" )) == QMessageBox::No ) {
            QMetaObject::invokeMethod( this, "close", Qt::QueuedConnection );
            return;
        }

        NetworkManager::instance()->execute( QString( "https://web.chemdoodle.com/demos/2d-sketcher" ), NetworkManager::CDDemoPage );
        return;
    } else {
        this->loadComponent();
    }
}

/**
 * @brief DrawDialog::~DrawDialog
 */
DrawDialog::~DrawDialog() {
    QTimer::disconnect( &this->resizeTimer, &QTimer::timeout, this, nullptr );

    NetworkManager::disconnect( NetworkManager::instance(), &NetworkManager::finished, this, nullptr );
    NetworkManager::disconnect( NetworkManager::instance(), &NetworkManager::error, this, nullptr );

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

/**
 * @brief DrawDialog::loadComponent
 */
void DrawDialog::loadComponent() {
    const QColor baseColour( QApplication::palette().color( QPalette::Window ));
    QString colour( QString( "rgb( %1, %2, %3 )" ).arg( baseColour.red()).arg( baseColour.green()).arg( baseColour.blue()));

    QFile file( ":/chemDoodle/index.html" );
    if ( file.open( QFile::ReadOnly )) {
        QString buffer( file.readAll());
        const QString path( QDir::currentPath() + "/chemDoodle" );

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
