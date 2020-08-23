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
    explicit TextEdit( QWidget *parent = nullptr );
    ~TextEdit() override;
    void insertImage( const QImage &image );
    void insertImageData( const int width, const int height, const QString &base64 );

    /**
     * @brief cleanHTML
     * @return
     */
    [[nodiscard]] bool cleanHTML() const { return this->m_cleanHTML; }

    /**
     * @brief isSimpleEditor
     * @return
     */
    [[nodiscard]] bool isSimpleEditor() const { return this->m_simpleEditor; }

    /**
     * @brief completer
     * @return
     */
    [[nodiscard]] QCompleter *completer() const { return this->m_completer; }

    /**
     * @brief id
     * @return
     */
    [[nodiscard]] quint32 id() const { return this->m_id; }

signals:
    void entered();

protected:
    bool canInsertFromMimeData( const QMimeData *source ) const override;
    void insertFromMimeData( const QMimeData *source ) override;
    void dropEvent( QDropEvent *event ) override;
    void focusInEvent( QFocusEvent *event ) override { emit this->entered(); QTextEdit::focusInEvent( event ); }
    void keyPressEvent( QKeyEvent *event ) override;
    bool eventFilter( QObject *object, QEvent *event ) override;

public slots:
    /**
     * @brief setCleanHTML
     * @param enable
     */
    void setCleanHTML( bool enable ) { this->m_cleanHTML = enable; }

    /**
     * @brief setSimpleEditor
     * @param simple
     */
    void setSimpleEditor( bool simple ) { this->m_simpleEditor = simple; }
    void setCompleter( QCompleter *completer );

private:
    bool m_cleanHTML = true;
    bool m_simpleEditor = false;
    QCompleter *m_completer = nullptr;
    quint32 m_id = 0;
};
