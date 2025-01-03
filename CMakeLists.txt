cmake_minimum_required(VERSION 3.16)
project(Vaa3D-x VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)

add_executable(Vaa3D-x WIN32 MACOSX_BUNDLE
    v3d_main/3drenderer/GLee2glew.c
    v3d_main/3drenderer/GLee2glew.h
    v3d_main/3drenderer/ItemEditor.cpp
    v3d_main/3drenderer/ItemEditor.h
    v3d_main/3drenderer/barFigureDialog.cpp
    v3d_main/3drenderer/barFigureDialog.h
    v3d_main/3drenderer/glsl_r.cpp
    v3d_main/3drenderer/glsl_r.h
    v3d_main/3drenderer/gradients.cpp
    v3d_main/3drenderer/gradients.h
    v3d_main/3drenderer/line_box_intersection_check.h
    v3d_main/3drenderer/nstroke.cpp
    v3d_main/3drenderer/nstroke_tracing.cpp
    v3d_main/3drenderer/qtr_widget.h
    v3d_main/3drenderer/renderer.cpp
    v3d_main/3drenderer/renderer.h
    v3d_main/3drenderer/renderer_gl1.h
    v3d_main/3drenderer/renderer_gl2.cpp
    v3d_main/3drenderer/renderer_gl2.h
    v3d_main/3drenderer/renderer_hit.cpp
    v3d_main/3drenderer/renderer_labelfield.cpp
    v3d_main/3drenderer/renderer_obj.cpp
    v3d_main/3drenderer/renderer_tex.cpp
    v3d_main/3drenderer/setVoxSize.ui
    v3d_main/3drenderer/v3d_hoverpoints.cpp
    v3d_main/3drenderer/v3d_hoverpoints.h
    v3d_main/3drenderer/v3dr_colormapDialog.cpp
    v3d_main/3drenderer/v3dr_colormapDialog.h
    v3d_main/3drenderer/v3dr_control_signal.cpp
    v3d_main/3drenderer/v3dr_glwidget.cpp
    v3d_main/3drenderer/v3dr_glwidget.h
    v3d_main/3drenderer/v3dr_mainwindow.cpp
    v3d_main/3drenderer/v3dr_mainwindow.h
    v3d_main/3drenderer/v3dr_surfaceDialog.cpp
    v3d_main/3drenderer/v3dr_surfaceDialog.h
    v3d_main/basic_c_fun/INeuronAssembler.h
    v3d_main/basic_c_fun/IPMain4NeuronAssembler.h
    v3d_main/basic_c_fun/basic_4dimage.cpp
    v3d_main/basic_c_fun/basic_4dimage.h
    v3d_main/basic_c_fun/basic_4dimage_create.cpp
    v3d_main/basic_c_fun/basic_landmark.h
    v3d_main/basic_c_fun/basic_memory.cpp
    v3d_main/basic_c_fun/basic_memory.h
    v3d_main/basic_c_fun/basic_surf_objs.cpp
    v3d_main/basic_c_fun/basic_surf_objs.h
    v3d_main/basic_c_fun/basic_thread.h
    v3d_main/basic_c_fun/basic_view3d.h
    v3d_main/basic_c_fun/color_xyz.h
    v3d_main/basic_c_fun/customary_structs/v3d_imaging_para.h
    v3d_main/basic_c_fun/img_definition.h
    v3d_main/basic_c_fun/mg_image_lib.cpp
    v3d_main/basic_c_fun/mg_image_lib.h
    v3d_main/basic_c_fun/mg_utilities.cpp
    v3d_main/basic_c_fun/mg_utilities.h
    v3d_main/basic_c_fun/stackutil.cpp
    v3d_main/basic_c_fun/stackutil.h
    v3d_main/basic_c_fun/v3d_global_preference.h
    v3d_main/basic_c_fun/v3d_interface.h
    v3d_main/basic_c_fun/v3d_message.cpp
    v3d_main/basic_c_fun/v3d_message.h
    v3d_main/basic_c_fun/volimg_proc.h
    v3d_main/basic_c_fun/volimg_proc_declare.h
    v3d_main/cell_counter/CellCounter3D.cpp
    v3d_main/cell_counter/CellCounter3D.h
    v3d_main/cellseg/template_matching_seg.cpp
    v3d_main/cellseg/template_matching_seg.h
    v3d_main/custom_toolbar/v3d_custom_toolbar.cpp
    v3d_main/custom_toolbar/v3d_custom_toolbar.h
    v3d_main/gmm/fit_gmm.cpp
    v3d_main/gmm/fit_gmm.h
    v3d_main/graph/dijk.cpp
    v3d_main/graph/dijk.h
    v3d_main/graph/graph.h
    v3d_main/graph/graph_basic.h
    v3d_main/imaging/v3d_imaging.cpp
    v3d_main/imaging/v3d_imaging.h
    v3d_main/io/asc_to_swc.cpp
    v3d_main/io/asc_to_swc.h
    v3d_main/io/io_bioformats.cpp
    v3d_main/io/io_bioformats.h
    v3d_main/io/v3d_nrrd.cpp
    v3d_main/io/v3d_nrrd.h
    v3d_main/jba/c++/convert_type2uint8.h
    v3d_main/jba/c++/histeq.cpp
    v3d_main/jba/c++/histeq.h
    v3d_main/jba/c++/jba_affine_xform.h
    v3d_main/jba/c++/jba_mainfunc.h
    v3d_main/jba/c++/jba_match_landmarks.h
    v3d_main/jba/c++/remove_nonaffine_points.h
    v3d_main/jba/c++/wkernel.h
    v3d_main/multithreadimageIO/v3d_multithreadimageIO.cpp
    v3d_main/multithreadimageIO/v3d_multithreadimageIO.h
    v3d_main/neuron_annotator/DataFlowModel.h
    v3d_main/neuron_annotator/ExportFile.h
    v3d_main/neuron_annotator/MultiColorImageStackNode.h
    v3d_main/neuron_annotator/NeuronAnnotatorResultNode.h
    v3d_main/neuron_annotator/NeuronSelector.h
    v3d_main/neuron_annotator/TimebasedIdentifierGenerator.h
    v3d_main/neuron_annotator/analysis/AlignerUtils.h
    v3d_main/neuron_annotator/analysis/AnalysisTools.h
    v3d_main/neuron_annotator/analysis/DilationErosion.h
    v3d_main/neuron_annotator/analysis/MaskChan.h
    v3d_main/neuron_annotator/analysis/NeuronFragmentEditor.h
    v3d_main/neuron_annotator/analysis/ScreenPatternAnnotator.h
    v3d_main/neuron_annotator/analysis/SleepThread.h
    v3d_main/neuron_annotator/analysis/VolumeIndex.h
    v3d_main/neuron_annotator/analysis/VolumePatternIndex.h
    v3d_main/neuron_annotator/animation/AnimationFrame.h
    v3d_main/neuron_annotator/animation/KeyFrame.h
    v3d_main/neuron_annotator/animation/Movie.h
    v3d_main/neuron_annotator/animation/interpolate.h
    v3d_main/neuron_annotator/data_model/CustomClipPlanes.h
    v3d_main/neuron_annotator/data_model/DataColorModel.h
    v3d_main/neuron_annotator/data_model/Dimension.h
    v3d_main/neuron_annotator/data_model/Fast3DTexture.h
    v3d_main/neuron_annotator/data_model/GalleryMipImages.h
    v3d_main/neuron_annotator/data_model/MipFragmentColors.h
    v3d_main/neuron_annotator/data_model/MipFragmentData.h
    v3d_main/neuron_annotator/data_model/MipMergedData.h
    v3d_main/neuron_annotator/data_model/NaLockableData.h
    v3d_main/neuron_annotator/data_model/NaSharedDataModel.h
    v3d_main/neuron_annotator/data_model/NaSharedDataSignaller.h
    v3d_main/neuron_annotator/data_model/NaVolumeData.h
    v3d_main/neuron_annotator/data_model/NeuronFragmentData.h
    v3d_main/neuron_annotator/data_model/NeuronSelectionModel.h
    v3d_main/neuron_annotator/data_model/PrivateDataColorModel.h
    v3d_main/neuron_annotator/data_model/PrivateNeuronFragmentData.h
    v3d_main/neuron_annotator/data_model/PrivateVolumeTexture.h
    v3d_main/neuron_annotator/data_model/SampledVolumeMetadata.h
    v3d_main/neuron_annotator/data_model/SlotMerger.h
    v3d_main/neuron_annotator/data_model/StagedFileLoader.h
    v3d_main/neuron_annotator/data_model/VolumeTexture.h
    v3d_main/neuron_annotator/data_model/ZSliceColors.h
    v3d_main/neuron_annotator/entity_model/AnnotatedBranch.h
    v3d_main/neuron_annotator/entity_model/AnnotationSession.h
    v3d_main/neuron_annotator/entity_model/Entity.h
    v3d_main/neuron_annotator/entity_model/EntityData.h
    v3d_main/neuron_annotator/entity_model/Ontology.h
    v3d_main/neuron_annotator/entity_model/OntologyAnnotation.h
    v3d_main/neuron_annotator/geometry/CameraModel.h
    v3d_main/neuron_annotator/geometry/Rotation3D.h
    v3d_main/neuron_annotator/geometry/Vector3D.h
    v3d_main/neuron_annotator/gui/AngleWidget.h
    v3d_main/neuron_annotator/gui/AnnotationWidget.h
    v3d_main/neuron_annotator/gui/ColorRangeSliderWidget.h
    v3d_main/neuron_annotator/gui/CompartmentMapComboBox.h
    v3d_main/neuron_annotator/gui/CompartmentMapWidget.h
    v3d_main/neuron_annotator/gui/DynamicRangeTool.h
    v3d_main/neuron_annotator/gui/FragmentGalleryWidget.h
    v3d_main/neuron_annotator/gui/GalleryButton.h
    v3d_main/neuron_annotator/gui/GammaWidget.h
    v3d_main/neuron_annotator/gui/MouseClickManager.h
    v3d_main/neuron_annotator/gui/Na2DViewer.h
    v3d_main/neuron_annotator/gui/Na3DWidget.h
    v3d_main/neuron_annotator/gui/NaLargeMIPWidget.h
    v3d_main/neuron_annotator/gui/NaMainWindow.h
    v3d_main/neuron_annotator/gui/NaViewer.h
    v3d_main/neuron_annotator/gui/NaZStackWidget.h
    v3d_main/neuron_annotator/gui/NeuronContextMenu.h
    v3d_main/neuron_annotator/gui/NeuronQAction.h
    v3d_main/neuron_annotator/gui/PreferencesDialog.h
    v3d_main/neuron_annotator/gui/RendererNeuronAnnotator.h
    v3d_main/neuron_annotator/gui/ScaleBar.h
    v3d_main/neuron_annotator/gui/Stereo3DMode.h
    v3d_main/neuron_annotator/gui/ZoomSpinBox.h
    v3d_main/neuron_annotator/gui/ZoomWidget.h
    v3d_main/neuron_annotator/gui/trees/AnnotatedBranchTreeModel.h
    v3d_main/neuron_annotator/gui/trees/AnnotatedBranchTreeView.h
    v3d_main/neuron_annotator/gui/trees/EntityTreeItem.h
    v3d_main/neuron_annotator/gui/trees/EntityTreeModel.h
    v3d_main/neuron_annotator/gui/trees/EntityTreeView.h
    v3d_main/neuron_annotator/gui/trees/OntologyTreeModel.h
    v3d_main/neuron_annotator/gui/trees/OntologyTreeView.h
    v3d_main/neuron_annotator/microCT/CutPlanner.h
    v3d_main/neuron_annotator/microCT/SingleCut.h
    v3d_main/neuron_annotator/render/ActorGL.h
    v3d_main/neuron_annotator/render/CameraTransformGL.h
    v3d_main/neuron_annotator/render/CubeTestActorGL.h
    v3d_main/neuron_annotator/render/LegacyVolumeActor.h
    v3d_main/neuron_annotator/render/ShaderProgramGL.h
    v3d_main/neuron_annotator/render/VoxelRayCastShader.h
    v3d_main/neuron_annotator/render/WireCube.h
    v3d_main/neuron_annotator/utility/ColorSeparatorConsolidator.h
    v3d_main/neuron_annotator/utility/ConsoleObserver.h
    v3d_main/neuron_annotator/utility/DataThread.h
    v3d_main/neuron_annotator/utility/FileTreeSearcher.h
    v3d_main/neuron_annotator/utility/FooDebug.h
    v3d_main/neuron_annotator/utility/Icons.h
    v3d_main/neuron_annotator/utility/ImageLoader.h
    v3d_main/neuron_annotator/utility/ImageLoaderBasic.cpp
    v3d_main/neuron_annotator/utility/ImageLoaderBasic.h
    v3d_main/neuron_annotator/utility/JacsUtil.h
    v3d_main/neuron_annotator/utility/NeuronMaskEntry.h
    v3d_main/neuron_annotator/utility/SingleNeuronLsmSetReader.h
    v3d_main/neuron_annotator/utility/StitchedFileUtility.h
    v3d_main/neuron_annotator/utility/url_tools.h
    v3d_main/neuron_editing/apo_xforms.cpp
    v3d_main/neuron_editing/apo_xforms.h
    v3d_main/neuron_editing/global_feature_compute.cpp
    v3d_main/neuron_editing/neuron_format_converter.cpp
    v3d_main/neuron_editing/neuron_format_converter.h
    v3d_main/neuron_editing/neuron_sim_scores.cpp
    v3d_main/neuron_editing/neuron_sim_scores.h
    v3d_main/neuron_editing/neuron_xforms.cpp
    v3d_main/neuron_editing/neuron_xforms.h
    v3d_main/neuron_editing/v_neuronswc.cpp
    v3d_main/neuron_editing/v_neuronswc.h
    v3d_main/neuron_toolbox/vaa3d_neurontoolbox.cpp
    v3d_main/neuron_toolbox/vaa3d_neurontoolbox.h
    v3d_main/neuron_tracing/dij_bgl.cpp
    v3d_main/neuron_tracing/fastmarching_linker.h
    v3d_main/neuron_tracing/heap.h
    v3d_main/neuron_tracing/neuron_tracing.h
    v3d_main/plugin_loader/pluginDialog.cpp
    v3d_main/plugin_loader/pluginDialog.h
    v3d_main/plugin_loader/v3d_plugin_loader.cpp
    v3d_main/plugin_loader/v3d_plugin_loader.h
    v3d_main/serverconnection/CheckGlWidget.cpp
    v3d_main/serverconnection/CheckGlWidget.h
    v3d_main/serverconnection/CheckManager.cpp
    v3d_main/serverconnection/CheckManager.h
    v3d_main/serverconnection/CheckWidget.cpp
    v3d_main/serverconnection/CheckWidget.h
    v3d_main/serverconnection/checkmapwidget.cpp
    v3d_main/serverconnection/checkmapwidget.h
    v3d_main/serverconnection/checkmapwidget.ui
    v3d_main/serverconnection/csmainwindow.cpp
    v3d_main/serverconnection/csmainwindow.h
    v3d_main/serverconnection/csmainwindow.ui
    v3d_main/serverconnection/infocache.cpp
    v3d_main/serverconnection/infocache.h
    v3d_main/serverconnection/logindialog.cpp
    v3d_main/serverconnection/logindialog.h
    v3d_main/serverconnection/logindialog.ui
    v3d_main/serverconnection/model/coordinateconvert.cpp
    v3d_main/serverconnection/model/coordinateconvert.h
    v3d_main/serverconnection/model/potentialarbormarkerinfo.cpp
    v3d_main/serverconnection/model/potentialarbormarkerinfo.h
    v3d_main/serverconnection/model/potentialsomainfo.cpp
    v3d_main/serverconnection/model/potentialsomainfo.h
    v3d_main/serverconnection/net/ctoscallback.h
    v3d_main/serverconnection/net/httputils.cpp
    v3d_main/serverconnection/net/httputils.h
    v3d_main/serverconnection/net/httputilsbrainlist.cpp
    v3d_main/serverconnection/net/httputilsbrainlist.h
    v3d_main/serverconnection/net/httputilsdownload.cpp
    v3d_main/serverconnection/net/httputilsdownload.h
    v3d_main/serverconnection/net/httputilsgetlocation.cpp
    v3d_main/serverconnection/net/httputilsgetlocation.h
    v3d_main/serverconnection/net/httputilsqualityinspection.cpp
    v3d_main/serverconnection/net/httputilsqualityinspection.h
    v3d_main/serverconnection/net/httputilsuser.cpp
    v3d_main/serverconnection/net/httputilsuser.h
    v3d_main/serverconnection/net/networkutils.cpp
    v3d_main/serverconnection/net/networkutils.h
    v3d_main/terafly/src/TiledVolume_partialdata.cpp
    v3d_main/terafly/src/control/CAnnotations.cpp
    v3d_main/terafly/src/control/CAnnotations.h
    v3d_main/terafly/src/control/CConverter.cpp
    v3d_main/terafly/src/control/CConverter.h
    v3d_main/terafly/src/control/CImageUtils.cpp
    v3d_main/terafly/src/control/CImageUtils.h
    v3d_main/terafly/src/control/CImport.cpp
    v3d_main/terafly/src/control/CImport.h
    v3d_main/terafly/src/control/COperation.cpp
    v3d_main/terafly/src/control/COperation.h
    v3d_main/terafly/src/control/CPlugin.cpp
    v3d_main/terafly/src/control/CPlugin.h
    v3d_main/terafly/src/control/CSettings.cpp
    v3d_main/terafly/src/control/CSettings.h
    v3d_main/terafly/src/control/CViewer.cpp
    v3d_main/terafly/src/control/CViewer.h
    v3d_main/terafly/src/control/CVolume.cpp
    v3d_main/terafly/src/control/CVolume.h
    v3d_main/terafly/src/control/QUndoMarkerCreate.cpp
    v3d_main/terafly/src/control/QUndoMarkerCreate.h
    v3d_main/terafly/src/control/QUndoMarkerDelete.cpp
    v3d_main/terafly/src/control/QUndoMarkerDelete.h
    v3d_main/terafly/src/control/QUndoMarkerDeleteROI.cpp
    v3d_main/terafly/src/control/QUndoMarkerDeleteROI.h
    v3d_main/terafly/src/control/V3Dsubclasses.cpp
    v3d_main/terafly/src/control/V3Dsubclasses.h
    v3d_main/terafly/src/control/VirtualPyramid.cpp
    v3d_main/terafly/src/control/VirtualPyramid.h
    v3d_main/terafly/src/presentation/loadmanagewidget.cpp
    v3d_main/terafly/src/presentation/loadmanagewidget.h
    v3d_main/terafly/src/presentation/PAbout.cpp
    v3d_main/terafly/src/presentation/PAbout.h
    v3d_main/terafly/src/presentation/PAnoToolBar.cpp
    v3d_main/terafly/src/presentation/PAnoToolBar.h
    v3d_main/terafly/src/presentation/PConverter.cpp
    v3d_main/terafly/src/presentation/PConverter.h
    v3d_main/terafly/src/presentation/PDialogImport.cpp
    v3d_main/terafly/src/presentation/PDialogImport.h
    v3d_main/terafly/src/presentation/PDialogProofreading.cpp
    v3d_main/terafly/src/presentation/PDialogProofreading.h
    v3d_main/terafly/src/presentation/PDialogVirtualPyramid.cpp
    v3d_main/terafly/src/presentation/PDialogVirtualPyramid.h
    v3d_main/terafly/src/presentation/PLog.cpp
    v3d_main/terafly/src/presentation/PLog.h
    v3d_main/terafly/src/presentation/PMain.cpp
    v3d_main/terafly/src/presentation/PMain.h
    v3d_main/terafly/src/presentation/PTabVolumeInfo.cpp
    v3d_main/terafly/src/presentation/PTabVolumeInfo.h
    v3d_main/terafly/src/presentation/QArrowButton.cpp
    v3d_main/terafly/src/presentation/QArrowButton.h
    v3d_main/terafly/src/presentation/QGLRefSys.cpp
    v3d_main/terafly/src/presentation/QGLRefSys.h
    v3d_main/terafly/src/presentation/QGradientBar.cpp
    v3d_main/terafly/src/presentation/QGradientBar.h
    v3d_main/terafly/src/presentation/QHelpBox.cpp
    v3d_main/terafly/src/presentation/QHelpBox.h
    v3d_main/terafly/src/presentation/QPixmapToolTip.cpp
    v3d_main/terafly/src/presentation/QPixmapToolTip.h
    v3d_main/terafly/src/presentation/QPrefixSuffixLineEdit.cpp
    v3d_main/terafly/src/presentation/QPrefixSuffixLineEdit.h
    v3d_main/terafly/src/presentation/theader.h
    v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.cpp
    v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.h
    v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.cpp
    v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.h
    v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlerror.cpp
    v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlparser.cpp
    v3d_main/terafly/src/terarepo/src/common/ProgressBar.cpp
    v3d_main/terafly/src/terarepo/src/common/ProgressBar.h
    v3d_main/terafly/src/terarepo/src/common/QProgressSender.cpp
    v3d_main/terafly/src/terarepo/src/common/QProgressSender.h
    v3d_main/terafly/src/terarepo/src/common/config.cpp
    v3d_main/terafly/src/terarepo/src/common/config.h
    v3d_main/terafly/src/terarepo/src/crossmips/CrossMIPs.h
    v3d_main/terafly/src/terarepo/src/crossmips/compute_funcs.cpp
    v3d_main/terafly/src/terarepo/src/crossmips/compute_funcs.h
    v3d_main/terafly/src/terarepo/src/crossmips/libcrossmips.cpp
    v3d_main/terafly/src/terarepo/src/crossmips/my_defs.h
    v3d_main/terafly/src/terarepo/src/imagemanager/BDVVolume.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/BDVVolume.h
    v3d_main/terafly/src/terarepo/src/imagemanager/ComposedVolume.h
    v3d_main/terafly/src/terarepo/src/imagemanager/ComposedVolume.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/HDF5Mngr.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/HDF5Mngr.h
    v3d_main/terafly/src/terarepo/src/imagemanager/IMS_HDF5Mngr.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/IMS_HDF5Mngr.h
    v3d_main/terafly/src/terarepo/src/imagemanager/IM_config.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/IM_config.h
    v3d_main/terafly/src/terarepo/src/imagemanager/MultiCycleVolume.h
    v3d_main/terafly/src/terarepo/src/imagemanager/MultiCycleVolume.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/MultiSliceVolume.h
    v3d_main/terafly/src/terarepo/src/imagemanager/MultiSliceVolume.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/RawFmtMngr.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/RawFmtMngr.h
    v3d_main/terafly/src/terarepo/src/imagemanager/RawVolume.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/RawVolume.h
    v3d_main/terafly/src/terarepo/src/imagemanager/SimpleVolume.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/SimpleVolume.h
    v3d_main/terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.h
    v3d_main/terafly/src/terarepo/src/imagemanager/Stack.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/Stack.h
    v3d_main/terafly/src/terarepo/src/imagemanager/StackRaw.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/StackRaw.h
    v3d_main/terafly/src/terarepo/src/imagemanager/StackedVolume.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/StackedVolume.h
    v3d_main/terafly/src/terarepo/src/imagemanager/Tiff3DMngr.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/Tiff3DMngr.h
    v3d_main/terafly/src/terarepo/src/imagemanager/TiledMCVolume.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/TiledMCVolume.h
    v3d_main/terafly/src/terarepo/src/imagemanager/TiledVolume.h
    v3d_main/terafly/src/terarepo/src/imagemanager/TimeSeries.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/TimeSeries.h
    v3d_main/terafly/src/terarepo/src/imagemanager/UnstitchedVolume.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/UnstitchedVolume.h
    v3d_main/terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.h
    v3d_main/terafly/src/terarepo/src/imagemanager/VirtualVolume.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/VirtualVolume.h
    v3d_main/terafly/src/terarepo/src/imagemanager/dirent_win.h
    v3d_main/terafly/src/terarepo/src/imagemanager/imBlock.cpp
    v3d_main/terafly/src/terarepo/src/imagemanager/imBlock.h
    v3d_main/terafly/src/terarepo/src/iomanager/IOPluginAPI.h
    v3d_main/terafly/src/terarepo/src/iomanager/iomanager.config.cpp
    v3d_main/terafly/src/terarepo/src/iomanager/iomanager.config.h
    v3d_main/terafly/src/terarepo/src/iomanager/ioplugins.h
    v3d_main/terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.cpp
    v3d_main/terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.h
    v3d_main/terafly/src/terarepo/src/iomanager/plugins/dcimg/dcimg.cpp
    v3d_main/terafly/src/terarepo/src/iomanager/plugins/dcimg/dcimg.h
    v3d_main/terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.cpp
    v3d_main/terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.h
    v3d_main/terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.cpp
    v3d_main/terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.h
    v3d_main/terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.cpp
    v3d_main/terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.h
    v3d_main/terafly/src/terarepo/src/stitcher/Displacement.cpp
    v3d_main/terafly/src/terarepo/src/stitcher/Displacement.h
    v3d_main/terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.cpp
    v3d_main/terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.h
    v3d_main/terafly/src/terarepo/src/stitcher/MergeTiles.cpp
    v3d_main/terafly/src/terarepo/src/stitcher/PDAlgo.cpp
    v3d_main/terafly/src/terarepo/src/stitcher/PDAlgo.h
    v3d_main/terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.cpp
    v3d_main/terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.h
    v3d_main/terafly/src/terarepo/src/stitcher/S_config.h
    v3d_main/terafly/src/terarepo/src/stitcher/StackRestorer.cpp
    v3d_main/terafly/src/terarepo/src/stitcher/StackRestorer.h
    v3d_main/terafly/src/terarepo/src/stitcher/StackStitcher.cpp
    v3d_main/terafly/src/terarepo/src/stitcher/StackStitcher.h
    v3d_main/terafly/src/terarepo/src/stitcher/TPAlgo.cpp
    v3d_main/terafly/src/terarepo/src/stitcher/TPAlgo.h
    v3d_main/terafly/src/terarepo/src/stitcher/TPAlgoLQP.h
    v3d_main/terafly/src/terarepo/src/stitcher/TPAlgoLQP.cpp
    v3d_main/terafly/src/terarepo/src/stitcher/TPAlgoMST.cpp
    v3d_main/terafly/src/terarepo/src/stitcher/TPAlgoMST.h
    v3d_main/terafly/src/terarepo/src/stitcher/resumer.cpp
    v3d_main/terafly/src/terarepo/src/stitcher/resumer.h
    v3d_main/terafly/src/terarepo/src/utils/volumeconverter/VolumeConverter.cpp
    v3d_main/terafly/src/terarepo/src/utils/volumeconverter/VolumeConverter.h
    v3d_main/terafly/src/terarepo/src/utils/volumeconverter/vcresumer.cpp
    v3d_main/terafly/src/terarepo/src/utils/volumeconverter/vcresumer.h
    v3d_main/terafly/src/terarepo/src/volumemanager/vmBlock.cpp
    v3d_main/terafly/src/terarepo/src/volumemanager/vmBlock.h
    v3d_main/terafly/src/terarepo/src/volumemanager/vmBlockVolume.cpp
    v3d_main/terafly/src/terarepo/src/volumemanager/vmBlockVolume.h
    v3d_main/terafly/src/terarepo/src/volumemanager/vmCacheManager.cpp
    v3d_main/terafly/src/terarepo/src/volumemanager/vmCacheManager.h
    v3d_main/terafly/src/terarepo/src/volumemanager/vmMCVolume.cpp
    v3d_main/terafly/src/terarepo/src/volumemanager/vmMCVolume.h
    v3d_main/terafly/src/terarepo/src/volumemanager/vmStack.cpp
    v3d_main/terafly/src/terarepo/src/volumemanager/vmStack.h
    v3d_main/terafly/src/terarepo/src/volumemanager/vmStackedVolume.cpp
    v3d_main/terafly/src/terarepo/src/volumemanager/vmStackedVolume.h
    v3d_main/terafly/src/terarepo/src/volumemanager/vmVirtualStack.cpp
    v3d_main/terafly/src/terarepo/src/volumemanager/vmVirtualStack.h
    v3d_main/terafly/src/terarepo/src/volumemanager/vmVirtualVolume.cpp
    v3d_main/terafly/src/terarepo/src/volumemanager/vmVirtualVolume.h
    v3d_main/terafly/src/terarepo/src/volumemanager/volumemanager.config.cpp
    v3d_main/terafly/src/terarepo/src/volumemanager/volumemanager.config.h
    v3d_main/vrrenderer/Communicate.cpp
    v3d_main/vrrenderer/Communicate.h
    v3d_main/vrrenderer/VRwidget.cpp
    v3d_main/vrrenderer/VRwidget.h
    v3d_main/webservice/console/cdsC.cpp
    v3d_main/webservice/console/cdsConsoleDataServiceProxy.cpp
    v3d_main/webservice/console/cdsConsoleDataServiceProxy.h
    v3d_main/webservice/console/cdsH.h
    v3d_main/webservice/console/cdsStub.h
    v3d_main/webservice/console/envC.cpp
    v3d_main/webservice/console/envH.h
    v3d_main/webservice/console/envStub.h
    v3d_main/webservice/console/obsC.cpp
    v3d_main/webservice/console/obsConsoleObserverService.cpp
    v3d_main/webservice/console/obsConsoleObserverService.h
    v3d_main/webservice/console/obsH.h
    v3d_main/webservice/console/obsStub.h
    v3d_main/webservice/gsoap2/stdsoap2.cpp
    v3d_main/webservice/gsoap2/stdsoap2.h
    v3d_main/webservice/impl/ConsoleObserverServiceImpl.cpp
    v3d_main/webservice/impl/ConsoleObserverServiceImpl.h
    v3d_main/webservice/impl/EntityAdapter.cpp
    v3d_main/webservice/impl/EntityAdapter.h
    v3d_main/worm_straighten_c/bdb_minus.cpp
    v3d_main/worm_straighten_c/bdb_minus.h
    v3d_main/worm_straighten_c/bfs.h
    v3d_main/worm_straighten_c/bfs_1root.cpp
    v3d_main/worm_straighten_c/mst_prim_c.cpp
    v3d_main/worm_straighten_c/mst_prim_c.h
    v3d_main/worm_straighten_c/spline_cubic.cpp
    v3d_main/worm_straighten_c/spline_cubic.h
    v3d_main/v3d/ChannelTable.cpp
    v3d_main/v3d/ChannelTable.h
    v3d_main/v3d/CommandManager.cpp
    v3d_main/v3d/CommandManager.h
    v3d_main/v3d/DownloadManager.cpp
    v3d_main/v3d/DownloadManager.h
    v3d_main/v3d/atlas_viewer.cpp
    v3d_main/v3d/atlas_viewer.h
    v3d_main/v3d/colormap.cpp
    v3d_main/v3d/colormap.h
    v3d_main/v3d/compute_win_diff.h
    v3d_main/v3d/compute_win_pca.h
    v3d_main/v3d/dialog_curve_trace.ui
    v3d_main/v3d/dialog_curve_trace_para.h
    v3d_main/v3d/dialog_imagecrop_bbox.h
    v3d_main/v3d/dialog_imagecrop_bbox.ui
    v3d_main/v3d/dialog_imageresample.h
    v3d_main/v3d/dialog_imageresample.ui
    v3d_main/v3d/dialog_keypoint_features.cpp
    v3d_main/v3d/dialog_keypoint_features.h
    v3d_main/v3d/dialog_keypoint_features.ui
    v3d_main/v3d/dialog_maskroi.h
    v3d_main/v3d/dialog_maskroi.ui
    v3d_main/v3d/dialog_pointcloudatlas_linkerloader.ui
    v3d_main/v3d/dialog_rotate.cpp
    v3d_main/v3d/dialog_rotate.h
    v3d_main/v3d/dialog_update_checking.ui
    v3d_main/v3d/dialog_update_downloading.ui
    v3d_main/v3d/dialog_update_list.ui
    v3d_main/v3d/dialog_update_options.ui
    v3d_main/v3d/dialog_update_v3d.ui
    v3d_main/v3d/dialog_url_entry.ui
    v3d_main/v3d/dialog_vano_linkerloader.ui
    v3d_main/v3d/histogramsimple.cpp
    v3d_main/v3d/histogramsimple.h
    v3d_main/v3d/import_filelist_dialog.h
    v3d_main/v3d/import_filelist_dialog.ui
    v3d_main/v3d/import_filelistname.cpp
    v3d_main/v3d/import_images_tool.ui
    v3d_main/v3d/import_images_tool_dialog.h
    v3d_main/v3d/import_tiffseries.cpp
    v3d_main/v3d/landmark_property.ui
    v3d_main/v3d/landmark_property_dialog.cpp
    v3d_main/v3d/landmark_property_dialog.h
    v3d_main/v3d/main.cpp
    v3d_main/v3d/mainwindow.cpp
    v3d_main/v3d/mainwindow.h
    v3d_main/v3d/mainwindow_interface.cpp
    v3d_main/v3d/mapview.cpp
    v3d_main/v3d/mapview.h
    v3d_main/v3d/my4dimage.cpp
    v3d_main/v3d/old_arthurstyle.cpp
    v3d_main/v3d/old_arthurstyle.h
    v3d_main/v3d/old_arthurwidgets.cpp
    v3d_main/v3d/old_arthurwidgets.h
    v3d_main/v3d/opt_rotate.h
    v3d_main/v3d/pluginfunchandler.cpp
    v3d_main/v3d/pluginfunchandler.h
    v3d_main/v3d/rotate_image.cpp
    v3d_main/v3d/rotate_image.h
    v3d_main/v3d/surface_obj_annotation.ui
    v3d_main/v3d/surfaceobj_annotation_dialog.h
    v3d_main/v3d/surfaceobj_geometry_dialog.cpp
    v3d_main/v3d/surfaceobj_geometry_dialog.h
    v3d_main/v3d/surfaceobj_geometry_dialog.ui
    v3d_main/v3d/template_matching_cellseg.ui
    v3d_main/v3d/template_matching_cellseg_dialog.h
    v3d_main/v3d/v3d_actions.cpp
    v3d_main/v3d/v3d_actions.h
    v3d_main/v3d/v3d_application.cpp
    v3d_main/v3d/v3d_application.h
    v3d_main/v3d/v3d_commandlineparser.cpp
    v3d_main/v3d/v3d_commandlineparser.h
    v3d_main/v3d/v3d_compile_constraints.h
    v3d_main/v3d/v3d_core.cpp
    v3d_main/v3d/v3d_core.h
    v3d_main/v3d/v3d_global_preference.ui
    v3d_main/v3d/v3d_global_preference_dialog.h
    v3d_main/v3d/v3d_version_info.cpp
    v3d_main/v3d/v3d_version_info.h
    v3d_main/v3d/v3dimg_proc_neuron.cpp
    v3d_main/v3d/v3dimgproc_entry.cpp
    v3d_main/v3d/vano_linker_loader_dialog.cpp
    v3d_main/v3d/vano_linker_loader_dialog.h
    v3d_main/v3d/vr_vaa3d_call.cpp
    v3d_main/v3d/vr_vaa3d_call.h
    v3d_main/v3d/xformwidget.h
    v3d_main/v3d/v3d_picture.rc
)

