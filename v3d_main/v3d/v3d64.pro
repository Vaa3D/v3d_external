# #####################################################################
# Created: 2009-7-6 v3d64 v0.1 
# ######################################################################

message(CONFIG=$$unique(CONFIG))
macx { #090705 RZC
	#contains(CONFIG, "ARCH64") #it seems DONOT work, 090702 RZC
	#options = $$find(CONFIG, "ARCH64") #it DOES work sometimes, 090702 by YY, 090705 RZC tested 
	#count(options, 1) {
		message(configure for MACX -arch x86_64)
		DEFINES += __MAC_x86_64__
        #QMAKE_CC += -arch x86_64        #there isn't QMAKE_CFLAGS or QMAKE_CCFLAGS
        #QMAKE_CXXFLAGS += -arch x86_64
        #QMAKE_LFLAGS += -arch x86_64
        CONFIG += x86_64
}

include(v3d.pro)
macx {
LIBS -= -L../common_lib/lib_mac32  #100811: -lv3dtiff for both 32/64-bit
LIBS += -L../common_lib/lib_mac64 
#LIBS += -L/Users/pengh/work/v3d_external/v3d_main/jba/c++/ -lv3dnewmat
}

# 090713 RZC cp tiff64/libtiff.a to libtiff64.a in /usr/local/lib
#LIBS -= -ltiff32  #added on 090901
#LIBS += -ltiff64

# 090731 PHC cp /usrlocal/lib/libgsl.a to libgsl64.a in /usr/local/lib
#LIBS -= -lgsl32   #added on 090901
#LIBS += -lgsl64

