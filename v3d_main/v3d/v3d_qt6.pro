#-----------From v3d.pro-------------------------
#include(v3d_essential.pro)
include(../terafly/terafly.pro)

## mozak module
#include(../mozak/m_terafly/m_teramanager.pro)
#RESOURCES += ../mozak/mozak.qrc
#HEADERS +=  ../mozak/MozakUI.h \
#            ../mozak/Mozak3DView.h \
#            customdebug.h
#SOURCES +=  ../mozak/MozakUI.cpp \
#            ../mozak/Mozak3DView.cpp


#CONFIG += console
DEFINES += __NAWEBSERVICE__
DEFINES += _ALLOW_WORKMODE_MENU_

#CONFIG  += debug

# Flags for gsoap (web services)
QMAKE_CXXFLAGS += -DWITH_NONAMESPACES
QMAKE_CXXFLAGS += -DWITH_PURE_VIRTUAL

# Resources such as icons
RESOURCES += ../neuron_annotator/resources.qrc

# Input
HEADERS += \
    ../neuron_annotator/animation/AnimationFrame.h \
    ../neuron_annotator/animation/interpolate.h \
    ../neuron_annotator/animation/KeyFrame.h \
    ../neuron_annotator/animation/Movie.h \
    ../neuron_annotator/data_model/CustomClipPlanes.h \
    ../neuron_annotator/data_model/Dimension.h \
    ../neuron_annotator/data_model/Fast3DTexture.h \
    ../neuron_annotator/data_model/NaLockableData.h \
    ../neuron_annotator/data_model/NaVolumeData.h \
    ../neuron_annotator/data_model/MipFragmentData.h \
    ../neuron_annotator/data_model/DataColorModel.h \
    ../neuron_annotator/data_model/MipFragmentColors.h \
    ../neuron_annotator/data_model/GalleryMipImages.h \
    ../neuron_annotator/data_model/MipMergedData.h \
    ../neuron_annotator/data_model/NeuronSelectionModel.h \
    ../neuron_annotator/data_model/ZSliceColors.h \
    ../neuron_annotator/data_model/SlotMerger.h \
    ../neuron_annotator/data_model/NaSharedDataSignaller.h \
    ../neuron_annotator/data_model/NaSharedDataModel.h \
    ../neuron_annotator/data_model/NeuronFragmentData.h \
    ../neuron_annotator/data_model/PrivateDataColorModel.h \
    ../neuron_annotator/data_model/PrivateNeuronFragmentData.h \
    ../neuron_annotator/data_model/SampledVolumeMetadata.h \
    ../neuron_annotator/data_model/StagedFileLoader.h \
    ../neuron_annotator/data_model/PrivateVolumeTexture.h \
    ../neuron_annotator/data_model/VolumeTexture.h \
    ../neuron_annotator/entity_model/EntityData.h \
    ../neuron_annotator/entity_model/Entity.h \
    ../neuron_annotator/entity_model/Ontology.h \
    ../neuron_annotator/entity_model/OntologyAnnotation.h \
    ../neuron_annotator/entity_model/AnnotatedBranch.h \
    ../neuron_annotator/entity_model/AnnotationSession.h \
    ../neuron_annotator/gui/NaViewer.h \
    ../neuron_annotator/gui/Na2DViewer.h \
    ../neuron_annotator/gui/NaMainWindow.h \
    ../neuron_annotator/gui/Na3DWidget.h \
    ../neuron_annotator/gui/NaZStackWidget.h \
    ../neuron_annotator/gui/NaLargeMIPWidget.h \
    ../neuron_annotator/gui/PreferencesDialog.h \
    ../neuron_annotator/gui/DynamicRangeTool.h \
    ../neuron_annotator/gui/ColorRangeSliderWidget.h \
    ../neuron_annotator/gui/Stereo3DMode.h \
    ../neuron_annotator/gui/ScaleBar.h \
    ../neuron_annotator/microCT/CutPlanner.h \
    ../neuron_annotator/microCT/SingleCut.h \
    ../neuron_annotator/render/ActorGL.h \
    ../neuron_annotator/render/CubeTestActorGL.h \
    ../neuron_annotator/render/CameraTransformGL.h \
    ../neuron_annotator/render/LegacyVolumeActor.h \
    ../neuron_annotator/render/ShaderProgramGL.h \
    ../neuron_annotator/render/VoxelRayCastShader.h \
    ../neuron_annotator/render/WireCube.h \
    ../neuron_annotator/utility/ColorSeparatorConsolidator.h \
    ../neuron_annotator/utility/FileTreeSearcher.h \
    ../neuron_annotator/utility/FooDebug.h \
    ../neuron_annotator/utility/SingleNeuronLsmSetReader.h \
    ../neuron_annotator/utility/StitchedFileUtility.h \
    ../neuron_annotator/utility/ImageLoaderBasic.h \
    ../neuron_annotator/utility/ImageLoader.h \
    ../neuron_annotator/utility/url_tools.h \
    ../neuron_annotator/DataFlowModel.h \
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
    ../neuron_annotator/gui/AnnotationWidget.h \
    ../neuron_annotator/gui/trees/EntityTreeItem.h \
    ../neuron_annotator/gui/trees/AnnotatedBranchTreeModel.h \
    ../neuron_annotator/gui/trees/OntologyTreeModel.h \
    ../neuron_annotator/gui/trees/EntityTreeModel.h \
    ../neuron_annotator/gui/trees/EntityTreeView.h \
    ../neuron_annotator/gui/trees/AnnotatedBranchTreeView.h \
    ../neuron_annotator/gui/trees/OntologyTreeView.h \
    ../neuron_annotator/gui/MouseClickManager.h \
    ../neuron_annotator/gui/ZoomSpinBox.h \
    ../neuron_annotator/gui/ZoomWidget.h \
    ../neuron_annotator/gui/CompartmentMapWidget.h \
    ../neuron_annotator/gui/CompartmentMapComboBox.h \
    ../neuron_annotator/gui/FragmentGalleryWidget.h \
    ../neuron_annotator/gui/NeuronQAction.h \
    ../neuron_annotator/gui/NeuronContextMenu.h \
    ../neuron_annotator/NeuronSelector.h \
    ../neuron_annotator/ExportFile.h \
    ../neuron_annotator/utility/JacsUtil.h \
    ../neuron_annotator/utility/Icons.h \
    ../neuron_annotator/utility/DataThread.h \
    ../neuron_annotator/utility/ConsoleObserver.h \
    ../neuron_annotator/analysis/ScreenPatternAnnotator.h \
    ../neuron_annotator/analysis/NeuronFragmentEditor.h \
    ../neuron_annotator/analysis/AlignerUtils.h \
    ../neuron_annotator/analysis/VolumePatternIndex.h \
    ../neuron_annotator/analysis/VolumeIndex.h \
    ../neuron_annotator/analysis/SleepThread.h \
    ../neuron_annotator/analysis/AnalysisTools.h \
    ../neuron_annotator/analysis/MaskChan.h \
    ../neuron_annotator/analysis/DilationErosion.h \
    ../webservice/gsoap2/stdsoap2.h \
    ../webservice/console/envH.h \
    ../webservice/console/envStub.h \
    ../webservice/console/cdsH.h \
    ../webservice/console/cdsStub.h \
    ../webservice/console/cdsConsoleDataServiceProxy.h \
    ../webservice/console/obsH.h \
    ../webservice/console/obsStub.h \
    ../webservice/console/obsConsoleObserverService.h \
    ../webservice/impl/ConsoleObserverServiceImpl.h \
    ../webservice/impl/EntityAdapter.h \
    ../cell_counter/CellCounter3D.h \
    CommandManager.h