set(INCLUDE_PATHS
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/common_lib/include/hdf5
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/common_lib/include/glew
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/common_lib/include/nlohmann_json
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/common_lib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/serverconnection
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/basic_c_fun
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/3drenderer
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/basic_c_fun/customary_structs
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/v3d
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/terafly/src/terarepo/src/iomanager
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/terafly/src/terarepo/src/utils/volumeconverter
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/terafly/src/terarepo/src/volumemanager
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/terafly/src/terarepo/src/crossmips
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/terafly/src/terarepo/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/terafly/src/terarepo/src/imagemanager
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/terafly/src/terarepo/src/stitcher
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/terafly/src/control
    ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/terafly/src/presentation
)

target_include_directories(Vaa3D-x PRIVATE
    ${INCLUDE_PATHS}
)

# 设置 CMAKE_PREFIX_PATH 指向 Qt6 的安装目录
set(CMAKE_PREFIX_PATH "D:/QT6/6.5.1" CACHE STRING "Qt6 installation path")

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core5Compat Gui Network OpenGL OpenGLWidgets Svg Widgets Xml)

# 启用多线程支持
set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

target_compile_definitions(Vaa3D-x PRIVATE
    ENABLE_BDV_HDF5
    ENABLE_IMS_HDF5
    TEEM_STATIC
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
    "v3d_main/terafly/changelog.txt"
    "v3d_main/terafly/icons/about.png"
    "v3d_main/terafly/icons/clear.png"
    "v3d_main/terafly/icons/close.png"
    "v3d_main/terafly/icons/cursor_marker_add.png"
    "v3d_main/terafly/icons/cursor_marker_add_2.png"
    "v3d_main/terafly/icons/cursor_marker_delete.png"
    "v3d_main/terafly/icons/cursor_marker_delete_roi.png"
    "v3d_main/terafly/icons/extend.png"
    "v3d_main/terafly/icons/help.png"
    "v3d_main/terafly/icons/help2.png"
    "v3d_main/terafly/icons/import.png"
    "v3d_main/terafly/icons/load.png"
    "v3d_main/terafly/icons/marker_add.png"
    "v3d_main/terafly/icons/marker_add_2.png"
    "v3d_main/terafly/icons/marker_delete.png"
    "v3d_main/terafly/icons/marker_delete_roi.png"
    "v3d_main/terafly/icons/marker_roi_view.png"
    "v3d_main/terafly/icons/open_ano.png"
    "v3d_main/terafly/icons/open_image_file.png"
    "v3d_main/terafly/icons/open_image_folder.png"
    "v3d_main/terafly/icons/open_image_hdf5.png"
    "v3d_main/terafly/icons/open_image_terafly.png"
    "v3d_main/terafly/icons/open_image_unconverted.png"
    "v3d_main/terafly/icons/open_volume.png"
    "v3d_main/terafly/icons/open_volume_recent.png"
    "v3d_main/terafly/icons/options.png"
    "v3d_main/terafly/icons/redo.png"
    "v3d_main/terafly/icons/reset.png"
    "v3d_main/terafly/icons/save.png"
    "v3d_main/terafly/icons/saveas.png"
    "v3d_main/terafly/icons/start.png"
    "v3d_main/terafly/icons/stop.png"
    "v3d_main/terafly/icons/terafly.png"
    "v3d_main/terafly/icons/toolbar.png"
    "v3d_main/terafly/icons/undo.png"
    "v3d_main/terafly/icons/virtual.pyramid.empty.png"
    "v3d_main/terafly/icons/virtual.pyramid.full.png"
    "v3d_main/terafly/icons/virtual.pyramid.lowres.png"
    "v3d_main/terafly/icons/wait.gif"
)

