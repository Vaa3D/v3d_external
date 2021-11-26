#plugin_image_registration.h
# by Lei Qu and Hanchuan Peng
# 2011

TEMPLATE      = lib
CONFIG       += plugin 
#CONFIG       += x86_64

V3DPATH = ../../../v3d_main

INCLUDEPATH  += $$V3DPATH/basic_c_fun
INCLUDEPATH  += $$V3DPATH/jba/newmat11

LIBS 	     += -L$$V3DPATH/common_lib/lib -lv3dtiff
LIBS         += -L$$V3DPATH/jba/c++ -lv3dnewmat

FORMS         = paradialog_rigidaffine.ui

HEADERS      += $$V3DPATH/basic_c_fun/v3d_message.h
HEADERS	     += $$V3DPATH/basic_c_fun/stackutil.h
HEADERS      += $$V3DPATH/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DPATH/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DPATH/basic_c_fun/basic_memory.h
HEADERS      += $$V3DPATH/basic_c_fun/basic_memory.cpp
HEADERS      += $$V3DPATH/basic_c_fun/v3d_interface.h
HEADERS      += common/q_convolve.h
HEADERS      += common/q_imresize.h
HEADERS      += histogram_matching/q_histogram_matching.h
HEADERS      += q_paradialog_rigidaffine.h
HEADERS      += q_registration_common.h
HEADERS      += q_rigidaffine_registration.h
HEADERS      += plugin_image_registration.h

SOURCES      += common/q_imresize.cpp
SOURCES      += common/q_convolve.cpp
SOURCES      += $$V3DPATH/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DPATH/basic_c_fun/stackutil.cpp
SOURCES      += $$V3DPATH/basic_c_fun/mg_image_lib.cpp
SOURCES      += $$V3DPATH/basic_c_fun/mg_utilities.cpp
SOURCES      += histogram_matching/q_histogram_matching.cpp
SOURCES      += q_paradialog_rigidaffine.cpp
SOURCES      += q_registration_common.cpp
SOURCES      += q_rigidaffine_registration.cpp
SOURCES      += plugin_image_registration.cpp

TARGET        = $$qtLibraryTarget(plugin_PQ_imagereg)

DESTDIR       = ../../v3d/plugins/optic_lobe_aligner/ 

