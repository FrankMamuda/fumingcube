/*
 * Copyright (C) 2013-2018 Factory #12
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
#include "calcedit.h"
#include "variable.h"
#include "script.h"
#include "mainwindow.h"
#include "tag.h"
#include "reagent.h"
#include <QComboBox>
#include <QSqlQuery>
#include <QStringListModel>

/**
 * @brief CalcEdit::completeCommand
 * @return
 */
bool CalcEdit::completeCommand() {
    // NOTE: lots of duplicate code, but it works for now

    QStringList functions;
    QSqlQuery query;
    query.exec( QString( "select %1, %2 from %3 where %2 not null" )
                .arg( Tag::instance()->fieldName( Tag::ID ))
                .arg( Tag::instance()->fieldName( Tag::Function ))
                .arg( Tag::instance()->tableName()));
    while ( query.next())
        functions << query.value( 1 ).toString();

    const QString functionExpression( functions.join( "|" ));
    QString left( this->text().left( this->cursorPosition()));
    const QString mid( this->text().mid( this->cursorPosition(), this->text().length() - left.length()));
    const int initialPosition = this->cursorPosition();

    {
        const QRegularExpression keywords( QString( "\\b((?:%1)\\s*\\(\\s*\\\".+?(?=\\\")\\\"\\s*,\\s*(?!\\s*\\\"|\\s*\\)))$" ).arg( functionExpression ));
        const QRegularExpressionMatch match( keywords.match( left ));
        if ( match.hasMatch() && !mid.contains( QRegularExpression( "^\\s*," ))) {
            QString captured( match.captured( 0 ));
            if ( captured.endsWith( " " ))
                captured = captured.remove( captured.length() - 1, 1 );

            const int index = left.indexOf( captured, initialPosition - captured.length());
            if ( index == -1 )
                return false;

            this->setText( left.insert( index + captured.length(), " \"\"" + mid ));
            this->setCursorPosition( initialPosition + 2 );
            return true;
        }
    }

    {
        const QRegularExpression keywords( "\\w\\\"\\s*$" );
        const QRegularExpressionMatch match( keywords.match( left ));
        if ( match.hasMatch() && !mid.contains( QRegularExpression( "^\\s*\\)" ))) {
            QString captured( match.captured( 0 ));
            if ( captured.endsWith( " " ))
                captured = captured.remove( captured.length() - 1, 1 );

            const int index = left.indexOf( captured, initialPosition - captured.length());
            if ( index == -1 )
                return false;

            this->setText( left.insert( index + captured.length(), " ) " + mid ));
            this->setCursorPosition( initialPosition + 3 );
            return true;
        }
    }

    {
        const QRegularExpression keywords( "\\w\\\"\\s*$" );
        const QRegularExpressionMatch match( keywords.match( left ));
        if ( match.hasMatch() && !mid.contains( QRegularExpression( "^\\s*\\)" ))) {
            QString captured( match.captured( 0 ));
            if ( captured.endsWith( " " ))
                captured = captured.remove( captured.length() - 1, 1 );

            const int index = left.indexOf( captured, initialPosition - captured.length());
            if ( index == -1 )
                return false;

            this->setText( left.insert( index + captured.length(), " ) " + mid ));
            this->setCursorPosition( initialPosition + 3 );
            return true;
        }
    }

    {
        // const QRegularExpression keywords( "\\(\\s*\\\"([^\\\"]*)$" );
        const QRegularExpression keywords( "(?:\\(|,)\\s*\\\"([^\\\"]*)$" );
        const QRegularExpressionMatch match( keywords.match( left ));
        if ( match.hasMatch() && mid.contains( QRegularExpression( "^\\\"" ))) {
            const QString captured( match.captured( 1 ));

            if ( captured.isEmpty())
                return false;

            QStringList reagents;
            QSqlQuery query;
            query.exec( QString( "select %1 from %2 where %1 like '%3%'" )
                        .arg( Reagent::instance()->fieldName( Reagent::Alias ))
                        .arg( Reagent::instance()->tableName())
                        .arg( captured )
                        );
            while ( query.next())
                reagents << query.value( 0 ).toString();

            // if aliases come up empty, search reagent names instead
            if ( reagents.isEmpty()) {
                query.exec( QString( "select %1 from %2 where %1 like '%3%'" )
                            .arg( Reagent::instance()->fieldName( Reagent::Name ))
                            .arg( Reagent::instance()->tableName())
                            .arg( captured )
                            );
                while ( query.next())
                    reagents << query.value( 0 ).toString();
            }

            if ( reagents.isEmpty())
                return true;

            // complete to the shortest string
            QString completion;
            int match, y;
            if ( reagents.count() == 1 ) {
                // append extra space (since it's the only match that will likely be follwed by an argument)
                completion = reagents.first();
            } else if ( reagents.count() > 1 ) {
                match = 1;
                for ( y = 0; y < reagents.count(); y++ ) {
                    // make sure we check string length
                    if ( reagents.first().length() == match || reagents.at( y ).length() == match )
                        break;

                    if ( reagents.first().at( match ) == reagents.at( y ).at( match )) {
                        if ( y == reagents.count()-1 ) {
                            match++;
                            y = 0;
                        }
                    }
                }
                completion = reagents.first().left( match );
            }

            reagents.removeDuplicates();
            const int index = left.indexOf( captured, initialPosition - captured.length());
            if ( index == -1 )
                return false;

            left = left.remove( index, captured.length());
            this->setText( left.insert( index, completion + mid ));
            this->setCursorPosition( initialPosition + completion.length() - captured.length() + ( reagents.count() == 1 ? 1 : 0 ));
            return true;
        }
    }

    {
        const QRegularExpression keywords( "(?<!\\\")\\s*\\b(\\w+)$" );
        const QRegularExpressionMatch match( keywords.match( left ));
        if ( match.hasMatch() && !mid.contains( QRegularExpression( "^\\\"" ))) {
            const QString captured( match.captured( 1 ));

            if ( captured.isEmpty())
                return false;

            const QStringList filtered( functions.filter( QRegularExpression( QString( "^%1" ).arg( captured ))));
            if ( filtered.isEmpty())
                return true;

            // complete to the shortest string
            QString completion;
            int match, y;
            if ( filtered.count() == 1 ) {
                // append extra space (since it's the only match that will likely be follwed by an argument)
                completion = filtered.first();
            } else if ( filtered.count() > 1 ) {
                match = 1;
                for ( y = 0; y < filtered.count(); y++ ) {
                    // make sure we check string length
                    if ( filtered.first().length() == match || filtered.at( y ).length() == match )
                        break;

                    if ( filtered.first().at( match ) == filtered.at( y ).at( match )) {
                        if ( y == filtered.count()-1 ) {
                            match++;
                            y = 0;
                        }
                    }
                }
                completion = filtered.first().left( match );
            }

            const int index = left.indexOf( captured, initialPosition - captured.length());
            if ( index == -1 )
                return false;

            left = left.remove( index, captured.length());
            this->setText( left.insert( index, completion + ( filtered.count() == 1 ? "( \"\"" : "" ) + mid ));
            this->setCursorPosition( initialPosition + completion.length() - captured.length() + ( filtered.count() == 1 ? 3 : 0 ));
            return true;
        }
    }

    return true;
}