qt_add_resources(Vaa3D-x "icons"
    PREFIX
        "/"
    BASE
        "v3d_main/terafly"
    FILES
        ${icons_resource_files}
)
set(resources_resource_files
    "v3d_main/neuron_annotator/resources/beaker.png"
    "v3d_main/neuron_annotator/resources/brain.png"
    "v3d_main/neuron_annotator/resources/brick.png"
    "v3d_main/neuron_annotator/resources/brick_grey.png"
    "v3d_main/neuron_annotator/resources/bricks.png"
    "v3d_main/neuron_annotator/resources/bullet_error.png"
    "v3d_main/neuron_annotator/resources/color_vertex_cmb.txt"
    "v3d_main/neuron_annotator/resources/folder.png"
    "v3d_main/neuron_annotator/resources/folder_brick.png"
    "v3d_main/neuron_annotator/resources/folder_image.png"
    "v3d_main/neuron_annotator/resources/folder_open.png"
    "v3d_main/neuron_annotator/resources/folder_open_page.png"
    "v3d_main/neuron_annotator/resources/folder_open_page_white.png"
    "v3d_main/neuron_annotator/resources/folder_page.png"
    "v3d_main/neuron_annotator/resources/fruit_fly_small.png"
    "v3d_main/neuron_annotator/resources/image.png"
    "v3d_main/neuron_annotator/resources/images.png"
    "v3d_main/neuron_annotator/resources/layers.png"
    "v3d_main/neuron_annotator/resources/page.png"
    "v3d_main/neuron_annotator/resources/page_copy.png"
    "v3d_main/neuron_annotator/resources/page_go.png"
    "v3d_main/neuron_annotator/resources/page_white.png"
    "v3d_main/neuron_annotator/resources/page_white_code.png"
    "v3d_main/neuron_annotator/resources/page_white_edit.png"
    "v3d_main/neuron_annotator/resources/page_white_stack.png"
    "v3d_main/neuron_annotator/resources/page_white_text.png"
    "v3d_main/neuron_annotator/resources/tex_fragment_cmb.txt"
)

