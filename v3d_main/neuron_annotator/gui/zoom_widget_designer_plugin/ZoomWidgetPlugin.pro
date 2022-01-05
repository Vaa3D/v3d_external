# project file required for creating custom ZoomWidget plugin for use in Qt Designer
TEMPLATE    = lib
QT += core gui
CONFIG      += designer plugin release
mac: CONFIG += x86 x86_64
TARGET      = $$qtLibraryTarget($$TARGET)
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
DEFINES += _DESIGNER_EXPORT
INCLUDEPATH += ../../../v3d

HEADERS += ../ZoomWidget.h \
           ../ZoomSpinBox.h \
           ZoomWidgetPlugin.h

SOURCES += ../ZoomWidget.cpp \
           ../ZoomSpinBox.cpp \
           ZoomWidgetPlugin.cpp

FORMS += ../ZoomWidget.ui

