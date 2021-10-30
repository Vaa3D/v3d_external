# project file required for creating custom FragmentGalleryWidget plugin for use in Qt Designer
TEMPLATE    = lib
QT += core gui
CONFIG      += designer plugin release
mac: CONFIG += x86 x86_64
TARGET      = $$qtLibraryTarget($$TARGET)
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
DEFINES += _DESIGNER_EXPORT

HEADERS += ../FragmentGalleryWidget.h \
          FragmentGalleryWidgetPlugin.h

SOURCES += ../FragmentGalleryWidget.cpp \
          FragmentGalleryWidgetPlugin.cpp