qt_add_resources(Vaa3D-x "resources"
    PREFIX
        "/neuron_annotator"
    BASE
        "v3d_main/neuron_annotator"
    FILES
        ${resources_resource_files}
)
set_source_files_properties("pic/button_add.png"
    PROPERTIES QT_RESOURCE_ALIAS "button_add.png"
)
set(v3d_resource_files
    "v3d_main/v3d/pic/Lockoff.png"
    "v3d_main/v3d/pic/Lockon.png"
    "v3d_main/v3d/pic/arrow_down.png"
    "v3d_main/v3d/pic/arrow_left.png"
    "v3d_main/v3d/pic/arrow_right.png"
    "v3d_main/v3d/pic/arrow_up.png"
    "v3d_main/v3d/pic/atlasView.png"
    "v3d_main/v3d/pic/button_add.png"
    "v3d_main/v3d/pic/copy.png"
    "v3d_main/v3d/pic/customize.png"
    "v3d_main/v3d/pic/cut.png"
    "v3d_main/v3d/pic/default_xy_pic.jpg"
    "v3d_main/v3d/pic/default_yz_pic.jpg"
    "v3d_main/v3d/pic/default_zx_pic.jpg"
    "v3d_main/v3d/pic/help.png"
    "v3d_main/v3d/pic/import.png"
    "v3d_main/v3d/pic/new.png"
    "v3d_main/v3d/pic/open.png"
    "v3d_main/v3d/pic/paste.png"
    "v3d_main/v3d/pic/rotate_icon.png"
    "v3d_main/v3d/pic/save.png"
    "v3d_main/v3d/pic/v3dIcon128.png"
    "v3d_main/v3d/pic/web.png"
    "v3d_main/v3d/v3dIcon128.ico"
)

