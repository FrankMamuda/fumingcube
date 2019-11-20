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
PropertyDock::PropertyDock( QWidget *parent ) : DockWidget( "propertyDock", parent ), ui( new Ui::PropertyDock ) {
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
            const int order = Property::instance()->order( Property::instance()->row( y ));
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
                Property::instance()->setOrder( Property::instance()->row( id ), y );
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

        const int order0 = Property::instance()->order( row0 );
        const int order1 = Property::instance()->order( row1 );

        // swap order
        Property::instance()->setOrder( Property::instance()->row( id0 ), order1 );
        Property::instance()->setOrder( Property::instance()->row( id1 ), order0 );

        Property::instance()->sort( Property::Index, Qt::AscendingOrder );
        Property::instance()->select();
        this->resizeViewContents();

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
 * @brief PropertyDock::resizeViewContents
 */
void PropertyDock::resizeViewContents() {
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
    if ( !this->reagentIndex.isValid())
        return;

    QMenu menu;
    const TreeItem *item( static_cast<TreeItem*>( this->reagentIndex.internalPointer()));

    const Id id = static_cast<Id>( item->data( TreeItem::Id ).toInt());
    if ( id == Id::Invalid )
        return;
    const Row row = Reagent::instance()->row( id );
    if ( row == Row::Invalid )
        return;

    /*
     * addProperty
     */
    auto addProperty = [ this, item ]( const QVariant &value, const Row &row ) {
        // less flickering with updates disabled
        this->ui->propertyView->setUpdatesEnabled( false );

        Property::instance()->add( Tag::instance()->name( row ),
                                   Tag::instance()->id( row ),
                                   value.toString().toUtf8().constData(),
                                   static_cast<Id>( item->data( TreeItem::Id ).toInt()));

        this->resizeViewContents();
        this->ui->propertyView->setUpdatesEnabled( true );
    };

    /*
     * addPropertyDialog
     * TODO: replace with custom/universal dialog
     */
    auto addPropertyDialog = [ this, addProperty, item ]( const Row &tagRow ) {
        QVariant value;

        switch ( Tag::instance()->type( tagRow )) {
        case Tag::CAS:
        case Tag::Text:
        case Tag::Integer:
        case Tag::Real:
        {
            PropertyDialog pd( Tag::instance()->id( tagRow ), static_cast<Id>( item->data( TreeItem::Id ).toInt()), this );
            if ( pd.exec() == QDialog::Accepted )
                value = pd.value();
        }
            break;

        case Tag::NFPA:
        {
            NFPABuilder nfpa( this );
            if ( nfpa.exec() == QDialog::Accepted )
                value = nfpa.parameters().join( " " );
            break;
        }

        case Tag::GHS:
        {
            GHSBuilder ghs( this );
            if ( ghs.exec() == QDialog::Accepted )
                value = ghs.parameters().join( " " );
            break;
        }

        case Tag::NoType:
        case Tag::State:
            ;
        }

        if ( !value.isNull())
            addProperty( qAsConst( value ), tagRow );
    };

    QMenu *subMenu( menu.addMenu( this->tr( "Add property" )));
    //foreach ( const Property::TagData &tag, Property::tags ) {
    //    subMenu->addAction( tag.name, std::bind( addPropertyDialog, tag ));
    //}

    for ( int y = 0; y < Tag::instance()->count(); y++ ) {
        const Row row = Tag::instance()->row( y );
        subMenu->addAction( Tag::instance()->name( row ), std::bind( addPropertyDialog, row ));
    }

    menu.addAction( this->tr( "Add custom property to '%1'" ).arg( Reagent::instance()->name( row )), [ this, item ]() {
        const Id id = static_cast<Id>( item->data( TreeItem::Id ).toInt());
        if ( id == Id::Invalid )
            return;

        this->addCustomProperty( id );
    } );

    menu.addAction( this->tr( "Add image to '%1'" ).arg( Reagent::instance()->name( row )), [ this, item ]() {
        const Id id = static_cast<Id>( item->data( TreeItem::Id ).toInt());
        if ( id == Id::Invalid )
            return;

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
            if ( ok && !title.isEmpty()) {
                this->ui->propertyView->setUpdatesEnabled( false );
                Property::instance()->add( title, PixmapTag, bytes, id );
                this->resizeViewContents();
                this->ui->propertyView->setUpdatesEnabled( true );
            }
        }
    } );

    menu.addAction( this->tr( "Extract properties from Wikipedia" ), [ this ]() {
        if ( !this->reagentIndex.isValid())
            return;

        const TreeItem *item( static_cast<TreeItem*>( this->reagentIndex.internalPointer()));
        if ( item == nullptr )
            return;

        const Id id = static_cast<Id>( item->data( TreeItem::Id ).toInt());
        if ( id == Id::Invalid )
            return;

        ExtractionDialog ed( this );
        ed.setReagentId( id );
        ed.exec();
    } );

    menu.exec( this->mapToGlobal( this->ui->addPropButton->pos()));

    //pe->show();
}


