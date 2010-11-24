
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64
QT += testlib

INCLUDEPATH  += ../../../v3d_main/basic_c_fun 

HEADERS       = plugin_moviefrom3dviewer_qwait.h
SOURCES       = plugin_moviefrom3dviewer_qwait.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(moviefrom3dviewer)

DESTDIR       = ../../v3d/plugins/moviefrom3dviewer

