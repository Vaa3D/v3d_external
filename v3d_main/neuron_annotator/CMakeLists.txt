# Link neuron annotator functionality into V3D
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../v3d)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../v3dbase)
include_directories(../v3d)
include_directories(../3drenderer)

QT4_WRAP_UI(NA_UI_SRCS 
    gui/NaMainWindow.ui
    gui/GammaWidget.ui
    gui/AngleWidget.ui
    gui/ZoomWidget.ui
    # Repeat some ui files to avoid circular dependencies
    ../v3d/v3d_global_preference.ui
    ../v3d/template_matching_cellseg.ui
)

QT4_WRAP_CPP(NA_MOC_SRCS
    geometry/CameraModel.h
    gui/AngleWidget.h
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
    AnnotationSession.h
    MipData.h
    MipLayer.h
    NeuronSelector.h
)

file(GLOB NA_SRCS "*.cpp" "gui/*.cpp" "geometry/*.cpp" "utility/*.cpp")

add_library(NeuronAnnotatorLib STATIC 
    ${NA_UI_SRCS} ${NA_MOC_SRCS} ${NA_SRCS})

