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
# 081016: add load ano file
# 081111: add convert_type2uint8.cpp
# 081115: add v3d_version_info.h/cpp
# 081123: add autotab.h, atlas_viewer.h/cpp
# 081127: add jba_match_landmarks.cpp and remove laff_features.cpp
# 081207: add my4dimage.cpp
# 081208: add landmark property dialog files
# 090108: add GMM fit
# 090118: add curve_trace_dialog.ui
# 090121: add the brainseg deformable model
# 090206: add v_neuronswc.cpp
# 090208: add dij_bgl.cpp
# 090219: add surfaceobj dialog and ui
# 090220: add surfaceobj_geometry_dialog.ui
# 090306: add some neuron basic operation files
# 090310: add the neuron_sim_score.h/cpp
# 090318: add jba_affine_xform.h/cpp
# 090426: add v3d_global_preference.ui
# 090426: add template_matching_seg.h/cpp
# 090428: add template_matching_cellseg.ui
# 090428: add template_matching_cellseg_dialog.h
# 090501: add v3d_global_preference_dialog.h
# 090507: add import image list dialog.ui and .h
# 090512: add dijkstra algorithm
# 090516: add neuron_tracing subproject and head files, move file locations
# 090516: add v3d_message 
# 090516: add filelist importer and related h/cpp files
# 090517: add linker loader for APO files and also rename them with vano_prefix
# 090517: add point cloud linker loader and related h/c++ files
# 090518: add point cloud atlas h/cpp io functions
# 090519: add compute_win_pca.h
# 090523: add basic_surf_objs.cpp
# 090605: move load_ano_file.h/cpp to basic_c_fun, merge the o_objects.h with basic_surf_objs.h
# 090605: merge load_ano_file.h into basic_surf_objs.h, and change name of load_ano_file.cpp to io_ano_file.cpp
# 090609: update the pointcloud_atlas_io.h/cpp location
# 090629: modified for macx to make free either in ../ or in ../v3d/, it means you can copy it to ../ then qmake it from ../     
# 090705: add ../neuron_editing/apo_xform.h/cpp
# 090710: add the kill_wrongmatchcode
# 090718: add histogramsimple.hcpp
# 090812: add v3d plugins
# 090819: separate the import_tiff_series.cpp from v3d_core.cpp
# 091114: add v3dimg_proc_neuron.cpp
# 100119: remove     ../basic_c_fun/io_ano_file.cpp 
# 100601: add v3d_global_proference.h
# 100801: separate module_flseg.cpp
# 100801: separate module_jba.cpp
# 100801: separate module_bdb.cpp
# 100802: remove the three modules
# 100807: remove some redundant libs
# 100810: add xformwidget.h extracted from v3d_core.h
# ######################################################################

TEMPLATE = app
TARGET += 
DEPENDPATH += . v3d
INCLUDEPATH += . 

# cross-OS-platform, cross-Qt-version
QT_DIR = $$dirname(QMAKE_QMAKE)/..  # cross-Qt-version
LOCAL_DIR = ../common_lib/ 				# unix-liked platform: macx, unix, win32-msys-mingw

MINGW_DIR = /mingw # platform: win32-msys-mingw
win32 { # platform: win32-command-mingw
	MINGW_DIR = c:/mingw
	LOCAL_DIR = ../common_lib/      # c:/msys/local
	CONFIG = $$unique(CONFIG)
	CONFIG -= debug # for Qt-win32 which only has release install(no debug)
	CONFIG += console
LIBS += -L$$MINGW_DIR/lib \
	-L$$LOCAL_DIR/lib_win32
}


INCLUDEPATH += $$LOCAL_DIR/include #./basic_c_fun
LIBS += -L$$LOCAL_DIR/lib


SHARED_FOLDER = $$QT_DIR/demos/shared # for arthurwidgets
include($$SHARED_FOLDER/shared.pri)
INCLUDEPATH += $$SHARED_FOLDER
LIBS += -L$$SHARED_FOLDER \
	-L$$SHARED_FOLDER/release # for Qt-win32 which only has release install(no debug)