#    myopenglwidget.h


SOURCES += \
#    ../neuron_annotator/animation/AnimationFrame.cpp \
#    ../neuron_annotator/animation/interpolate.cpp \
#    ../neuron_annotator/gui/NaViewer.cpp \
#    ../neuron_annotator/gui/Na2DViewer.cpp \
#    ../neuron_annotator/gui/NaMainWindow.cpp \
#    ../neuron_annotator/gui/Na3DWidget.cpp \
#    ../neuron_annotator/gui/NaZStackWidget.cpp \
#    ../neuron_annotator/gui/NaLargeMIPWidget.cpp \
#    ../neuron_annotator/gui/PreferencesDialog.cpp \
#    ../neuron_annotator/render/ActorGL.cpp \
#    ../neuron_annotator/render/CubeTestActorGL.cpp \
#    ../neuron_annotator/render/CameraTransformGL.cpp \
#    ../neuron_annotator/render/LegacyVolumeActor.cpp \
#    ../neuron_annotator/render/ShaderProgramGL.cpp \
#    ../neuron_annotator/render/VoxelRayCastShader.cpp \
#    ../neuron_annotator/render/WireCube.cpp \
#    ../neuron_annotator/utility/ColorSeparatorConsolidator.cpp \
#    ../neuron_annotator/utility/FileTreeSearcher.cpp \
#    ../neuron_annotator/utility/FooDebug.cpp \
#    ../neuron_annotator/utility/SingleNeuronLsmSetReader.cpp \
#    ../neuron_annotator/utility/StitchedFileUtility.cpp \
#    ../neuron_annotator/DataFlowModel.cpp \
#    ../neuron_annotator/utility/NeuronMaskEntry.cpp \
#    ../neuron_annotator/MultiColorImageStackNode.cpp \
#    ../neuron_annotator/data_model/CustomClipPlanes.cpp \
#    ../neuron_annotator/data_model/Dimension.cpp \
#    ../neuron_annotator/data_model/Fast3DTexture.cpp \
#    ../neuron_annotator/data_model/NaLockableData.cpp \
#    ../neuron_annotator/data_model/NaVolumeData.cpp \
#    ../neuron_annotator/data_model/MipFragmentData.cpp \
#    ../neuron_annotator/data_model/DataColorModel.cpp \
#    ../neuron_annotator/data_model/MipFragmentColors.cpp \
#    ../neuron_annotator/data_model/GalleryMipImages.cpp \
#    ../neuron_annotator/data_model/MipMergedData.cpp \
#    ../neuron_annotator/data_model/NeuronSelectionModel.cpp \
#    ../neuron_annotator/data_model/ZSliceColors.cpp \
#    ../neuron_annotator/data_model/SlotMerger.cpp \
#    ../neuron_annotator/data_model/NaSharedDataSignaller.cpp \
#    ../neuron_annotator/data_model/NaSharedDataModel.cpp \
#    ../neuron_annotator/data_model/NeuronFragmentData.cpp \
#    ../neuron_annotator/data_model/PrivateDataColorModel.cpp \
#    ../neuron_annotator/data_model/PrivateNeuronFragmentData.cpp \
#    ../neuron_annotator/data_model/StagedFileLoader.cpp \
#    ../neuron_annotator/data_model/SampledVolumeMetadata.cpp \
#    ../neuron_annotator/data_model/VolumeTexture.cpp \
#    ../neuron_annotator/data_model/PrivateVolumeTexture.cpp \
#    ../neuron_annotator/entity_model/EntityData.cpp \
#    ../neuron_annotator/entity_model/Entity.cpp \
#    ../neuron_annotator/entity_model/Ontology.cpp \
#    ../neuron_annotator/entity_model/OntologyAnnotation.cpp \
#    ../neuron_annotator/entity_model/AnnotatedBranch.cpp \
#    ../neuron_annotator/entity_model/AnnotationSession.cpp \
#    ../neuron_annotator/NeuronAnnotatorResultNode.cpp \
#    ../neuron_annotator/TimebasedIdentifierGenerator.cpp \
#   ../neuron_annotator/gui/RendererNeuronAnnotator.cpp \
#    ../neuron_annotator/gui/GalleryButton.cpp \
#    ../neuron_annotator/geometry/Vector3D.cpp \
#    ../neuron_annotator/geometry/Rotation3D.cpp \
#    ../neuron_annotator/geometry/CameraModel.cpp \
#    ../neuron_annotator/gui/GammaWidget.cpp \
#    ../neuron_annotator/gui/MouseClickManager.cpp \
#    ../neuron_annotator/gui/AngleWidget.cpp \
#    ../neuron_annotator/gui/AnnotationWidget.cpp \
#    ../neuron_annotator/gui/trees/EntityTreeItem.cpp \
#    ../neuron_annotator/gui/trees/EntityTreeModel.cpp \
#    ../neuron_annotator/gui/trees/AnnotatedBranchTreeModel.cpp \
#    ../neuron_annotator/gui/trees/OntologyTreeModel.cpp \
#    ../neuron_annotator/gui/trees/EntityTreeView.cpp \
#    ../neuron_annotator/gui/trees/AnnotatedBranchTreeView.cpp \
#    ../neuron_annotator/gui/trees/OntologyTreeView.cpp \
#    ../neuron_annotator/gui/ZoomSpinBox.cpp \
#    ../neuron_annotator/gui/ZoomWidget.cpp \
#    ../neuron_annotator/gui/CompartmentMapWidget.cpp \
#    ../neuron_annotator/gui/CompartmentMapComboBox.cpp \
#    ../neuron_annotator/gui/FragmentGalleryWidget.cpp \
#    ../neuron_annotator/gui/NeuronQAction.cpp \
#    ../neuron_annotator/gui/NeuronContextMenu.cpp \
#    ../neuron_annotator/gui/DynamicRangeTool.cpp \
#    ../neuron_annotator/gui/ColorRangeSliderWidget.cpp \
#    ../neuron_annotator/gui/Stereo3DMode.cpp \
#    ../neuron_annotator/gui/ScaleBar.cpp \
#    ../neuron_annotator/microCT/CutPlanner.cpp \
#    ../neuron_annotator/microCT/SingleCut.cpp \
#    ../neuron_annotator/NeuronSelector.cpp \
#    ../neuron_annotator/ExportFile.cpp \
#    ../neuron_annotator/utility/JacsUtil.cpp \
#    ../neuron_annotator/utility/Icons.cpp \
#    ../neuron_annotator/utility/DataThread.cpp \
#    ../neuron_annotator/utility/ConsoleObserver.cpp \
#    ../neuron_annotator/utility/ImageLoaderBasic.cpp \
#    ../neuron_annotator/utility/ImageLoader.cpp \
#    ../neuron_annotator/utility/url_tools.cpp \
#    ../neuron_annotator/analysis/ScreenPatternAnnotator.cpp \
#    ../neuron_annotator/analysis/AlignerUtils.cpp \
#    ../neuron_annotator/analysis/NeuronFragmentEditor.cpp \
#    ../neuron_annotator/analysis/VolumePatternIndex.cpp \
#    ../neuron_annotator/analysis/VolumeIndex.cpp \
#    ../neuron_annotator/analysis/SleepThread.cpp \
#    ../neuron_annotator/analysis/AnalysisTools.cpp \
#    ../neuron_annotator/analysis/MaskChan.cpp \
#    ../neuron_annotator/analysis/DilationErosion.cpp \
    ../webservice/gsoap2/stdsoap2.cpp \
    ../webservice/console/envC.cpp \
    ../webservice/console/cdsC.cpp \
    ../webservice/console/cdsConsoleDataServiceProxy.cpp \
    ../webservice/console/obsC.cpp \
    ../webservice/console/obsConsoleObserverService.cpp \
    ../webservice/impl/ConsoleObserverServiceImpl.cpp \
    ../webservice/impl/EntityAdapter.cpp \
    ../cell_counter/CellCounter3D.cpp \
    CommandManager.cpp
