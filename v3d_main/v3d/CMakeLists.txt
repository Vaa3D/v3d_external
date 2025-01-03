cmake_minimum_required(VERSION 3.16)
project(Vaa3D-x VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core5Compat Gui Network OpenGL OpenGLWidgets Svg Widgets Xml)
find_package(WrapZLIB 1.0.8)

qt_add_executable(Vaa3D-x WIN32 MACOSX_BUNDLE
    ../3drenderer/GLee2glew.c ../3drenderer/GLee2glew.h
    ../3drenderer/ItemEditor.cpp ../3drenderer/ItemEditor.h
    ../3drenderer/barFigureDialog.cpp ../3drenderer/barFigureDialog.h
    ../3drenderer/glsl_r.cpp ../3drenderer/glsl_r.h
    ../3drenderer/gradients.cpp ../3drenderer/gradients.h
    ../3drenderer/line_box_intersection_check.h
    ../3drenderer/nstroke.cpp
    ../3drenderer/nstroke_tracing.cpp
    ../3drenderer/qtr_widget.h
    ../3drenderer/renderer.cpp ../3drenderer/renderer.h
    ../3drenderer/renderer_gl1.h
    ../3drenderer/renderer_gl2.cpp ../3drenderer/renderer_gl2.h
    ../3drenderer/renderer_hit.cpp
    ../3drenderer/renderer_labelfield.cpp
    ../3drenderer/renderer_obj.cpp
    ../3drenderer/renderer_tex.cpp
    ../3drenderer/setVoxSize.ui
    ../3drenderer/v3d_hoverpoints.cpp ../3drenderer/v3d_hoverpoints.h
    ../3drenderer/v3dr_colormapDialog.cpp ../3drenderer/v3dr_colormapDialog.h
    ../3drenderer/v3dr_control_signal.cpp
    ../3drenderer/v3dr_glwidget.cpp ../3drenderer/v3dr_glwidget.h
    ../3drenderer/v3dr_mainwindow.cpp ../3drenderer/v3dr_mainwindow.h
    ../3drenderer/v3dr_surfaceDialog.cpp ../3drenderer/v3dr_surfaceDialog.h
    ../basic_c_fun/INeuronAssembler.h
    ../basic_c_fun/IPMain4NeuronAssembler.h
    ../basic_c_fun/basic_4dimage.cpp ../basic_c_fun/basic_4dimage.h
    ../basic_c_fun/basic_4dimage_create.cpp
    ../basic_c_fun/basic_landmark.h
    ../basic_c_fun/basic_memory.cpp ../basic_c_fun/basic_memory.h
    ../basic_c_fun/basic_surf_objs.cpp ../basic_c_fun/basic_surf_objs.h
    ../basic_c_fun/basic_thread.h
    ../basic_c_fun/basic_view3d.h
    ../basic_c_fun/color_xyz.h
    ../basic_c_fun/customary_structs/v3d_imaging_para.h
    ../basic_c_fun/img_definition.h
    ../basic_c_fun/mg_image_lib.cpp ../basic_c_fun/mg_image_lib.h
    ../basic_c_fun/mg_utilities.cpp ../basic_c_fun/mg_utilities.h
    ../basic_c_fun/stackutil.cpp ../basic_c_fun/stackutil.h
    ../basic_c_fun/v3d_global_preference.h
    ../basic_c_fun/v3d_interface.h
    ../basic_c_fun/v3d_message.cpp ../basic_c_fun/v3d_message.h
    ../basic_c_fun/volimg_proc.h
    ../basic_c_fun/volimg_proc_declare.h
    ../cell_counter/CellCounter3D.h
    ../cellseg/template_matching_seg.cpp ../cellseg/template_matching_seg.h
    ../custom_toolbar/v3d_custom_toolbar.cpp ../custom_toolbar/v3d_custom_toolbar.h
    ../gmm/fit_gmm.cpp ../gmm/fit_gmm.h
    ../graph/dijk.cpp ../graph/dijk.h
    ../graph/graph.h
    ../graph/graph_basic.h
    ../imaging/v3d_imaging.cpp ../imaging/v3d_imaging.h
    ../io/asc_to_swc.cpp ../io/asc_to_swc.h
    ../io/io_bioformats.cpp ../io/io_bioformats.h
    ../io/v3d_nrrd.cpp ../io/v3d_nrrd.h
    ../jba/c++/convert_type2uint8.h
    ../jba/c++/histeq.cpp ../jba/c++/histeq.h
    ../jba/c++/jba_affine_xform.h
    ../jba/c++/jba_mainfunc.h
    ../jba/c++/jba_match_landmarks.h
    ../jba/c++/remove_nonaffine_points.h
    ../jba/c++/wkernel.h
    ../multithreadimageIO/v3d_multithreadimageIO.cpp ../multithreadimageIO/v3d_multithreadimageIO.h
    ../neuron_annotator/DataFlowModel.h
    ../neuron_annotator/ExportFile.h
    ../neuron_annotator/MultiColorImageStackNode.h
    ../neuron_annotator/NeuronAnnotatorResultNode.h
    ../neuron_annotator/NeuronSelector.h
    ../neuron_annotator/TimebasedIdentifierGenerator.h
    ../neuron_annotator/analysis/AlignerUtils.h
    ../neuron_annotator/analysis/AnalysisTools.h
    ../neuron_annotator/analysis/DilationErosion.h
    ../neuron_annotator/analysis/MaskChan.h
    ../neuron_annotator/analysis/NeuronFragmentEditor.h
    ../neuron_annotator/analysis/ScreenPatternAnnotator.h
    ../neuron_annotator/analysis/SleepThread.h
    ../neuron_annotator/analysis/VolumeIndex.h
    ../neuron_annotator/analysis/VolumePatternIndex.h
    ../neuron_annotator/animation/AnimationFrame.h
    ../neuron_annotator/animation/KeyFrame.h
    ../neuron_annotator/animation/Movie.h
    ../neuron_annotator/animation/interpolate.h
    ../neuron_annotator/data_model/CustomClipPlanes.h
    ../neuron_annotator/data_model/DataColorModel.h
    ../neuron_annotator/data_model/Dimension.h
    ../neuron_annotator/data_model/Fast3DTexture.h
    ../neuron_annotator/data_model/GalleryMipImages.h
    ../neuron_annotator/data_model/MipFragmentColors.h
    ../neuron_annotator/data_model/MipFragmentData.h
    ../neuron_annotator/data_model/MipMergedData.h
    ../neuron_annotator/data_model/NaLockableData.h
    ../neuron_annotator/data_model/NaSharedDataModel.h
    ../neuron_annotator/data_model/NaSharedDataSignaller.h
    ../neuron_annotator/data_model/NaVolumeData.h
    ../neuron_annotator/data_model/NeuronFragmentData.h
    ../neuron_annotator/data_model/NeuronSelectionModel.h
    ../neuron_annotator/data_model/PrivateDataColorModel.h
    ../neuron_annotator/data_model/PrivateNeuronFragmentData.h
    ../neuron_annotator/data_model/PrivateVolumeTexture.h
    ../neuron_annotator/data_model/SampledVolumeMetadata.h
    ../neuron_annotator/data_model/SlotMerger.h
    ../neuron_annotator/data_model/StagedFileLoader.h
    ../neuron_annotator/data_model/VolumeTexture.h
    ../neuron_annotator/data_model/ZSliceColors.h
    ../neuron_annotator/entity_model/AnnotatedBranch.h
    ../neuron_annotator/entity_model/AnnotationSession.h
    ../neuron_annotator/entity_model/Entity.h
    ../neuron_annotator/entity_model/EntityData.h
    ../neuron_annotator/entity_model/Ontology.h
    ../neuron_annotator/entity_model/OntologyAnnotation.h
    ../neuron_annotator/geometry/CameraModel.h
    ../neuron_annotator/geometry/Rotation3D.h
    ../neuron_annotator/geometry/Vector3D.h
    ../neuron_annotator/gui/AngleWidget.h
    ../neuron_annotator/gui/AnnotationWidget.h
    ../neuron_annotator/gui/ColorRangeSliderWidget.h
    ../neuron_annotator/gui/CompartmentMapComboBox.h
    ../neuron_annotator/gui/CompartmentMapWidget.h
    ../neuron_annotator/gui/DynamicRangeTool.h
    ../neuron_annotator/gui/FragmentGalleryWidget.h
    ../neuron_annotator/gui/GalleryButton.h
    ../neuron_annotator/gui/GammaWidget.h
    ../neuron_annotator/gui/MouseClickManager.h
    ../neuron_annotator/gui/Na2DViewer.h
    ../neuron_annotator/gui/Na3DWidget.h
    ../neuron_annotator/gui/NaLargeMIPWidget.h
    ../neuron_annotator/gui/NaMainWindow.h
    ../neuron_annotator/gui/NaViewer.h
    ../neuron_annotator/gui/NaZStackWidget.h
    ../neuron_annotator/gui/NeuronContextMenu.h
    ../neuron_annotator/gui/NeuronQAction.h
    ../neuron_annotator/gui/PreferencesDialog.h
    ../neuron_annotator/gui/RendererNeuronAnnotator.h
    ../neuron_annotator/gui/ScaleBar.h
    ../neuron_annotator/gui/Stereo3DMode.h
    ../neuron_annotator/gui/ZoomSpinBox.h
    ../neuron_annotator/gui/ZoomWidget.h
    ../neuron_annotator/gui/trees/AnnotatedBranchTreeModel.h
    ../neuron_annotator/gui/trees/AnnotatedBranchTreeView.h
    ../neuron_annotator/gui/trees/EntityTreeItem.h
    ../neuron_annotator/gui/trees/EntityTreeModel.h
    ../neuron_annotator/gui/trees/EntityTreeView.h
    ../neuron_annotator/gui/trees/OntologyTreeModel.h
    ../neuron_annotator/gui/trees/OntologyTreeView.h
    ../neuron_annotator/microCT/CutPlanner.h
    ../neuron_annotator/microCT/SingleCut.h
    ../neuron_annotator/render/ActorGL.h
    ../neuron_annotator/render/CameraTransformGL.h
    ../neuron_annotator/render/CubeTestActorGL.h
    ../neuron_annotator/render/LegacyVolumeActor.h
    ../neuron_annotator/render/ShaderProgramGL.h
    ../neuron_annotator/render/VoxelRayCastShader.h
    ../neuron_annotator/render/WireCube.h
    ../neuron_annotator/utility/ColorSeparatorConsolidator.h
    ../neuron_annotator/utility/ConsoleObserver.h
    ../neuron_annotator/utility/DataThread.h
    ../neuron_annotator/utility/FileTreeSearcher.h
    ../neuron_annotator/utility/FooDebug.h
    ../neuron_annotator/utility/Icons.h
    ../neuron_annotator/utility/ImageLoader.h
    ../neuron_annotator/utility/ImageLoaderBasic.h
    ../neuron_annotator/utility/JacsUtil.h
    ../neuron_annotator/utility/NeuronMaskEntry.h
    ../neuron_annotator/utility/SingleNeuronLsmSetReader.h
    ../neuron_annotator/utility/StitchedFileUtility.h
    ../neuron_annotator/utility/url_tools.h
    ../neuron_editing/apo_xforms.cpp ../neuron_editing/apo_xforms.h
    ../neuron_editing/global_feature_compute.cpp
    ../neuron_editing/neuron_format_converter.cpp ../neuron_editing/neuron_format_converter.h
    ../neuron_editing/neuron_sim_scores.cpp ../neuron_editing/neuron_sim_scores.h
    ../neuron_editing/neuron_xforms.cpp ../neuron_editing/neuron_xforms.h
    ../neuron_editing/v_neuronswc.cpp ../neuron_editing/v_neuronswc.h
    ../neuron_toolbox/vaa3d_neurontoolbox.cpp ../neuron_toolbox/vaa3d_neurontoolbox.h
    ../neuron_tracing/dij_bgl.cpp
    ../neuron_tracing/fastmarching_linker.h
    ../neuron_tracing/heap.h
    ../neuron_tracing/neuron_tracing.h
    ../plugin_loader/pluginDialog.cpp ../plugin_loader/pluginDialog.h
    ../plugin_loader/v3d_plugin_loader.cpp ../plugin_loader/v3d_plugin_loader.h
    ../serverconnection/CheckGlWidget.h
    ../serverconnection/CheckManager.h
    ../serverconnection/CheckWidget.h
    ../serverconnection/checkmapwidget.h ../serverconnection/checkmapwidget.ui
    ../serverconnection/csmainwindow.h ../serverconnection/csmainwindow.ui
    ../serverconnection/infocache.h
    ../serverconnection/logindialog.h ../serverconnection/logindialog.ui
    ../serverconnection/model/coordinateconvert.h
    ../serverconnection/model/potentialarbormarkerinfo.h
    ../serverconnection/model/potentialsomainfo.h
    ../serverconnection/net/ctoscallback.h
    ../serverconnection/net/httputils.h
    ../serverconnection/net/httputilsbrainlist.h
    ../serverconnection/net/httputilsdownload.h
    ../serverconnection/net/httputilsgetlocation.h
    ../serverconnection/net/httputilsqualityinspection.h
    ../serverconnection/net/httputilsuser.h
    ../serverconnection/net/networkutils.h
    ../terafly/src/TiledVolume_partialdata.cpp
    ../terafly/src/control/CAnnotations.cpp ../terafly/src/control/CAnnotations.h
    ../terafly/src/control/CConverter.cpp ../terafly/src/control/CConverter.h
    ../terafly/src/control/CImageUtils.cpp ../terafly/src/control/CImageUtils.h
    ../terafly/src/control/CImport.cpp ../terafly/src/control/CImport.h
    ../terafly/src/control/COperation.cpp ../terafly/src/control/COperation.h
    ../terafly/src/control/CPlugin.cpp ../terafly/src/control/CPlugin.h
    ../terafly/src/control/CSettings.cpp ../terafly/src/control/CSettings.h
    ../terafly/src/control/CViewer.cpp ../terafly/src/control/CViewer.h
    ../terafly/src/control/CVolume.cpp ../terafly/src/control/CVolume.h
    ../terafly/src/control/QUndoMarkerCreate.cpp ../terafly/src/control/QUndoMarkerCreate.h
    ../terafly/src/control/QUndoMarkerDelete.cpp ../terafly/src/control/QUndoMarkerDelete.h
    ../terafly/src/control/QUndoMarkerDeleteROI.cpp ../terafly/src/control/QUndoMarkerDeleteROI.h
    ../terafly/src/control/V3Dsubclasses.cpp ../terafly/src/control/V3Dsubclasses.h
    ../terafly/src/control/VirtualPyramid.cpp ../terafly/src/control/VirtualPyramid.h
    ../terafly/src/presentation/PAbout.cpp ../terafly/src/presentation/PAbout.h
    ../terafly/src/presentation/PAnoToolBar.cpp ../terafly/src/presentation/PAnoToolBar.h
    ../terafly/src/presentation/PConverter.cpp ../terafly/src/presentation/PConverter.h
    ../terafly/src/presentation/PDialogImport.cpp ../terafly/src/presentation/PDialogImport.h
    ../terafly/src/presentation/PDialogProofreading.cpp ../terafly/src/presentation/PDialogProofreading.h
    ../terafly/src/presentation/PDialogVirtualPyramid.cpp ../terafly/src/presentation/PDialogVirtualPyramid.h
    ../terafly/src/presentation/PLog.cpp ../terafly/src/presentation/PLog.h
    ../terafly/src/presentation/PMain.cpp ../terafly/src/presentation/PMain.h
    ../terafly/src/presentation/PTabVolumeInfo.cpp ../terafly/src/presentation/PTabVolumeInfo.h
    ../terafly/src/presentation/QArrowButton.cpp ../terafly/src/presentation/QArrowButton.h
    ../terafly/src/presentation/QGLRefSys.cpp ../terafly/src/presentation/QGLRefSys.h
    ../terafly/src/presentation/QGradientBar.cpp ../terafly/src/presentation/QGradientBar.h
    ../terafly/src/presentation/QHelpBox.cpp ../terafly/src/presentation/QHelpBox.h
    ../terafly/src/presentation/QPixmapToolTip.cpp ../terafly/src/presentation/QPixmapToolTip.h
    ../terafly/src/presentation/QPrefixSuffixLineEdit.cpp ../terafly/src/presentation/QPrefixSuffixLineEdit.h
    ../terafly/src/presentation/theader.h
    ../terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.cpp ../terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.h
    ../terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.cpp ../terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.h
    ../terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlerror.cpp
    ../terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlparser.cpp
    ../terafly/src/terarepo/src/common/ProgressBar.cpp ../terafly/src/terarepo/src/common/ProgressBar.h
    ../terafly/src/terarepo/src/common/QProgressSender.cpp ../terafly/src/terarepo/src/common/QProgressSender.h
    ../terafly/src/terarepo/src/common/config.cpp ../terafly/src/terarepo/src/common/config.h
    ../terafly/src/terarepo/src/crossmips/CrossMIPs.h
    ../terafly/src/terarepo/src/crossmips/compute_funcs.cpp ../terafly/src/terarepo/src/crossmips/compute_funcs.h
    ../terafly/src/terarepo/src/crossmips/libcrossmips.cpp
    ../terafly/src/terarepo/src/crossmips/my_defs.h
    ../terafly/src/terarepo/src/imagemanager/BDVVolume.cpp ../terafly/src/terarepo/src/imagemanager/BDVVolume.h
    ../terafly/src/terarepo/src/imagemanager/ComposedVolume.cpp
    ../terafly/src/terarepo/src/imagemanager/HDF5Mngr.cpp ../terafly/src/terarepo/src/imagemanager/HDF5Mngr.h
    ../terafly/src/terarepo/src/imagemanager/IMS_HDF5Mngr.cpp ../terafly/src/terarepo/src/imagemanager/IMS_HDF5Mngr.h
    ../terafly/src/terarepo/src/imagemanager/IM_config.cpp ../terafly/src/terarepo/src/imagemanager/IM_config.h
    ../terafly/src/terarepo/src/imagemanager/MultiCycleVolume.cpp
    ../terafly/src/terarepo/src/imagemanager/MultiSliceVolume.cpp
    ../terafly/src/terarepo/src/imagemanager/RawFmtMngr.cpp ../terafly/src/terarepo/src/imagemanager/RawFmtMngr.h
    ../terafly/src/terarepo/src/imagemanager/RawVolume.cpp ../terafly/src/terarepo/src/imagemanager/RawVolume.h
    ../terafly/src/terarepo/src/imagemanager/SimpleVolume.cpp ../terafly/src/terarepo/src/imagemanager/SimpleVolume.h
    ../terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.cpp ../terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.h
    ../terafly/src/terarepo/src/imagemanager/Stack.cpp ../terafly/src/terarepo/src/imagemanager/Stack.h
    ../terafly/src/terarepo/src/imagemanager/StackRaw.cpp ../terafly/src/terarepo/src/imagemanager/StackRaw.h
    ../terafly/src/terarepo/src/imagemanager/StackedVolume.cpp ../terafly/src/terarepo/src/imagemanager/StackedVolume.h
    ../terafly/src/terarepo/src/imagemanager/Tiff3DMngr.cpp ../terafly/src/terarepo/src/imagemanager/Tiff3DMngr.h
    ../terafly/src/terarepo/src/imagemanager/TiledMCVolume.cpp ../terafly/src/terarepo/src/imagemanager/TiledMCVolume.h
    ../terafly/src/terarepo/src/imagemanager/TiledVolume.h
    ../terafly/src/terarepo/src/imagemanager/TimeSeries.cpp ../terafly/src/terarepo/src/imagemanager/TimeSeries.h
    ../terafly/src/terarepo/src/imagemanager/UnstitchedVolume.cpp ../terafly/src/terarepo/src/imagemanager/UnstitchedVolume.h
    ../terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.cpp ../terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.h
    ../terafly/src/terarepo/src/imagemanager/VirtualVolume.cpp ../terafly/src/terarepo/src/imagemanager/VirtualVolume.h
    ../terafly/src/terarepo/src/imagemanager/dirent_win.h
    ../terafly/src/terarepo/src/imagemanager/imBlock.cpp ../terafly/src/terarepo/src/imagemanager/imBlock.h
    ../terafly/src/terarepo/src/iomanager/IOPluginAPI.h
    ../terafly/src/terarepo/src/iomanager/iomanager.config.cpp ../terafly/src/terarepo/src/iomanager/iomanager.config.h
    ../terafly/src/terarepo/src/iomanager/ioplugins.h
    ../terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.cpp ../terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.h
    ../terafly/src/terarepo/src/iomanager/plugins/dcimg/dcimg.cpp ../terafly/src/terarepo/src/iomanager/plugins/dcimg/dcimg.h
    ../terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.cpp ../terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.h
    ../terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.cpp ../terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.h
    ../terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.cpp ../terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.h
    ../terafly/src/terarepo/src/stitcher/Displacement.cpp ../terafly/src/terarepo/src/stitcher/Displacement.h
    ../terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.cpp ../terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.h
    ../terafly/src/terarepo/src/stitcher/MergeTiles.cpp
    ../terafly/src/terarepo/src/stitcher/PDAlgo.cpp ../terafly/src/terarepo/src/stitcher/PDAlgo.h
    ../terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.cpp ../terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.h
    ../terafly/src/terarepo/src/stitcher/S_config.h
    ../terafly/src/terarepo/src/stitcher/StackRestorer.cpp ../terafly/src/terarepo/src/stitcher/StackRestorer.h
    ../terafly/src/terarepo/src/stitcher/StackStitcher.cpp ../terafly/src/terarepo/src/stitcher/StackStitcher.h
    ../terafly/src/terarepo/src/stitcher/TPAlgo.cpp ../terafly/src/terarepo/src/stitcher/TPAlgo.h
    ../terafly/src/terarepo/src/stitcher/TPAlgoLQP.cpp
    ../terafly/src/terarepo/src/stitcher/TPAlgoMST.cpp ../terafly/src/terarepo/src/stitcher/TPAlgoMST.h
    ../terafly/src/terarepo/src/stitcher/resumer.cpp ../terafly/src/terarepo/src/stitcher/resumer.h
    ../terafly/src/terarepo/src/utils/volumeconverter/VolumeConverter.cpp ../terafly/src/terarepo/src/utils/volumeconverter/VolumeConverter.h
    ../terafly/src/terarepo/src/utils/volumeconverter/vcresumer.cpp ../terafly/src/terarepo/src/utils/volumeconverter/vcresumer.h
    ../terafly/src/terarepo/src/volumemanager/vmBlock.cpp ../terafly/src/terarepo/src/volumemanager/vmBlock.h
    ../terafly/src/terarepo/src/volumemanager/vmBlockVolume.cpp ../terafly/src/terarepo/src/volumemanager/vmBlockVolume.h
    ../terafly/src/terarepo/src/volumemanager/vmCacheManager.cpp ../terafly/src/terarepo/src/volumemanager/vmCacheManager.h
    ../terafly/src/terarepo/src/volumemanager/vmMCVolume.cpp ../terafly/src/terarepo/src/volumemanager/vmMCVolume.h
    ../terafly/src/terarepo/src/volumemanager/vmStack.cpp ../terafly/src/terarepo/src/volumemanager/vmStack.h
    ../terafly/src/terarepo/src/volumemanager/vmStackedVolume.cpp ../terafly/src/terarepo/src/volumemanager/vmStackedVolume.h
    ../terafly/src/terarepo/src/volumemanager/vmVirtualStack.cpp ../terafly/src/terarepo/src/volumemanager/vmVirtualStack.h
    ../terafly/src/terarepo/src/volumemanager/vmVirtualVolume.cpp ../terafly/src/terarepo/src/volumemanager/vmVirtualVolume.h
    ../terafly/src/terarepo/src/volumemanager/volumemanager.config.cpp ../terafly/src/terarepo/src/volumemanager/volumemanager.config.h
    ../vrrenderer/Communicate.h
    ../vrrenderer/VRwidget.h
    ../webservice/console/cdsConsoleDataServiceProxy.h
    ../webservice/console/cdsH.h
    ../webservice/console/cdsStub.h
    ../webservice/console/envH.h
    ../webservice/console/envStub.h
    ../webservice/console/obsConsoleObserverService.h
    ../webservice/console/obsH.h
    ../webservice/console/obsStub.h
    ../webservice/gsoap2/stdsoap2.h
    ../webservice/impl/ConsoleObserverServiceImpl.h
    ../webservice/impl/EntityAdapter.h
    ../worm_straighten_c/bdb_minus.cpp ../worm_straighten_c/bdb_minus.h
    ../worm_straighten_c/bfs.h
    ../worm_straighten_c/bfs_1root.cpp
    ../worm_straighten_c/mst_prim_c.cpp ../worm_straighten_c/mst_prim_c.h
    ../worm_straighten_c/spline_cubic.cpp ../worm_straighten_c/spline_cubic.h
    ChannelTable.cpp ChannelTable.h
    CommandManager.h
    DownloadManager.cpp DownloadManager.h
    atlas_viewer.cpp atlas_viewer.h
    colormap.cpp colormap.h
    compute_win_diff.h
    compute_win_pca.h
    dialog_curve_trace.ui
    dialog_curve_trace_para.h
    dialog_imagecrop_bbox.h dialog_imagecrop_bbox.ui
    dialog_imageresample.h dialog_imageresample.ui
    dialog_keypoint_features.cpp dialog_keypoint_features.h dialog_keypoint_features.ui
    dialog_maskroi.h dialog_maskroi.ui
    dialog_pointcloudatlas_linkerloader.ui
    dialog_rotate.cpp dialog_rotate.h
    dialog_update_checking.ui
    dialog_update_downloading.ui
    dialog_update_list.ui
    dialog_update_options.ui
    dialog_update_v3d.ui
    dialog_url_entry.ui
    dialog_vano_linkerloader.ui
    histogramsimple.cpp histogramsimple.h
    import_filelist_dialog.h import_filelist_dialog.ui
    import_filelistname.cpp
    import_images_tool.ui
    import_images_tool_dialog.h
    import_tiffseries.cpp
    landmark_property.ui
    landmark_property_dialog.cpp landmark_property_dialog.h
    main.cpp
    mainwindow.cpp mainwindow.h
    mainwindow_interface.cpp
    mapview.cpp mapview.h
    my4dimage.cpp
    old_arthurstyle.cpp old_arthurstyle.h
    old_arthurwidgets.cpp old_arthurwidgets.h
    opt_rotate.h
    pluginfunchandler.cpp pluginfunchandler.h
    rotate_image.cpp rotate_image.h
    surface_obj_annotation.ui
    surfaceobj_annotation_dialog.h
    surfaceobj_geometry_dialog.cpp surfaceobj_geometry_dialog.h surfaceobj_geometry_dialog.ui
    template_matching_cellseg.ui
    template_matching_cellseg_dialog.h
    v3d_actions.cpp v3d_actions.h
    v3d_application.cpp v3d_application.h
    v3d_commandlineparser.cpp v3d_commandlineparser.h
    v3d_compile_constraints.h
    v3d_core.cpp v3d_core.h
    v3d_global_preference.ui
    v3d_global_preference_dialog.h
    v3d_version_info.cpp v3d_version_info.h
    v3dimg_proc_neuron.cpp
    v3dimgproc_entry.cpp
    vano_linker_loader_dialog.cpp vano_linker_loader_dialog.h
    vr_vaa3d_call.cpp vr_vaa3d_call.h
    xformwidget.h
)
target_include_directories(Vaa3D-x PRIVATE
    (INCLUDEPATH)
)

