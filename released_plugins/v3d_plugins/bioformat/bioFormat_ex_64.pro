
TEMPLATE	= lib
CONFIG		+= plugin warn_off
CONFIG		+= x86_64

INCLUDEPATH	+= /System/Library/Frameworks/JavaVM.framework/Headers \
               ../../../v3d_main/basic_c_fun \
                ../bioformat_lib/include
LIBS		 = -framework JavaVM \
		-L/usr/local/lib/ -lboost_thread \
		../bioformat_lib/lib64/libbfcpp.dylib \
		../bioformat_lib/lib64/libjace.dylib

HEADERS       = bioFormat_ex.h
SOURCES       = bioFormat_ex.cpp

TARGET        = $$qtLibraryTarget(readImage_bioformat)
QMAKE_POST_LINK = cp ../bioformat_lib/lib64/* ../../v3d;

DESTDIR       =  ../../../v3d_main/v3d/plugins/ImageIO_bioformat

