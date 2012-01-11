
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += release x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

HEADERS      += imageblend.h
HEADERS      +=	$$V3DMAINDIR/basic_c_fun/stackutil.h
HEADERS      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/imageio_mylib.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_4dimage.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/v3d_message.h

SOURCES      += imageblend.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/imageio_mylib.cpp
#SOURCES      += $$V3DMAINDIR/basic_c_fun/basic_4dimage.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
SOURCES      += basic_4dimage_ori.cpp

LIBS         += -lm -lpthread
LIBS         += -L$$V3DMAINDIR/common_lib/lib/libv3dtiff.a
LIBS         += -L$$V3DMAINDIR/common_lib/lib/libfftw3f.a
LIBS         += -L$$V3DMAINDIR/common_lib/lib/libfftw3f_threads.a 
LIBS         += -L$$V3DMAINDIR/common_lib/src_packages/mylib_tiff/libmylib.a

TARGET        = $$qtLibraryTarget(blend_multiscanstacks)
DESTDIR       = ../../v3d/plugins/image_blending/blend_multiscanstacks

