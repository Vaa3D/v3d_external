# #####################################################################
# Last change: 2011-04-30, by Hanchuan Peng
# Release: 2011-04-29 v3dwebservice v0.03 by Yang Yu
# Release: 2011-04-27 v3dwebservice v0.02 by Yang Yu 
# Created: 2011-3-7 v3dwebservice v0.01 by Yang Yu
# ######################################################################

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

INCLUDEPATH += $$WEBSERVICESRCFOLDER/gsoap2

}
