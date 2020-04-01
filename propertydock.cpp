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
#include "propertydock.h"
#include "ui_propertydock.h"
#include "property.h"
#include "tag.h"
#include "propertydelegate.h"
#include "reagentdock.h"
#include "propertydialog.h"
#include "propertyeditor.h"
#include "textedit.h"
#include "reagent.h"
#include "mainwindow.h"
#include "nfpabuilder.h"
#include "nfpawidget.h"
#include "ghswidget.h"
#include "ghsbuilder.h"
#include "extractiondialog.h"
#include "imageutils.h"
#include "pixmaputils.h"
#include "htmlutils.h"
#include "textutils.h"
#include <QBuffer>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QClipboard>
#include <QTimer>
#include <QSqlQuery>
#include <QMimeData>
#include <QSqlError>
#include <QDesktopServices>

/**
 * @brief PropertyDock::PropertyDock
 * @param parent
 */
PropertyDock::PropertyDock( QWidget *parent ) : DockWidget( parent ), ui( new Ui::PropertyDock ) {
    this->ui->setupUi( this );

    // load hidden tags
    this->loadHiddenTags();

    this->ui->addPropButton->setEnabled( false );
    this->ui->removePropButton->setEnabled( false );
    this->ui->extractButton->setEnabled( false );

    // check buttons on selection change
    QItemSelectionModel::connect( this->ui->propertyView->selectionModel(), &QItemSelectionModel::currentChanged, this, &PropertyDock::buttonTest );
    this->buttonTest();

    // move up/down lambda
    auto move = [ this ]( bool up ) {
        // test integrity
        QSet<int> orderSet;
        bool reindex = false;
        int y;
        for ( y = 0; y < Property::instance()->count(); y++ ) {
            const int order = Property::instance()->tableOrder( Property::instance()->row( y ));
            if ( orderSet.contains( order )) {
                reindex = true;
                break;
            }

            orderSet << order;
        }

        // reindex tasks if requested
        if ( reindex ) {
            QList<Id> idList;

            // get id list
            for ( y = 0; y < Property::instance()->count(); y++ )
                idList << Property::instance()->id( Property::instance()->row( y ));

            // reorder tasks according to id list
            y = 0;
            for ( const Id id : qAsConst( idList )) {
                Property::instance()->setTableOrder( Property::instance()->row( id ), y );
                y++;
            }
        }

        // get container pointer and order indexes
        PropertyView *container( this->ui->propertyView );
        const QModelIndex index( container->currentIndex());
        const QModelIndex other( container->model()->index( container->currentIndex().row() + ( up ? -1 : 1 ), 0 ));

        if ( !index.isValid() || !other.isValid())
            return;

        const Row row0 = Property::instance()->row( index );
        const Row row1 = Property::instance()->row( other );

        // use ids in lookup (QPersistentModel index should work too?)
        const Id id0 = Property::instance()->id( row0 );
        const Id id1 = Property::instance()->id( row1 );

        if ( id0 == Id::Invalid || id1 == Id::Invalid )
            return;

        const int order0 = Property::instance()->tableOrder( row0 );
        const int order1 = Property::instance()->tableOrder( row1 );

        // swap order
        Property::instance()->setTableOrder( Property::instance()->row( id0 ), order1 );
        Property::instance()->setTableOrder( Property::instance()->row( id1 ), order0 );

        Property::instance()->sort( Property::TableOrder, Qt::AscendingOrder );
        Property::instance()->select();
        this->updateView();

        const QModelIndex current( container->model()->index( static_cast<int>( Property::instance()->row( id0 )), 0 ));
        container->setCurrentIndex( current );

        container->setFocus();
        this->buttonTest();
    };

    // move
    QToolButton::connect( this->ui->upButton, &QToolButton::clicked, std::bind( move, true ));
    QToolButton::connect( this->ui->downButton, &QToolButton::clicked, std::bind( move, false ));
}

/**
 * @brief PropertyDock::getPropertyValue
 * @param reagentId
 * @param tagRow
 * @return
 */
