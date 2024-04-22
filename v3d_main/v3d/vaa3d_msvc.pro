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
CONFIG += warn_on console
#CONFIG += debug
#CONFIG += debug_and_release_target

QMAKE_CXXFLAGS += /MD /Zi

QMAKE_CXXFLAGS += /MP
QMAKE_LFLAGS   += /STACK:104857600

include(vaa3d.pro)

win32 { 
    ## Windows common build here 
	
	HEADERS += ../basic_c_fun/v3d_basicdatatype.h \ 
				../basic_c_fun/vcdiff.h
				
	SOURCES += ../basic_c_fun/vcdiff.cpp
	
	# re-define LIBS under windows os
        LIBS  = -L../common_lib
	LIBS += -ldemo_shared
 
        message("x86_64 build") 
 
        ## Windows x64 (64bit) specific build here 
 	
 		MSVC_DIR = "C:\\Program Files\\Microsoft Visual Studio 9.0\\VC"
		MSSDK_DIR = "C:\\Program Files\\Microsoft SDKs\\Windows\\v6.0A\\"
		LOCAL_DIR = ..\ 

		LIBS += -L$$MSVC_DIR\\lib	
		LIBS += -L$$MSSDK_DIR\\Lib\\x64
		LIBS += -L$$LOCAL_DIR\\common_lib\\winlib64

		MSVCVERSION = $$(QMAKESPEC)
		BOOSTPATH = $$(BOOST_PATH)
                LIBS += -L$$BOOSTPATH\\lib64-msvc-12.0
		equals(MSVCVERSION, "win32-msvc2013") {
			LIBS += -L$$BOOSTPATH\\lib64-msvc-12.0
		}
		equals(MSVCVERSION, "win32-msvc2010") {
			LIBS += -L$$BOOSTPATH\\lib64-msvc-10.0
		}

		exists(..\\..\\..\\vaa3d_tools\\hackathon\\MK\\v3d_imgManagerMK\\v3d_imgManagerMK.lib) {
			message("Fragment tracing essential libraries are built. v3d_imgManagerMK.lib will be included.")
			
			INCLUDEPATH += ..\\..\\..\\vaa3d_tools\\hackathon\\MK\\v3d_imgManagerMK \
						   ..\\..\\..\\vaa3d_tools\\hackathon\\MK\\v3d_imgManagerMK\\imgAnalyzer \
					       ..\\..\\..\\vaa3d_tools\\hackathon\\MK\\v3d_imgManagerMK\\imgProcessor 
			
			LIBS += -L..\\..\\..\\vaa3d_tools\\hackathon\\MK\\v3d_imgManagerMK
			LIBS += -lv3d_imgManagerMK
		}

		exists(..\\..\\..\\vaa3d_tools\\hackathon\\MK\\NeuronStructNavigator\\NeuronStructNavigator.lib) {
			message("Fragment tracing essential libraries are built. NeuronStructNavigator.lib will be included.")
			
			INCLUDEPATH += ..\\..\\..\\vaa3d_tools\\hackathon\\MK\\NeuronStructNavigator

			LIBS += -L..\\..\\..\\vaa3d_tools\\hackathon\\MK\\NeuronStructNavigator
			LIBS += -lNeuronStructNavigator
		}
				
		LIBS += \
				-llibtiff \
				-lteem \
				-llibjba \ 
				-llibFL_cellseg \
                                -llibFL_brainseg \
                                -lopenvr_api \
                                -lglew32 \
                                -lSDL2 \
                                -lSDL2main \
                                -llibhdf5 \
                                -llibszip \
                                -llibzlib \
                                -lqjson

CONFIG(release, debug|release): LIBS += -L..\\common_lib
CONFIG(debug, debug|release): LIBS += -L..\\common_lib

CONFIG(release, debug|release): LIBS += -lnewmat11d
CONFIG(debug, debug|release): LIBS += -lnewmat11d

        DEFINES *= TEEM_STATIC
        QMAKE_CXXFLAGS += -DTEEMSTATIC
	
		INCLUDEPATH += $$LOCAL_DIR\\basic_c_fun\\include \
		               $$LOCAL_DIR\\common_lib\\include \
					   $$BOOSTPATH \ 

    
    INCLUDEPATH = $$unique(INCLUDEPATH)
    BOOSTPATH = $$(BOOST_PATH)
    LIBS = $$unique(LIBS)
}


win32:CONFIG(release, debug|release): LIBS += -LC:/local/boost_1_61_0/lib64-msvc-12.0/ -llibboost_regex-vc120-mt-1_61
else:win32:CONFIG(debug, debug|release): LIBS += -LC:/local/boost_1_61_0/lib64-msvc-12.0/ -llibboost_regex-vc120-mt-1_61d
else:unix: LIBS += -LC:/local/boost_1_61_0/lib64-msvc-12.0/ -llibboost_regex-vc120-mt-1_61

INCLUDEPATH += C:/local/boost_1_61_0/lib64-msvc-12.0
DEPENDPATH += C:/local/boost_1_61_0/lib64-msvc-12.0

QT += widgets
