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
#include "pixmaputils.h"
#include "structurefragment.h"
#include "ui_structurefragment.h"
#include "variable.h"
#include <QDir>
#include <utility>

/**
 * @brief StructureFragment::StructureFragment
 * @param cidList
 * @param parent
 */
StructureFragment::StructureFragment( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::StructureFragment ) {
    this->ui->setupUi( this );  
}

/**
 * @brief StructureFragment::~StructureFragment
 */
StructureFragment::~StructureFragment() {
    StructureFragment::disconnect( NetworkManager::instance(),
            SIGNAL( finished( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )),
            this, SLOT( replyReceived( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )));
    StructureFragment::disconnect( NetworkManager::instance(), SIGNAL( error( const QString &, NetworkManager::Types, const QString & )),
            this, SLOT( error( const QString &, NetworkManager::Types, const QString & )));

    delete this->ui;
}

/**
 * @brief StructureFragment::replyReceived
 * @param url
 * @param type
 * @param userData
 * @param data
 */
void StructureFragment::replyReceived( const QString &, NetworkManager::Types type, const QVariant &,
                                      const QByteArray &data ) {
    const QString cache( this->path() + "/" + QString::number( this->cidList.at( this->index())));

    switch ( type ) {
        case NetworkManager::NoType:
        case NetworkManager::CIDRequestInitial:
        case NetworkManager::DataRequest:
        case NetworkManager::FormulaRequest:
            break;

        case NetworkManager::IUPACName: {
            QFile file( cache );
            if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate )) {
                file.write( data.constData(), data.length());
                file.close();
            }

            this->ui->IUPACEdit->setText( QString( data ));
            this->setStatus( this->status() & ~FetchName );
            this->buttonTest();
            qDebug() << "status rmn" << this->status();
        }
            break;

        case NetworkManager::FormulaRequestBrowser: {
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
        case NetworkManager::FavIcon:
            break;
    }
}

/**
 * @brief StructureFragment::getFormula
 * @param cid
 */
void StructureFragment::getFormula( const int cid ) {
    const QString cache( this->path() + "/" + QString::number( cid ) + ".png" );
    if ( QFileInfo::exists( cache )) {
        QFile file( cache );
        if ( file.open( QIODevice::ReadOnly )) {
            this->readFormula( file.readAll());
            file.close();
            this->setStatus( this->status() & ~FetchFormula );
            this->buttonTest();
            return;
        }
    }

    NetworkManager::instance()->execute(
            QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/%1/PNG" ).arg( cid ),
            NetworkManager::FormulaRequestBrowser );
}

/**
 * @brief StructureFragment::getName
 * @param cid
 */
void StructureFragment::getName( const int cid ) {
    const QString cache( this->path() + "/" + QString::number( cid ));
    if ( QFileInfo::exists( cache )) {
        QFile file( cache );
        if ( file.open( QIODevice::ReadOnly )) {
            this->ui->queryEdit->setText( QString( file.readAll()));
            file.close();
            this->setStatus( this->status() & ~FetchName );
            this->buttonTest();
            return;
        }
    }

    NetworkManager::instance()->execute(
            QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/%1/property/IUPACName/TXT" ).arg( cid ),
            NetworkManager::IUPACName );
}

/**
 * @brief StructureFragment::readFormula
 * @param data
 */
void StructureFragment::readFormula( const QByteArray &data ) {
    QPixmap pixmap;
    if ( !pixmap.loadFromData( data ))
        return;

    if ( pixmap.isNull())
        return;

    const QPixmap cropped( PixmapUtils::autoCrop( qAsConst( pixmap ), QColor::fromRgb( 245, 245, 245, 255 )));
    const bool darkMode = Variable::isEnabled( "darkMode" );
    this->ui->structurePixmap->setPixmap( darkMode ? PixmapUtils::invert( cropped ) : cropped );
}

/**
 * @brief StructureFragment::setSearchMode
 */
void StructureFragment::setSearchMode() {
    //this->ui->buttonBox->setStandardButtons( QDialogButtonBox::Save | QDialogButtonBox::Close );
}

/**
 * @brief StructureFragment::setup
 * @param list
 */
void StructureFragment::setup( QList<int> list ) {
    this->cidList = std::move( list );

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

    this->ui->cidEdit->setText( QString::number( this->cidList.first()));
    this->buttonTest();

    QAction::connect( this->ui->actionPrevious, &QAction::triggered, [ this ]() {
        this->m_index--;
        this->buttonTest();
        this->getInfo();
    } );

    QAction::connect( this->ui->actionNext, &QAction::triggered, [ this ]() {
        this->m_index++;
        this->buttonTest();
        this->getInfo();
    } );

    StructureFragment::connect( NetworkManager::instance(),
                               SIGNAL( finished( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )),
                               this,
                               SLOT( replyReceived( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )));

    StructureFragment::connect( NetworkManager::instance(),
                               SIGNAL( error( const QString &, NetworkManager::Types, const QString & )),
                               this,
                               SLOT( error( const QString &, NetworkManager::Types, const QString & )));

    this->getInfo();
}

/**
 * @brief StructureFragment::buttonTest
 */
void StructureFragment::buttonTest() {
    if ( this->cidList.count() <= 1 )
        return;

    this->ui->actionPrevious->setDisabled( true );
    this->ui->actionNext->setDisabled( true );

    if ( this->status() == Idle ) {
        this->ui->actionPrevious->setEnabled( this->index() > 0 );
        this->ui->actionNext->setEnabled( this->index() < this->cidList.count() - 1 );
        return;
    }
}

/**
 * @brief StructureFragment::error
 */
void StructureFragment::error( const QString &, NetworkManager::Types, const QString &errorString ) {
    this->setStatus( Error );
    //this->ui->name->setText( StructureFragment::tr( "Error" ));
    this->ui->structurePixmap->setText( errorString );
}

/**
 * @brief StructureFragment::cid
 * @return
 */
int StructureFragment::cid() const {
    return this->cidList.isEmpty() ? -1 : this->cidList.at( this->index());
}

/**
 * @brief StructureFragment::name
 * @return
 */
QString StructureFragment::name() const {
    QString name( this->ui->queryEdit->text().remove( "\n" ).simplified());

    if ( !name.isEmpty())
        name.replace( 0, 1, name.at( 0 ).toUpper());

    return name;
}

/**
 * @brief StructureFragment::getInfo
 */
void StructureFragment::getInfo() {
    this->ui->IUPACEdit->setText( StructureFragment::tr( "loading..." ));
    this->ui->structurePixmap->setText( StructureFragment::tr( "fetching formula..." ));

    this->setStatus( FetchName | FetchFormula );

    const int cid = cidList.at( this->index());
    this->getName( cid );
    this->getFormula( cid );
}
