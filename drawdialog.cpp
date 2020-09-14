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
#include <QBuffer>
#include <QImageWriter>

/**
 * @brief DrawDialog::DrawDialog
 * @param parent
 */
DrawDialog::DrawDialog( QWidget *parent, const QString &json ) : QDialog( parent ), json( json ), ui( new Ui::DrawDialog ) {
    this->ui->setupUi( this );
    this->ui->contents->setWindowFlags( Qt::Widget );

    // check if draw component has been downloaded
    QDir dir( QDir::currentPath() + "/chemDoodle/" );
    if ( !dir.exists()) {
        // make a directory in app path to store scripts
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

            // parse 2d sketcher demo page (so that we don't have to download the whole archive and bother with compression)
            const QString page( data );

            // we need three files - jQuery css and two js scripts
            QRegularExpression reJSQ( R"(href=\"(.+uis\/jquery.+?\.css))" );
            const QRegularExpressionMatch matchJSQ( reJSQ.match( page ));
            QRegularExpression reCDW( R"(src=\"(.+ChemDoodleWeb.js))" );
            const QRegularExpressionMatch matchCDW( reCDW.match( page ));
            QRegularExpression reUI( R"(src=\"(.+uis\/ChemDoodleWeb-uis.js))" );
            const QRegularExpressionMatch matchUI( reUI.match( page ));

            // find these in the demo page and downloaded them
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
            // write out base script
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
            // write out ui script
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
            // write out jQuery css
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

        // load draw component when we're done
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
            // just throw a generic error for now
            QMessageBox::critical( this, DrawDialog::tr( "Error" ), DrawDialog::tr( "Something went wrong.\nTry again later." ));
            QMetaObject::invokeMethod( this, "close", Qt::QueuedConnection );
            return;

        default:
            ;
        }
    } );

    // download draw components if required
    if ( !QFileInfo( QDir::currentPath() + "/chemDoodle/ChemDoodleWeb.js" ).exists() || !QFileInfo( QDir::currentPath() + "/chemDoodle/ChemDoodleWeb-uis.js" ).exists() || !QFileInfo( QDir::currentPath() + "/chemDoodle/jquery-ui.css" ).exists()) {
        if ( QMessageBox::question( this, DrawDialog::tr( "Install draw tool component" ), DrawDialog::tr( "To use the draw tool, ChemDoodle Web Components must be downloaded.\nContinue with the download?" )) == QMessageBox::No ) {
            QMetaObject::invokeMethod( this, "close", Qt::QueuedConnection );
            return;
        }

        NetworkManager::instance()->execute( QString( "https://web.chemdoodle.com/demos/2d-sketcher" ), NetworkManager::CDDemoPage );
        return;
    } else {
        // load draw component if already available
        this->loadComponent();
    }
}

/**
 * @brief DrawDialog::~DrawDialog
 */
DrawDialog::~DrawDialog() {
    // disconnect canvas resize
    QTimer::disconnect( &this->resizeTimer, &QTimer::timeout, this, nullptr );

    // disconnect edit callback
    QWebEnginePage::disconnect( this->ui->webView->page(), &QWebEnginePage::loadFinished, this, nullptr );

    // disconnect NetworkManager
    NetworkManager::disconnect( NetworkManager::instance(), &NetworkManager::finished, this, nullptr );
    NetworkManager::disconnect( NetworkManager::instance(), &NetworkManager::error, this, nullptr );

    // get rid of ui
    delete this->ui;
}

/**
 * @brief DrawDialog::getPixmapAndAccept
 */
