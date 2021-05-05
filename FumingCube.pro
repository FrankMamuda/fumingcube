#-------------------------------------------------
#
# Project created by QtCreator 2019-09-19T14:17:30
#
#-------------------------------------------------

QT       += core gui sql xml qml

win32:QT += winextras
win32:LIBS += -lgdi32 -luser32 -lole32
win32:CONFIG += openssl-linked
win32:RC_FILE = icon.rc

win32:SOURCES = emfmime.cpp
win32:HEADERS = emfmime.h

macx:CONFIG += sdk_no_version_check

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32:TARGET = fumingcube
unix:TARGET = fumingcube
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
CONFIG += c++14
CONFIG += c++17

SOURCES += \
    about.cpp \
    cache.cpp \
    calcview.cpp \
    charactermap.cpp \
    cropwidget.cpp \
    datepicker.cpp \
    extractiondialog.cpp \
    fragmentnavigation.cpp \
    ghsbuilder.cpp \
    ghswidget.cpp \
    htmlutils.cpp \
    imageutils.cpp \
    imagewidget.cpp \
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
    propertyfragment.cpp \
    propertyview.cpp \
    propertywidget.cpp \
    reagentdelegate.cpp \
    reagentdialog.cpp \
    reagentdock.cpp \
    reagentmodel.cpp \
    reagentview.cpp \
    script.cpp \
    scriptmath.cpp \
    searchengine.cpp \
    searchfragment.cpp \
    settingsdialog.cpp \
    structurefragment.cpp \
    syntaxhighlighter.cpp \
    system.cpp \
    table.cpp \
    tabledialog.cpp \
    tableentry.cpp \
    tableproperty.cpp \
    tableviewer.cpp \
    tag.cpp \
    tagdialog.cpp \
    tagselectiondialog.cpp \
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
    calcview.h \
    charactermap.h \
    cropwidget.h \
    datepicker.h \
    dockwidget.h \
    extractiondialog.h \
    extractionmodel.h \
    fragment.h \
    fragmentnavigation.h \
    ghsbuilder.h \
    ghspictograms.h \
    ghswidget.h \
    htmlutils.h \
    imageutils.h \
    imagewidget.h \
    label.h \
    labeldialog.h \
    labeldock.h \
    labelselector.h \
    labelset.h \
    listutils.h \
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
    propertyfragment.h \
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
    scriptmath.h \
    searchengine.h \
    searchfragment.h \
    settingsdialog.h \
    structurefragment.h \
    syntaxhighlighter.h \
    system.h \
    tabbar.h \
    table.h \
    tabledialog.h \
    tableentry.h \
    tableproperty.h \
    tableviewer.h \
    tag.h \
    tagdialog.h \
    tagselectiondialog.h \
    textedit.h \
    textutils.h \
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
        datepicker.ui \
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
        propertyfragment.ui \
        reagentdialog.ui \
        reagentdock.ui \
        searchfragment.ui \
        settingsdialog.ui \
        structurefragment.ui \
        tabledialog.ui \
        tableviewer.ui \
        tagdialog.ui \
        tagselectiondialog.ui

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
# DEFINES += FORCE_EN_LOCALE
TRANSLATIONS = i18n/fumingCube_lv_LV.ts

# CONFIG += lrelease

DISTFILES += \
    .gitattributes \
    .gitignore \
    CMakeLists.txt \
    README.md \
    _config.yml \
    icon.ico \
    icon.rc \
    snap/gui/FumingCube.desktop \
    snapcraft.yaml

TRANSLATION_TARGET_DIR = $${_PRO_FILE_PWD_}/i18n/

isEmpty(QMAKE_LUPDATE) {
    win32:LANGUPD = $$[QT_INSTALL_BINS]\lupdate.exe
    else:LANGUPD = $$[QT_INSTALL_BINS]/lupdate
}

isEmpty(QMAKE_LRELEASE) {
    win32:LANGREL = $$[QT_INSTALL_BINS]\lrelease.exe
    else:LANGREL = $$[QT_INSTALL_BINS]/lrelease
}

langupd.command = $$LANGUPD $$shell_path($$_PRO_FILE_) -ts $$_PRO_FILE_PWD_/$$TRANSLATIONS
langrel.depends = langupd
langrel.input = TRANSLATIONS
langrel.output = $$TRANSLATION_TARGET_DIR/${QMAKE_FILE_BASE}.qm
langrel.commands = $$LANGREL ${QMAKE_FILE_IN} -qm $$TRANSLATION_TARGET_DIR/${QMAKE_FILE_BASE}.qm
langrel.CONFIG += no_link

QMAKE_EXTRA_TARGETS += langupd
QMAKE_EXTRA_COMPILERS += langrel
PRE_TARGETDEPS += langupd compiler_langrel_make_all

