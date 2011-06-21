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
)

QT4_WRAP_CPP(NA_MOC_SRCS
    AngleWidget.h
    AnnotationSession.h
    geometry/CameraModel.h
    GalleryButton.h
    GammaWidget.h
    gui/ZoomSpinBox.h
    gui/ZoomWidget.h
    MipData.h
    MipLayer.h
    MouseClickManager.h
    Na3DWidget.h
    NaLargeMIPWidget.h
    NaMainWindow.h
    NaZStackWidget.h
    NeuronMipManager.h
    NeuronSelector.h
)

file(GLOB NA_SRCS "*.cpp" "gui/*.cpp" "geometry/*.cpp" "utility/*.cpp")

add_library(NeuronAnnotatorLib STATIC 
    ${NA_SRCS} ${NA_UI_SRCS} ${NA_MOC_SRCS})

