# #####################################################################
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

HEADERS += ../webservice/v3dwebservice.hpp ../webservice/v3dwebserver.h ../webservice/gsoap2/stdsoap2.h ../webservice/soapdep/soapH.h ../webservice/soapdep/soapStub.h ../webservice/soapdep/soapv3dwebserverService.h
SOURCES += ../webservice/v3dwebservice.cpp ../webservice/gsoap2/stdsoap2.cpp ../webservice/soapdep/soapC.cpp ../webservice/soapdep/soapv3dwebserverService.cpp

INCLUDEPATH += ../webservice/gsoap2

}