qt_add_resources(Vaa3D-x "v3d"
    PREFIX
        "/"
    BASE
        "v3d_main/v3d"
    FILES
        ${v3d_resource_files}
)
set(3drenderer_resource_files
    "v3d_main/3drenderer/icons/click1.png"
    "v3d_main/3drenderer/icons/click2.png"
    "v3d_main/3drenderer/icons/click3.png"
    "v3d_main/3drenderer/icons/neuronwbg.png"
    "v3d_main/3drenderer/icons/neuronwbgref.png"
    "v3d_main/3drenderer/icons/neuronwobg.png"
    "v3d_main/3drenderer/icons/neuronwref.png"
    "v3d_main/3drenderer/icons/start.png"
    "v3d_main/3drenderer/icons/stroke1.png"
    "v3d_main/3drenderer/icons/stroke2.png"
    "v3d_main/3drenderer/icons/stroke3.png"
    "v3d_main/3drenderer/icons/strokeN.png"
    "v3d_main/3drenderer/icons/trace.png"
    "v3d_main/3drenderer/shader/color_fragment.txt"
    "v3d_main/3drenderer/shader/color_vertex.txt"
    "v3d_main/3drenderer/shader/lighting.txt"
    "v3d_main/3drenderer/shader/obj_fragment.txt"
    "v3d_main/3drenderer/shader/tex_fragment.txt"
    "v3d_main/3drenderer/shader/vertex_normal.txt"
)