#    myopenglwidget.cpp

#FORMS += \
#    ../neuron_annotator/gui/NaMainWindow.ui \
#    ../neuron_annotator/gui/GammaWidget.ui \
#    ../neuron_annotator/gui/AngleWidget.ui \
#    ../neuron_annotator/gui/AnnotationWidget.ui \
#    ../neuron_annotator/gui/DynamicRangeTool.ui \
#    ../neuron_annotator/gui/PreferencesNa.ui \
#    ../neuron_annotator/gui/ZoomWidget.ui \
#    ../neuron_annotator/microCT/micro_ct.ui \
#    ../neuron_annotator/microCT/single_cut.ui

INCLUDEPATH = $$unique(INCLUDEPATH)
#LIBS = $$unique(LIBS)  # @FIXED by Alessandro on 2015-05-11. Proven buggy on Qt 4.7.1/MacOSX10.10 since it removed some -framework from LIBS.
message(CONFIG=$$CONFIG)
message(DEFINES=$$DEFINES)

# aborted attempt to implement reentrant tiff image loading
# SOURCES -= ../basic_c_fun/mg_image_lib.cpp
# SOURCES += ../neuron_annotator/utility/mg_image_lib_reentrant.cpp


#------------From v3d_essential.pro------------------
TEMPLATE = app
TARGET +=
DEPENDPATH += . v3d
INCLUDEPATH += .


