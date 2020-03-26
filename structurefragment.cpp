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
#include "fragmentnavigation.h"
#include "reagentdock.h"

// TODO: disable property fragment on error

/**
 * @brief StructureFragment::StructureFragment
 * @param cidList
 * @param parent
 */
StructureFragment::StructureFragment( QWidget *parent ) : Fragment( parent ), ui( new Ui::StructureFragment ) {
    // setup ui
    this->ui->setupUi( this );

    // set tip icons
    const QPixmap pixmap( QIcon::fromTheme( "info" ).pixmap( 16, 16 ));
    const QList<QLabel*> tips( QList<QLabel*>() << this->ui->tipIcon );
    for ( QLabel *tip : tips )
        tip->setPixmap( pixmap );

    // done action leads to the property fragment
    QAction::connect( this->ui->actionSelect, &QAction::triggered, this, [ this ]() {
        this->host()->setCurrentFragment( this->host()->propertyFragment());
        this->host()->propertyFragment()->getDataAndFormula( this->cid());
    } );

    // add reagent lambda
    auto addDialog = [ this ]( const QString &name ) {
        // TODO: special mode for this
        const Id reagentId = ReagentDock::instance()->addReagent( Id::Invalid, name, this->cid());
        if ( reagentId != Id::Invalid ) {
            this->host()->setReagentId( reagentId );
            this->host()->setCurrentFragment( this->host()->propertyFragment());
            this->host()->propertyFragment()->getDataAndFormula( this->cid());
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
        if ( this->host()->mode() == ExtractionDialog::ExistingMode )
            this->ui->actionSelect->trigger();
        else if ( this->host()->mode() == ExtractionDialog::SearchMode )
            this->ui->actionAddReagent->trigger();
    }

    Fragment::keyReleaseEvent( event );
}

/**
 * @brief StructureFragment::sendFormulaRequest
 */
void StructureFragment::sendFormulaRequest() {
    qDebug() << "  request formula (BROWSER)" << this->queryName();
    NetworkManager::instance()->execute( QString( "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/%1/PNG" ).arg( this->cid()), NetworkManager::FormulaRequestBrowser );
}

/**
 * @brief StructureFragment::sendIUPACNameRequest
 */
void StructureFragment::sendIUPACNameRequest() {
    qDebug() << "  request name  (BROWSER)" << this->queryName();
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
    this->host()->setStatusMessage( StructureFragment::tr( "Working" ));

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
    this->validate();
}

/**
 * @brief StructureFragment::readIUPACName
 * @param name
 */
void StructureFragment::readIUPACName( const QString &name ) {
    this->ui->IUPACEdit->setText( name );

    this->setStatus( this->status() & ~FetchName );
    this->validate();
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
            this->setStatus( Error );
            return;
        }
        break;

    case NetworkManager::FormulaRequestBrowser:
        qDebug() << "network->formula (browser)" << this->queryName();
        if ( !this->parseFormulaRequest( data )) {
            qDebug() << "  parseFormulaRequest failed (browser)";
            this->setStatus( Error );
            return;
        }
        break;

    default:
        break;
    }
}

/**
 * @brief StructureFragment::validate
 */
void StructureFragment::validate() {
    this->ui->actionPrevious->setEnabled( !this->cidList.isEmpty() && this->index() > 0 );
    this->ui->actionNext->setEnabled( !this->cidList.isEmpty() && this->index() < this->cidList.count() - 1 );
    this->ui->actionSelect->setEnabled( !this->cidList.isEmpty() && this->status() == Idle );
    this->ui->actionAddReagent->setEnabled( !this->cidList.isEmpty() && this->status() == Idle );
    this->ui->actionAddIUPAC->setEnabled( !this->cidList.isEmpty() && this->status() == Idle );

    if ( this->status() == Idle )
        this->host()->clearStatusMessage();
    else
        this->host()->fragmentNavigation()->setFragmentEnabled( this->host()->propertyFragment(), false );
}

/**
 * @brief StructureFragment::setup
 * @param list
 */
void StructureFragment::setup( const QList<int> &list ) {
    // hide/show actions according to the mode
    if ( this->host()->mode() == ExtractionDialog::ExistingMode ) {
        this->ui->toolBar->insertAction( this->ui->actionPrevious, this->ui->actionSelect );
        this->ui->toolBar->removeAction( this->ui->actionAddReagent );
        this->ui->toolBar->removeAction( this->ui->actionAddIUPAC );
    } else if ( this->host()->mode() == ExtractionDialog::SearchMode ) {
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
    this->validate();

    // connect prev button
    QAction::connect( this->ui->actionPrevious, &QAction::triggered, [ this ]() {
        this->m_index--;
        this->validate();
        this->getNameAndFormula();
    } );

    // connect next button
    QAction::connect( this->ui->actionNext, &QAction::triggered, [ this ]() {
        this->m_index++;
        this->validate();
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
void StructureFragment::error( const QString &, NetworkManager::Types, const QString &errorMessage ) {
    this->setStatus( Error );
    this->host()->setErrorMessage( StructureFragment::tr( "Error: " ) + errorMessage );
    this->ui->structurePixmap->setText( StructureFragment::tr( "Could not load structure" ));
    this->host()->adjustSize();
}
