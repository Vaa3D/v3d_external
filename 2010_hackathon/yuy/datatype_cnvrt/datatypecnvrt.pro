
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64

V3DMAINDIR = ../../../v3d_external/v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun

HEADERS       = datatypecnvrt.h 

SOURCES       = datatypecnvrt.cpp

TARGET        = $$qtLibraryTarget(datatypeconvert)
DESTDIR       = $$V3DMAINDIR/../v3d/plugins/DataTypeConvert

