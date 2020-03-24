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
#include "extractiondialog.h"
#include "searchfragment.h"
#include "cache.h"
#include "propertyfragment.h"
#include "reagentdialog.h"

/**
 * @brief StructureFragment::StructureFragment
 * @param cidList
 * @param parent
 */
StructureFragment::StructureFragment( QWidget *parent ) : Fragment( parent ), ui( new Ui::StructureFragment ) {
    this->ui->setupUi( this );

    // done action leads to the property fragment
    QAction::connect( this->ui->actionSelect, &QAction::triggered, this, [ this ]() {
        this->host()->setCurrentFragment( this->host()->propertyFragment());
    } );

    // add reagent lambda
    auto addDialog = [ this ]( const QString &name ) {
        // TODO: special mode for this
        ReagentDialog rd( this, name, this->queryName(), ReagentDialog::EditMode );
        if ( rd.exec() == QDialog::Accepted ) {
            // TODO: add reagent
            this->host()->setCurrentFragment( this->host()->propertyFragment());
        }
    };

    // add action leads to the ReagentDialog
    QAction::connect( this->ui->actionAddReagent, &QAction::triggered, this, [ this, addDialog ]() { addDialog( this->queryName()); } );// std::bind( addDialog, this->queryName()));
    QAction::connect( this->ui->actionAddIUPAC, &QAction::triggered, this, [ this, addDialog ]() { addDialog( this->IUPACName()); } );//std::bind( addDialog, this->ui->IUPACEdit->text()));
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
    QAction::disconnect( this->ui->actionSelect, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionAddReagent, &QAction::triggered, this, nullptr );
    QAction::disconnect( this->ui->actionAddIUPAC, &QAction::triggered, this, nullptr );

    delete this->ui;
}

/**
 * @brief StructureFragment::keyPressEvent
 * @param event
 */
void StructureFragment::keyPressEvent( QKeyEvent *event ) {
    if (( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ) && this->host()->fragmentHost()->currentWidget() == this ) {
        if ( this->host()->mode() == ExtractionDialog::SearchMode )
            this->ui->actionSelect->trigger();
        else if ( this->host()->mode() == ExtractionDialog::ExistingMode )
            this->ui->actionAddReagent->trigger();
    }

    Fragment::keyReleaseEvent( event );
}

/**
 * @brief StructureFragment::sendFormulaRequest
 */
void StructureFragment::sendFormulaRequest() {
    qDebug() << "  request formula" << this->queryName();
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/%1/PNG" ).arg( this->cid()), NetworkManager::FormulaRequestBrowser );
}

/**
 * @brief StructureFragment::sendIUPACNameRequest
 */
void StructureFragment::sendIUPACNameRequest() {
    qDebug() << "  request name" << this->queryName();
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/%1/property/IUPACName/TXT" ).arg( this->cid()), NetworkManager::IUPACName );
}

/**
 * @brief StructureFragment::getNameAndFormula
 */
void StructureFragment::getNameAndFormula() {
    // abort if id list is empty
    if ( this->cidList.isEmpty())
        return;

    // update status visually
    this->ui->IUPACEdit->setText( StructureFragment::tr( "loading..." ));
    this->ui->structurePixmap->setText( StructureFragment::tr( "fetching formula..." ));
    this->ui->cidEdit->clear();

    // update status
    this->setStatus( FetchName | FetchFormula );

    // get current id (selected reagent)
    const int cid = cidList.at( this->index());
    this->ui->cidEdit->setText( QString::number( cid ));

    // get formula
    if ( Cache::instance()->contains( Cache::FormulaContext, QString( "%1.png" ).arg( this->cid()))) {
        qDebug() << "    cache->formula (BROWSER)" << this->queryName();
        this->readFormula( Cache::instance()->getData( Cache::FormulaContext, QString( "%1.png" ).arg( this->cid())));
    } else {
        // formula not in the cache, fetch it
        this->sendFormulaRequest();
    }

    // get IUPAC name
    if ( Cache::instance()->contains( Cache::IUPACContext, QString( "%1" ).arg( this->cid()))) {
        qDebug() << "    cache->iupac (BROWSER)" << this->queryName();
        this->readIUPACName( Cache::instance()->getData( Cache::IUPACContext, QString( "%1" ).arg( this->cid())));
    } else {
        // IUPAC name not in the cache, fetch it
        this->sendIUPACNameRequest();
    }
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

    // trigger size adjustment
    this->ui->structurePixmap->hide();
    this->ui->structurePixmap->show();
    this->host()->adjustSize();

    this->setStatus( this->status() & ~FetchFormula );
    this->buttonTest();
}

/**
 * @brief StructureFragment::readIUPACName
 * @param name
 */
void StructureFragment::readIUPACName( const QString &name ) {
    this->ui->IUPACEdit->setText( name );

    this->setStatus( this->status() & ~FetchName );
    this->buttonTest();
}

/**
 * @brief StructureFragment::parseFormulaRequest
 * @param data
 * @return
 */
