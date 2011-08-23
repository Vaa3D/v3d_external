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


HEADERS += parser.h
HEADERS += img_rotate.h
HEADERS += gaussian_blur.h
HEADERS += img_threshold.h
HEADERS += img_center.h
HEADERS += img_sampling.h
HEADERS += img_segment.h
HEADERS += img_operate.h
HEADERS += sort_algorithms.h
HEADERS += fastmarching.h
HEADERS += dist_transform.h
HEADERS += FL_bwdist.h
HEADERS += ../../../v3d_main/basic_c_fun/basic_memory.h
HEADERS += ../../../v3d_main/basic_c_fun/stackutil.h
HEADERS += ../../../v3d_main/basic_c_fun/mg_utilities.h
HEADERS += ../../../v3d_main/basic_c_fun/mg_image_lib.h

SOURCES += img_rotate.cpp
SOURCES += img_threshold.cpp
SOURCES += img_center.cpp
SOURCES += img_sampling.cpp
SOURCES += img_segment.cpp
SOURCES += img_operate.cpp
SOURCES += sort_algorithms.cpp
SOURCES += fastmarching.cpp
SOURCES += main.cpp
SOURCES += ../../../v3d_main/basic_c_fun/basic_memory.cpp
SOURCES += ../../../v3d_main/basic_c_fun/stackutil.cpp
SOURCES += ../../../v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES += ../../../v3d_main/basic_c_fun/mg_image_lib.cpp
