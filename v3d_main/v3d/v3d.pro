
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
# 110906: by PHC. Now include v3d_essential.pro
# 150506: by AB. Now include teramanager.pro
# 170623: by RZC. move mozak module here from v3d_essential.pro, change name of terafly/teramanager.pro to terafly.pro
# ######################################################################


include(v3d_essential.pro)
include(../terafly/terafly.pro)
QMAKE_CXXFLAGS +=-std=c++0x
# mozak module
include(../mozak/m_terafly/m_teramanager.pro)
RESOURCES += ../mozak/mozak.qrc
HEADERS +=  ../mozak/MozakUI.h \
            ../mozak/Mozak3DView.h
SOURCES +=  ../mozak/MozakUI.cpp \
            ../mozak/Mozak3DView.cpp


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


SOURCES += \
    ../neuron_annotator/animation/AnimationFrame.cpp \
    ../neuron_annotator/animation/interpolate.cpp \
    ../neuron_annotator/gui/NaViewer.cpp \
    ../neuron_annotator/gui/Na2DViewer.cpp \
    ../neuron_annotator/gui/NaMainWindow.cpp \
    ../neuron_annotator/gui/Na3DWidget.cpp \
    ../neuron_annotator/gui/NaZStackWidget.cpp \
    ../neuron_annotator/gui/NaLargeMIPWidget.cpp \
    ../neuron_annotator/gui/PreferencesDialog.cpp \
    ../neuron_annotator/render/ActorGL.cpp \
    ../neuron_annotator/render/CubeTestActorGL.cpp \
    ../neuron_annotator/render/CameraTransformGL.cpp \
    ../neuron_annotator/render/LegacyVolumeActor.cpp \
    ../neuron_annotator/render/ShaderProgramGL.cpp \
    ../neuron_annotator/render/VoxelRayCastShader.cpp \
    ../neuron_annotator/render/WireCube.cpp \
    ../neuron_annotator/utility/ColorSeparatorConsolidator.cpp \
    ../neuron_annotator/utility/FileTreeSearcher.cpp \
    ../neuron_annotator/utility/FooDebug.cpp \
    ../neuron_annotator/utility/SingleNeuronLsmSetReader.cpp \
    ../neuron_annotator/utility/StitchedFileUtility.cpp \
    ../neuron_annotator/DataFlowModel.cpp \
    ../neuron_annotator/utility/NeuronMaskEntry.cpp \
    ../neuron_annotator/MultiColorImageStackNode.cpp \
    ../neuron_annotator/data_model/CustomClipPlanes.cpp \
    ../neuron_annotator/data_model/Dimension.cpp \
    ../neuron_annotator/data_model/Fast3DTexture.cpp \
    ../neuron_annotator/data_model/NaLockableData.cpp \
    ../neuron_annotator/data_model/NaVolumeData.cpp \
    ../neuron_annotator/data_model/MipFragmentData.cpp \
    ../neuron_annotator/data_model/DataColorModel.cpp \
    ../neuron_annotator/data_model/MipFragmentColors.cpp \
    ../neuron_annotator/data_model/GalleryMipImages.cpp \
    ../neuron_annotator/data_model/MipMergedData.cpp \
    ../neuron_annotator/data_model/NeuronSelectionModel.cpp \
    ../neuron_annotator/data_model/ZSliceColors.cpp \
    ../neuron_annotator/data_model/SlotMerger.cpp \
    ../neuron_annotator/data_model/NaSharedDataSignaller.cpp \
    ../neuron_annotator/data_model/NaSharedDataModel.cpp \
    ../neuron_annotator/data_model/NeuronFragmentData.cpp \
    ../neuron_annotator/data_model/PrivateDataColorModel.cpp \
    ../neuron_annotator/data_model/PrivateNeuronFragmentData.cpp \
    ../neuron_annotator/data_model/StagedFileLoader.cpp \
    ../neuron_annotator/data_model/SampledVolumeMetadata.cpp \
    ../neuron_annotator/data_model/VolumeTexture.cpp \
    ../neuron_annotator/data_model/PrivateVolumeTexture.cpp \
    ../neuron_annotator/entity_model/EntityData.cpp \
    ../neuron_annotator/entity_model/Entity.cpp \
    ../neuron_annotator/entity_model/Ontology.cpp \
    ../neuron_annotator/entity_model/OntologyAnnotation.cpp \
    ../neuron_annotator/entity_model/AnnotatedBranch.cpp \
    ../neuron_annotator/entity_model/AnnotationSession.cpp \
    ../neuron_annotator/NeuronAnnotatorResultNode.cpp \
    ../neuron_annotator/TimebasedIdentifierGenerator.cpp \
    ../neuron_annotator/gui/RendererNeuronAnnotator.cpp \
    ../neuron_annotator/gui/GalleryButton.cpp \
    ../neuron_annotator/geometry/Vector3D.cpp \
    ../neuron_annotator/geometry/Rotation3D.cpp \
    ../neuron_annotator/geometry/CameraModel.cpp \
    ../neuron_annotator/gui/GammaWidget.cpp \
    ../neuron_annotator/gui/MouseClickManager.cpp \
    ../neuron_annotator/gui/AngleWidget.cpp \
    ../neuron_annotator/gui/AnnotationWidget.cpp \
    ../neuron_annotator/gui/trees/EntityTreeItem.cpp \
    ../neuron_annotator/gui/trees/EntityTreeModel.cpp \
    ../neuron_annotator/gui/trees/AnnotatedBranchTreeModel.cpp \
    ../neuron_annotator/gui/trees/OntologyTreeModel.cpp \
    ../neuron_annotator/gui/trees/EntityTreeView.cpp \
    ../neuron_annotator/gui/trees/AnnotatedBranchTreeView.cpp \
    ../neuron_annotator/gui/trees/OntologyTreeView.cpp \
    ../neuron_annotator/gui/ZoomSpinBox.cpp \
    ../neuron_annotator/gui/ZoomWidget.cpp \
    ../neuron_annotator/gui/CompartmentMapWidget.cpp \
    ../neuron_annotator/gui/CompartmentMapComboBox.cpp \
    ../neuron_annotator/gui/FragmentGalleryWidget.cpp \
    ../neuron_annotator/gui/NeuronQAction.cpp \
    ../neuron_annotator/gui/NeuronContextMenu.cpp \
    ../neuron_annotator/gui/DynamicRangeTool.cpp \
    ../neuron_annotator/gui/ColorRangeSliderWidget.cpp \
    ../neuron_annotator/gui/Stereo3DMode.cpp \
    ../neuron_annotator/gui/ScaleBar.cpp \
    ../neuron_annotator/microCT/CutPlanner.cpp \
    ../neuron_annotator/microCT/SingleCut.cpp \
    ../neuron_annotator/NeuronSelector.cpp \
    ../neuron_annotator/ExportFile.cpp \
    ../neuron_annotator/utility/JacsUtil.cpp \
    ../neuron_annotator/utility/Icons.cpp \
    ../neuron_annotator/utility/DataThread.cpp \
    ../neuron_annotator/utility/ConsoleObserver.cpp \
    ../neuron_annotator/utility/ImageLoaderBasic.cpp \
    ../neuron_annotator/utility/ImageLoader.cpp \
    ../neuron_annotator/utility/url_tools.cpp \
    ../neuron_annotator/analysis/ScreenPatternAnnotator.cpp \
    ../neuron_annotator/analysis/AlignerUtils.cpp \
    ../neuron_annotator/analysis/NeuronFragmentEditor.cpp \
    ../neuron_annotator/analysis/VolumePatternIndex.cpp \
    ../neuron_annotator/analysis/VolumeIndex.cpp \
    ../neuron_annotator/analysis/SleepThread.cpp \
    ../neuron_annotator/analysis/AnalysisTools.cpp \
    ../neuron_annotator/analysis/MaskChan.cpp \
    ../neuron_annotator/analysis/DilationErosion.cpp \
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

FORMS += \
    ../neuron_annotator/gui/NaMainWindow.ui \
    ../neuron_annotator/gui/GammaWidget.ui \
    ../neuron_annotator/gui/AngleWidget.ui \
    ../neuron_annotator/gui/AnnotationWidget.ui \
    ../neuron_annotator/gui/DynamicRangeTool.ui \
    ../neuron_annotator/gui/PreferencesNa.ui \
    ../neuron_annotator/gui/ZoomWidget.ui \
    ../neuron_annotator/microCT/micro_ct.ui \
    ../neuron_annotator/microCT/single_cut.ui

INCLUDEPATH = $$unique(INCLUDEPATH)
#LIBS = $$unique(LIBS)  # @FIXED by Alessandro on 2015-05-11. Proven buggy on Qt 4.7.1/MacOSX10.10 since it removed some -framework from LIBS.
message(CONFIG=$$CONFIG)

# aborted attempt to implement reentrant tiff image loading
# SOURCES -= ../basic_c_fun/mg_image_lib.cpp
# SOURCES += ../neuron_annotator/utility/mg_image_lib_reentrant.cpp