QPair<QString, QVariant>
PropertyDock::getPropertyValue( const Id &reagentId, const Id &tagId, const Id &propertyId ) const {
    QPair<QString, QVariant> values;
    QString name;
    QString value;
    PropertyEditor::Modes mode = PropertyEditor::Add;

    if ( reagentId == Id::Invalid )
        return values;

    if ( propertyId != Id::Invalid ) {
        name = Property::instance()->name( propertyId );
        value = Property::instance()->propertyData( propertyId ).toString();
        mode = PropertyEditor::Edit;
    }

    if ( tagId != Id::Invalid ) {
        const Tag::Types type = Tag::instance()->type( Tag::instance()->row( tagId ));
        switch ( type ) {
            case Tag::CAS:
            case Tag::Text:
            case Tag::PubChemId:
            case Tag::Integer:
            case Tag::Real:
            case Tag::State: {
                // goto rich text editor
                if ( type == Tag::Text && Qt::mightBeRichText( qAsConst( value )))
                    break;

                PropertyDialog pd( PropertyDock::instance(), tagId, qAsConst( value ));
                int result = pd.exec();
                if ( result == QDialog::Accepted )
                    return { QString(), pd.value() };

                if ( result == PropertyDialog::Rejected )
                    return values;

                if ( result == PropertyDialog::Advanced )
                    value = pd.value().toString();

                break;
            }

            case Tag::NFPA: {
                NFPABuilder nfpa( PropertyDock::instance(), value.split( " " ));
                return ( nfpa.exec() == QDialog::Accepted ) ? QPair<QString, QVariant>( QString(),
                                                                                        nfpa.parameters().join( " " ))
                                                            : values;
            }

            case Tag::GHS: {
                GHSBuilder ghs( PropertyDock::instance(), value.split( " " ));
                return ( ghs.exec() == QDialog::Accepted ) ?
                         QPair<QString, QVariant>( QString(), ghs.parameters().join( " " )) :
                         values;
            }

            case Tag::Formula: {
                // load image
                const QPixmap pixmap( PixmapUtils::getOpenPixmap( PropertyDock::instance()));
                if ( !pixmap.isNull())
                    return QPair<QString, QVariant>( QString(), PixmapUtils::convertToData( pixmap ));

                break;
            }

            case Tag::NoType:
                return values;
        }
    }

    auto *pe( new PropertyEditor( PropertyDock::instance(), qAsConst( mode ), qAsConst( name ), qAsConst( value )));
    if ( pe->exec() == QDialog::Accepted ) {
        const QString strippedName( HTMLUtils::convertToPlainText( pe->name()));

        QTextEdit ed;
        ed.setText( strippedName );
        if ( ed.toPlainText().isEmpty() && tagId == Id::Invalid ) {
            QMessageBox::warning( PropertyDock::instance(), PropertyDock::tr( "Cannot add property" ),
                                  PropertyDock::tr( "Property missing name" ));
            return values;
        }

        return { strippedName, pe->value() };
    }

    return values;
}

/**
 * @brief PropertyDock::~PropertyDock
 */
PropertyDock::~PropertyDock() {
    QItemSelectionModel::disconnect( this->ui->propertyView->selectionModel(), &QItemSelectionModel::currentChanged, this, &PropertyDock::buttonTest );

    delete this->ui;
}

/**
 * @brief PropertyDock::sectionSize
 * @param column
 * @return
 */
int PropertyDock::sectionSize( int column ) const {
    return this->ui->propertyView->columnWidth( column );
}

/**
 * @brief PropertyDock::updateView
 */
void PropertyDock::updateView() {
    this->buttonTest();
    this->setSpecialWidgets();
    this->ui->propertyView->resizeToContents();
}

/**
 * @brief PropertyDock::clearDocumentCache
 */
void PropertyDock::clearDocumentCache() {
    this->ui->propertyView->clearDocumentCache();
}

/**
 * @brief PropertyDock::on_addPropButton_clicked
 */
