# #####################################################################
# Created: 2010-6-17 v3d_msvc.pro under windows v0.1 by YUY
# revised by PHC, 2011-08-11
# ######################################################################

message(CONFIG=$$unique(CONFIG))

CONFIG += warn_off  # should turn off later to view all warning during compilation
CONFIG += CONSOLE   # make a console application instead of a windows GUI only application

include(v3d.pro)

win32 { 
    ## Windows common build here 
	
	HEADERS += ../basic_c_fun/v3d_basicdatatype.h \ 
				../basic_c_fun/vcdiff.h
				
	SOURCES += ../basic_c_fun/vcdiff.cpp
	
	# re-define LIBS under windows os
	LIBS  = -L$$SHARED_FOLDER \ # for arthurwidgets
			-L$$SHARED_FOLDER/release # for Qt windows which only has release install(no debug)
		   
	LIBS += -ldemo_shared
 
    !contains(QMAKE_HOST.arch, x86_64) { 
        message("x86 build") 
 
        ## Windows x86 (32bit) specific build here 
        
        MSVC_DIR = "C:\\Program Files\\Microsoft Visual Studio 9.0\\VC"
		MSSDK_DIR = "C:\\Program Files\\Microsoft SDKs\\Windows\\v6.0A\\"
		LOCAL_DIR = ..\ 

		LIBS += -L$$MSVC_DIR\\lib
		LIBS += -L$$MSSDK_DIR\\Lib
		LIBS += -L$$LOCAL_DIR\\common_lib\\winlib
		
		LIBS += \ #-lm
				-llibtiff \
				-llibnewmat \ #libnewmat.lib also works, 2010-05-21. PHC
				-llibjba \    #libjba.lib also works
				-llibFL_cellseg \
				-llibFL_brainseg 
	
		INCLUDEPATH += $$LOCAL_DIR\\basic_c_fun\\include \
		               $$LOCAL_DIR\\common_lib\\include
 
    } else { 
        message("x86_64 build") 
 
        ## Windows x64 (64bit) specific build here 
 	
 		MSVC_DIR = "C:\\Program Files\\Microsoft Visual Studio 9.0\\VC"
		MSSDK_DIR = "C:\\Program Files\\Microsoft SDKs\\Windows\\v6.0A\\"
		LOCAL_DIR = ..\ 

		LIBS += -L$$MSVC_DIR\\lib	
		LIBS += -L$$MSSDK_DIR\\Lib\\x64
		LIBS += -L$$LOCAL_DIR\\common_lib\\winlib64
		
		LIBS += \
				-llibtiff \
				-llibnewmat \ 
				-llibjba \ 
				-llibFL_cellseg \
				-llibFL_brainseg 
	
		INCLUDEPATH += $$LOCAL_DIR\\basic_c_fun\\include \
		               $$LOCAL_DIR\\common_lib\\include
    } 
    
    INCLUDEPATH = $$unique(INCLUDEPATH)
    LIBS = $$unique(LIBS)
}


