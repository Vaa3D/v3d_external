# #####################################################################
# Created: 2011-11-08
# produced by Hanchuan Peng, 2011
# noted added by PHC: 2011-09-30: if people use MSVC 2010 (instead of MSVC 2008), maybe they can simply change
#                                 the MSVC_DIR and MSSDK_DIR folders to the correct ones
#                                 under the "c:\\Program Files". Also note that Yang indeed
#                                 redefine the LIBS variables, which explains why my previous
#                                 change of libv3dnewmat, etc does not play a role here any more.
# ######################################################################

message(CONFIG=$$unique(CONFIG))

CONFIG += warn_off  # should turn off later to view all warning during compilation
CONFIG += CONSOLE   # make a console application instead of a windows GUI only application

include(vaa3d.pro)

win32 {
    ## Windows common build here

	HEADERS += ../basic_c_fun/v3d_basicdatatype.h \
				../basic_c_fun/vcdiff.h

	SOURCES += ../basic_c_fun/vcdiff.cpp

	# re-define LIBS under windows os
	LIBS  = -L$$SHARED_FOLDER \ # for arthurwidgets
			-L$$SHARED_FOLDER/release # for Qt windows which only has release install(no debug)

	LIBS += -ldemo_shared
        message("x86_64 build")

        ## Windows x64 (64bit) specific build here

 		MSVC_DIR = "C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC"
		MSSDK_DIR = "C:\\Program Files (x86)\\Microsoft SDKs\\Windows\\v7.1A\\"
		LOCAL_DIR = ..\

		LIBS += -L$$LOCAL_DIR\\common_lib\\winlib64
		LIBS += -L$$MSVC_DIR\\lib\\amd64
		LIBS += -L$$MSSDK_DIR\\Lib\\x64


		LIBS += \
				-lvs12_libtiff \
				-lteem \
        -lvs12_newmat11 \
				-llibjba \
				-llibFL_cellseg \
				-llibFL_brainseg
        DEFINES *= TEEM_STATIC
        QMAKE_CXXFLAGS += -DTEEMSTATIC

		INCLUDEPATH += $$LOCAL_DIR\\basic_c_fun\\include \
		               $$LOCAL_DIR\\common_lib\\include

    INCLUDEPATH = $$unique(INCLUDEPATH)
    LIBS = $$unique(LIBS)
}


