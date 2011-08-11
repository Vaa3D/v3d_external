# by hang
# Aug-11-2011

TEMPLATE = app
CONFIG -= qt app_bundle
CONFIG += debug warn_off

INCLUDEPATH += ../../../v3d_main/basic_c_fun
INCLUDEPATH += ../../../v3d_main/common_lib/include

macx{
	LIBS += -L../../../v3d_main/common_lib/lib_mac64 -lv3dtiff
	CONFIG += x86_64
}

win32{
}

unix:!macx {
	LIBS += -L../../../v3d_main/common_lib/lib -ltiff
}


HEADERS += img_rotate.h
HEADERS += parser.h
HEADERS += gaussian_blur.h
HEADERS += img_threshold.h
HEADERS += v3d_funcs.h
HEADERS += ../../../v3d_main/basic_c_fun/basic_memory.h
HEADERS += ../../../v3d_main/basic_c_fun/stackutil.h
HEADERS += ../../../v3d_main/basic_c_fun/mg_utilities.h
HEADERS += ../../../v3d_main/basic_c_fun/mg_image_lib.h

SOURCES += gaussian_blur.cpp
SOURCES += img_threshold.cpp
SOURCES += img_rotate.cpp
SOURCES += main.cpp
SOURCES += ../../../v3d_main/basic_c_fun/basic_memory.cpp
SOURCES += ../../../v3d_main/basic_c_fun/stackutil.cpp
SOURCES += ../../../v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES += ../../../v3d_main/basic_c_fun/mg_image_lib.cpp
