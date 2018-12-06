#-------------------------------------------------
#
# Project created by QtCreator 2017-10-31T11:44:31
#
#-------------------------------------------------

QT       += core gui sql qml xml winextras network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FumingCube
TEMPLATE = app

win32:LIBS += -lgdi32
win32:CONFIG += openssl-linked

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32:RC_FILE = icon.rc

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    entry.cpp \
    reagentdialog.cpp \
    templatewidget.cpp \
    lineedit.cpp \
    messagedock.cpp \
    charactermap.cpp \
    textedit.cpp \
    propertydialog.cpp \
    propertyeditor.cpp \
    propertydelegate.cpp \
    imageutils.cpp \
    variable.cpp \
    xmltools.cpp \
    extractiondialog.cpp \
    table.cpp \
    database.cpp \
    property.cpp \
    reagent.cpp \
    template.cpp \
    networkmanager.cpp \
    tagdialog.cpp \
    tag.cpp

HEADERS += \
        mainwindow.h \
    entry.h \
    reagentdialog.h \
    templatewidget.h \
    lineedit.h \
    messagedock.h \
    charactermap.h \
    textedit.h \
    propertydialog.h \
    propertyeditor.h \
    propertydelegate.h \
    imageutils.h \
    variable.h \
    xmltools.h \
    networkmanager.h \
    extractiondialog.h \
    extractionmodel.h \
    variableentry.h \
    widget.h \
    main.h \
    field.h \
    table.h \
    database.h \
    property.h \
    reagent.h \
    template.h \
    nfpawidget.h \
    ghswidget.h \
    tagdialog.h \
    tag.h

FORMS += \
        mainwindow.ui \
    reagentdialog.ui \
    templatewidget.ui \
    propertydialog.ui \
    propertyeditor.ui \
    imageutils.ui \
    extractiondialog.ui \
    tagdialog.ui

RESOURCES += \
    resources.qrc
