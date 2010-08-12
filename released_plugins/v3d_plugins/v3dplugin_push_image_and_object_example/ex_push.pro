
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64
INCLUDEPATH  += ../../../v3d_main/basic_c_fun 
HEADERS       = ex_push.h
SOURCES       = ex_push.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(ex_push)
DESTDIR       = ../../v3d/plugins/V3D_Plugin_Example_push_image_and_objects

