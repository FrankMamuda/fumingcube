#===========================================================================
# Copyright (C) 2016 Avotu Briezhaudzetava
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see http://www.gnu.org/licenses/.
#
#===========================================================================

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FumingCube
TEMPLATE = app
win32:RC_FILE = icon.rc

SOURCES += main.cpp\
        gui_main.cpp \
    database.cpp \
    template.cpp \
    gui_addtemplate.cpp \
    gui_properties.cpp \
    property.cpp \
    propertiesmodel.cpp \
    gui_addproperty.cpp

HEADERS  += gui_main.h \
    database.h \
    entry.h \
    template.h \
    main.h \
    gui_addtemplate.h \
    gui_properties.h \
    propertiesmodel.h \
    property.h \
    gui_addproperty.h

FORMS    += gui_main.ui \
    gui_addtemplate.ui \
    gui_properties.ui \
    gui_addproperty.ui

DISTFILES += \
    icon.ico \
    icon.rc

RESOURCES += \
    resources.qrc