target_compile_definitions(Vaa3D-x PRIVATE
    ENABLE_BDV_HDF5
    ENABLE_IMS_HDF5
    TEEM_STATIC TEEM_STATIC
    TERACONVERTER_MAJOR
    TERACONVERTER_MINOR
    TERACONVERTER_PATCH
    TERASTITCHER2_MAJOR
    TERASTITCHER2_MINOR
    TERASTITCHER2_PATCH
    TERASTITCHER_MAJOR
    TERASTITCHER_MINOR
    TERASTITCHER_PATCH
    USE_EXPERIMENTAL_FEATURES
    WITH_NONAMESPACES
    WITH_PURE_VIRTUAL
    WITH_QT
    _ALLOW_WORKMODE_MENU_
    _VAA3D_TERAFLY_PLUGIN_MODE
    __NAWEBSERVICE__
)

target_link_libraries(Vaa3D-x PRIVATE
    Qt::Core
    Qt::Core5Compat
    Qt::Gui
    Qt::Network
    Qt::OpenGL
    Qt::OpenGLWidgets
    Qt::Svg
    Qt::Widgets
    Qt::Xml
)


# Resources:
set(icons_resource_files
    "../terafly/changelog.txt"
    "../terafly/icons/about.png"
    "../terafly/icons/clear.png"
    "../terafly/icons/close.png"
    "../terafly/icons/cursor_marker_add.png"
    "../terafly/icons/cursor_marker_add_2.png"
    "../terafly/icons/cursor_marker_delete.png"
    "../terafly/icons/cursor_marker_delete_roi.png"
    "../terafly/icons/extend.png"
    "../terafly/icons/help.png"
    "../terafly/icons/help2.png"
    "../terafly/icons/import.png"
    "../terafly/icons/load.png"
    "../terafly/icons/marker_add.png"
    "../terafly/icons/marker_add_2.png"
    "../terafly/icons/marker_delete.png"
    "../terafly/icons/marker_delete_roi.png"
    "../terafly/icons/marker_roi_view.png"
    "../terafly/icons/open_ano.png"
    "../terafly/icons/open_image_file.png"
    "../terafly/icons/open_image_folder.png"
    "../terafly/icons/open_image_hdf5.png"
    "../terafly/icons/open_image_terafly.png"
    "../terafly/icons/open_image_unconverted.png"
    "../terafly/icons/open_volume.png"
    "../terafly/icons/open_volume_recent.png"
    "../terafly/icons/options.png"
    "../terafly/icons/redo.png"
    "../terafly/icons/reset.png"
    "../terafly/icons/save.png"
    "../terafly/icons/saveas.png"
    "../terafly/icons/start.png"
    "../terafly/icons/stop.png"
    "../terafly/icons/terafly.png"
    "../terafly/icons/toolbar.png"
    "../terafly/icons/undo.png"
    "../terafly/icons/virtual.pyramid.empty.png"
    "../terafly/icons/virtual.pyramid.full.png"
    "../terafly/icons/virtual.pyramid.lowres.png"
    "../terafly/icons/wait.gif"
)

