
TEMPLATE      = lib
CONFIG       += plugin warn_off
INCLUDEPATH  += ../../basic_c_fun
HEADERS       = minMaxfilterplugin.h
SOURCES       = minMaxfilterplugin.cpp
TARGET        = $$qtLibraryTarget(minMaxfilter)
DESTDIR       = ../../v3d/plugins/min_Max_Filter

