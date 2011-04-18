# #####################################################################
# Created: 2011-3-7 v3dwebservice v0.1 by Yang Yu
# ######################################################################

soapdep.target = soapdep
soapdep.commands = soapcpp2 -2 -S -i -L ../webservice/v3dwebserver.h
QMAKE_EXTRA_TARGETS += soapdep

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

HEADERS += ../webservice/v3dwebservice.hpp ../webservice/v3dwebserver.h ../webservice/gsoap2/stdsoap2.h soapH.h soapStub.h soapv3dwebserverService.h
SOURCES += ../webservice/v3dwebservice.cpp ../webservice/gsoap2/stdsoap2.cpp soapC.cpp soapv3dwebserverService.cpp

INCLUDEPATH += ../webservice/gsoap2

}
