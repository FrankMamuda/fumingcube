/*
 * Copyright (C) 2013-2018 Factory #12
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
#include <QLineEdit>
#include <QMainWindow>
#include <QMouseEvent>

/**
 * @brief The CalcEdit class
 */
class CalcEdit final : public QLineEdit {
    Q_OBJECT
    Q_PROPERTY( int offset READ offset WRITE set RESET reset )

public:
    explicit CalcEdit( QWidget *parent = nullptr );
    ~CalcEdit() override { this->history.clear(); }
    int offset() const { return this->m_historyOffset; }
    bool completeCommand();

public slots:
    void set( int offset = 0 ) { this->m_historyOffset = offset; }
    void reset() { this->set(); }
    void push() { this->m_historyOffset++; }
    void pop() { this->m_historyOffset--; }
    void add( const QString &text ) {
        if ( this->history.count()) {
            if ( !QString::compare( this->history.last(), text ))
                return;
        }

        if ( this->history.count() >= MaxHistory )
            this->history.removeFirst();

        this->history << text;
    }
    void saveHistory();

protected:
    bool eventFilter( QObject *object, QEvent *event ) override;

private:
    int m_historyOffset = 0;
    QStringList history;
    static constexpr int MaxHistory = 32;
};
