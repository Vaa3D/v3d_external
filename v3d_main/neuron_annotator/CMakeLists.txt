# Link neuron annotator functionality into V3D
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../v3d)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../v3dbase)
include_directories(../v3d)
include_directories(../3drenderer)

file(GLOB NA_UI_FILES gui/*.ui)
QT4_WRAP_UI(NA_UI_SRCS 
    ${NA_UI_FILES}
    # Repeat some ui files to avoid circular dependencies
    ../v3d/v3d_global_preference.ui
    ../v3d/template_matching_cellseg.ui
)

# Look for "Q_OBJECT" in headers to determine which ones to MOC
# Begin with a list of all header file candidates
message(STATUS "Finding header files containing 'Q_OBJECT' macro...")
file(GLOB NA_MOC_CANDIDATES gui/*.h data_model/*.h gui/trees/*.h)
foreach(NA_MOC_CAND ${NA_MOC_CANDIDATES})
    # Examine beginning of each header file
    file(READ "${NA_MOC_CAND}" HEADER_TEXT LIMIT 10000)
    # message("${HEADER_TEXT}")
    # Does it contain the string "Q_OBJECT"?
    string(REGEX MATCH "Q_OBJECT" QOB_RESULT ${HEADER_TEXT})
    # message("${QOB_RESULT}")
    if(QOB_RESULT)
        # This is a Q_OBJECT header, and thus needs to be parsed for Qt
        set(NA_MOC_FILES ${NA_MOC_FILES} ${NA_MOC_CAND})
        # message("${NA_MOC_CAND}")
    endif()
endforeach()
message(STATUS "Finished examining header files")
# message("${NA_MOC_FILES}")

# Only headers with "Q_OBJECT" macro go here in QT4_WRAP_CPP stanza
QT4_WRAP_CPP(NA_MOC_SRCS
    ${NA_MOC_FILES}
    geometry/CameraModel.h
    utility/ConsoleObserver.h
    utility/DataThread.h
    utility/ImageLoader.h
    DataFlowModel.h
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

