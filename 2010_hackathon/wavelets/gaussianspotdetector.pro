
TEMPLATE      = lib
CONFIG       += plugin warning_off
CONFIG       += x86
INCLUDEPATH  += ../../../basic_c_fun
HEADERS       += gaussianspotdetector.h
SOURCES       += gaussianspotdetector.cpp
LIBS	      += -lfftw3
TARGET        = $$qtLibraryTarget(gaussianspotdetector)
DESTDIR       = /Applications/v3d/plugins/Hackathon/

