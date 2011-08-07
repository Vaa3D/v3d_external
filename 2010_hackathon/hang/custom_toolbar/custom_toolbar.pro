
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
INCLUDEPATH	+= ../../../v3d_main/basic_c_fun

HEADERS	= custom_toolbar_plugin.h
HEADERS	+= custom_toolbar_func.h
HEADERS	+= custom_toolbar_gui.h

SOURCES	= custom_toolbar_plugin.cpp
SOURCES	+= custom_toolbar_func.cpp
SOURCES += custom_toolbar_gui.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(custom_toolbar)
DESTDIR	= ~/Applications/v3d/plugins/custom_toolbar/

