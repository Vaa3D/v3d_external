
TEMPLATE      = lib
CONFIG       += plugin warning_off
INCLUDEPATH  += ../../basic_c_fun
HEADERS       = gaussianfilterplugin.h
SOURCES       = gaussianfilterplugin.cpp
TARGET        = $$qtLibraryTarget(gaussianfilter)
DESTDIR       = ../../v3d/plugins/Gaussian_Filter