void DrawDialog::getPixmapAndAccept() {
    // we do a couple of things here:
    //
    // 1) get internal JSON string, which contains all shapes and molecules
    // 2) render and upscale a PNG of the canvas
    // 3) store JSON inside PNG (for editing in the future)

    // get the image export script (it is a little too big to store here)
    QFile file( ":/chemDoodle/export.js" );
    QString buffer;
    if ( file.open( QFile::ReadOnly )) {
        buffer = file.readAll();
        file.close();
    }

    // retrieve json from canvas and store it
    this->ui->webView->page()->runJavaScript( "{ const shapes = sketcher.shapes; const molecules = sketcher.molecules; ChemDoodle.writeJSON( molecules, shapes ); }", [ this ]( const QVariant &v ) {
        this->json = v.toString();
    } );

    // make PNG of the canvas
    this->ui->webView->page()->runJavaScript( buffer, [ & ]( const QVariant &v ) {
        QRegularExpression reg( "data:image\\/png;base64,(.+)" );

        // js should output PNG in base64 format, so there we capture it...
        const QRegularExpressionMatch match( reg.match( v.toString()));
        if ( match.hasMatch()) {
            QString captured( match.captured( 1 ));

            // ...and reconstruct a pixmap from base64 data
            QPixmap pixmap;
            pixmap.loadFromData( QByteArray::fromBase64( QByteArray( captured.toUtf8().constData())));

            // unfortunately, we cannot directly store textual data in QPixmap, so we have to convert it into QImage...
            QImage image( pixmap.toImage());
            image.setText( "ChemDoodleData", this->json );

            // ...and write it back to QByteArray via buffer and QImageWriter
            QByteArray byteArray;
            QBuffer buffer( &byteArray );
            buffer.open( QIODevice::WriteOnly );

            QImageWriter writer( &buffer, "png" );
            writer.write( image );
            buffer.close();

            // at last we store the data in class and return success
            this->data = byteArray;
            this->accept();
            //QMetaObject::invokeMethod( this, "accept", Qt::QueuedConnection );
        }

        // failure
        QMetaObject::invokeMethod( this, "reject", Qt::QueuedConnection );
    } );
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

    QString jsonx( "{\"m\":[{\"a\":[{\"x\":229,\"y\":137,\"i\":\"a0\"},{\"x\":246.3205080756888,\"y\":147,\"i\":\"a1\"},{\"x\":246.32050807568882,\"y\":167,\"i\":\"a2\"},{\"x\":229.00000000000006,\"y\":177.00000000000003,\"i\":\"a3\"},{\"x\":211.67949192431126,\"y\":167.00000000000009,\"i\":\"a4\"},{\"x\":211.67949192431118,\"y\":147.00000000000009,\"i\":\"a5\"},{\"x\":263.6410161513776,\"y\":136.99999999999997,\"i\":\"a6\"},{\"x\":280.9615242270664,\"y\":146.99999999999997,\"i\":\"a7\"},{\"x\":280.9615242270664,\"y\":166.99999999999997,\"i\":\"a8\"},{\"x\":263.64101615137764,\"y\":177,\"i\":\"a9\"},{\"x\":298.28203230275517,\"y\":136.99999999999997,\"i\":\"a10\"},{\"x\":315.60254037844396,\"y\":146.99999999999997,\"i\":\"a11\"},{\"x\":315.60254037844396,\"y\":166.99999999999997,\"i\":\"a12\"},{\"x\":298.2820323027552,\"y\":177,\"i\":\"a13\"},{\"x\":332.92304845413275,\"y\":136.99999999999997,\"i\":\"a14\"},{\"x\":350.24355652982155,\"y\":146.99999999999997,\"i\":\"a15\"},{\"x\":350.24355652982155,\"y\":166.99999999999997,\"i\":\"a16\"},{\"x\":332.9230484541328,\"y\":177,\"i\":\"a17\"}],\"b\":[{\"b\":0,\"e\":1,\"i\":\"b0\"},{\"b\":1,\"e\":2,\"i\":\"b1\",\"o\":2},{\"b\":2,\"e\":3,\"i\":\"b2\"},{\"b\":3,\"e\":4,\"i\":\"b3\",\"o\":2},{\"b\":4,\"e\":5,\"i\":\"b4\"},{\"b\":5,\"e\":0,\"i\":\"b5\",\"o\":2},{\"b\":1,\"e\":6,\"i\":\"b6\"},{\"b\":6,\"e\":7,\"i\":\"b7\",\"o\":2},{\"b\":7,\"e\":8,\"i\":\"b8\"},{\"b\":8,\"e\":9,\"i\":\"b9\",\"o\":2},{\"b\":9,\"e\":2,\"i\":\"b10\"},{\"b\":7,\"e\":10,\"i\":\"b11\"},{\"b\":10,\"e\":11,\"i\":\"b12\",\"o\":2},{\"b\":11,\"e\":12,\"i\":\"b13\"},{\"b\":12,\"e\":13,\"i\":\"b14\",\"o\":2},{\"b\":13,\"e\":8,\"i\":\"b15\"},{\"b\":11,\"e\":14,\"i\":\"b16\"},{\"b\":14,\"e\":15,\"i\":\"b17\",\"o\":2},{\"b\":15,\"e\":16,\"i\":\"b18\"},{\"b\":16,\"e\":17,\"i\":\"b19\",\"o\":2},{\"b\":17,\"e\":12,\"i\":\"b20\"}]}]}" );

    this->ui->webView->page()->runJavaScript( QString( "ChemDoodle.readJSON( '%1' ); sketcher.repaint();" ).arg( jsonx ));

#if 0
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
#endif
}

/**
 * @brief DrawDialog::loadComponent
 */
void DrawDialog::loadComponent() {
    // get background colours from theme (to make an uniform look)
    const QColor baseColour( QApplication::palette().color( QPalette::Window ));
    QString colour( QString( "rgb( %1, %2, %3 )" ).arg( baseColour.red()).arg( baseColour.green()).arg( baseColour.blue()));

    // create and render a webpage, using draw components
    QFile file( ":/chemDoodle/index.html" );
    if ( file.open( QFile::ReadOnly )) {
        QString buffer( file.readAll());

        // update paths to app path and update colours to match app theme
        const QString path( QDir::currentPath() + "/chemDoodle" );
        buffer.replace( "[_PATH]", path );
        buffer.replace( "[_COLOUR]", colour );

        // close file
        file.close();

        // render page
        this->ui->webView->setHtml( buffer, path );
    }

    // setup timer (so that canvas is resized, when dialog window is resized)
    this->resizeTimer.setSingleShot( true );
    QTimer::connect( &this->resizeTimer, &QTimer::timeout, this, [ this ]() {
        this->m_resizeInProgress = false;
        this->ui->webView->page()->runJavaScript( "if ( typeof( sketcher) != \"undefined\" ) { sketcher.resize( window.innerWidth, window.innerHeight - 76 ); sketcher.repaint(); }" );
    } );

    // reload content (edit mode)
    if ( !json.isEmpty()) {
        QWebEnginePage::connect( this->ui->webView->page(), &QWebEnginePage::loadFinished, [ this ]() {
            this->ui->webView->page()->runJavaScript( QString( "{ const { molecules, shapes } = ChemDoodle.readJSON( '%1' ); sketcher.loadContent( molecules, shapes ); sketcher.repaint(); }" ).arg( this->json ));
        } );
    }
}

/**
 * @brief DrawDialog::on_buttonBox_accepted
 */
void DrawDialog::on_buttonBox_accepted() {
    this->getPixmapAndAccept();
}