qt_add_resources(Vaa3D-x "icons"
    PREFIX
        "/"
    BASE
        "../terafly"
    FILES
        ${icons_resource_files}
)
set(resources_resource_files
    "../neuron_annotator/resources/beaker.png"
    "../neuron_annotator/resources/brain.png"
    "../neuron_annotator/resources/brick.png"
    "../neuron_annotator/resources/brick_grey.png"
    "../neuron_annotator/resources/bricks.png"
    "../neuron_annotator/resources/bullet_error.png"
    "../neuron_annotator/resources/color_vertex_cmb.txt"
    "../neuron_annotator/resources/folder.png"
    "../neuron_annotator/resources/folder_brick.png"
    "../neuron_annotator/resources/folder_image.png"
    "../neuron_annotator/resources/folder_open.png"
    "../neuron_annotator/resources/folder_open_page.png"
    "../neuron_annotator/resources/folder_open_page_white.png"
    "../neuron_annotator/resources/folder_page.png"
    "../neuron_annotator/resources/fruit_fly_small.png"
    "../neuron_annotator/resources/image.png"
    "../neuron_annotator/resources/images.png"
    "../neuron_annotator/resources/layers.png"
    "../neuron_annotator/resources/page.png"
    "../neuron_annotator/resources/page_copy.png"
    "../neuron_annotator/resources/page_go.png"
    "../neuron_annotator/resources/page_white.png"
    "../neuron_annotator/resources/page_white_code.png"
    "../neuron_annotator/resources/page_white_edit.png"
    "../neuron_annotator/resources/page_white_stack.png"
    "../neuron_annotator/resources/page_white_text.png"
    "../neuron_annotator/resources/tex_fragment_cmb.txt"
)

