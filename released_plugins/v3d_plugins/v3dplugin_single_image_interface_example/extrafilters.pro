
TEMPLATE      = lib
CONFIG       += plugin warning_off 
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = extrafiltersplugin.h
SOURCES       = extrafiltersplugin.cpp
TARGET        = $$qtLibraryTarget(extrafilters)
DESTDIR       = ../../v3d/plugins/V3D_Plugin_Single_Image_Interface_demo #win32 qmake couldn't handle space in path

