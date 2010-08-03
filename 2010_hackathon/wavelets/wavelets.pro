#if 1
# nicolas's version
TEMPLATE      = lib
CONFIG       += plugin warning_off 
INCLUDEPATH  += /Users/nicolas/prog/v3d/my_v3d_plugins/basic_c_fun
HEADERS     = wavelets.h scaleinfo.h ioV3dUtils.h waveletConfigException.h waveletTransform.h		 
SOURCES     = wavelets.cpp scaleinfo.cpp ioV3dUtils.cpp waveletConfigException.cpp waveletTransform.cpp
LIBS          += -lfftw3 			
			
TARGET        = $$qtLibraryTarget(wavelets)
DESTDIR       = .

#endif

#if 0

# fab's version

TEMPLATE      = lib
CONFIG       += plugin warning_off 
INCLUDEPATH  += ../../basic_c_fun
HEADERS     = wavelets.h scaleinfo.h ioV3dUtils.h waveletConfigException.h waveletTransform.h	
SOURCES     = wavelets.cpp scaleinfo.cpp ioV3dUtils.cpp waveletConfigException.cpp waveletTransform.cpp
TARGET        = $$qtLibraryTarget(wavelets)
DESTDIR       = ../../v3d/plugins/Wavelets
#endif
