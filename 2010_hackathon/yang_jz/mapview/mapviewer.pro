
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
CONFIG       += x86_64
CONFIG       +=release

#include(/usr/local/Trolltech/Qt-4.5.2/demos/shared/shared.pri)

INCLUDEPATH  += ../../../v3d_main/basic_c_fun
INCLUDEPATH  += ../../../v3d_main/common_lib/include
INCLUDEPATH  += /usr/local/Trolltech/Qt-4.5.2/demos/shared/
#HEADERS      +=/usr/local/Trolltech/Qt-4.5.2/demos/shared/arthurwidgets.h
HEADERS      +=colormap.h

HEADERS      += mg_image_lib11.h
HEADERS      += stackutil-11.h
HEADERS      += y_imglib.h
HEADERS      += mapviewer.h \
#		../../../v3d_main/basic_c_fun/stackutil.h \
		../../../v3d_main/basic_c_fun/mg_utilities.h \
#		../../../v3d_main/basic_c_fun/mg_image_lib.h 

SOURCES       += mg_image_lib11.cpp 
SOURCES       += stackutil-11.cpp
SOURCES       += mapviewer.cpp \
#		../../../v3d_main/basic_c_fun/stackutil.cpp \
		../../../v3d_main/basic_c_fun/mg_utilities.cpp \
#		../../../v3d_main/basic_c_fun/mg_image_lib.cpp

LIBS     += -lm -lpthread
LIBS	    += -L../../../v3d_main/common_lib/lib -lv3dtiff
        += -L/usr/local/Trolltech/Qt-4.5.2/demos/shared/
#LIBS += -framework CoreServices
TARGET        = $$qtLibraryTarget(mapviewer)
DESTDIR       = ../../../v3d/plugins/mapviewer
