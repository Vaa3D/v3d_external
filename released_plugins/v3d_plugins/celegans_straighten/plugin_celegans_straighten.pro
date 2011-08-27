
TEMPLATE      = lib
CONFIG       += plugin 
CONFIG       += x86_64

INCLUDEPATH  += ../../v3d_main/basic_c_fun
INCLUDEPATH += ../../v3d_main/jba/newmat11

LIBS += -L../../v3d_main/common_lib/lib -lv3dtiff
LIBS += -L../../v3d_main/jba/c++ -lv3dnewmat

FORMS         = paradialog.ui

HEADERS      += ../../v3d_main/basic_c_fun/v3d_message.h
HEADERS      += ../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS	     += ../../v3d_main/basic_c_fun/stackutil.h
HEADERS      += ../../v3d_main/basic_c_fun/mg_image_lib.h
HEADERS      += ../../v3d_main/basic_c_fun/mg_utilities.h
HEADERS      += ../../v3d_main/jba/c++/convert_type2uint8.h
HEADERS      += ../../v3d_main/worm_straighten_c/spline_cubic.h
HEADERS      += q_morphology.h
HEADERS      += q_principalskeleton_detection.h
HEADERS      += q_celegans_straighten.h
HEADERS      += q_paradialog_straighten.h
HEADERS      += plugin_celegans_straighten.h

SOURCES      += ../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES      += ../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES      += ../../v3d_main/basic_c_fun/stackutil.cpp
SOURCES      += ../../v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES      += ../../v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES      += ../../v3d_main/worm_straighten_c/spline_cubic.cpp
SOURCES      += ../common/q_imresize.cpp
SOURCES      += q_morphology.cpp
SOURCES      += q_principalskeleton_detection.cpp
SOURCES      += q_celegans_straighten.cpp
SOURCES      += q_paradialog_straighten.cpp
SOURCES      += plugin_celegans_straighten.cpp

TARGET        = $$qtLibraryTarget(celegans_straighten)

DESTDIR       = ../../v3d_main/v3d/plugins/celegans_straighten/ #win32 qmake couldn't handle space in path


