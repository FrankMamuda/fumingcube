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
    er zijn enkele caching problemen, maar het ExtractionDialog
    werkt
    moeten een uniforme caching oplossing maken
*/

/*
 * includes
 */
#include "imageutils.h"
#include "structurebrowser.h"
#include "ui_structurebrowser.h"
#include "variable.h"
#include <QDir>

/**
 * @brief StructureBrowser::StructureBrowser
 * @param cidList
 * @param parent
 */
StructureBrowser::StructureBrowser( const QList<int> &list, QWidget *parent ) : QDialog( parent ), ui( new Ui::StructureBrowser ), cidList( list ) {
    this->ui->setupUi( this );

    // make cache dir
    this->m_path = QDir( QDir::homePath() + "/" + Main::Path + "/cache/browser/" ).absolutePath();
    const QDir dir( this->path());
    if ( !dir.exists()) {
        dir.mkpath( dir.absolutePath());
        if ( !dir.exists())
            return;
    }

    if ( this->cidList.isEmpty())
        return;

    this->ui->name->setText( QString::number( this->cidList.first()));
    this->buttonTest();

    this->ui->prevButton->connect( this->ui->prevButton, &QPushButton::clicked, [ this ]() {
        this->m_index--;
        this->buttonTest();
        this->getInfo();
    } );

    this->ui->nextButton->connect( this->ui->nextButton, &QPushButton::clicked, [ this ]() {
        this->m_index++;
        this->buttonTest();
        this->getInfo();
    } );

    this->connect( NetworkManager::instance(), SIGNAL( finished( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )), this, SLOT( replyReceived( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )));
    this->connect( NetworkManager::instance(), SIGNAL( error( const QString &, NetworkManager::Types, const QString & )), this, SLOT( error( const QString &, NetworkManager::Types, const QString & )));

    this->getInfo();
}

/**
 * @brief StructureBrowser::~StructureBrowser
 */
StructureBrowser::~StructureBrowser() {
    this->disconnect( NetworkManager::instance(), SIGNAL( finished( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )), this, SLOT( replyReceived( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )));
    this->disconnect( NetworkManager::instance(), SIGNAL( error( const QString &, NetworkManager::Types, const QString & )), this, SLOT( error( const QString &, NetworkManager::Types, const QString & )));
    delete this->ui;
}

/**
 * @brief StructureBrowser::replyReceived
 * @param url
 * @param type
 * @param userData
 * @param data
 */
void StructureBrowser::replyReceived( const QString &, NetworkManager::Types type, const QVariant &, const QByteArray &data ) {
    const QString cache( this->path() + "/" + QString::number( this->cidList.at( this->index())));

    switch ( type ) {
    case NetworkManager::NoType:
    case NetworkManager::CIDRequestInitial:
    case NetworkManager::DataRequest:
    case NetworkManager::FormulaRequest:
        break;

    case NetworkManager::IUPACName:
    {
        QFile file( cache );
        if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate )) {
            file.write( data.constData(), data.length());
            file.close();
        }

        this->ui->name->setText( QString( data ));
        this->setStatus( this->status() & ~FetchName );
        this->buttonTest();
        qDebug() << "status rmn" << this->status();
    }
        break;

    case NetworkManager::FormulaRequestBrowser:
    {
        QFile file( cache + ".png" );
        if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate )) {
            file.write( data.constData(), data.length());
            file.close();
        }

        this->readFormula( data );
        this->setStatus( this->status() & ~FetchFormula );
        this->buttonTest();
        qDebug() << "status rmf" << this->status();
    }
        break;

    case NetworkManager::CIDRequestSimilar:
        break;
    }
}

/**
 * @brief StructureBrowser::getFormula
 * @param cid
 */
void StructureBrowser::getFormula( const int cid ) {
    const QString cache( this->path() + "/" + QString::number( cid ) + ".png" );
    if ( QFileInfo( cache ).exists()) {
        QFile file( cache );
        if ( file.open( QIODevice::ReadOnly )) {
            this->readFormula( file.readAll());
            file.close();
            this->setStatus( this->status() & ~FetchFormula );
            this->buttonTest();

            qDebug() << "F FROM CACHE";
            return;
        }
    }

    qDebug() << "F FROM THE INTERNET";
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/%1/PNG" ).arg( cid ), NetworkManager::FormulaRequestBrowser );
}

/**
 * @brief StructureBrowser::getName
 * @param cid
 */
void StructureBrowser::getName( const int cid ) {
    const QString cache( this->path() + "/" + QString::number( cid ));
    if ( QFileInfo( cache ).exists()) {
        QFile file( cache );
        if ( file.open( QIODevice::ReadOnly )) {
            this->ui->name->setText( QString( file.readAll()));
            file.close();
            this->setStatus( this->status() & ~FetchName );
            this->buttonTest();

            qDebug() << "N FROM CACHE";
            return;
        }
    }

    qDebug() << "N FROM THE INTERNET";
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/%1/property/IUPACName/TXT" ).arg( cid ), NetworkManager::IUPACName );
}

/**
 * @brief StructureBrowser::readFormula
 * @param data
 */
void StructureBrowser::readFormula( const QByteArray &data ) {
    QPixmap pixmap;
    if ( !pixmap.loadFromData( data ))
        return;

    if ( pixmap.isNull())
        return;

    const QPixmap cropped( ImageUtils::autoCropPixmap( qAsConst( pixmap ), QColor::fromRgb( 245, 245, 245, 255 )));
    const bool darkMode = Variable::instance()->isEnabled( "darkMode" );
    this->ui->formula->setPixmap( darkMode ? ImageUtils::invertPixmap( cropped ) : cropped );
}

/**
 * @brief StructureBrowser::buttonTest
 */
void StructureBrowser::buttonTest() {
    if ( this->cidList.count() <= 1 )
        return;

    this->ui->prevButton->setDisabled( true );
    this->ui->nextButton->setDisabled( true );

    if ( this->status() == Idle ) {
        this->ui->prevButton->setEnabled( this->index() > 0 );
        this->ui->nextButton->setEnabled( this->index() < this->cidList.count() - 1 );
        return;
    }
}

/**
 * @brief StructureBrowser::error
 */
void StructureBrowser::error( const QString &, NetworkManager::Types, const QString &errorString ) {
    this->setStatus( Error );
    this->ui->name->setText( this->tr( "Error" ));
    this->ui->formula->setText( errorString );
}

/**
 * @brief StructureBrowser::cid
 * @return
 */
int StructureBrowser::cid() const {
    return this->cidList.isEmpty() ? -1 : this->cidList.at( this->index());
}

/**
 * @brief StructureBrowser::getInfo
 */
void StructureBrowser::getInfo() {
    this->ui->name->setText( this->tr( "loading..." ));
    this->ui->formula->setText( this->tr( "fetching formula..." ));

    this->setStatus( FetchName | FetchFormula );

    const int cid = cidList.at( this->index());
    this->getName( cid );
    this->getFormula( cid );
}
