# Link neuron annotator functionality into Vaa3D
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../v3d)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../v3dbase)
include_directories(../v3d)
include_directories(../3drenderer)
include_directories(${Boost_INCLUDE_DIR})

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

# 3D Stereo detection hoses Mac/ Off by default for now.
if(WIN32)
    set(DEFAULT_ENABLE_STEREO TRUE)
else()
    set(DEFAULT_ENABLE_STEREO FALSE)
endif()
set(NA_ENABLE_3D_STEREO ${DEFAULT_ENABLE_STEREO} CACHE BOOL "Whether to compile in support for hardware 3D stereoscopic viewing")
if(NA_ENABLE_3D_STEREO)
    add_definitions(-DENABLE_STEREO)
endif()

# Permit loading 3D volumes from movie files
find_library(AVDEVICE_LIBRARY avdevice PATHS /usr/local/lib)
find_library(AVFORMAT_LIBRARY avformat PATHS /usr/local/lib)
find_library(AVFILTER_LIBRARY avfilter PATHS /usr/local/lib)
find_library(AVCODEC_LIBRARY avcodec PATHS /usr/local/lib)
find_library(SWSCALE_LIBRARY swscale PATHS /usr/local/lib)
find_library(AVUTIL_LIBRARY avutil PATHS /usr/local/lib)
find_package(ZLIB)
find_path(AVCODEC_INCLUDE_DIR
    libavcodec/avcodec.h
    PATHS
        /usr/local/include
)
set(have_avcodec false)
if(AVCODEC_LIBRARY AND AVCODEC_INCLUDE_DIR)
    set(have_avcodec true)
endif()
set(V3D_ENABLE_LOAD_MOVIE ${have_avcodec} CACHE BOOL "Whether to load 3D volumes from movies using libavcodec")
if(V3D_ENABLE_LOAD_MOVIE)
    include_directories(${AVCODEC_INCLUDE_DIR})
    add_definitions(-DV3D_ENABLE_LOAD_MOVIE)
endif()

add_library(NeuronAnnotatorLib STATIC
    ${NA_UI_SRCS} ${NA_MOC_SRCS} ${NA_SRCS}
)
add_custom_target(NaGeneratedFiles DEPENDS ${NA_UI_SRCS})
add_dependencies(NeuronAnnotatorLib NaGeneratedFiles)
if(V3D_ENABLE_LOAD_MOVIE)
    target_link_libraries(NeuronAnnotatorLib ${AVCODEC_LIBRARY})
endif()

add_subdirectory(tests)

