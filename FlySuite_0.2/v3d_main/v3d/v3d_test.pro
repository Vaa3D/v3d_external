# #####################################################################
# Updated: 060924 v3d v0.12
# Updated: 080131: change from Qt 4.1.4 to 4.3.3 and then back to 4.1.4, as 4.3.3 has a crash on the program exit. sent bug report to Qt already.
# Updated: 080302. Use the cpp version of mg_image and mg_utilities. Also include the basic_memory functions
# Updated: 080314. Add the straightening codes.
# Updated: 080318. Add -lnewmat
# Updated: 080321. Add rotate_image.cpp
# Updated: 080323. Add dialog_rotate.xxx
# Updated: 080326. Add laff
# Updated: 080401: Add img_definition.h
# Updated: 080403: Add spline_cubic.h and .cpp
# Updated: 080411: add thres_regist.h
# Updated: 080414: add volimg_proc.h
# Updated: 080422: try to add Zhao Ting's corr functions
# 080429: try Qt 4.3.4
# 080611: add histeq.h
# 080720: qt 4.4.0
# 080723: add v3d_compile_constraints.h
# 080730: add compiling option to specify if display warning etc
# 080806: add viz codes
# 080817: change laff to jba
# 080818: change JBA dir structure
# 080822: add the lobeseg function
# 080823: add the cellseg functions
# 080824: add colormap
# ######################################################################
TEMPLATE = app
TARGET += 
DEPENDPATH += .
INCLUDEPATH += .

# for win32, by RZC 080916, 080921, 080923
# QT_DIR = /usr/local/Trolltech/Qt-4.1.4
# QT_DIR = /usr/local/Trolltech/Qt-4.4.0
# win32:QT_DIR = c:/qt/4.4.0
QT_DIR = $$dirname(QMAKE_QMAKE)/.. 
LOCAL_DIR = /usr/local # platform: macx, unix, win32-msys-mingw
WINGW_DIR = /mingw # platform: unix, win32-msys-mingw
win32 { # platform: win32-mingw
	WINGW_DIR = c:/mingw
	LOCAL_DIR = c:/msys/local
	CONFIG = $$unique(CONFIG)
	CONFIG -= debug # for Qt-win32 which only has release install(no debug)
}
!macx:LOCAL_DIR = ../common_lib # copy from /usr/local

SHARED_FOLDER = $$QT_DIR/demos/shared # for arthurwidgets
include($$SHARED_FOLDER/shared.pri)
INCLUDEPATH += $$SHARED_FOLDER
LIBS += -L$$SHARED_FOLDER \
	-L$$SHARED_FOLDER/release # for Qt-win32 which only has release install(no debug)
macx {  # for Qt-4.1.4 linked name of Qt-4.4.0 on macx
	INCLUDEPATH += /usr/local/Trolltech/Qt-4.4.0/demos/shared/
	LIBS += -L/usr/local/Trolltech/Qt-4.4.0/demos/shared/
}

INCLUDEPATH += $$LOCAL_DIR/include \
	../neurolabi/lib/myers.pack/ \
	../neurolabi/c/include/
LIBS += -L$$LOCAL_DIR/lib \
	-L$$WINGW_DIR/lib
CONFIG += qtestlib # for preview movie animation, by RZC

# the following trick was figured out by Ruan Zongcai
# CONFIG += release ### disable most of warnings
CONFIG += warn_off # ## only work for complier

# Gene's suggestion to remove annoying QT ld warnings
# QMAKE_CXXFLAGS += -fvisibility=hidden # unrecognized command line option on mingw
# Input
HEADERS += ../basic_c_fun/mg_utilities.h \
    ../basic_c_fun/mg_image_lib.h \
    ../basic_c_fun/basic_memory.h \
    ../basic_c_fun/stackutil.h \
    ../basic_c_fun/img_definition.h \
    ../basic_c_fun/volimg_proc_declare.h \
    ../basic_c_fun/volimg_proc.h \
    ../worm_straighten_c/bdb_minus.h \
    ../worm_straighten_c/mst_prim_c.h \
    ../worm_straighten_c/bfs.h \
    ../worm_straighten_c/spline_cubic.h \
    ../lobeseg/lobeseg.h \
    ../jba/c++/jba_mainfunc.h \
    ../jba/c++/wkernel.h \
    ../jba/c++/histeq.h \
    ../cellseg/FL_cellSegmentation3D.h \
    ../neurolabi/c/tz_dimage_lib.h \
    ../3drenderer/v3dr_mainwindow.h \
    ../3drenderer/v3dr_glwidget.h \
    ../3drenderer/GLee_r.h \
    ../3drenderer/renderer.h \
    ../3drenderer/renderer_tex2.h \
    v3d_compile_constraints.h \
    colormap.h \
    rotate_image.h \
    dialog_rotate.h \
    opt_rotate.h \
    thread_regist.h \
    v3d_core.h \
    mainwindow.h

# glwidget.h \
# idrawmain.h \
# HEADERS += mdichild.h
SOURCES += ../basic_c_fun/mg_utilities.cpp \
    ../basic_c_fun/mg_image_lib.cpp \
    ../basic_c_fun/stackutil.cpp \
    ../basic_c_fun/basic_memory.cpp \
    ../worm_straighten_c/bdb_minus.cpp \
    ../worm_straighten_c/mst_prim_c.cpp \
    ../worm_straighten_c/bfs_1root.cpp \
    ../worm_straighten_c/spline_cubic.cpp \
    ../lobeseg/lobeseg.cpp \
    ../jba/c++/jba_mainfunc.cpp \
    ../jba/c++/wkernel.cpp \
    ../jba/c++/histeq.cpp \
    ../3drenderer/v3dr_mainwindow.cpp \
    ../3drenderer/v3dr_glwidget.cpp \
    ../3drenderer/GLee_r.c \
    ../3drenderer/renderer.cpp \
    ../3drenderer/renderer_tex2.cpp \
    ../3drenderer/renderer_obj2.cpp \
    ../3drenderer/renderer_labelfield.cpp \
    main.cpp \
    mainwindow.cpp \
    v3d_core.cpp \
    colormap.cpp \
    rotate_image.cpp \
    dialog_rotate.cpp

# ../neurolabi/c/tz_dimage_lib.c
# glwidget.cpp idrawmain.cpp \
# SOURCES += mdichild.cpp
RESOURCES += v3d.qrc
QT += opengl
macx:LIBS += -lm -ltiff \
    -L../jba/c++ \
    -lnewmat \
    -L../cellseg \
    -lFL_cellseg \
    -lfftw3 \
    -lfftw3f \
    -lgsl \
    -L../neurolabi/c/lib \
    -lneurolabi \
    -L../neurolabi/lib/myers.pack \
    -lmyers \
    -framework GLUT
!macx:LIBS += -lm -ltiff \
    -L../jba/c++ \
    -lnewmat \
    -L../cellseg \
    -lFL_cellseg \
    -lglut32 # platform: win32-mingw, on unix link libglut32.a to libglut.a


INCLUDEPATH = $$unique(INCLUDEPATH)
LIBS = $$unique(LIBS)
# CONFIG = $$unique(CONFIG) # this only NOT work on macx, very strange, by RZC 20080923
