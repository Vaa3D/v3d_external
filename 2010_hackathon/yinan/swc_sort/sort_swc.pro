TEMPLATE      = lib
CONFIG       += qt plugin warn_off

INCLUDEPATH  += ../../../v3d_main/basic_c_fun

HEADERS      +=../../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS      += sort_swc.h

SOURCES      +=../../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES      += sort_swc.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(sort_swc)
DESTDIR       = ../../../v3d/plugins/neuron_utilities/sort_a_swc
