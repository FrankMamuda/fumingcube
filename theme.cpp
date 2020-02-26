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
#include "theme.h"
#include <QColor>
#include <QDir>
#include <QSettings>
#include "variable.h"
#include <QApplication>
#include <utility>

/**
 * @brief Theme::Theme
 */
Theme::Theme( const QString &name ) {
    this->m_style = QApplication::style();

    const QMap<QString, QString> themes( this->availableThemes());
    if ( !themes.contains( name )) {
        this->m_dark = Variable::isEnabled( "darkMode" );
        this->initializeSyntaxColours();
    } else
        this->readThemeFile( themes[name] );
}

/**
 * @brief Theme::palette
 * @return
 */
QPalette Theme::palette() const {
    /**
     * @brief The ThemeColour struct
     */
    struct ThemeColour {
        ThemeColour( QString key, const QPalette::ColorRole &role, const QPalette::ColorGroup &group, bool isBrush ) :
            m_key( std::move( key )), m_role( role ), m_group( group ), m_brush( isBrush ) {}

        [[nodiscard]] bool isBrush() const { return this->m_brush; }
        [[nodiscard]] QString key() const { return this->m_key; }
        [[nodiscard]] QPalette::ColorRole role() const { return this->m_role; }
        [[nodiscard]] QPalette::ColorGroup group() const { return this->m_group; }

    private:
        QString m_key;
        QPalette::ColorRole m_role;
        QPalette::ColorGroup m_group;
        bool m_brush;
    };

    const QList<ThemeColour> themeColours( QList<ThemeColour>() <<
                                           ThemeColour( "PaletteWindow",                    QPalette::Window,           QPalette::All,      false ) <<
                                           ThemeColour( "PaletteWindowDisabled",            QPalette::Window,           QPalette::Disabled, false ) <<
                                           ThemeColour( "PaletteWindowText",                QPalette::WindowText,       QPalette::All,      true  ) <<
                                           ThemeColour( "PaletteWindowTextDisabled",        QPalette::WindowText,       QPalette::Disabled, true  ) <<
                                           ThemeColour( "PaletteBase",                      QPalette::Base,             QPalette::All,      false ) <<
                                           ThemeColour( "PaletteBaseDisabled",              QPalette::Base,             QPalette::Disabled, false ) <<
                                           ThemeColour( "PaletteAlternateBase",             QPalette::AlternateBase,    QPalette::All,      false ) <<
                                           ThemeColour( "PaletteAlternateBaseDisabled",     QPalette::AlternateBase,    QPalette::Disabled, false ) <<
                                           ThemeColour( "PaletteToolTipBase",               QPalette::ToolTipBase,      QPalette::All,      true  ) <<
                                           ThemeColour( "PaletteToolTipBaseDisabled",       QPalette::ToolTipBase,      QPalette::Disabled, true  ) <<
                                           ThemeColour( "PaletteToolTipText",               QPalette::ToolTipText,      QPalette::All,      false ) <<
                                           ThemeColour( "PaletteToolTipTextDisabled",       QPalette::ToolTipText,      QPalette::Disabled, false ) <<
                                           ThemeColour( "PaletteText",                      QPalette::Text,             QPalette::All,      true  ) <<
                                           ThemeColour( "PaletteTextDisabled",              QPalette::Text,             QPalette::Disabled, true  ) <<
                                           ThemeColour( "PaletteButton",                    QPalette::Button,           QPalette::All,      false ) <<
                                           ThemeColour( "PaletteButtonDisabled",            QPalette::Button,           QPalette::Disabled, false ) <<
                                           ThemeColour( "PaletteButtonText",                QPalette::ButtonText,       QPalette::All,      true  ) <<
                                           ThemeColour( "PaletteButtonTextDisabled",        QPalette::ButtonText,       QPalette::Disabled, true  ) <<
                                           ThemeColour( "PaletteBrightText",                QPalette::BrightText,       QPalette::All,      false ) <<
                                           ThemeColour( "PaletteBrightTextDisabled",        QPalette::BrightText,       QPalette::Disabled, false ) <<
                                           ThemeColour( "PaletteHighlight",                 QPalette::Highlight,        QPalette::All,      true  ) <<
                                           ThemeColour( "PaletteHighlightDisabled",         QPalette::Highlight,        QPalette::Disabled, true  ) <<
                                           ThemeColour( "PaletteHighlightedText",           QPalette::HighlightedText,  QPalette::All,      true  ) <<
                                           ThemeColour( "PaletteHighlightedTextDisabled",   QPalette::HighlightedText,  QPalette::Disabled, true  ) <<
                                           ThemeColour( "PaletteLink",                      QPalette::Link,             QPalette::All,      false ) <<
                                           ThemeColour( "PaletteLinkDisabled",              QPalette::Link,             QPalette::Disabled, false ) <<
                                           ThemeColour( "PaletteLinkVisited",               QPalette::LinkVisited,      QPalette::All,      false ) <<
                                           ThemeColour( "PaletteLinkVisitedDisabled",       QPalette::LinkVisited,      QPalette::Disabled, false ) <<
                                           ThemeColour( "PaletteLight",                     QPalette::Light,            QPalette::All,      false ) <<
                                           ThemeColour( "PaletteLightDisabled",             QPalette::Light,            QPalette::Disabled, false ) <<
                                           ThemeColour( "PaletteMidlight",                  QPalette::Midlight,         QPalette::All,      false ) <<
                                           ThemeColour( "PaletteMidlightDisabled",          QPalette::Midlight,         QPalette::Disabled, false ) <<
                                           ThemeColour( "PaletteDark",                      QPalette::Dark,             QPalette::All,      false ) <<
                                           ThemeColour( "PaletteDarkDisabled",              QPalette::Dark,             QPalette::Disabled, false ) <<
                                           ThemeColour( "PaletteMid",                       QPalette::Mid,              QPalette::All,      false ) <<
                                           ThemeColour( "PaletteMidDisabled",               QPalette::Mid,              QPalette::Disabled, false ) <<
                                           ThemeColour( "PaletteShadow",                    QPalette::Shadow,           QPalette::All,      false ) <<
                                           ThemeColour( "PaletteShadowDisabled",            QPalette::Shadow,           QPalette::Disabled, false ));

    QPalette palette( QApplication::palette());
    for ( const ThemeColour &themeColour : themeColours ) {
        if ( this->paletteMap.contains( themeColour.key())) {
            const QColor colour( this->paletteMap[themeColour.key()] );
            if ( colour.isValid()) {
                if ( themeColour.isBrush())
                    palette.setBrush( themeColour.group(), themeColour.role(), qAsConst( colour ));
                else
                    palette.setColor( themeColour.group(), themeColour.role(), qAsConst( colour ));
            }
        }
    }

    return qAsConst( palette );
}

