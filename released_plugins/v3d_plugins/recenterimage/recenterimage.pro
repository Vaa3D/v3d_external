# revised by Hanchuan Peng
# 2010-05-20. Note that there is a problem about non-VC compliers (e.g. gcc) may complain the redundant compiling of vcdiff.h/.cpp

TEMPLATE      = lib
CONFIG       += plugin warn_off
CONFIG	     += x86_64
INCLUDEPATH  += ../../../v3d_main/basic_c_fun 
HEADERS       = recenterimageplugin.h
HEADERS      += ../../../v3d_main/basic_c_fun/vcdiff.h
SOURCES       = recenterimageplugin.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/vcdiff.cpp
TARGET        = $$qtLibraryTarget(recenterimage)
DESTDIR       = ../../v3d/plugins/ReCenter_Image