qt_add_resources(Vaa3D-x "3drenderer"
    PREFIX
        "/"
    BASE
        "v3d_main/3drenderer"
    FILES
        ${3drenderer_resource_files}
)

if(MACOS)
    target_sources(Vaa3D-x PUBLIC
        yosemiteFileURLfix.mm
    )

    # 设置库的搜索路径
    link_directories(${CMAKE_SOURCE_DIR}/v3d_main/common_lib/lib_mac64)

    target_include_directories(Vaa3D-x PRIVATE
        v3d_main/common_lib/include/hdf5
    )

    target_compile_definitions(Vaa3D-x PRIVATE
        MACOS_SYSTEM
        _ENABLE_MACX_DRAG_DROP_FIX_
    )

    target_link_libraries(Vaa3D-x PRIVATE
        "-framework CoreServices"
        bz2
        hdf5
        mylib
        szip
        teem
        v3dnewmat
        v3dtiff
        z
        Threads::Threads
    )

    link_directories("/System/Library/Frameworks")
    target_link_libraries(Vaa3D-x PRIVATE
        "-framework Foundation"
    )

endif()

if(WIN32)
    target_sources(Vaa3D-x PUBLIC
        v3d_main/terafly/src/presentation/fileserver.cpp v3d_main/terafly/src/presentation/fileserver.h
        v3d_main/vrrenderer/Cylinder.cpp v3d_main/vrrenderer/Cylinder.h
        v3d_main/vrrenderer/Matrices.cpp v3d_main/vrrenderer/Matrices.h
        v3d_main/vrrenderer/RenderableObject.cpp v3d_main/vrrenderer/RenderableObject.h
        v3d_main/vrrenderer/Sphere.cpp v3d_main/vrrenderer/Sphere.h
        v3d_main/vrrenderer/V3dR_Communicator.cpp v3d_main/vrrenderer/V3dR_Communicator.h
        v3d_main/vrrenderer/VRFinger.cpp v3d_main/vrrenderer/VRFinger.h
        v3d_main/vrrenderer/VR_MainWindow.cpp v3d_main/vrrenderer/VR_MainWindow.h
        v3d_main/vrrenderer/lodepng.cpp v3d_main/vrrenderer/lodepng.h
        v3d_main/vrrenderer/v3dr_gl_vr.cpp v3d_main/vrrenderer/v3dr_gl_vr.h
    )

    set_target_properties(Vaa3D-x PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    target_include_directories(Vaa3D-x PRIVATE
        v3d_main/common_lib/include/hdf5-win64
        v3d_main/common_lib/include/SDL
        v3d_main/common_lib/include/openvr
    )

    target_compile_definitions(Vaa3D-x PRIVATE
        WINDOWS_SYSTEM
        __ALLOW_VR_FUNCS__
    )

#    find_package(ZLIB REQUIRED)

#    target_link_libraries(Vaa3D-x PRIVATE ZLIB::ZLIB)

    # 设置库的搜索路径
    link_directories(${CMAKE_SOURCE_DIR}/v3d_main/common_lib/mingw64)

    target_link_libraries(Vaa3D-x PRIVATE
        ${CMAKE_SOURCE_DIR}/v3d_main/common_lib/mingw64/glew32.dll
        ${CMAKE_SOURCE_DIR}/v3d_main/common_lib/mingw64/SDL2.dll
        ${CMAKE_SOURCE_DIR}/v3d_main/common_lib/mingw64/openvr_api.dll
        Opengl32
        ${CMAKE_SOURCE_DIR}/v3d_main/common_lib/mingw64/libzlib.lib
        glu32
        ${CMAKE_SOURCE_DIR}/v3d_main/common_lib/mingw64/libhdf5.lib
        ${CMAKE_SOURCE_DIR}/v3d_main/common_lib/mingw64/libszip.lib
        ${CMAKE_SOURCE_DIR}/v3d_main/common_lib/mingw64/libteem.lib
        z
        ${CMAKE_SOURCE_DIR}/v3d_main/common_lib/mingw64/libv3dnewmat.lib
        ${CMAKE_SOURCE_DIR}/v3d_main/common_lib/mingw64/libv3dtiff.lib
        wsock32
        Threads::Threads
    )
endif()

if(UNIX AND NOT MACOS)
    target_sources(Vaa3D-x PUBLIC
        v3d_main/vrrenderer/Cylinder.cpp v3d_main/vrrenderer/Cylinder.h
        v3d_main/vrrenderer/Matrices.cpp v3d_main/vrrenderer/Matrices.h
        v3d_main/vrrenderer/RenderableObject.cpp v3d_main/vrrenderer/RenderableObject.h
        v3d_main/vrrenderer/Sphere.cpp v3d_main/vrrenderer/Sphere.h
        v3d_main/vrrenderer/V3dR_Communicator.cpp v3d_main/vrrenderer/V3dR_Communicator.h
        v3d_main/vrrenderer/VRFinger.cpp v3d_main/vrrenderer/VRFinger.h
        v3d_main/vrrenderer/VR_MainWindow.cpp v3d_main/vrrenderer/VR_MainWindow.h
        v3d_main/vrrenderer/lodepng.cpp v3d_main/vrrenderer/lodepng.h
        v3d_main/vrrenderer/v3dr_gl_vr.cpp v3d_main/vrrenderer/v3dr_gl_vr.h
    )

    set_target_properties(Vaa3D-x PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    target_include_directories(Vaa3D-x PRIVATE
        v3d_main/common_lib/include/glew
        v3d_main/common_lib/include/SDL
        v3d_main/common_lib/include/hdf5
        v3d_main/common_lib/include/openvr
    )

    target_compile_definitions(Vaa3D-x PRIVATE
        LINUX_SYSTEM
        __ALLOW_VR_FUNCS__
    )

    # 添加库路径和库
    link_directories(${CMAKE_SOURCE_DIR}/v3d_main/common_lib/lib_ubuntu)

    target_link_libraries(Vaa3D-x PRIVATE
        GLEW
        SDL2-2.0
        dl
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
        openvr_api
        Threads::Threads
    )
endif()

if(UNIX)
    target_sources(Vaa3D-x PUBLIC
        v3d_main/basic_c_fun/imageio_mylib.cpp v3d_main/basic_c_fun/imageio_mylib.h
    )

    target_include_directories(Vaa3D-x PRIVATE
        v3d_main/common_lib/include/glew
    )
endif()

install(TARGETS Vaa3D-x
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 安装资源文件（如图标和资源文件）
install(FILES
    ${CMAKE_SOURCE_DIR}/v3d_main/v3d/v3dIcon128.ico
    ${CMAKE_SOURCE_DIR}/v3d_main/v3d/v3d_picture.rc
    DESTINATION ${CMAKE_INSTALL_PREFIX}/resources
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