qt_add_resources(Vaa3D-x "resources"
    PREFIX
        "/neuron_annotator"
    BASE
        "../neuron_annotator"
    FILES
        ${resources_resource_files}
)
set_source_files_properties("pic/button_add.png"
    PROPERTIES QT_RESOURCE_ALIAS "button_add.png"
)
set(v3d_resource_files
    "pic/Lockoff.png"
    "pic/Lockon.png"
    "pic/arrow_down.png"
    "pic/arrow_left.png"
    "pic/arrow_right.png"
    "pic/arrow_up.png"
    "pic/atlasView.png"
    "pic/button_add.png"
    "pic/copy.png"
    "pic/customize.png"
    "pic/cut.png"
    "pic/default_xy_pic.jpg"
    "pic/default_yz_pic.jpg"
    "pic/default_zx_pic.jpg"
    "pic/help.png"
    "pic/import.png"
    "pic/new.png"
    "pic/open.png"
    "pic/paste.png"
    "pic/rotate_icon.png"
    "pic/save.png"
    "pic/v3dIcon128.png"
    "pic/web.png"
    "v3dIcon128.ico"
)

qt_add_resources(Vaa3D-x "v3d"
    PREFIX
        "/"
    FILES
        ${v3d_resource_files}
)
set(3drenderer_resource_files
    "../3drenderer/icons/click1.png"
    "../3drenderer/icons/click2.png"
    "../3drenderer/icons/click3.png"
    "../3drenderer/icons/neuronwbg.png"
    "../3drenderer/icons/neuronwbgref.png"
    "../3drenderer/icons/neuronwobg.png"
    "../3drenderer/icons/neuronwref.png"
    "../3drenderer/icons/start.png"
    "../3drenderer/icons/stroke1.png"
    "../3drenderer/icons/stroke2.png"
    "../3drenderer/icons/stroke3.png"
    "../3drenderer/icons/strokeN.png"
    "../3drenderer/icons/trace.png"
    "../3drenderer/shader/color_fragment.txt"
    "../3drenderer/shader/color_vertex.txt"
    "../3drenderer/shader/lighting.txt"
    "../3drenderer/shader/obj_fragment.txt"
    "../3drenderer/shader/tex_fragment.txt"
    "../3drenderer/shader/vertex_normal.txt"
)

