
TEMPLATE      = lib
CONFIG       += plugin warn_off
INCLUDEPATH  += /System/Library/Frameworks/JavaVM.framework/Headers \
                ../../basic_c_fun \
                ../bioformat_lib/include
LIBS          = -framework JavaVM \
		-L/usr/local/lib/ -lboost_thread \
		../bioformat_lib/lib/libbfcpp.dylib \
		../bioformat_lib/lib/libjace.dylib

HEADERS       = bioFormat_ex.h
SOURCES       = bioFormat_ex.cpp

TARGET        = $$qtLibraryTarget(readImage_bioformat)
QMAKE_POST_LINK = cp ../bioformat_lib/lib/* ../../v3d;

DESTDIR       = ../../v3d/plugins/ReadImage_bioformat

