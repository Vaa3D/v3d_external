
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
CONFIG       += x86_64
INCLUDEPATH  += ../../v3d_main/basic_c_fun

HEADERS       = miviewer.h \
		../../v3d_main/basic_c_fun/stackutil.h \
		../../v3d_main/basic_c_fun/mg_utilities.h \
		../../v3d_main/basic_c_fun/mg_image_lib.h

SOURCES       = miviewer.cpp \
		../../v3d_main/basic_c_fun/stackutil.cpp \
		../../v3d_main/basic_c_fun/mg_utilities.cpp \
		../../v3d_main/basic_c_fun/mg_image_lib.cpp

LIBS         += -lm -lpthread
LIBS	     += -L../../v3d_main/common_lib/lib_mac64 -ltiff64

TARGET        = $$qtLibraryTarget(miviewer)
DESTDIR       = ../../v3d64/plugins/miviewer