#DEFINES += USE_Qt5  #this might be invoked from commandline like "~/Qt5.4.1/5.4/clang_64/bin/qmake DEFINES+=USE_Qt5 vaa3d64.pro", however it seems there is some bug


# commented the -app_bundle as on Mac the not-automatically closed terminal is quite annoying!
# macx: CONFIG-=app_bundle #by PHC, 101119

# cross-OS-platform, cross-Qt-version
QT_DIR = $$[QT_INSTALL_PREFIX]
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
    
    DEFINES += __ALLOW_VR_FUNCS__  
    
    # @ADD 2020-2-14 RZC: Microsoft C errors: 
    # error C2589: '(' : illegal token on right side of '::' for std::min/max 
    # error C2011: 'sockaddr' : 'struct' type redefinition
    DEFINES += NOMINMAX  _WINSOCKAPI_
 
}


INCLUDEPATH += $$LOCAL_DIR/include #./basic_c_fun
LIBS += -L$$LOCAL_DIR/lib

USE_Qt5 {
  INCLUDEPATH += $$QT_DIR/lib/QtConcurrent.framework/Versions/5/Headers  # for QtConcurrent, by PHC 2015May
  #SHARED_FOLDER = $$QT_DIR/demos/shared # for arthurwidgets
  SHARED_FOLDER = ./painting/shared/ # for arthurwidgets
  include($$SHARED_FOLDER/shared.pri)
  INCLUDEPATH += $$SHARED_FOLDER
  LIBS += -L$$SHARED_FOLDER
} else {
#  SHARED_FOLDER = $$QT_DIR/demos/shared # for arthurwidgets
#  include($$SHARED_FOLDER/shared.pri)
  INCLUDEPATH += $$SHARED_FOLDER
  LIBS += -L$$SHARED_FOLDER
}