void PropertyDock::on_addPropButton_clicked() {
    // check if reagent is valid
    if ( !ReagentDock::instance()->view()->currentIndex().isValid())
        return;

    // get reagent id
    const Id reagentId = ReagentDock::instance()->view()->idFromIndex(
            ReagentDock::instance()->view()->filterModel()->mapToSource(
                    ReagentDock::instance()->view()->currentIndex()));
    if ( reagentId == Id::Invalid )
        return;

    // get reagent name
    const QString reagentName(
            HTMLUtils::convertToPlainText( Reagent::instance()->name( Reagent::instance()->row( reagentId ))));

    // get UNFILTERED tags that have been set
    QSqlQuery query;
    query.exec( QString( "select %1 from %2 where %3=%4" )
                        .arg( Property::instance()->fieldName( Property::TagId ),
                              Property::instance()->tableName(),
                              Property::instance()->fieldName( Property::ReagentId ),
                              QString::number( static_cast<int>( reagentId ))));
    QList<Id> allSetTags;
    while ( query.next())
        allSetTags << query.value( 0 ).value<Id>();

    // add built-in properties to menu
    QMenu menu;
    QMenu *subMenu( menu.addMenu( PropertyDock::tr( "Add property" )));

    // get UNFILTERED tag list
    query.exec( QString( "select %1 from %2" )
                        .arg( Tag::instance()->fieldName( Tag::ID ),
                              Tag::instance()->tableName()));
    QList<Id> allTags;
    while ( query.next())
        allTags << query.value( 0 ).value<Id>();

    for ( const Id tagId : qAsConst( allTags )) {
        if ( qAsConst( allSetTags ).contains( tagId ))
            continue;

        subMenu->addAction(  QApplication::translate( "Tag", Tag::instance()->name( tagId ).toUtf8().constData()) /*Tag::instance()->name( tagId )*/, this, [ this, reagentId, tagId ]() {
            const QPair<QString, QVariant> values( this->getPropertyValue( reagentId, tagId ));
            this->addProperty( values.first, values.second, reagentId, values.first.isEmpty() ? tagId : Id::Invalid );
        } );
        //action->setDisabled( tags.contains( tagId ));
    }
    subMenu->setIcon( QIcon::fromTheme( "add" ));

    // add an option to add custom properties
    menu.addAction( PropertyDock::tr( "Add custom property to '%1'" ).arg( TextUtils::elidedString( reagentName )), this, [ this, reagentId ]() {
        const QPair<QString, QVariant> values( this->getPropertyValue( reagentId, Id::Invalid ));
        this->addProperty( values.first, values.second, reagentId );
    } )->setIcon( QIcon::fromTheme( "star" ));

    // add an option to embed images
    menu.addAction( PropertyDock::tr( "Add image to '%1'" ).arg( TextUtils::elidedString( reagentName )), this, [ this, reagentId ]() {
        // load image
        const QPixmap pixmap( PixmapUtils::getOpenPixmap( this ));
        if ( !pixmap.isNull()) {
            bool ok;
            const QString title(
                    QInputDialog::getText( this, PropertyDock::tr( "Set title" ), PropertyDock::tr( "Title:" ), QLineEdit::Normal, "",
                                           &ok ));
            if ( ok && !title.isEmpty())
                this->addProperty( title, PixmapUtils::convertToData( pixmap ), reagentId, PixmapTag );
        }
    } )->setIcon( QIcon::fromTheme( "image" ));

    // add an option to get properties from the internet
    menu.addAction( PropertyDock::tr( "Get properties from the internet" ), this, [ this, reagentId ]() {
        const Id parentId = Reagent::instance()->parentId( reagentId );

        ExtractionDialog ed( this, parentId != Id::Invalid ? parentId : reagentId );
        ed.exec();
        this->updateView();
    } )->setIcon( QIcon::fromTheme( "extract" ));

    // display the menu
    menu.exec( this->mapToGlobal( this->ui->addPropButton->pos()));
}


/**
 * @brief PropertyDock::on_propertyView_customContextMenuRequested
 * @param pos
 */
