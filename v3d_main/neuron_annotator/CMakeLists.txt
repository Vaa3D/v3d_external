# Link neuron annotator functionality into V3D
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../v3d)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../v3dbase)
include_directories(../v3d)
include_directories(../3drenderer)

QT4_WRAP_UI(NA_UI_SRCS 
    NaMainWindow.ui
    GammaWidget.ui
)
QT4_WRAP_CPP(NA_MOC_SRCS
    AnnotationSession.h
    CameraModel.h
    GalleryButton.h
    GammaWidget.h
    Na3DWidget.h
    NaLargeMIPWidget.h
    NaMainWindow.h
    NaZStackWidget.h
    NeuronSelector.h
)

file(GLOB NA_SRCS "*.cpp")

add_library(NeuronAnnotatorLib STATIC 
    ${NA_SRCS} ${NA_UI_SRCS} ${NA_MOC_SRCS})

