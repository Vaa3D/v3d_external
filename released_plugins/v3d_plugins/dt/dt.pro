
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64
INCLUDEPATH  += ../../basic_c_fun
HEADERS       = dt.h
SOURCES       = dt.cpp
SOURCES      += ../../basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(dt)
DESTDIR       = ../../v3d/plugins/Distance_Transform

