#-------------------------------------------------
#
# Project created by QtCreator 2019-09-19T14:17:30
#
#-------------------------------------------------

QT       += core gui sql xml qml

win32:QT += winextras
win32:LIBS += -lgdi32 -luser32
win32:CONFIG += openssl-linked

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FumingCube
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    charactermap.cpp \
    extractiondialog.cpp \
    ghsbuilder.cpp \
    imageutils.cpp \
    main.cpp \
    mainwindow.cpp \
    database.cpp \
    networkmanager.cpp \
    nfpabuilder.cpp \
    propertydelegate.cpp \
    propertydialog.cpp \
    propertydock.cpp \
    propertyeditor.cpp \
    propertyview.cpp \
    reagentdialog.cpp \
    reagentdock.cpp \
    reagentmodel.cpp \
    script.cpp \
    syntaxhighlighter.cpp \
    table.cpp \
    tag.cpp \
    tagdialog.cpp \
    textedit.cpp \
    variable.cpp \
    xmltools.cpp \
    reagent.cpp \
    property.cpp \
    calcedit.cpp

HEADERS += \
    buttonbox.h \
    charactermap.h \
    dockwidget.h \
    extractiondialog.h \
    extractionmodel.h \
    ghsbuilder.h \
    ghswidget.h \
    imageutils.h \
    mainwindow.h \
    database.h \
    field.h \
    networkmanager.h \
    nfpabuilder.h \
    nfpawidget.h \
    propertydelegate.h \
    propertydialog.h \
    propertydock.h \
    propertyeditor.h \
    propertyview.h \
    propertywidget.h \
    reagentdialog.h \
    reagentdock.h \
    reagentmodel.h \
    script.h \
    syntaxhighlighter.h \
    table.h \
    tag.h \
    tagdialog.h \
    textedit.h \
    variable.h \
    variableentry.h \
    widget.h \
    xmltools.h \
    main.h \
    reagent.h \
    property.h \
    calcedit.h

FORMS += \
        extractiondialog.ui \
        imageutils.ui \
        mainwindow.ui \
        nfpabuilder.ui \
        propertydialog.ui \
        propertydock.ui \
        propertyeditor.ui \
        reagentdialog.ui \
        reagentdock.ui \
        tagdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

win32:INCLUDEPATH += C:/OpenSSL-Win64/include/openssl
win32:LIBS += -LC:/OpenSSL-Win64/lib -llibeay32 -lssleay32
