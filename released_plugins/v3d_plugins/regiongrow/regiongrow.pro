
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = regiongrow.h
SOURCES       = regiongrow.cpp
TARGET        = $$qtLibraryTarget(regiongrow)
DESTDIR       = ../../v3d/plugins/regiongrow


