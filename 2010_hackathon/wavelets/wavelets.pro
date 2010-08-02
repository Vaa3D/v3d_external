
TEMPLATE      = lib
CONFIG       += plugin warning_off
INCLUDEPATH  += ../../basic_c_fun
CONFIG        += x86
HEADERS     = wavelets.h scaleinfo.h


SOURCES     = wavelets.cpp scaleinfo.cpp


TARGET        = $$qtLibraryTarget(wavelets)
DESTDIR       = /home/ibanez/bin/v3d/v3d_2.485_ubuntu9.1_64bit

#/Applications/v3d/plugins/Wavelets #win32 qmake couldn't handle space in path
