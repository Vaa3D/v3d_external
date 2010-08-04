
TEMPLATE      = lib
CONFIG       += plugin warning_off
CONFIG       += x86
INCLUDEPATH  += ../../../basic_c_fun
HEADERS       += gaussianspotdetector.h v3d_utils.h
SOURCES       += gaussianspotdetector.cpp v3d_utils.cpp
LIBS	      += -lfftw3
TARGET        = $$qtLibraryTarget(gaussianspotdetector)
DESTDIR       = /Applications/v3d/plugins/Hackathon/

