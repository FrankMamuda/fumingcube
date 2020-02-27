#-------------------------------------------------
#
# Project created by QtCreator 2019-09-19T14:17:30
#
#-------------------------------------------------

QT       += core gui sql xml qml

win32:QT += winextras
win32:LIBS += -lgdi32 -luser32
win32:CONFIG += openssl-linked
win32:RC_FILE = icon.rc

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FumingCube
TEMPLATE = app

macx:ICON = icon.icns

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17
CONFIG += c++1z
#QMAKE_CXXFLAGS += /std:c++17

SOURCES += \
    about.cpp \
    cache.cpp \
    charactermap.cpp \
    extractiondialog.cpp \
    ghsbuilder.cpp \
    ghswidget.cpp \
    htmlutils.cpp \
    imageutils.cpp \
    label.cpp \
    labeldialog.cpp \
    labeldock.cpp \
    labelselector.cpp \
    labelset.cpp \
    main.cpp \
    mainwindow.cpp \
    database.cpp \
    networkmanager.cpp \
    nfpabuilder.cpp \
    nfpawidget.cpp \
    pixmaputils.cpp \
    propertydelegate.cpp \
    propertydialog.cpp \
    propertydock.cpp \
    propertyeditor.cpp \
    propertyview.cpp \
    propertywidget.cpp \
    reagentdelegate.cpp \
    reagentdialog.cpp \
    reagentdock.cpp \
    reagentmodel.cpp \
    reagentview.cpp \
    script.cpp \
    settingsdialog.cpp \
    structurebrowser.cpp \
    syntaxhighlighter.cpp \
    table.cpp \
    tag.cpp \
    tagdialog.cpp \
    textedit.cpp \
    theme.cpp \
    variable.cpp \
    xmltools.cpp \
    reagent.cpp \
    property.cpp \
    calcedit.cpp \
    nodehistory.cpp \
    editortoolbar.cpp

HEADERS += \
    about.h \
    buttonbox.h \
    cache.h \
    charactermap.h \
    dockwidget.h \
    extractiondialog.h \
    extractionmodel.h \
    ghsbuilder.h \
    ghspictograms.h \
    ghswidget.h \
    htmlutils.h \
    imageutils.h \
    label.h \
    labeldialog.h \
    labeldock.h \
    labelselector.h \
    labelset.h \
    mainwindow.h \
    database.h \
    field.h \
    networkmanager.h \
    nfpabuilder.h \
    nfpawidget.h \
    pixmaputils.h \
    propertydelegate.h \
    propertydialog.h \
    propertydock.h \
    propertyeditor.h \
    propertyinput.h \
    propertyview.h \
    propertyviewwidget.h \
    propertywidget.h \
    reagentdelegate.h \
    reagentdialog.h \
    reagentdock.h \
    reagentmodel.h \
    reagentview.h \
    script.h \
    settingsdialog.h \
    structurebrowser.h \
    syntaxhighlighter.h \
    table.h \
    tag.h \
    tagdialog.h \
    textedit.h \
    theme.h \
    variable.h \
    variableentry.h \
    widget.h \
    xmltools.h \
    main.h \
    reagent.h \
    property.h \
    calcedit.h \
    nodehistory.h \
    editortoolbar.h

FORMS += \
        about.ui \
        extractiondialog.ui \
        imageutils.ui \
        labeldialog.ui \
        labeldock.ui \
        labelselector.ui \
        mainwindow.ui \
        nfpabuilder.ui \
        propertydialog.ui \
        propertydock.ui \
        propertyeditor.ui \
        reagentdialog.ui \
        reagentdock.ui \
        settingsdialog.ui \
        structurebrowser.ui \
        tagdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc \
    dark.qrc \
    light.qrc

# NOTE: using libs from https://bintray.com/vszakats/generic/openssl
win32:INCLUDEPATH += C:/openssl-win64/include/openssl
win32:LIBS += -LC:/openssl-win64/lib -lcrypto -lssl

# latvian locale
# DEFINES += FORCE_LV_LOCALE
TRANSLATIONS = i18n/fumingCube_lv_LV.ts

DISTFILES += \
    CMakeLists.txt
