/*
 * Copyright (C) 2020 Armands Aleksejevs
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
#include <QObject>
#ifdef Q_OS_WIN
#include <QWinMime>

/**
 * @brief The EMFMime class
 */
class EMFMime : public QWinMime {

public:
    /**
     * @brief EMFMime
     */
    explicit EMFMime() : QWinMime() { this->id = QWinMime::registerMimeType( "Enahnced metafile" ); }

    /**
     * @brief canConvertFromMime
     * @return
     */
    bool canConvertFromMime( const FORMATETC &, const QMimeData * ) const override { return false; }
    bool canConvertToMime( const QString &, IDataObject * ) const override;

    /**
     * @brief convertFromMime
     * @return
     */
    bool convertFromMime( const FORMATETC &, const QMimeData *, STGMEDIUM * ) const override { return false; }
    QVariant convertToMime( const QString &, IDataObject *, QVariant::Type ) const override;

    /**
     * @brief formatsForMime
     * @return
     */
    QVector<FORMATETC> formatsForMime( const QString &, const QMimeData * ) const override { return QVector<FORMATETC>(); }

    /**
     * @brief mimeForFormat
     * @param format
     * @return
     */
    QString mimeForFormat( const FORMATETC &format ) const override { return ( format.cfFormat == CF_ENHMETAFILE ) ? "application/x-qt-image" : QString(); }

private:
    FORMATETC initFormat() const;

private:
    int id = 0;
};

#endif
