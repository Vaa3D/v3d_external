# Link neuron annotator functionality into V3D
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../v3d)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../v3dbase)
include_directories(../v3d)
include_directories(../3drenderer)

QT4_WRAP_UI(NA_UI_SRCS 
    gui/AnnotationWidget.ui
    gui/NaMainWindow.ui
    gui/GammaWidget.ui
    gui/AngleWidget.ui
    gui/ZoomWidget.ui
    # Repeat some ui files to avoid circular dependencies
    ../v3d/v3d_global_preference.ui
    ../v3d/template_matching_cellseg.ui
)

# Only headers with "Q_OBJECT" macro go here in QT4_WRAP_CPP stanza
QT4_WRAP_CPP(NA_MOC_SRCS
    data_model/NaLockableData.h
    data_model/NaVolumeData.h
    data_model/MipFragmentData.h
    data_model/MipFragmentColors.h
    data_model/DataColorModel.h
    data_model/GalleryMipImages.h
    data_model/MipMergedData.h
    data_model/NeuronSelectionModel.h
    data_model/ZSliceColors.h
    data_model/VolumeColors.h
    data_model/Texture2dColors.h
    data_model/NaSharedDataSignaller.h
    geometry/CameraModel.h
    gui/AngleWidget.h
    gui/AnnotationWidget.h
    gui/CompartmentMapComboBox.h
    gui/CompartmentMapWidget.h
    gui/GalleryButton.h
    gui/GammaWidget.h
    gui/ZoomSpinBox.h
    gui/ZoomWidget.h
    gui/MouseClickManager.h
    gui/Na2DViewer.h
    gui/Na3DWidget.h
    gui/NaLargeMIPWidget.h
    gui/NaMainWindow.h
    gui/NaZStackWidget.h
    gui/FragmentGalleryWidget.h
    gui/RendererNeuronAnnotator.h
    gui/trees/AnnotatedBranchTreeModel.h
    gui/trees/AnnotatedBranchTreeView.h
    gui/trees/EntityTreeModel.h
    gui/trees/EntityTreeView.h
    gui/trees/OntologyTreeModel.h
    gui/trees/OntologyTreeView.h
    utility/ConsoleObserver.h
    utility/DataThread.h
    AnnotationSession.h
    ExportFile.h
    NeuronSelector.h
    ../webservice/impl/ConsoleObserverServiceImpl.h
)

file(GLOB NA_SRCS "*.cpp" "*/*.cpp" "gui/trees/*.cpp" "../webservice/*/*.cpp")
add_definitions(-DWITH_PURE_VIRTUAL -DWITH_NONAMESPACES) # needed for webservice soap stuff

add_library(NeuronAnnotatorLib STATIC 
    ${NA_UI_SRCS} ${NA_MOC_SRCS} ${NA_SRCS}
)
add_custom_target(NaGeneratedFiles DEPENDS ${NA_UI_SRCS})
add_dependencies(NeuronAnnotatorLib NaGeneratedFiles)

