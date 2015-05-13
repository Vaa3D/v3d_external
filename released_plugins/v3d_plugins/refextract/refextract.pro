
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += release x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

HEADERS     = refextract.h

SOURCES     = refextract.cpp
SOURCES     +=$$V3DMAINDIR/basic_c_fun/v3d_message.cpp


LIBS         += -lpthread
#LIBS	     += -lv3dfftw3f -lv3dfftw3f_threads

TARGET        = $$qtLibraryTarget(refExtract)
DESTDIR       = $$V3DMAINDIR/../bin/plugins/refExtract


