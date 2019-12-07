/*
 * Copyright (C) 2019 Armands Aleksejevs
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
#include <QBuffer>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>

/**
 * @brief PropertyDock::PropertyDock
 * @param parent
 */
PropertyDock::PropertyDock( QWidget *parent ) : DockWidget( parent ), ui( new Ui::PropertyDock ) {
    this->ui->setupUi( this );

    // buttonTest lambda
    auto buttonTest = [ this ]( const QModelIndex &index ) {
        if ( Property::instance()->count() < 2 ) {
            this->ui->upButton->setDisabled( true );
            this->ui->downButton->setDisabled( true );
        }

        this->ui->upButton->setEnabled( index.isValid() && index.row() != 0 );
        this->ui->downButton->setEnabled( index.isValid() && index.row() != Property::instance()->count() - 1 );
    };

    this->ui->addPropButton->setEnabled( false );
    this->ui->removePropButton->setEnabled( false );
    this->ui->propertyView->selectionModel()->connect( this->ui->propertyView->selectionModel(), &QItemSelectionModel::currentChanged, [ this, buttonTest ]( const QModelIndex &current, const QModelIndex & ) {
        this->ui->removePropButton->setEnabled( current.isValid());
        this->ui->editPropButton->setEnabled( current.isValid());

        buttonTest( current );
    } );

    ReagentDock::instance()->connect( ReagentDock::instance(), &ReagentDock::currentIndexChanged, [ this ]( const QModelIndex &current ) {
        this->reagentIndex = current;
        this->ui->addPropButton->setEnabled( this->reagentIndex.isValid());
    } );

    // move up/down lambda
    auto move = [ this, buttonTest ]( bool up ) {
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
            for ( int y = 0; y < Property::instance()->count(); y++ )
                idList << Property::instance()->id( Property::instance()->row( y ));

            // reorder tasks accordint to id list
            y = 0;
            foreach ( const Id id, idList ) {
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
        buttonTest( current );
    };
    buttonTest( this->ui->propertyView->currentIndex());

    // move
    this->ui->upButton->connect( this->ui->upButton, &QToolButton::clicked, std::bind( move, true ));
    this->ui->downButton->connect( this->ui->downButton, &QToolButton::clicked, std::bind( move, false ));
}

/**
 * @brief PropertyDock::getPropertyValue
 * @param reagentId
 * @param tagRow
 * @return
 */
QPair<QString, QVariant> PropertyDock::getPropertyValue( const Id &reagentId, const Id &tagId, const Id &propertyId ) const {
    QPair<QString, QVariant> values;
    QString name, value;
    PropertyEditor::Modes mode = PropertyEditor::Add;

    if ( reagentId == Id::Invalid )
        return values;

    if ( propertyId != Id::Invalid ) {
        name = Property::instance()->name( propertyId );
        value = Property::instance()->propertyData( propertyId ).toString();
        mode = PropertyEditor::Edit;
    }

    if ( tagId != Id::Invalid ) {
        switch ( Tag::instance()->type( Tag::instance()->row( tagId ))) {
        case Tag::CAS:
        case Tag::Text:
        case Tag::Integer:
        case Tag::Real:
        {
            PropertyDialog pd( PropertyDock::instance(), tagId, qAsConst( value ));
            int result = pd.exec();
            if ( result == QDialog::Accepted )
                return { QString(), pd.value() };
            else if ( result == PropertyDialog::Rejected )
                return values;

            break;
        }

        case Tag::NFPA:
        {
            NFPABuilder nfpa( PropertyDock::instance(), value.split( " " ));
            return ( nfpa.exec() == QDialog::Accepted ) ? QPair<QString, QVariant>( QString(), nfpa.parameters().join( " " )) : values;
        }

        case Tag::GHS:
        {
            GHSBuilder ghs( PropertyDock::instance(), value.split( " " ));
            return ( ghs.exec() == QDialog::Accepted ) ? QPair<QString, QVariant>( QString(), ghs.parameters().join( " " )) : values;
        }

        case Tag::NoType:
        case Tag::State:
            return values;
        }
    }

    PropertyEditor *pe( new PropertyEditor( PropertyDock::instance(), qAsConst( mode ), qAsConst( name ), qAsConst( value )));
    if ( pe->exec() == QDialog::Accepted ) {
        const QString name( TextEdit::stripHTML( pe->name()));

        QTextEdit ed;
        ed.setText( name );
        if ( ed.toPlainText().isEmpty()) {
            QMessageBox::warning( PropertyDock::instance(), this->tr( "Cannot add property" ), this->tr( "Property missing name" ));
            return values;
        }

        return { name, pe->value() };
    }

    return values;
}

/**
 * @brief PropertyDock::~PropertyDock
 */
PropertyDock::~PropertyDock() {
    delete this->ui;
}

/**
 * @brief PropertyDock::sectionSize
 * @param column
 * @return
 */
int PropertyDock::sectionSize( int column ) const {
    const int sectionSize = Variable::instance()->integer( "propertyNameColumnSize" );
    if ( column == 0 )
        return sectionSize;
    else if ( column == 1 )
        return this->ui->propertyView->viewport()->width() - sectionSize;

    return 0;//this->ui->propertyView->columnWidth( column );
}

/**
 * @brief PropertyDock::updateView
 */
void PropertyDock::updateView() {
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
    if ( !this->reagentIndex.isValid())
        return;

    // get reagent id
    const Id reagentId = static_cast<TreeItem*>( this->reagentIndex.internalPointer())->data( TreeItem::Id ).value<Id>();
    if ( reagentId == Id::Invalid )
        return;

    // get reagent name
    const QString reagentName( Reagent::instance()->name( Reagent::instance()->row( reagentId )));

    // add built-in properties to menu
    QMenu menu;
    QMenu *subMenu( menu.addMenu( this->tr( "Add property" )));
    for ( int y = 0; y < Tag::instance()->count(); y++ ) {
        const Row tagRow = Tag::instance()->row( y );
        const Id tagId = Tag::instance()->id( tagRow );

        subMenu->addAction( Tag::instance()->name( tagRow ), [ this, reagentId, tagId ]() {
            const QPair<QString, QVariant> values( this->getPropertyValue( reagentId, tagId ));
            this->addProperty( values.first, values.second, reagentId, values.first.isEmpty() ? tagId : Id::Invalid );
        } );
    }

    // add an option to add custom properties
    menu.addAction( this->tr( "Add custom property to '%1'" ).arg( reagentName ), [ this, reagentId ]() {
        const QPair<QString, QVariant> values( this->getPropertyValue( reagentId, Id::Invalid ));
        this->addProperty( values.first, values.second, reagentId );
    } );

    // add an option to embed images
    menu.addAction( this->tr( "Add image to '%1'" ).arg( reagentName ), [ this, reagentId ]() {
        const QString fileName( QFileDialog::getOpenFileName( this, this->tr( "Open Image" ), "", this->tr( "Images (*.png *.jpg)" )));
        if ( fileName.isEmpty())
            return;

        // load image
        const QPixmap pixmap( fileName );
        if ( !pixmap.isNull()) {
            QByteArray bytes;
            QBuffer buffer( &bytes );
            buffer.open( QIODevice::WriteOnly );
            QPixmap( pixmap ).scaledToWidth( qMin( PropertyDock::instance()->sectionSize( 1 ), pixmap.width()), Qt::SmoothTransformation ).save( &buffer, "PNG" );

            bool ok;
            const QString title( QInputDialog::getText( this, this->tr( "Set title" ), this->tr( "Title:" ), QLineEdit::Normal, "", &ok ));
            if ( ok && !title.isEmpty())
                this->addProperty( title, bytes, reagentId, PixmapTag );
        }
    } );

    // add an option to get properties from the internet
    menu.addAction( this->tr( "Get properties from the internet" ), [ this, reagentId ]() {
        ExtractionDialog ed( this, reagentId );
        ed.exec();
        this->updateView();
    } );

    // display the menu
    menu.exec( this->mapToGlobal( this->ui->addPropButton->pos()));
}


/**
 * @brief PropertyDock::on_propertyView_customContextMenuRequested
 * @param pos
 */
void PropertyDock::on_propertyView_customContextMenuRequested( const QPoint &pos ) {
    if ( !this->ui->propertyView->currentIndex().isValid())
        return;

    QMenu menu;
    menu.addAction( this->tr( "Copy" ));
    menu.addAction( this->tr( "Edit" ));

    const Row row = Property::instance()->row( this->ui->propertyView->currentIndex());
    const Id tagId = Property::instance()->tagId( row );
    if ( tagId == Id::Invalid )
        return;

    const Tag::Types type = Tag::instance()->type( tagId );
    const QString functionName( Tag::instance()->function( tagId ));

    if (( type == Tag::Integer || type == Tag::Real ) && !functionName.isEmpty()) {
        auto paste = [ this, functionName, row ]() {
            QStringList parents;

            const Id reagentId = Property::instance()->reagentId( row );
            if ( reagentId == Id::Invalid )
                return;

            const Row reagentRow = Reagent::instance()->row( reagentId );
            if ( reagentRow == Row::Invalid )
                return;

            const Id parentId = Reagent::instance()->parentId( reagentRow );
            if ( parentId != Id::Invalid )
                parents << QString( "\"%1\"" ).arg( Reagent::instance()->alias( Reagent::instance()->row( parentId )));

            parents << QString( "\"%1\"" ).arg( Reagent::instance()->alias( reagentRow ));

            QLineEdit *calc( qobject_cast<MainWindow*>( this->parentWidget())->calculatorWidget());
            calc->setText( calc->text().append( QString( " %1( %2 )" ).arg( functionName ).arg( parents.join( ", " ))));
        };

        // tag.
        QMenu *subMenu2( menu.addMenu( this->tr( "Paste to calculator" )));
        subMenu2->addAction( this->tr( "Reference" ), paste );
        subMenu2->addAction( this->tr( "Value" ));
    }

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
        foreach ( const QModelIndex &index, indexList ) {
            if ( !index.isValid())
                continue;

            const Row row = Property::instance()->row( index );
            if ( row == Row::Invalid )
                return;

            idList << Property::instance()->id( row );
        }

        foreach ( const Id &id, idList )
            Property::instance()->remove( Property::instance()->row( id ));
    };

    if ( indexes.count() == 1 ) {
        const QModelIndex index( this->ui->propertyView->currentIndex());
        if ( !index.isValid())
            return;

        if ( QMessageBox::question( this, this->tr( "Confirm removal" ), this->tr( "Remove selected property?" )) == QMessageBox::Yes )
            removeProperty( QModelIndexList() << this->ui->propertyView->currentIndex());
    } else if ( indexes.count() > 1 ) {
        if ( QMessageBox::question( this, this->tr( "Confirm removal" ), this->tr( "Remove %1 properties?" ).arg( indexes.count())) == QMessageBox::Yes )
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

        if ( tagId == Id::Invalid || tagId == PixmapTag )
            continue;

        const Tag::Types type = Tag::instance()->type( tagId );
        if ( type == Tag::NFPA || type == Tag::GHS ) {
            const QModelIndex index( Property::instance()->index( y, Property::PropertyData ));
            const QStringList parms( Property::instance()->propertyData( row ).toString().split( " " ));
            QWidget *widget( this->ui->propertyView->indexWidget( index ));
            bool hasWidget = widget != nullptr;

            auto setWidget = [ this, index, hasWidget, parms ]( PropertyWidget *widget ) {
                if ( hasWidget && widget != nullptr ) {
                    widget->update( parms );
                } else {
                    // set widget and make sure to delete it on close
                    this->ui->propertyView->setIndexWidget( index, widget );
                    widget->setAttribute( Qt::WA_DeleteOnClose, true );
                }
            };

            if ( type == Tag::NFPA ) {
                NFPAWidget *nfpa( hasWidget ? dynamic_cast<NFPAWidget*>( widget ) : new NFPAWidget( nullptr, parms ));
                setWidget( nfpa );
            } else if ( type == Tag::GHS ) {
                GHSWidget *ghs( hasWidget ? dynamic_cast<GHSWidget*>( widget ) : new GHSWidget( nullptr, parms ));
                setWidget( ghs );
            }
        }
    }
}

/**
 * @brief PropertyDock::on_editPropButton_clicked
 */
void PropertyDock::on_editPropButton_clicked() {
    if ( !this->ui->propertyView->currentIndex().isValid() || this->ui->propertyView->selectionModel()->selectedRows().count() > 1 || !this->reagentIndex.isValid())
        return;

    // get reagent id
    const Id reagentId = static_cast<TreeItem*>( this->reagentIndex.internalPointer())->data( TreeItem::Id ).value<Id>();
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

    // set new value
    this->ui->propertyView->setUpdatesEnabled( false );
    const QPair<QString, QVariant> values( this->getPropertyValue( reagentId, Property::instance()->tagId( propertyId ), propertyId ));
    if ( !values.second.isNull())
        Property::instance()->setPropertyData( propertyRow, values.second.toString());

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

    // add property
    Property::instance()->add(( tagId == Id::Invalid || tagId == PixmapTag ) ? name : "", tagId, value, reagentId );

    // clear document cache and resize view
    this->updateView();

    // enable updates
    this->ui->propertyView->setUpdatesEnabled( true );
}
