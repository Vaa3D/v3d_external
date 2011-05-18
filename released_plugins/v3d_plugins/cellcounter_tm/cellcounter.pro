
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
CONFIG       += x86_64
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = cellcounter.h
SOURCES       = cellcounter.cpp
TARGET        = $$qtLibraryTarget(cellcounter)
DESTDIR       = ../../v3d/plugins/CellCounterUsingTemplateMatching