qt_add_resources(Vaa3D-x "3drenderer"
    PREFIX
        "/"
    BASE
        "../3drenderer"
    FILES
        ${3drenderer_resource_files}
)

if(MACOS)
    target_sources(Vaa3D-x PUBLIC
        yosemiteFileURLfix.mm
    )

    target_include_directories(Vaa3D-x PRIVATE
        ../common_lib/include/hdf5
    )

    target_compile_definitions(Vaa3D-x PRIVATE
        MACOS_SYSTEM
        _ENABLE_MACX_DRAG_DROP_FIX_
    )

    target_link_libraries(Vaa3D-x PRIVATE
        "-framework CoreServices"
        "-framework Foundation"
        # Remove: L../common_lib/lib_mac64
        bz2
        hdf5
        mylib
        szip
        teem
        v3dnewmat
        v3dtiff
        z
    )
endif()

if(WIN32)
    target_sources(Vaa3D-x PUBLIC
        ../terafly/src/presentation/fileserver.cpp ../terafly/src/presentation/fileserver.h
        ../vrrenderer/Cylinder.cpp ../vrrenderer/Cylinder.h
        ../vrrenderer/Matrices.cpp ../vrrenderer/Matrices.h
        ../vrrenderer/RenderableObject.cpp ../vrrenderer/RenderableObject.h
        ../vrrenderer/Sphere.cpp ../vrrenderer/Sphere.h
        ../vrrenderer/V3dR_Communicator.cpp ../vrrenderer/V3dR_Communicator.h
        ../vrrenderer/VRFinger.cpp ../vrrenderer/VRFinger.h
        ../vrrenderer/VR_MainWindow.cpp ../vrrenderer/VR_MainWindow.h
        ../vrrenderer/lodepng.cpp ../vrrenderer/lodepng.h
        ../vrrenderer/v3dr_gl_vr.cpp ../vrrenderer/v3dr_gl_vr.h
    )

    set_target_properties(Vaa3D-x PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    target_include_directories(Vaa3D-x PRIVATE
        ../common_lib/include/hdf5-win64
        ..\common_lib\include\SDL
        ..\common_lib\include\glew
        ..\common_lib\include\openvr
    )

    target_compile_definitions(Vaa3D-x PRIVATE
        WINDOWS_SYSTEM
        __ALLOW_VR_FUNCS__
    )

    target_link_libraries(Vaa3D-x PRIVATE
        # Remove: L../common_lib/mingw64
        Opengl32
        SDL2
        WrapZLIB::WrapZLIB
        glew32
        glu32
        hdf5
        openvr_api
        szip
        teem
        v3dnewmat
        v3dtiff
        wsock32
        z
    )
endif()

if(UNIX AND NOT MACOS)
    target_sources(Vaa3D-x PUBLIC
        ../vrrenderer/Cylinder.cpp ../vrrenderer/Cylinder.h
        ../vrrenderer/Matrices.cpp ../vrrenderer/Matrices.h
        ../vrrenderer/RenderableObject.cpp ../vrrenderer/RenderableObject.h
        ../vrrenderer/Sphere.cpp ../vrrenderer/Sphere.h
        ../vrrenderer/V3dR_Communicator.cpp ../vrrenderer/V3dR_Communicator.h
        ../vrrenderer/VRFinger.cpp ../vrrenderer/VRFinger.h
        ../vrrenderer/VR_MainWindow.cpp ../vrrenderer/VR_MainWindow.h
        ../vrrenderer/lodepng.cpp ../vrrenderer/lodepng.h
        ../vrrenderer/v3dr_gl_vr.cpp ../vrrenderer/v3dr_gl_vr.h
    )

    set_target_properties(Vaa3D-x PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    target_include_directories(Vaa3D-x PRIVATE
        ../common_lib/include/SDL
        ../common_lib/include/glew
        ../common_lib/include/hdf5
        ../common_lib/include/openvr
    )

    target_compile_definitions(Vaa3D-x PRIVATE
        LINUX_SYSTEM
        __ALLOW_VR_FUNCS__
    )

    target_link_libraries(Vaa3D-x PRIVATE
        # Remove: L../common_lib/lib_ubuntu
        GLU
        bz2
        glut
        hdf5
        mylib
        szip
        teem
        v3dnewmat
        v3dtiff
        z
    )
endif()

if(UNIX)
    target_sources(Vaa3D-x PUBLIC
        ../basic_c_fun/imageio_mylib.cpp ../basic_c_fun/imageio_mylib.h
    )

    target_include_directories(Vaa3D-x PRIVATE
        ../common_lib/include/glew
    )
endif()

install(TARGETS Vaa3D-x
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
