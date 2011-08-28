
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
INCLUDEPATH	+= ../../../v3d_main/basic_c_fun
V3DCONVERTPATH = ../internal/v3d_convert
INCLUDEPATH += $$V3DCONVERTPATH 

#todo : define __DISPLAY_TEMP_POINTS

HEADERS	= tangent_plane_plugin.h
HEADERS	+= tangent_plane_func.h
HEADERS	+= tangent_plane_gui.h
HEADERS	+= $$V3DCONVERTPATH/neuron_tracing.h
HEADERS	+= $$V3DCONVERTPATH/img_threshold.h
HEADERS	+= $$V3DCONVERTPATH/img_segment.h

SOURCES	= tangent_plane_plugin.cpp
SOURCES	+= tangent_plane_func.cpp
SOURCES	+= tangent_plane_gui.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(tangent_plane)
DESTDIR	= ~/Applications/v3d/plugins/tangent_plane/
