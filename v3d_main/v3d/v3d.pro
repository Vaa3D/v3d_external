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
# 100820: add restriction of the win32 to use MYLIB_TIFF 
# 101008: add the imaging module
# 101008: (CMB) add web url download
# 101019: (CMB) add dialog_url_entry.ui
# 101025: (CMB) add dialog_update_v3d.ui
# 101119: add bundle test for mac 
# 101129: (CMB) add link to CoreServices framework on Snow Leopard
# 101201: (CMB) add dialog_update_list.ui
# 101213: (CMB) add more update dialogs
# 110806: (Hang) add ../custom_toolbar/v3d_custom_toolbar.h/.cpp
# ######################################################################

TEMPLATE = app
TARGET += 
DEPENDPATH += . v3d
INCLUDEPATH += . 

# commented the -app_bundle as on Mac the not-automatically closed terminal is quite annoying!
# macx: CONFIG-=app_bundle #by PHC, 101119

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

macx {
    # Mac possible location of arthurwidgets.h with official Qt 4.7 installer
    QTINST_SHARED_FOLDER = /Developer/Examples/Qt/Demos/shared
    include($$QTINST_SHARED_FOLDER/shared.pri)
    INCLUDEPATH += $$QTINST_SHARED_FOLDER
    LIBS += -L$$QTINST_SHARED_FOLDER
    # For faster neuron toggling. CMB 20 June 2011
    QMAKE_CXXFLAGS+=-O3
    CXXFLAGS+=-O3
}

unix:!macx {
    # Ubuntu linux possible location of arthurwidgets.h with official Qt install
    # QTINST_SHARED_FOLDER = /usr/lib/qt4/demos/shared
    QTINST_SHARED_FOLDER = /usr/lib64/qt4/demos/shared
    include($$QTINST_SHARED_FOLDER/shared.pri)
    INCLUDEPATH += $$QTINST_SHARED_FOLDER
    LIBS += -L$$QTINST_SHARED_FOLDER
}

# the following trick was figured out by Ruan Zongcai
CONFIG += warn_off  # only work for complier
#CONFIG += debug  # Fabrice and Luis tracking a bug. disable by PHC, 100819
#macx: QMAKE_LFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden # turn off this type warnings
#macx: QMAKE_CXXFLAGS += -gdwarf-2 # turn off visibility warnings
# need Qt 4.5.0 above and reCreate Makefile, this will be automatic.
CONFIG += thread

DEFINES += __v3dwebservice__

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
	../basic_c_fun/customary_structs/v3d_imaging_para.h \
	../basic_c_fun/basic_thread.h \
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
    ../3drenderer/renderer_gl1.h \
    ../3drenderer/v3dr_surfaceDialog.h \
    ../3drenderer/ItemEditor.h \
    ../3drenderer/renderer_gl2.h \
    ../3drenderer/v3dr_colormapDialog.h \
    ../3drenderer/gradients.h \
    ../3drenderer/hoverpoints.h \
    ../3drenderer/barFigureDialog.h \
	../imaging/v3d_imaging.h \
	../multithreadimageIO/v3d_multithreadimageIO.h \
    v3d_global_preference_dialog.h \
    v3d_compile_constraints.h \
    v3d_version_info.h \
    v3d_application.h \
    ../neuron_annotator/data_model/NaLockableData.h \
    ../neuron_annotator/data_model/NaVolumeData.h \
    ../neuron_annotator/data_model/MipFragmentData.h \
    ../neuron_annotator/data_model/DataColorModel.h \
    ../neuron_annotator/data_model/MipFragmentColors.h \
    ../neuron_annotator/data_model/GalleryMipImages.h \
    ../neuron_annotator/data_model/MipMergedData.h \
    ../neuron_annotator/data_model/NeuronSelectionModel.h \
    ../neuron_annotator/data_model/ZSliceColors.h \
    ../neuron_annotator/data_model/VolumeColors.h \
    ../neuron_annotator/data_model/SlotMerger.h \
    ../neuron_annotator/gui/NaViewer.h \
    ../neuron_annotator/gui/Na2DViewer.h \
    ../neuron_annotator/gui/NaMainWindow.h \
    ../neuron_annotator/gui/Na3DWidget.h \
    ../neuron_annotator/gui/NaZStackWidget.h \
    ../neuron_annotator/gui/NaLargeMIPWidget.h \
    ../neuron_annotator/utility/ColorSeparatorConsolidator.h \
    ../neuron_annotator/utility/FileTreeSearcher.h \
    ../neuron_annotator/utility/SingleNeuronLsmSetReader.h \
    ../neuron_annotator/utility/StitchedFileUtility.h \
    ../neuron_annotator/AnnotationSession.h \
    ../neuron_annotator/utility/NeuronMaskEntry.h \
    ../neuron_annotator/MultiColorImageStackNode.h \
    ../neuron_annotator/NeuronAnnotatorResultNode.h \
    ../neuron_annotator/TimebasedIdentifierGenerator.h \
    ../neuron_annotator/gui/RendererNeuronAnnotator.h \
    ../neuron_annotator/gui/GalleryButton.h \
    ../neuron_annotator/geometry/Vector3D.h \
    ../neuron_annotator/geometry/Rotation3D.h \
    ../neuron_annotator/geometry/CameraModel.h \
    ../neuron_annotator/gui/GammaWidget.h \
    ../neuron_annotator/gui/AngleWidget.h \
    ../neuron_annotator/gui/MouseClickManager.h \
    ../neuron_annotator/gui/BrightnessCalibrator.h \
    ../neuron_annotator/gui/ZoomSpinBox.h \
    ../neuron_annotator/gui/ZoomWidget.h \
    ../neuron_annotator/gui/CompartmentMapWidget.h \
    ../neuron_annotator/gui/CompartmentMapComboBox.h \
    ../neuron_annotator/gui/FragmentGalleryWidget.h \
	../neuron_annotator/NeuronSelector.h \
    ../neuron_annotator/ExportFile.h \
	../webservice/v3dwebservice.hpp \
	../webservice/v3dwebserver.h \
	../webservice/gsoap2/stdsoap2.h \
	../webservice/soapdep/soapH.h \
	../webservice/soapdep/soapStub.h \
	../webservice/soapdep/soapv3dwebserverService.h \
    ../cell_counter/CellCounter3D.h \
    colormap.h \
    ChannelTable.h \
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
    DownloadManager.h \
    v3d_actions.h \
    CommandManager.h \
    v3d_commandlineparser.h \
    pluginfunchandler.h \
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
    ../jba/c++/remove_nonaffine_points.h \
	../custom_toolbar/v3d_custom_toolbar.h       