void PropertyDock::on_propertyView_customContextMenuRequested( const QPoint &pos ) {
    QMenu menu;
    const QModelIndex index( this->ui->propertyView->indexAt( pos ));
    constexpr const char *mimeTag = "application/x-fumingcube-tag";
    constexpr const char *mimeData = "application/x-fumingcube-data";
    constexpr const char *mimeName = "application/x-fumingcube-name";

    // check reagent
    const QModelIndex reagentIndex( ReagentDock::instance()->view()->filterModel()->mapToSource(
            ReagentDock::instance()->view()->currentIndex()));
    if ( !reagentIndex.isValid())
        return;

    // get reagent id
    const Id reagentId = ReagentDock::instance()->view()->idFromIndex( reagentIndex );
    if ( reagentId == Id::Invalid )
        return;

    if ( index.isValid()) {

        // get property and tag
        const Row row = Property::instance()->row( index );
        const Id tagId = Property::instance()->tagId( row );
        const Tag::Types type = ( tagId != Id::Invalid ) ? Tag::instance()->type( tagId ) : Tag::NoType;

        if ( type == Tag::Text || type == Tag::Integer || type == Tag::PubChemId || type == Tag::Real || type == Tag::CAS ||
             type == Tag::Formula || tagId == Id::Invalid ) {
            menu.addAction( PropertyDock::tr( "Copy" ), this, [ row, type, tagId ]() {
                const QVariant data( Property::instance()->propertyData( row ));

                if ( type == Tag::Formula )
                    QGuiApplication::clipboard()->setImage( QImage::fromData( data.toByteArray()));
                else if ( type == Tag::Real )
                    QGuiApplication::clipboard()->setText(
                            data.toString().replace( QRegularExpression( "(\\d+)[,.](\\d+)" ),
                                                     QString( "\\1%1\\2" ).arg(
                                                             Variable::string( "decimalSeparator" ))));
                else
                    QGuiApplication::clipboard()->setText( HTMLUtils::convertToPlainText( data.toString()));


                QMimeData *propertyData( new QMimeData());
                propertyData->setData( mimeTag, QString::number( static_cast<int>( tagId )).toLatin1().constData());
                propertyData->setData( mimeName, Property::instance()->name( row ).toLatin1().constData());
                propertyData->setData( mimeData, data.toByteArray());
                propertyData->setText( QGuiApplication::clipboard()->text());

                QGuiApplication::clipboard()->setMimeData( propertyData );
            } )->setIcon( QIcon::fromTheme( "copy" ));
        }

        if ( type == Tag::Formula || tagId == PixmapTag ) {
            menu.addAction( PropertyDock::tr( "View" ), this, [ this, type, row ]() {
                QPixmap pixmap;

                if ( pixmap.loadFromData( Property::instance()->propertyData( row ).toByteArray())) {
                    if ( Variable::isEnabled( "darkMode" ) && type == Tag::Formula )
                        pixmap = PixmapUtils::invert( PixmapUtils::autoCrop( pixmap ));

                    ImageUtils iu( this, qAsConst( pixmap ), 0, true );
                    iu.setWindowFlags( Qt::Dialog );
                    iu.exec();
                }
            } )->setIcon( QIcon::fromTheme( "image" ));

            menu.addAction( PropertyDock::tr( "Replace" ), this, [ this, row ]() {
                this->replacePixmap( row );
            } )->setIcon( QIcon::fromTheme( "replace" ));
        }

        if ( type != Tag::Formula && type != Tag::NoType && tagId != PixmapTag )
            menu.addAction( PropertyDock::tr( "Edit" ), this, SLOT( on_editPropButton_clicked()))->setIcon(
                    QIcon::fromTheme( "edit" ));

        if ( tagId != Id::Invalid ) {
            const QString functionName( Tag::instance()->function( tagId ));
            if (( type == Tag::Integer || type == Tag::Real ) && !functionName.isEmpty()) {
                auto paste = [ row, tagId ]() {
                    // paste
                    const QString value( QString::number(
                            Property::instance()->propertyData( row ).toReal() * Tag::instance()->scale( tagId )));
                    MainWindow::instance()->insertCommand( value );
                };

                // paste to calculator
                QMenu *subMenu2( menu.addMenu( PropertyDock::tr( "Paste to calculator" )));
                subMenu2->setIcon( QIcon::fromTheme( "paste" ));
                subMenu2->addAction( PropertyDock::tr( "Reference" ), this, [ this ]() {
                    this->on_propertyView_doubleClicked( this->ui->propertyView->currentIndex());
                } )->setIcon( QIcon::fromTheme( "clean" ));
                subMenu2->addAction( PropertyDock::tr( "Value" ), this, paste )->setIcon( QIcon::fromTheme( "paste" ));
            }

            menu.addAction( PropertyDock::tr( "Hide property \"%1\"" ).arg( Tag::instance()->name( tagId )), this, [ this, tagId ]() {
                this->hiddenTags << QString::number( static_cast<int>( tagId ));
                this->hiddenTags.removeAll( "" );
                ReagentDock::instance()->view()->updateView();
            } )->setIcon( QIcon::fromTheme( "hide" ));
        }
    }

    if ( !this->hiddenTags.isEmpty()) {
        menu.addAction( PropertyDock::tr( "Show all properties" ), this, [ this ]() {
            this->hiddenTags.clear();
            ReagentDock::instance()->view()->updateView();
        } )->setIcon( QIcon::fromTheme( "show" ));
    }

    // paste property from clipboard (paste between reagents)
    if ( QGuiApplication::clipboard()->mimeData() != nullptr ) {
        QByteArray tag( QGuiApplication::clipboard()->mimeData()->data( mimeTag ));
        QByteArray data( QGuiApplication::clipboard()->mimeData()->data( mimeData ));
        QByteArray name( QGuiApplication::clipboard()->mimeData()->data( mimeName ));

        if ( !tag.isEmpty() && !data.isEmpty()) {
            const Id tagId = static_cast<Id>( QString( tag.constData()).toInt());
            bool ok = true;

            if ( tagId != Id::Invalid ) {
                QSqlQuery query;
                query.exec( QString( "select %1 from %2 where %1=%3" ).arg(
                                Tag::instance()->fieldName( Tag::ID ),
                                Tag::instance()->tableName(),
                                QString::number( static_cast<int>( tagId ))));

                ok = query.next();
            }

            if ( ok ) {
                const QString prettyName( PropertyDock::tr( R"(Paste property "%1")" ).arg( tagId != Id::Invalid ? Tag::instance()->name( tagId ) : name ));
                menu.addAction( prettyName, this, [ this, name, tagId, reagentId, data ]() {
                    this->addProperty( name.constData(), data, reagentId, tagId );
                } )->setIcon( QIcon::fromTheme( "paste" ));
            }
        }
    }

    if ( !menu.actions().isEmpty())
        menu.exec( this->ui->propertyView->mapToGlobal( pos ));
}

