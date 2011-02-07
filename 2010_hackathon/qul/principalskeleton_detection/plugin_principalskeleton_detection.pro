
TEMPLATE      = lib
CONFIG       += plugin 

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/jba/newmat11
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

LIBS += -L$$V3DMAINDIR/jba/c++ -lnewmat

HEADERS      += $$V3DMAINDIR/basic_c_fun/v3d_message.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.h
HEADERS      += $$V3DMAINDIR/worm_straighten_c/spline_cubic.h
HEADERS      += q_morphology.h
HEADERS      += q_principalskeleton_detection.h
HEADERS      += plugin_principalskeleton_detection.h

SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINDIR/worm_straighten_c/spline_cubic.cpp
SOURCES      += q_morphology.cpp
SOURCES      += q_principalskeleton_detection.cpp
SOURCES      += plugin_principalskeleton_detection.cpp

TARGET        = $$qtLibraryTarget(principalskeleton_detection)

DESTDIR       = $$V3DMAINDIR/../v3d/plugins/unfinished/principalskeleton_detection #win32 qmake couldn't handle space in path

