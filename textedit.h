/*
 * Copyright (C) 2017-2018 Factory #12
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

#pragma once

//
// includes
//
#include <QTextEdit>

/**
 * @brief The TextEdit class
 */
class TextEdit : public QTextEdit {
    Q_OBJECT
    Q_PROPERTY( bool cleanHTML READ cleanHTML WRITE setCleanHTML )

public:
    TextEdit( QWidget *parent = 0 ) : QTextEdit( parent ), m_cleanHTML( true ) {}
    void insertPixmap( const QPixmap &image, const int forcedSize = -1 );
    bool cleanHTML() const { return this->m_cleanHTML; }
    static QString stripHTML( const QString &input );

signals:
    void entered();

protected:
    bool canInsertFromMimeData( const QMimeData *source ) const override;
    void insertFromMimeData( const QMimeData *source ) override;
    void dropEvent( QDropEvent *event ) override;
    void focusInEvent( QFocusEvent *event ) override { emit this->entered(); QTextEdit::focusInEvent( event ); }

public slots:
    void setCleanHTML( bool enable ) { this->m_cleanHTML = enable; }

private:
    bool m_cleanHTML;
};
