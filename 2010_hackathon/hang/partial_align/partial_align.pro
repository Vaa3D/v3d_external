
TEMPLATE      = lib
CONFIG       += plugin warning_off
#CONFIG	     += X86_64 
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = partial_align_plugin.h
SOURCES       = partial_align_plugin.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.h
TARGET        = $$qtLibraryTarget(partial_align)
DESTDIR       = ~/Applications/v3d/plugins/partial_align/