unix:HEADERS += ../basic_c_fun/imageio_mylib.h 
#macx:HEADERS += ../basic_c_fun/imageio_mylib.h 


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
    ../3drenderer/renderer_tex.cpp \
    ../3drenderer/renderer_obj.cpp \
    ../3drenderer/renderer_hit.cpp \
    ../3drenderer/renderer_labelfield.cpp \
    ../3drenderer/v3dr_surfaceDialog.cpp \
    ../3drenderer/ItemEditor.cpp \
    ../3drenderer/renderer_gl2.cpp \
    ../3drenderer/v3dr_colormapDialog.cpp \
    ../3drenderer/gradients.cpp \
    ../3drenderer/hoverpoints.cpp \
    ../3drenderer/barFigureDialog.cpp \
	../imaging/v3d_imaging.cpp \
	../multithreadimageIO/v3d_multithreadimageIO.cpp \
    v3d_version_info.cpp \
    v3d_application.cpp \
    ../neuron_annotator/gui/NaViewer.cpp \
    ../neuron_annotator/gui/Na2DViewer.cpp \
    ../neuron_annotator/gui/NaMainWindow.cpp \
    ../neuron_annotator/gui/Na3DWidget.cpp \
    ../neuron_annotator/gui/NaZStackWidget.cpp \
    ../neuron_annotator/gui/NaLargeMIPWidget.cpp \
    ../neuron_annotator/utility/ColorSeparatorConsolidator.cpp \
    ../neuron_annotator/utility/FileTreeSearcher.cpp \
    ../neuron_annotator/utility/SingleNeuronLsmSetReader.cpp \
    ../neuron_annotator/utility/StitchedFileUtility.cpp \
    ../neuron_annotator/AnnotationSession.cpp \
    ../neuron_annotator/utility/NeuronMaskEntry.cpp \
    ../neuron_annotator/MultiColorImageStackNode.cpp \
    ../neuron_annotator/data_model/NaLockableData.cpp \
    ../neuron_annotator/data_model/NaVolumeData.cpp \
    ../neuron_annotator/data_model/MipFragmentData.cpp \
    ../neuron_annotator/data_model/DataColorModel.cpp \
    ../neuron_annotator/data_model/MipFragmentColors.cpp \
    ../neuron_annotator/data_model/GalleryMipImages.cpp \
    ../neuron_annotator/data_model/MipMergedData.cpp \
    ../neuron_annotator/data_model/NeuronSelectionModel.cpp \
    ../neuron_annotator/data_model/ZSliceColors.cpp \
    ../neuron_annotator/data_model/VolumeColors.cpp \
    ../neuron_annotator/data_model/SlotMerger.cpp \
    ../neuron_annotator/NeuronAnnotatorResultNode.cpp \
    ../neuron_annotator/TimebasedIdentifierGenerator.cpp \
    ../neuron_annotator/gui/RendererNeuronAnnotator.cpp \
    ../neuron_annotator/gui/GalleryButton.cpp \
    ../neuron_annotator/geometry/Vector3D.cpp \
    ../neuron_annotator/geometry/Rotation3D.cpp \
    ../neuron_annotator/geometry/CameraModel.cpp \
    ../neuron_annotator/gui/GammaWidget.cpp \
    ../neuron_annotator/gui/MouseClickManager.cpp \
    ../neuron_annotator/gui/BrightnessCalibrator.cpp \
    ../neuron_annotator/gui/AngleWidget.cpp \
    ../neuron_annotator/gui/ZoomSpinBox.cpp \
    ../neuron_annotator/gui/ZoomWidget.cpp \
    ../neuron_annotator/gui/CompartmentMapWidget.cpp \
    ../neuron_annotator/gui/CompartmentMapComboBox.cpp \
    ../neuron_annotator/gui/FragmentGalleryWidget.cpp \
	../neuron_annotator/NeuronSelector.cpp \
    ../neuron_annotator/ExportFile.cpp \
	../webservice/v3dwebservice.cpp \
	../webservice/gsoap2/stdsoap2.cpp \
	../webservice/soapdep/soapC.cpp \
	../webservice/soapdep/soapv3dwebserverService.cpp \
    ../cell_counter/CellCounter3D.cpp \
    main.cpp \
    mainwindow.cpp \
    mainwindow_interface.cpp \
    v3d_core.cpp \
    v3dimgproc_entry.cpp \
    my4dimage.cpp \
    v3dimg_proc_neuron.cpp \
    colormap.cpp \
    ChannelTable.cpp \
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
    DownloadManager.cpp \
    v3d_actions.cpp \
    CommandManager.cpp \
    v3d_commandlineparser.cpp \
    pluginfunchandler.cpp \
    ../worm_straighten_c/bdb_minus.cpp \
    ../worm_straighten_c/mst_prim_c.cpp \
    ../worm_straighten_c/bfs_1root.cpp \
    ../worm_straighten_c/spline_cubic.cpp \
    ../jba/c++/histeq.cpp \
	../custom_toolbar/v3d_custom_toolbar.cpp