# the following trick was figured out by Ruan Zongcai
CONFIG += warn_off  # only work for complier
CONFIG += debug  # Fabrice and Luis tracking a bug
#macx: QMAKE_LFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden # turn off this type warnings
#macx: QMAKE_CXXFLAGS += -gdwarf-2 # turn off visibility warnings
# need Qt 4.5.0 above and reCreate Makefile, this will be automatic.

# Input
HEADERS += ../basic_c_fun/mg_utilities.h \
    ../basic_c_fun/mg_image_lib.h \
    ../basic_c_fun/basic_memory.h \
    ../basic_c_fun/stackutil.h \
    ../basic_c_fun/img_definition.h \
    ../basic_c_fun/volimg_proc_declare.h \
    ../basic_c_fun/volimg_proc.h \
    ../basic_c_fun/v3d_message.h \
    ../basic_c_fun/color_xyz.h \
    ../basic_c_fun/basic_surf_objs.h \
    ../basic_c_fun/basic_4dimage.h \
    ../basic_c_fun/basic_landmark.h \
    ../basic_c_fun/v3d_interface.h \ 
    ../basic_c_fun/v3d_global_preference.h \
    ../plugin_loader/pluginDialog.h \
    ../plugin_loader/v3d_plugin_loader.h \
    ../graph/graph.h \
    ../graph/graph_basic.h \
    ../graph/dijk.h \
    ../gmm/fit_gmm.h \
    ../neuron_editing/apo_xforms.h \
    ../neuron_editing/neuron_xforms.h \
    ../neuron_editing/neuron_sim_scores.h \
    ../neuron_editing/v_neuronswc.h \
	../neuron_editing/neuron_format_converter.h \
    ../neuron_tracing/neuron_tracing.h \
    ../3drenderer/v3dr_mainwindow.h \
    ../3drenderer/v3dr_glwidget.h \
    ../3drenderer/qtr_widget.h \
    ../3drenderer/GLee_r.h \
    ../3drenderer/renderer.h \
    ../3drenderer/renderer_tex2.h \
    ../3drenderer/v3dr_surfaceDialog.h \
    ../3drenderer/ItemEditor.h \
    ../3drenderer/Renderer_gl2.h \
    ../3drenderer/v3dr_colormapDialog.h \
    ../3drenderer/gradients.h \
    ../3drenderer/hoverpoints.h \
    ../3drenderer/barFigureDialog.h \
    v3d_global_preference_dialog.h \
    v3d_compile_constraints.h \
    v3d_version_info.h \
    colormap.h \
    rotate_image.h \
    dialog_rotate.h \
    dialog_curve_trace_para.h \
    template_matching_cellseg_dialog.h \
    opt_rotate.h \
    landmark_property_dialog.h \
    dialog_keypoint_features.h \
	histogramsimple.h \
    compute_win_pca.h \
    compute_win_diff.h \
    surfaceobj_annotation_dialog.h \
    surfaceobj_geometry_dialog.h \
    import_filelist_dialog.h \
    import_images_tool_dialog.h \
    vano_linker_loader_dialog.h \
    dialog_imagecrop_bbox.h \
    dialog_imageresample.h \
    dialog_maskroi.h \
    atlas_viewer.h \
    v3d_core.h \
    xformwidget.h \
    mainwindow.h \
    dialog_watershed_para.h \
    FL_levelsetSegPara_dialog.h \
    ../worm_straighten_c/bdb_minus.h \
    ../worm_straighten_c/mst_prim_c.h \
    ../worm_straighten_c/bfs.h \
    ../worm_straighten_c/spline_cubic.h \
    ../cellseg/template_matching_seg.h \
    ../jba/c++/jba_mainfunc.h \
    ../jba/c++/jba_match_landmarks.h \
    ../jba/c++/wkernel.h \
    ../jba/c++/histeq.h \
    ../jba/c++/convert_type2uint8.h \
    ../jba/c++/jba_affine_xform.h \
    ../jba/c++/remove_nonaffine_points.h 

