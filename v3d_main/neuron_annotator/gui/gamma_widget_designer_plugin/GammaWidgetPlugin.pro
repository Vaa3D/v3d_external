# project file required for creating custom GammaWidget plugin for use in Qt Designer
TEMPLATE    = lib
QT += core gui
CONFIG      += designer plugin release
mac: CONFIG += x86 x86_64
TARGET      = $$qtLibraryTarget($$TARGET)
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
DEFINES += _DESIGNER_EXPORT

HEADERS += ../GammaWidget.h \
          GammaWidgetPlugin.h

SOURCES += ../GammaWidget.cpp \
          GammaWidgetPlugin.cpp

FORMS +=  ../GammaWidget.ui