/**
 * @brief CalcEdit::saveHistory
 */
void CalcEdit::saveHistory() {
    Variable::instance()->setString( "calculator/commands", Variable::compressedString( this->history.join( ";" )));
}

/**
 * @brief CalcEdit::eventFilter
 * @param object
 * @param event
 * @return
 */
bool CalcEdit::eventFilter( QObject *, QEvent *event ) {
    if ( this->hasFocus()) {
        if ( event->type() == QEvent::KeyPress ) {
            QKeyEvent *keyEvent( static_cast<QKeyEvent*>( event ));

            if ( keyEvent->key() == Qt::Key_Up ) {
                if ( !this->history.isEmpty()) {
                    if ( this->offset() < this->history.count())
                        this->push();

                    const int offset = this->history.count() - this->offset();
                    this->setText( offset > 0 ? this->history.at( offset ) : this->history.first());
                }
                return true;
            } else if ( keyEvent->key() == Qt::Key_Down ) {
                if ( !this->history.isEmpty()) {
                    if ( this->offset() > 0 )
                        this->pop();

                    if ( this->offset() == 0 ) {
                        this->clear();
                        return true;
                    }

                    const int offset = this->history.count() - this->offset();
                    this->setText( offset < this->history.count() ? this->history.at( offset ) : this->history.last());
                }
                return true;
            } else if ( keyEvent->key() == Qt::Key_Tab ) {
                if ( this->text().isEmpty())
                    return true;

                return this->completeCommand();
            }
        }
    }
    return false;
}


/**
 * @brief CalcEdit::CalcEdit
 * @param parent
 */
CalcEdit::CalcEdit( QWidget *parent ) : QLineEdit( parent ) {
    // install event filter
    this->installEventFilter( this );
    this->history = Variable::uncompressedString( Variable::instance()->string( "calculator/commands" )).split( ";" );

    this->connect( this, &QLineEdit::returnPressed, [ this ]( ) {
        const QString st( this->text().simplified());
        MainWindow::instance()->appendToCalculator( st );
        this->add( st );

        // set min offset
        this->reset();
        this->clear();
    } );
}
