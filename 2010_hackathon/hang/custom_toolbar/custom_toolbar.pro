
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
INCLUDEPATH	+= ../../../v3d_main/basic_c_fun

HEADERS	= custom_button_plugin.h
HEADERS	+= custom_button_func.h
HEADERS	+= custom_button_gui.h

SOURCES	= custom_button_plugin.cpp
SOURCES	+= custom_button_func.cpp
SOURCES += custom_button_gui.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(custom_button)
DESTDIR	= ~/Applications/v3d/plugins/custom_button/