win32:LIBS += -L$$SHARED_FOLDER/release # for Qt-win32 which only has release install(no debug)

macx {
    # Mac possible location of arthurwidgets.h with official Qt 4.7 installer
    QTINST_SHARED_FOLDER = /Developer/Examples/Qt/Demos/shared
    include($$QTINST_SHARED_FOLDER/shared.pri)
    INCLUDEPATH += $$QTINST_SHARED_FOLDER
    LIBS += -L$$QTINST_SHARED_FOLDER
    # For faster neuron toggling. CMB 20 June 2011
    QMAKE_CXXFLAGS+=-O3
    CXXFLAGS+=-O3
    # On mountain lion use clang compiler, not broken llvm/gcc compiler
#    QMAKE_CXX=c++  #commented this on 2013-08-09 by HP as this sentence will make the building on Qt 4.7 and OSX 10.7 fail.
                    #Indeed it seems the compiler is better to be set by a symbolic link on the local machine, instead of in
                    #the generic pro file here. See the Vaa3D Google Code page http://code.google.com/p/vaa3d/wiki/BuildVaa3D
                    #at the section "What you need: Qt & C++ compilers"

#    for Mac OS X 10.9
#    QMAKE_CXXFLAGS += " -stdlib=libstdc++"
#    QMAKE_LFLAGS += " -stdlib=libstdc++"
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

#CONFIG += console

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
    ../basic_c_fun/basic_view3d.h \
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
    ../3drenderer/GLee2glew.h \
    ../3drenderer/glsl_r.h \
    ../3drenderer/renderer.h \
    ../3drenderer/renderer_gl1.h \
    ../3drenderer/renderer_gl2.h \
    ../3drenderer/v3dr_surfaceDialog.h \
    ../3drenderer/ItemEditor.h \
    ../3drenderer/v3dr_colormapDialog.h \
    ../3drenderer/gradients.h \
    ../3drenderer/v3d_hoverpoints.h \
    ../3drenderer/barFigureDialog.h \
    ../3drenderer/line_box_intersection_check.h \
    ../neuron_tracing/heap.h \
    ../neuron_tracing/fastmarching_linker.h \
	../imaging/v3d_imaging.h \
    ../neuron_toolbox/vaa3d_neurontoolbox.h \
	../multithreadimageIO/v3d_multithreadimageIO.h \
    v3d_global_preference_dialog.h \
    v3d_compile_constraints.h \
    v3d_version_info.h \
    v3d_application.h \
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
    mapview.h \
    DownloadManager.h \
    v3d_actions.h \
    v3d_commandlineparser.h \
    pluginfunchandler.h \
    vr_vaa3d_call.h \
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
    ../custom_toolbar/v3d_custom_toolbar.h \
    ../io/io_bioformats.h \
    ../io/asc_to_swc.h \
    ../io/v3d_nrrd.h \
    ../terafly/src/presentation/theader.h \
    ../basic_c_fun/INeuronAssembler.h \
    ../basic_c_fun/IPMain4NeuronAssembler.h \
    ./old_arthurstyle.h \
    ./old_arthurwidgets.h


unix:HEADERS += ../basic_c_fun/imageio_mylib.h
#macx:HEADERS += ../basic_c_fun/imageio_mylib.h

#v3d_main/v3d
SOURCES += ../basic_c_fun/mg_utilities.cpp \
    ../basic_c_fun/mg_image_lib.cpp \
    ../basic_c_fun/stackutil.cpp \
    ../basic_c_fun/basic_memory.cpp \
    ../basic_c_fun/v3d_message.cpp \
    ../basic_c_fun/basic_surf_objs.cpp \
    ../basic_c_fun/basic_4dimage.cpp \
    ../basic_c_fun/basic_4dimage_create.cpp \
    ../plugin_loader/v3d_plugin_loader.cpp \
    ../plugin_loader/pluginDialog.cpp \
    ../graph/dijk.cpp \
    ../neuron_editing/apo_xforms.cpp \
    ../neuron_editing/neuron_xforms.cpp \
    ../neuron_editing/neuron_sim_scores.cpp \
    ../neuron_editing/v_neuronswc.cpp \
    ../neuron_editing/neuron_format_converter.cpp \
    ../neuron_editing/global_feature_compute.cpp \
    ../neuron_tracing/dij_bgl.cpp \
    ../gmm/fit_gmm.cpp \
    ../cellseg/template_matching_seg.cpp \
    ../3drenderer/v3dr_mainwindow.cpp \
    ../3drenderer/v3dr_glwidget.cpp \
    ../3drenderer/renderer.cpp \
    ../3drenderer/renderer_tex.cpp \
    ../3drenderer/renderer_obj.cpp \
    ../3drenderer/renderer_labelfield.cpp \
    ../3drenderer/renderer_gl2.cpp \
    ../3drenderer/glsl_r.cpp \
    ../3drenderer/GLee2glew.c \
    ../3drenderer/renderer_hit.cpp \
    ../3drenderer/nstroke.cpp \
    ../3drenderer/nstroke_tracing.cpp \
    ../3drenderer/v3dr_surfaceDialog.cpp \
    ../3drenderer/ItemEditor.cpp \
    ../3drenderer/v3dr_colormapDialog.cpp \
    ../3drenderer/gradients.cpp \
    ../3drenderer/v3dr_control_signal.cpp \
    ../3drenderer/v3d_hoverpoints.cpp \
    ../3drenderer/barFigureDialog.cpp \
        ../imaging/v3d_imaging.cpp \
    ../neuron_toolbox/vaa3d_neurontoolbox.cpp \
	../multithreadimageIO/v3d_multithreadimageIO.cpp \
    v3d_version_info.cpp \
    v3d_application.cpp \
    main.cpp \
    mainwindow.cpp \
    mainwindow_interface.cpp \
    mapview.cpp \
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
    v3d_commandlineparser.cpp \
    pluginfunchandler.cpp \
    vr_vaa3d_call.cpp \
    ../worm_straighten_c/bdb_minus.cpp \
    ../worm_straighten_c/mst_prim_c.cpp \
    ../worm_straighten_c/bfs_1root.cpp \
    ../worm_straighten_c/spline_cubic.cpp \
    ../jba/c++/histeq.cpp \
    ../custom_toolbar/v3d_custom_toolbar.cpp \
    ../io/io_bioformats.cpp \
    ../io/asc_to_swc.cpp \
    ../io/v3d_nrrd.cpp \
    ./old_arthurstyle.cpp \
    ./old_arthurwidgets.cpp

unix:SOURCES += ../basic_c_fun/imageio_mylib.cpp
#macx:SOURCES += ../basic_c_fun/imageio_mylib.cpp

unix:INCLUDEPATH += ../common_lib/include/glew/  #by RZC 2020-2-15

win32 {
INCLUDEPATH += ..\common_lib\include\SDL/ #for VR, by PHC 20170615
INCLUDEPATH += ..\common_lib\include\glew/  #for VR, by PHC 20170615
INCLUDEPATH += ..\common_lib\include\openvr/   #for VR, by PHC 20170615

HEADERS += \
    ../vrrenderer/v3dr_gl_vr.h \
    ../vrrenderer/Cylinder.h \
    ../vrrenderer/Sphere.h \
    ../vrrenderer/Matrices.h \
    ../vrrenderer/lodepng.h \
    ../vrrenderer/RenderableObject.h \
    ../vrrenderer/VRFinger.h \
    ../vrrenderer/V3dR_Communicator.h \
    ../vrrenderer/VR_MainWindow.h

SOURCES += \
    ../vrrenderer/v3dr_gl_vr.cpp \
    ../vrrenderer/Cylinder.cpp \
    ../vrrenderer/Sphere.cpp \
    ../vrrenderer/Matrices.cpp \
    ../vrrenderer/lodepng.cpp \
    ../vrrenderer/RenderableObject.cpp \
    ../vrrenderer/VRFinger.cpp \
    ../vrrenderer/V3dR_Communicator.cpp \
    ../vrrenderer/VR_MainWindow.cpp
}

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
    ../3drenderer/setVoxSize.ui



RESOURCES += v3d.qrc
RESOURCES += ../3drenderer/3drenderer.qrc
QT += core gui widgets opengl openglwidgets
QT += network
QT += xml svg

LIBS += -L../jba/c++

unix:!macx {
  LIBS += -L../common_lib/lib
  LIBS += -lm -lv3dtiff
  LIBS += -lv3dnewmat
#unix:LIBS += -L/usr/lib/qt4/demos/shared -ldemo_shared
  LIBS += -L../common_lib/src_packages/mylib_tiff -lmylib
  LIBS += -L../common_lib/lib_unix64  -lteem  -lbz2 -lz  -lGLU #for nrrd support
  ### LIBS += -lGLEW # static link by including glew.c
}

#added 20140324 to cope with centos 64bit GL library issue. by HP
#add -lglut -lGLU to fix the GL referencing issue on Ubuntu, otherwise it complains 
unix!macx:LIBS += -lglut -lGLU

macx:LIBS += -L../common_lib/lib_mac32
macx:LIBS += -lm -lv3dtiff -lv3dnewmat
#    -framework GLUT
macx:LIBS += -L../common_lib/src_packages/mylib_tiff -lmylib
macx:LIBS += -L../common_lib/lib_mac64 -lteem  -lbz2 -lz  #for nrrd support
# CMB Nov 29 2010 Snow leopard GLee_r.o requires CoreServices framework
macx:LIBS += -framework CoreServices

# @ADDED by Alessandro on 2015-05-09. Method to get the path-based URL from the file-based URL
#CONFIG += dragdropfix
macx:dragdropfix{
    DEFINES += _ENABLE_MACX_DRAG_DROP_FIX_
    OBJECTIVE_SOURCES += yosemiteFileURLfix.mm
    QMAKE_LFLAGS += -F /System/Library/Frameworks/Foundation.framework/
    LIBS += -framework Foundation
}


# NOT REALLY USED, LIBS are replaced by vaa3d_msvc.pro
win32:LIBS += -lm -lv3dtiff \
-lv3dnewmat

win32:LIBS += -L../common_lib/mingw64 -lglew32 -lhdf5 -lszip -lzlib -lSDL2 -lteem  -lz -lopenvr_api -lwsock32 #for nrrd support #for nrrd support
#    -lglut32 # win32-mingw, on unix link libglut.a
#win32:LIBS += -L../common_lib/src_packages/mylib_tiff -lmylib	#for win32 disable using MYLIB


INCLUDEPATH += ../common_lib/include
DEFINES *= TEEM_STATIC
QMAKE_CXXFLAGS += -DTEEM_STATIC

#removed LIBS+=./??? for Eclipse IDE using customized Build-command or Make-target instead, by RZC 20110709
INCLUDEPATH = $$unique(INCLUDEPATH)
# LIBS = $$unique(LIBS) # @FIXED by Alessandro on 2015-05-11. Proven buggy on Qt 4.7.1/MacOSX10.10 since it removed some -framework from LIBS.
# CONFIG = $$unique(CONFIG) # this only DOESN'T work on macx, very strange, by RZC 20080923
message(CONFIG=$$CONFIG)

#自己加的，不然会出现gl函数没有实现的问题
win32:LIBS += -lOpengl32 \
               -lglu32
unix:LIBS += -lglut -lGLU