/**
 * @brief Theme::syntaxColour
 * @param key
 * @return
 */
QColor Theme::syntaxColour( const QString &key ) const {
    return this->syntaxMap.contains( key ) ? this->syntaxMap[key] : Qt::black;
}

/**
 * @brief Theme::availableThemes
 * @return
 */
QMap<QString, QString> Theme::availableThemes() {
    QMap<QString, QString> themes;

    QDir dir( QString( ":/themes/" ));
    dir.setNameFilters( QStringList() << "*.theme" );

    const QStringList internalList( dir.entryList( QDir::Files | QDir::NoDotDot ));
    for ( const QString &name : internalList )
        themes[QString( name ).remove( ".theme" )] = dir.filePath( name );

    // extenal themes take priority (override internal themes)
    dir.setPath( QDir::currentPath() + "/themes/" );
    const QStringList externalList( dir.entryList( QDir::Files | QDir::NoDotDot ));
    for ( const QString &name : externalList )
        themes[QString( name ).remove( ".theme" )] = dir.absoluteFilePath( name );

    return themes;
}

/**
 * @brief Theme::readThemeFile
 * @param fileName
 */
void Theme::readThemeFile( const QString &fileName ) {
    /**
     * @brief parseColour
     */
    auto parseColour = [ this ]( const QString &key ) {
        if ( this->paletteMap.contains( key ))
            return this->paletteMap[key];

        const QColor colour( QString( "#%1" ).arg( key ));
        return colour.isValid() ? colour : Qt::black;
    };

    // open settings file (plain INI format)
    QSettings settings( fileName, QSettings::IniFormat );

    // parse palette
    settings.beginGroup( "Palette" );
    for ( const QString &key : settings.allKeys())
        this->paletteMap[key] = parseColour( settings.value( key ).toString());
    settings.endGroup();

    // parse general settings
    settings.beginGroup( "Theme" );
    for ( const QString &key : settings.allKeys()) {
        if ( !QString::compare( key, "Dark" ))
            this->m_dark = settings.value( key ).toBool();

        if ( !QString::compare( key, "Style" ))
            this->m_style = QStyleFactory::create( "Fusion" );
    }
    settings.endGroup();
    // initialize default values for syntax highlighter (based on theme colour mode)
    this->initializeSyntaxColours();

    // override syntax highlighter colours from theme file
    settings.beginGroup( "Syntax" );
    for ( const QString &key : settings.allKeys())
        this->syntaxMap[key] = parseColour( settings.value( key ).toString());
    settings.endGroup();
}

/**
 * @brief Theme::initializeSyntaxColours
 */
void Theme::initializeSyntaxColours() {
    // set default syntax highlighter colours
    this->syntaxMap["Number"] = ( this->isDark() ? QColor::fromRgb( 102, 163, 52 ) : QColor::fromRgb( 0, 0, 128 ));
    this->syntaxMap["Operator"] = ( this->isDark() ? QColor::fromRgb( 214, 207, 154 ) : Qt::black );
    this->syntaxMap["Comment"] = ( this->isDark() ? QColor::fromRgb( 168, 171, 176 ) : QColor::fromRgb( 0, 128, 0 ));
    this->syntaxMap["Parenthesis"] = this->syntaxMap["Operator"];
    this->syntaxMap["Keyword"] = ( this->isDark() ? QColor::fromRgb( 69, 198, 214 ) : QColor::fromRgb( 0, 103, 124 ));
    this->syntaxMap["Reference"] = ( this->isDark() ? QColor::fromRgb( 214, 149, 69 ) : QColor::fromRgb( 0, 128, 0 ));
    this->syntaxMap["Error"] = ( this->isDark() ? QColor::fromRgb( 214, 86, 69 ) : QColor::fromRgb( 255, 0, 0 ));
    this->syntaxMap["Undefined"] = Qt::darkYellow;
    this->syntaxMap["String"] = this->syntaxMap["Undefined"];
}