/**
 * @brief PropertyDock::on_propertyView_customContextMenuRequested
 * @param pos
 */
void PropertyDock::on_propertyView_customContextMenuRequested(const QPoint &pos) {
    if ( !this->ui->propertyView->currentIndex().isValid())
        return;

    QMenu menu;
    menu.addAction( this->tr( "Copy" ));
    menu.addAction( this->tr( "Edit" ));

    const Row row = Property::instance()->row( this->ui->propertyView->currentIndex());
    const Id tagId = Property::instance()->tagId( row );
    if ( tagId == Id::Invalid )
        return;

    const Row tagRow = Tag::instance()->row( tagId );
    const Tag::Types type = Tag::instance()->type( tagRow );
    const QString functionName( Tag::instance()->function( tagRow ));

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
        if ( QMessageBox::question( this, this->tr( "Confirm removal" ), this->tr( "Remove selected property" )) == QMessageBox::Yes )
            removeProperty( QModelIndexList() << this->ui->propertyView->currentIndex());
    } else if ( indexes.count() > 1 ) {
        if ( QMessageBox::question( this, this->tr( "Confirm removal" ), this->tr( "Remove %1 properties" ).arg( indexes.count())) == QMessageBox::Yes )
            removeProperty( indexes );
    }

    this->ui->removePropButton->setEnabled( false );
    this->resizeViewContents();
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

        const Row tagRow = Tag::instance()->row( tagId );
        const Tag::Types type = Tag::instance()->type( tagRow );

        if ( type == Tag::NFPA || type == Tag::GHS ) {
            const QModelIndex index( Property::instance()->index( y, Property::Value ));
            const QStringList parms( QString( Property::instance()->valueData( row ).constData()).split( " " ));
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
 * @brief PropertyDock::addCustomProperty
 * @param reagent
 */
void PropertyDock::addCustomProperty( const Id &reagent ) {
    PropertyEditor *pe( new PropertyEditor());
    pe->setAttribute( Qt::WA_DeleteOnClose, true );
    pe->setWindowModality( Qt::ApplicationModal );
    pe->open( PropertyEditor::Add, this->tr( "Add custom property" ));
    pe->connect( pe, &PropertyEditor::accepted, [ this, reagent ]( PropertyEditor::Modes, const QString &name, const QString &value ) {
        // less flickering with updates disabled
        this->ui->propertyView->setUpdatesEnabled( false );


        const QString strippedName( TextEdit::stripHTML( name ));

        // HACK: checking if name is empty
        QTextEdit ed;
        ed.setText( strippedName );
        if ( ed.toPlainText().isEmpty()) {
            QMessageBox::warning( this,  this->tr( "Cannot add property" ), this->tr( "Property missing name" ));
            this->ui->propertyView->setUpdatesEnabled( true );
            return;
        }

        Property::instance()->add( strippedName, Id::Invalid, value.toUtf8().constData(), reagent );
        this->resizeViewContents();
        this->ui->propertyView->setUpdatesEnabled( true );
    } );
}

/**
 * @brief PropertyDock::on_editPropButton_clicked
 */
void PropertyDock::on_editPropButton_clicked() {
    if ( !this->ui->propertyView->currentIndex().isValid())
        return;

    qDebug() << "STUB";
}
