#-------------------------------------------------
#
# Project created by QtCreator 2017-10-31T11:44:31
#
#-------------------------------------------------

QT       += core gui sql qml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FumingCube
TEMPLATE = app

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
    template.cpp \
    reagent.cpp \
    reagentdialog.cpp \
    database.cpp \
    templatewidget.cpp \
    lineedit.cpp \
    messagedock.cpp

HEADERS += \
        mainwindow.h \
    entry.h \
    reagent.h \
    template.h \
    reagentdialog.h \
    callonce.h \
    singleton.h \
    database.h \
    templatewidget.h \
    lineedit.h \
    messagedock.h

FORMS += \
        mainwindow.ui \
    reagentdialog.ui \
    templatewidget.ui
