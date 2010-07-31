
TEMPLATE      = lib
CONFIG       += plugin warning_off 
INCLUDEPATH  += ../../basic_c_fun
HEADERS     = wavelets.h scaleinfo.h	
				 
				
				
SOURCES     = wavelets.cpp scaleinfo.cpp
 			
			
TARGET        = $$qtLibraryTarget(wavelets)
DESTDIR       = ../../v3d/plugins/Wavelets #win32 qmake couldn't handle space in path

