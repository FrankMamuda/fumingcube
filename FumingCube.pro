#-------------------------------------------------
#
# Project created by QtCreator 2016-06-09T10:38:54
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FumingCube
TEMPLATE = app


SOURCES += main.cpp\
        gui_main.cpp \
    database.cpp \
    template.cpp \
    gui_addtemplate.cpp

HEADERS  += gui_main.h \
    database.h \
    entry.h \
    template.h \
    main.h \
    gui_addtemplate.h

FORMS    += gui_main.ui \
    gui_addtemplate.ui