SOURCES += ../basic_c_fun/mg_utilities.cpp \
    ../basic_c_fun/mg_image_lib.cpp \
    ../basic_c_fun/stackutil.cpp \
    ../basic_c_fun/basic_memory.cpp \
    ../basic_c_fun/v3d_message.cpp \
    ../basic_c_fun/basic_surf_objs.cpp \
    ../basic_c_fun/basic_4dimage.cpp \
    ../plugin_loader/v3d_plugin_loader.cpp \
    ../plugin_loader/pluginDialog.cpp \
    ../graph/dijk.cpp \
    ../neuron_editing/apo_xforms.cpp \
    ../neuron_editing/neuron_xforms.cpp \
    ../neuron_editing/neuron_sim_scores.cpp \
    ../neuron_editing/v_neuronswc.cpp \
	../neuron_editing/neuron_format_converter.cpp \
    ../neuron_tracing/dij_bgl.cpp \
    ../gmm/fit_gmm.cpp \
    ../cellseg/template_matching_seg.cpp \
    ../3drenderer/v3dr_mainwindow.cpp \
    ../3drenderer/v3dr_glwidget.cpp \
    ../3drenderer/GLee_r.c \
    ../3drenderer/renderer.cpp \
    ../3drenderer/renderer_tex2.cpp \
    ../3drenderer/renderer_obj2.cpp \
    ../3drenderer/renderer_hit2.cpp \
    ../3drenderer/renderer_labelfield.cpp \
    ../3drenderer/v3dr_surfaceDialog.cpp \
    ../3drenderer/ItemEditor.cpp \
    ../3drenderer/Renderer_gl2.cpp \
    ../3drenderer/v3dr_colormapDialog.cpp \
    ../3drenderer/gradients.cpp \
    ../3drenderer/hoverpoints.cpp \
    ../3drenderer/barFigureDialog.cpp \
    v3d_version_info.cpp \
    main.cpp \
    mainwindow.cpp \
    mainwindow_interface.cpp \
    v3d_core.cpp \
    v3dimgproc_entry.cpp \
    my4dimage.cpp \
	v3dimg_proc_neuron.cpp \
    colormap.cpp \
    rotate_image.cpp \
    dialog_rotate.cpp \
    landmark_property_dialog.cpp \
    dialog_keypoint_features.cpp \
	histogramsimple.cpp \
    surfaceobj_geometry_dialog.cpp \
    import_filelistname.cpp \
    import_tiffseries.cpp \
    vano_linker_loader_dialog.cpp \
    atlas_viewer.cpp \
    dialog_watershed_para.cpp \
    ../worm_straighten_c/bdb_minus.cpp \
    ../worm_straighten_c/mst_prim_c.cpp \
    ../worm_straighten_c/bfs_1root.cpp \
    ../worm_straighten_c/spline_cubic.cpp \
	../jba/c++/histeq.cpp

FORMS += landmark_property.ui \
    surface_obj_annotation.ui \
    surfaceobj_geometry_dialog.ui \
    dialog_curve_trace.ui \
    template_matching_cellseg.ui \
    v3d_global_preference.ui \
    import_filelist_dialog.ui \
    import_images_tool.ui \
    dialog_vano_linkerloader.ui \
    dialog_pointcloudatlas_linkerloader.ui \
    dialog_imagecrop_bbox.ui \
    dialog_imageresample.ui \
    dialog_maskroi.ui \
    dialog_keypoint_features.ui \
    FL_watershedSegPara.ui \
    FL_levelsetSegPara.ui 

RESOURCES += v3d.qrc
RESOURCES += ../3drenderer/3drenderer.qrc
QT += opengl

LIBS += -L../jba/c++  

unix:LIBS += -L../common_lib/lib
unix:LIBS += -lm -lv3dtiff \
    -lv3dnewmat 
    -L/usr/lib/qt4/demos/shared -ldemo_shared
    
macx:LIBS += -L../common_lib/lib_mac32
macx:LIBS += -lm -lv3dtiff \
    -lv3dnewmat 
#    -framework GLUT

win32:LIBS += -lm -lv3dtiff \
    -lv3dnewmat 
#    -lglut32 # win32-mingw, on unix link libglut.a

INCLUDEPATH += ../common_lib/include   

# 100809 RZC: Eclipse need "-L./???" to start make at root dir of project (copy .pro to there)
LIBS += -L./jba/c++  
macx:LIBS += -L./common_lib/lib_mac32

INCLUDEPATH = $$unique(INCLUDEPATH)
LIBS = $$unique(LIBS)
# CONFIG = $$unique(CONFIG) # this only DOESN'T work on macx, very strange, by RZC 20080923
message(CONFIG=$$CONFIG)