/**
 * @brief PropertyDock::on_removePropButton_clicked
 */
void PropertyDock::on_removePropButton_clicked() {
    if ( !this->ui->propertyView->currentIndex().isValid())
        return;

    // NOTE: could this be fixed with persistent indexes?
    const QModelIndexList indexes( this->ui->propertyView->selectionModel()->selectedRows());
    auto removeProperty = []( const QModelIndexList &indexList ) {
        QList<Id> idList;

        // must build an id list, because indexes/rows change on removal
        for ( const QModelIndex &index : indexList ) {
            if ( !index.isValid())
                continue;

            const Row row = Property::instance()->row( index );
            if ( row == Row::Invalid )
                return;

            idList << Property::instance()->id( row );
        }

        for ( const Id &id : qAsConst( idList ))
            Property::instance()->remove( Property::instance()->row( id ));
    };

    if ( indexes.count() == 1 ) {
        const QModelIndex index( this->ui->propertyView->currentIndex());
        if ( !index.isValid())
            return;

        if ( QMessageBox::question( this, PropertyDock::tr( "Confirm removal" ), PropertyDock::tr( "Remove selected property?" )) ==
             QMessageBox::Yes )
            removeProperty( QModelIndexList() << this->ui->propertyView->currentIndex());
    } else if ( indexes.count() > 1 ) {
        if ( QMessageBox::question( this, PropertyDock::tr( "Confirm removal" ),
                                    PropertyDock::tr( "Remove %1 properties?" ).arg( indexes.count())) == QMessageBox::Yes )
            removeProperty( indexes );
    }

    this->ui->removePropButton->setEnabled( false );
    this->updateView();
}

/**
 * @brief PropertyDock::setSpecialWidgets
 */
void PropertyDock::setSpecialWidgets() {
    for ( int y = 0; y < Property::instance()->count(); y++ ) {
        const Row row = Property::instance()->row( y );
        const Id tagId = Property::instance()->tagId( row );

        bool pixmap = false;
        if ( tagId != Id::Invalid )
            pixmap = Tag::instance()->type( tagId ) == Tag::Formula || tagId == PixmapTag;

        if ( tagId == Id::Invalid || pixmap )
            continue;

        const Tag::Types type = Tag::instance()->type( tagId );
        if ( type == Tag::NFPA || type == Tag::GHS ) {
            const QModelIndex index( Property::instance()->index( y, Property::PropertyData ));
            const QStringList parms( Property::instance()->propertyData( row ).toString().split( " " ));
            QWidget *widget( this->ui->propertyView->indexWidget( index ));
            bool hasWidget = widget != nullptr;

            auto setWidget = [ this, index, hasWidget, parms ]( PropertyViewWidget *widget ) {
                if ( widget == nullptr )
                    return;

                if ( hasWidget ) {
                    widget->update( parms );
                } else {
                    // set widget and make sure to delete it on close
                    this->ui->propertyView->setIndexWidget( index, widget );
                    widget->setAttribute( Qt::WA_DeleteOnClose, true );
                }
            };

            if ( type == Tag::NFPA ) {
                NFPAWidget *nfpa( hasWidget ? dynamic_cast<NFPAWidget *>( widget ) : new NFPAWidget( nullptr, parms ));
                setWidget( nfpa );
            } else {
                GHSWidget *ghs( hasWidget ? dynamic_cast<GHSWidget *>( widget ) : new GHSWidget( nullptr, parms ));
                setWidget( ghs );
            }
        }
    }
}

