/*
 * Copyright (C) 2017-2018 Factory #12
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

#pragma once

/*
 * includes
 */
#include <QCompleter>
#include <QTextEdit>

/**
 * @brief The TextEdit class
 */
class TextEdit : public QTextEdit {
    Q_OBJECT
    Q_PROPERTY( bool cleanHTML READ cleanHTML WRITE setCleanHTML )

public:
    TextEdit( QWidget *parent = nullptr ) : QTextEdit( parent ), m_cleanHTML( true ) {}
    void insertPixmap( const QPixmap &image, const int forcedSize = -1 );
    bool cleanHTML() const { return this->m_cleanHTML; }
    static QString stripHTML( const QString &input );
    bool isSimpleEditor() const { return this->m_simpleEditor; }
    QCompleter *completer() const { return this->m_completer; }

signals:
    void entered();

protected:
    bool canInsertFromMimeData( const QMimeData *source ) const override;
    void insertFromMimeData( const QMimeData *source ) override;
    void dropEvent( QDropEvent *event ) override;
    void focusInEvent( QFocusEvent *event ) override { emit this->entered(); QTextEdit::focusInEvent( event ); }
    void keyPressEvent( QKeyEvent *event ) override;

public slots:
    void setCleanHTML( bool enable ) { this->m_cleanHTML = enable; }
    void setSimpleEditor( bool simple ) { this->m_simpleEditor = simple; }
    void setCompleter( QCompleter *completer );

private:
    bool m_cleanHTML;
    bool m_simpleEditor = false;
    QCompleter *m_completer = nullptr;
};