bool StructureFragment::parseFormulaRequest( const QByteArray &data ) {
    if ( !data.isEmpty()) {
        qDebug() << "    network->formula (browser)" << this->queryName();
        Cache::instance()->insert( Cache::FormulaContext, QString( "%1.png" ).arg( this->cid()), data );
        this->readFormula( data );
        return true;
    }

    return false;
}

/**
 * @brief StructureFragment::parseIUPACNameRequest
 * @param data
 * @return
 */
bool StructureFragment::parseIUPACNameRequest( const QByteArray &data ) {
    if ( !data.isEmpty()) {
        const QString name( data );
        qDebug() << "    network->name (browser)" << this->queryName();
        Cache::instance()->insert( Cache::IUPACContext, QString( "%1" ).arg( this->cid()), data );
        this->readIUPACName( name );
        return true;
    }

    return false;
}

/**
 * @brief StructureFragment::cid
 * @return
 */
int StructureFragment::cid() const {
    return this->cidList.isEmpty() ? -1 : this->cidList.at( this->index());
}

/**
 * @brief StructureFragment::queryName
 * @return
 */
QString StructureFragment::queryName() const {
    QString name( this->ui->queryEdit->text().remove( "\n" ).simplified());

    if ( !name.isEmpty())
        name.replace( 0, 1, name.at( 0 ).toUpper());

    return name;
}

/**
 * @brief StructureFragment::IUPACName
 * @return
 */
QString StructureFragment::IUPACName() const {
    return this->ui->IUPACEdit->text().remove( "\n" ).simplified();
}

/**
 * @brief StructureFragment::replyReceived
 * @param url
 * @param type
 * @param userData
 * @param data
 */
void StructureFragment::replyReceived( const QString &, NetworkManager::Types type, const QVariant &, const QByteArray &data ) {
    switch ( type ) {
    case NetworkManager::IUPACName:
        qDebug() << "network->IUPACName" << this->queryName();
        if ( !this->parseIUPACNameRequest( data )) {
            qDebug() << "  parseIUPACNameRequest failed";

            // TODO: report error
            this->setStatus( Error );
            //emit this->status( "Error" );
            return;
        }
        break;

    case NetworkManager::FormulaRequestBrowser:
        qDebug() << "network->formula (browser)" << this->queryName();
        if ( !this->parseFormulaRequest( data )) {
            qDebug() << "  parseFormulaRequest failed (browser)";

            // TODO: report error
            this->setStatus( Error );
            //emit this->status( "Error" );
            return;
        }
        break;

    default:
        break;
    }
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
 * @brief StructureFragment::setup
 * @param list
 */
void StructureFragment::setup( const QList<int> &list ) {
    // hide/show actions according to the mode
    if ( this->host()->mode() == ExtractionDialog::SearchMode ) {
        this->ui->toolBar->insertAction( this->ui->actionPrevious, this->ui->actionSelect );
        this->ui->toolBar->removeAction( this->ui->actionAddReagent );
        this->ui->toolBar->removeAction( this->ui->actionAddIUPAC );
    } else if ( this->host()->mode() == ExtractionDialog::ExistingMode ) {
        this->ui->toolBar->removeAction( this->ui->actionSelect );
        this->ui->toolBar->insertAction( this->ui->actionPrevious, this->ui->actionAddReagent );
        this->ui->toolBar->insertAction( this->ui->actionPrevious, this->ui->actionAddIUPAC );
    }

    // reset status to idle
    this->setStatus( Idle );

    // copy identifier from search fragment
    this->ui->queryEdit->setText( this->host()->searchFragment()->identifier());

    // check list of ids provided by the search fragment
    this->cidList = list;
    if ( this->cidList.isEmpty())
        return;

    // select the first id and update previous/next button status
    this->ui->cidEdit->setText( QString::number( this->cidList.first()));
    this->buttonTest();

    // connect prev button
    QAction::connect( this->ui->actionPrevious, &QAction::triggered, [ this ]() {
        this->m_index--;
        this->buttonTest();
        this->getNameAndFormula();
    } );

    // connect next button
    QAction::connect( this->ui->actionNext, &QAction::triggered, [ this ]() {
        this->m_index++;
        this->buttonTest();
        this->getNameAndFormula();
    } );

    // setup finished connection to the NetworkManager
    StructureFragment::connect( NetworkManager::instance(),
                                SIGNAL( finished( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )),
                                this,
                                SLOT( replyReceived( const QString &, NetworkManager::Types, const QVariant &, const QByteArray & )));

    // setup error connection to the NetworkManager
    StructureFragment::connect( NetworkManager::instance(),
                                SIGNAL( error( const QString &, NetworkManager::Types, const QString & )),
                                this,
                                SLOT( error( const QString &, NetworkManager::Types, const QString & )));
}

/**
 * @brief StructureFragment::error
 */
void StructureFragment::error( const QString &, NetworkManager::Types, const QString &errorString ) {
    this->setStatus( Error );
    //this->ui->name->setText( StructureFragment::tr( "Error" ));
    this->ui->structurePixmap->setText( errorString );
}