/**
 * @brief PropertyDock::setCurrentIndex
 * @param index
 */
void PropertyDock::setCurrentIndex( const QModelIndex &index ) {
    this->ui->propertyView->setCurrentIndex( index );
}

/**
 * @brief PropertyDock::replacePixmap
 * @param row
 */
void PropertyDock::replacePixmap( const Row &row ) {
    // load image
    const QPixmap pixmap( PixmapUtils::getOpenPixmap( this ));
    if ( !pixmap.isNull()) {
        Property::instance()->setPropertyData( row, PixmapUtils::convertToData( pixmap ));
        this->updateView();
    }
}

/**
 * @brief PropertyDock::saveHiddenTags
 */
void PropertyDock::saveHiddenTags() {
    this->hiddenTags.removeAll( "" );
    Variable::setValue( "propertyDock/hiddenTags", this->hiddenTags );
}

/**
 * @brief PropertyDock::loadHiddenTags
 */
void PropertyDock::loadHiddenTags() {
    this->hiddenTags = Variable::value<QStringList>( "propertyDock/hiddenTags" );
    this->hiddenTags.removeAll( "" );
}

/**
 * @brief PropertyDock::on_editPropButton_clicked
 */
void PropertyDock::on_editPropButton_clicked() {
    const QModelIndex reagentIndex( ReagentDock::instance()->view()->filterModel()->mapToSource(
            ReagentDock::instance()->view()->currentIndex()));
    if ( !this->ui->propertyView->currentIndex().isValid() ||
         this->ui->propertyView->selectionModel()->selectedRows().count() > 1 || !reagentIndex.isValid())
        return;

    // get reagent id
    const Id reagentId = ReagentDock::instance()->view()->idFromIndex( reagentIndex );
    if ( reagentId == Id::Invalid )
        return;

    // get property row
    const Row propertyRow = Property::instance()->row( this->ui->propertyView->currentIndex());
    if ( propertyRow == Row::Invalid )
        return;

    // get property id
    const Id propertyId = Property::instance()->id( propertyRow );
    if ( propertyId == Id::Invalid )
        return;

    // handle pixmaps
    const Id tagId = Property::instance()->tagId( propertyId );
    if (( tagId == PixmapTag ) ||
        ( tagId != Id::Invalid  && Tag::instance()->type( tagId ) == Tag::Formula )) {
        this->replacePixmap( propertyRow );
    } else {
        // set new value
        this->ui->propertyView->setUpdatesEnabled( false );
        const QPair<QString, QVariant> values( this->getPropertyValue( reagentId, tagId, propertyId ));
        if ( !values.second.isNull())
            Property::instance()->setPropertyData( propertyRow, values.second.toString());
    }

    // update view
    this->updateView();
    this->ui->propertyView->setUpdatesEnabled( true );
}

/**
 * @brief PropertyDock::addProperty
 * @param name
 * @param value
 * @param reagentId
 * @param tagId
 */
