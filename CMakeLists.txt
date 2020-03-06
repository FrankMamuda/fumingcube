cmake_minimum_required( VERSION 3.5 )

project( fumingcube LANGUAGES CXX )

find_package( Qt5 COMPONENTS REQUIRED Core Gui Widgets Network Sql Qml Xml )

if( WIN32 )
    find_package(Qt5WinExtras REQUIRED)
    set( dep_libs ${dep_libs} Qt5::WinExtras )
endif( WIN32 )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_INCLUDE_CURRENT_DIR ON )

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

set( project_sources
    about.cpp
    cache.cpp
    charactermap.cpp
    extractiondialog.cpp
    ghsbuilder.cpp
    ghswidget.cpp
    imageutils.cpp
    label.cpp
    labeldialog.cpp
    labeldock.cpp
    labelselector.cpp
    labelset.cpp
    main.cpp
    mainwindow.cpp
    database.cpp
    networkmanager.cpp
    nfpabuilder.cpp
    nfpawidget.cpp
    propertydelegate.cpp
    propertydialog.cpp
    propertydock.cpp
    propertyeditor.cpp
    propertyview.cpp
    propertywidget.cpp
    reagentdelegate.cpp
    reagentdialog.cpp
    reagentdock.cpp
    reagentmodel.cpp
    reagentview.cpp
    script.cpp
    settingsdialog.cpp
    structurebrowser.cpp
    syntaxhighlighter.cpp
    table.cpp
    tag.cpp
    tagdialog.cpp
    textedit.cpp
    theme.cpp
    variable.cpp
    xmltools.cpp
    reagent.cpp
    property.cpp
    calcedit.cpp
    htmlutils.cpp
    pixmaputils.cpp
    searchengine.cpp
    nodehistory.cpp
    editortoolbar.cpp )

set( project_headers
    about.h
    buttonbox.h
    cache.h
    charactermap.h
    dockwidget.h
    extractiondialog.h
    extractionmodel.h
    ghsbuilder.h
    ghspictograms.h
    ghswidget.h
    imageutils.h
    label.h
    labeldialog.h
    labeldock.h
    labelselector.h
    labelset.h
    mainwindow.h
    database.h
    field.h
    networkmanager.h
    nfpabuilder.h
    nfpawidget.h
    propertydelegate.h
    propertydialog.h
    propertydock.h
    propertyeditor.h
    propertyview.h
    propertyviewwidget.h
    propertywidget.h
    reagentdelegate.h
    reagentdialog.h
    reagentdock.h
    reagentmodel.h
    reagentview.h
    script.h
    settingsdialog.h
    structurebrowser.h
    syntaxhighlighter.h
    table.h
    tag.h
    tagdialog.h
    textedit.h
    theme.h
    variable.h
    variableentry.h
    widget.h
    xmltools.h
    main.h
    reagent.h
    property.h
    calcedit.h
    htmlutils.h
    pixmaputils.h
    searchengine.h
    textutils.h
    propertyinput.h
    nodehistory.h
    editortoolbar.h )

qt5_wrap_cpp( moc_sources ${project_headers} )

set( project_ui
    about.ui
    extractiondialog.ui
    imageutils.ui
    labeldialog.ui
    labeldock.ui
    labelselector.ui
    mainwindow.ui
    nfpabuilder.ui
    propertydialog.ui
    propertydock.ui
    propertyeditor.ui
    reagentdialog.ui
    reagentdock.ui
    settingsdialog.ui
    structurebrowser.ui
    tagdialog.ui )

qt5_wrap_ui( ui_wrap ${project_ui} )

set( project_rcc
     dark.qrc
     light.qrc
     resources.qrc )

qt5_add_resources( rcc_sources ${project_rcc} )

add_executable( fumingcube
    ${project_sources}
    ${project_headers}
    ${ui_wrap}
    ${moc_sources}
    ${rcc_sources}
    )

target_link_libraries( fumingcube
        PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Network
        Qt5::Sql
        Qt5::Qml
        Qt5::Xml
        ${dep_libs} )