unix:SOURCES += ../basic_c_fun/imageio_mylib.cpp
#macx:SOURCES += ../basic_c_fun/imageio_mylib.cpp


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
    dialog_url_entry.ui \
    dialog_update_v3d.ui \
    dialog_update_list.ui \
    dialog_update_options.ui \
    dialog_update_downloading.ui \
    dialog_update_checking.ui \
    ../neuron_annotator/gui/NaMainWindow.ui \
    ../neuron_annotator/gui/GammaWidget.ui \
    ../neuron_annotator/gui/AngleWidget.ui \
    ../neuron_annotator/gui/ZoomWidget.ui

RESOURCES += v3d.qrc
RESOURCES += ../3drenderer/3drenderer.qrc
QT += opengl
QT += network
QT += xml svg

LIBS += -L../jba/c++  

unix:LIBS += -L../common_lib/lib
unix:LIBS += -lm -lv3dtiff \
    -lv3dnewmat \
    -L/usr/lib/qt4/demos/shared -ldemo_shared
unix:LIBS += -L../common_lib/src_packages/mylib_tiff -lmylib
    
macx:LIBS += -L../common_lib/lib_mac32
macx:LIBS += -lm -lv3dtiff \
    -lv3dnewmat 
#    -framework GLUT
macx:LIBS += -L../common_lib/src_packages/mylib_tiff -lmylib	
# CMB Nov 29 2010 Snow leopard GLee_r.o requires CoreServices framework
macx:LIBS += -framework CoreServices

win32:LIBS += -lm -lv3dtiff \
    -lv3dnewmat 
#    -lglut32 # win32-mingw, on unix link libglut.a
#win32:LIBS += -L../common_lib/src_packages/mylib_tiff -lmylib	#for win32 disable using MYLIB


INCLUDEPATH += ../common_lib/include   


#removed LIBS+=./??? for Eclipse IDE using customized Build-command or Make-target instead, by RZC 20110709 
INCLUDEPATH = $$unique(INCLUDEPATH)
LIBS = $$unique(LIBS)
# CONFIG = $$unique(CONFIG) # this only DOESN'T work on macx, very strange, by RZC 20080923
message(CONFIG=$$CONFIG)
