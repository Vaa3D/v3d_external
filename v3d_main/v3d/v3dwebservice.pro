# #####################################################################
# Release: 2011-05-18 v3dwebservice v0.08 by Yang Yu
# Release: 2011-05-11 v3dwebservice v0.07 by Yang Yu
# Release: 2011-05-06 v3dwebservice v0.06 by Yang Yu
# Release: 2011-05-04 v3dwebservice v0.05 by Yang Yu
# Release: 2011-05-03 v3dwebservice v0.04 by Yang Yu
# Last change: 2011-04-30, by Hanchuan Peng
# Release: 2011-04-29 v3dwebservice v0.03 by Yang Yu
# Release: 2011-04-27 v3dwebservice v0.02 by Yang Yu 
# Created: 2011-3-7 v3dwebservice v0.01 by Yang Yu
# ######################################################################

# release 0.08 avoid conflict with old version __v3dwebservice__ DEFINES
# release 0.07 add save file function
# release 0.06 adjust conditional compile for v3d webservice application
# release 0.05 rewrite all soap related functions in mainwindow_wsi.cpp, now modular architecture is concise and easy read
# release 0.04 add support to call all control 3d viewer functions
# release 0.03 add support to call plugin with specified method. Meanwhile, the inputs support incomplete name either plugin name or file name.
# release 0.02 add support to control 3d viewer functions, for example, set rotation positions, zoom, and shift.


message(CONFIG=$$unique(CONFIG))
macx {
		message(configure for MACX -arch x86_64)
		DEFINES += __MAC_x86_64__
	        CONFIG += x86_64
}

include(v3d.pro)
macx {
LIBS -= -L../common_lib/lib_mac32 -L./common_lib/lib_mac32
LIBS += -L../common_lib/lib_mac64 -L./common_lib/lib_mac64

WEBSERVICESRCFOLDER = ../webservice/src

message(configure for v3d webservice)
DEFINES += __V3DWSDEVELOP__

HEADERS -= ../webservice/v3dwebservice_conf.h
HEADERS -= ../webservice/v3dwebservice.hpp
HEADERS -= ../webservice/v3dwebserver.h
HEADERS -= ../webservice/gsoap2/stdsoap2.h
HEADERS -= ../webservice/soapdep/soapH.h 
HEADERS -= ../webservice/soapdep/soapStub.h
HEADERS -= ../webservice/soapdep/soapv3dwebserverService.h

SOURCES -=../webservice/v3dwebservice.cpp 
SOURCES -= ../webservice/gsoap2/stdsoap2.cpp 
SOURCES -= ../webservice/soapdep/soapC.cpp 
SOURCES -= ../webservice/soapdep/soapv3dwebserverService.cpp 

HEADERS += $$WEBSERVICESRCFOLDER/v3dwebservice.hpp \
	$$WEBSERVICESRCFOLDER/v3dwebserver.h \
	$$WEBSERVICESRCFOLDER/gsoap2/stdsoap2.h \ 
	$$WEBSERVICESRCFOLDER/soapdep/soapH.h \
	$$WEBSERVICESRCFOLDER/soapdep/soapStub.h \ 
	$$WEBSERVICESRCFOLDER/soapdep/soapv3dwebserverService.h

SOURCES += $$WEBSERVICESRCFOLDER/v3dwebservice.cpp \ 
	$$WEBSERVICESRCFOLDER/gsoap2/stdsoap2.cpp \ 
	$$WEBSERVICESRCFOLDER/soapdep/soapC.cpp \
	$$WEBSERVICESRCFOLDER/soapdep/soapv3dwebserverService.cpp
	
SOURCES += mainwindow_wsi.cpp

INCLUDEPATH += $$WEBSERVICESRCFOLDER/gsoap2

}