void PropertyDock::addProperty( const QString &name, const QVariant &value, const Id &reagentId, const Id &tagId ) {
    if ( reagentId == Id::Invalid || value.isNull())
        return;

    // less flickering with updates disabled
    this->ui->propertyView->setUpdatesEnabled( false );

    bool pixmap = false;
    if ( tagId != Id::Invalid ) {
        pixmap = Tag::instance()->type( tagId ) == Tag::Formula || tagId == PixmapTag;

        // warn if already exists
        QSqlQuery query;
        query.exec( QString( "select * from %1 where %2=%3 and %4=%5" )
                    .arg( Property::instance()->tableName(),
                          Property::instance()->fieldName( Property::ReagentId ),
                          QString::number( static_cast<int>( reagentId )),
                          Property::instance()->fieldName( Property::TagId ),
                          QString::number( static_cast<int>( tagId ))));
        if ( query.next()) {
            if ( QMessageBox::question( this, PropertyDock::tr( "Duplicate property" ),
                                        PropertyDock::tr( "Reagent already has this property, add regardless?" )) == QMessageBox::No ) {
                this->ui->propertyView->setUpdatesEnabled( true );
                return;
            }
        }
    }


    // add property
    Property::instance()->add(( tagId == Id::Invalid || pixmap ) ? name : QString(), tagId, value, reagentId );

    // clear document cache and resize view
    this->updateView();

    // enable updates
    this->ui->propertyView->setUpdatesEnabled( true );
}

/**
 * @brief PropertyDock::on_propertyView_doubleClicked
 * @param index
 */
void PropertyDock::on_propertyView_doubleClicked( const QModelIndex &index ) {
    if ( !index.isValid())
        return;

    const Row row = Property::instance()->row( index );
    const Id tagId = Property::instance()->tagId( row );
    if ( tagId == Id::Invalid )
        return;

    const Tag::Types type = Tag::instance()->type( tagId );
    const QString functionName( Tag::instance()->function( tagId ));

    if (( type == Tag::Integer || type == Tag::Real ) && !functionName.isEmpty()) {
        QString parents;

        const Id reagentId = Property::instance()->reagentId( row );
        if ( reagentId == Id::Invalid )
            return;

        const Id parentId = Reagent::instance()->parentId( reagentId );
        if ( parentId != Id::Invalid ) {
            parents = QString( R"("%1", "%2")" ).arg(
                    HTMLUtils::convertToPlainText( Reagent::instance()->reference( parentId )),
                    HTMLUtils::convertToPlainText( Reagent::instance()->name( reagentId )));
            //qDebug() << "has parent";
        } else {
            parents = QString( "\"%1\"" ).arg( HTMLUtils::convertToPlainText( Reagent::instance()->reference( reagentId )));
        }

        // paste
        const QString completed( QString( "%1( %2 ) " ).arg( functionName, qAsConst( parents )));
        MainWindow::instance()->insertCommand( completed );
    }

    if ( type == Tag::PubChemId ) {
        const int cid = Property::instance()->propertyData( row ).toInt();
        if ( cid <= 0 )
            return;

        // TODO add link icon in property (via delegate)
        QDesktopServices::openUrl( QString( "https://pubchem.ncbi.nlm.nih.gov/compound/%1" ).arg( cid ));
    }
}

/**
 * @brief PropertyDock::on_extractButton_clicked
 */
void PropertyDock::on_extractButton_clicked() {
    // get reagent id
    const Id reagentId = ReagentDock::instance()->view()->idFromIndex(
            ReagentDock::instance()->view()->filterModel()->mapToSource(
            ReagentDock::instance()->view()->currentIndex()));

    if ( reagentId == Id::Invalid )
        return;

    const Id parentId = Reagent::instance()->parentId( reagentId );

    // FIXME DUP CODE
    ExtractionDialog ed( this, parentId != Id::Invalid ? parentId : reagentId );
    ed.exec();
    this->updateView();
}

/**
 * @brief PropertyDock::buttonTest
 */
void PropertyDock::buttonTest() {
    if ( Property::instance()->count() < 2 ) {
        this->ui->upButton->setDisabled( true );
        this->ui->downButton->setDisabled( true );
    }

    const QModelIndex propertyIndex( this->ui->propertyView->currentIndex());
    const bool validProperty = propertyIndex.isValid();
    const bool validReagent = ReagentDock::instance()->view()->currentIndex().isValid();
    this->ui->upButton->setEnabled( validProperty && propertyIndex.row() != 0 && validReagent );
    this->ui->downButton->setEnabled( validProperty && propertyIndex.row() != Property::instance()->count() - 1 && validReagent );
    this->ui->editPropButton->setEnabled( validProperty && validReagent );
    this->ui->removePropButton->setEnabled( validProperty && validReagent );
    this->ui->extractButton->setEnabled( validReagent );
    this->ui->addPropButton->setEnabled( validReagent );
}
