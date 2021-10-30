# project file required for creating custom AngleWidget plugin for use in Qt Designer
TEMPLATE    = lib
QT += core gui
CONFIG      += designer plugin release
mac: CONFIG += x86 x86_64
TARGET      = $$qtLibraryTarget($$TARGET)
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
DEFINES += _DESIGNER_EXPORT

HEADERS += ../AngleWidget.h \
          AngleWidgetPlugin.h

SOURCES += ../AngleWidget.cpp \
          AngleWidgetPlugin.cpp

FORMS +=  ../AngleWidget.ui

