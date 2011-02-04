
TEMPLATE      = lib
CONFIG       += plugin 

INCLUDEPATH  += ../../v3d_main/basic_c_fun
INCLUDEPATH += ../../v3d_main/jba/newmat11

LIBS += -L../../v3d_main/jba/c++ -lnewmat

HEADERS      += ../../v3d_main/basic_c_fun/v3d_message.h
HEADERS      += ../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS      += ../../v3d_main/worm_straighten_c/spline_cubic.h
HEADERS      += q_morphology.h
HEADERS      += q_principalskeleton_detection.h
HEADERS      += plugin_principalskeleton_detection.h

SOURCES      += ../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES      += ../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES      += ../../v3d_main/worm_straighten_c/spline_cubic.cpp
SOURCES      += q_morphology.cpp
SOURCES      += q_principalskeleton_detection.cpp
SOURCES      += plugin_principalskeleton_detection.cpp

TARGET        = $$qtLibraryTarget(principalskeleton_detection)

DESTDIR       = ../../../trunk/released_plugins/v3d/plugins/unfinished/principalskeleton_detection #win32 qmake couldn't handle space in path

