# #####################################################################
# Created: 2009-7-6 v3d64 v0.1 
# Last change: 2014-09-21
# ######################################################################
QMAKE_CXXFLAGS +=-std=c++0x
message(CONFIG=$$unique(CONFIG))
macx {
        message(configure for MACX -arch x86_64)
        DEFINES += __MAC_x86_64__
        CONFIG += x86_64
}

include(v3d.pro)

macx {
    #QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++ -Wno-c++11-narrowing
    #LIBS += -lc++
    LIBS -= -L../common_lib/lib_mac32  #100811: -lv3dtiff for both 32/64-bit
    LIBS += -L../common_lib/lib_mac64
}
