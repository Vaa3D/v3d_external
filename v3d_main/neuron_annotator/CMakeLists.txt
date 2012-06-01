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

file(GLOB NA_SRCS 
    "*.cpp" 
    # "*/*.cpp" # don't want "tests"
    "analysis/*.cpp" 
    "data_model/*.cpp" 
    "entity_model/*.cpp" 
    "geometry/*.cpp" 
    "gui/*.cpp" 
    "utility/*.cpp" 
    "gui/trees/*.cpp" 
    "../webservice/*/*.cpp")

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

# Movie loading required libavcodec from ffmpeg
# FFMpeg requires YASM
set(USE_FFMPEG FALSE CACHE BOOL "Whether to load 3D volumes from movies using libavcodec")
if(USE_FFMPEG)
    include(ExternalProject)
    ExternalProject_Add(
        Yasm-1-2-0
        URL http://www.tortall.net/projects/yasm/releases/yasm-1.2.0.tar.gz
        URL_MD5 4cfc0686cf5350dd1305c4d905eb55a6
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/yasm/src
        INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/yasm/install
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND <SOURCE_DIR>/configure
            --prefix=<INSTALL_DIR>
        BUILD_IN_SOURCE 1
    )
    set(av_dir "${CMAKE_CURRENT_BINARY_DIR}/libavcodec")
    ExternalProject_Add(
        LibAvcodec-0-11
        DEPENDS Yasm-1-2-0
        URL http://ffmpeg.org/releases/ffmpeg-0.11.tar.gz
        SOURCE_DIR ${av_dir}/src
        BINARY_DIR ${av_dir}/build
        INSTALL_DIR ${av_dir}/install
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND <SOURCE_DIR>/configure
            --prefix=<INSTALL_DIR>
            --yasmexe=${CMAKE_CURRENT_BINARY_DIR}/yasm/install/bin/yasm
            --arch=x86_64
            --disable-vda
    )
    set(AVCODEC_INCLUDE_DIR "${av_dir}/install/include")
    set(AVDEVICE_LIBRARY ${av_dir}/install/lib/libavdevice.a)
    set(AVFORMAT_LIBRARY ${av_dir}/install/lib/libavformat.a)
    set(AVFILTER_LIBRARY ${av_dir}/install/lib/libavfilter.a)
    set(AVCODEC_LIBRARY ${av_dir}/install/lib/libavcodec.a)
    set(SWSCALE_LIBRARY ${av_dir}/install/lib/libswscale.a)
    set(AVUTIL_LIBRARY ${av_dir}/install/lib/libavutil.a)

    find_package(ZLIB)

    include_directories(${AVCODEC_INCLUDE_DIR})
    add_definitions(-DUSE_FFMPEG)
endif()

add_library(NeuronAnnotatorLib STATIC
    ${NA_UI_SRCS} ${NA_MOC_SRCS} ${NA_SRCS}
)
add_custom_target(NaGeneratedFiles DEPENDS ${NA_UI_SRCS})
add_dependencies(NeuronAnnotatorLib NaGeneratedFiles)
if(USE_FFMPEG)
    add_dependencies(NeuronAnnotatorLib LibAvcodec-0-11)
    target_link_libraries(NeuronAnnotatorLib
        ${AVUTIL_LIBRARY}
        ${AVFORMAT_LIBRARY}
        ${SWSCALE_LIBRARY}
        ${AVCODEC_LIBRARY}
        ${BZIP2_LIBRARIES}
        ${ZLIB_LIBRARY}
    )
    if(APPLE)
        find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
        find_library(CORE_VIDEO_FRAMEWORK CoreVideo)
        # find_library(VIDEO_DECODE_ACCELERATION_FRAMEWORK VideoDecodeAcceleration)
        target_link_libraries(NeuronAnnotatorLib
            # ${VIDEO_DECODE_ACCELERATION_FRAMEWORK}
            ${CORE_VIDEO_FRAMEWORK}
            ${CORE_FOUNDATION_FRAMEWORK}
        )
    endif()
endif()

add_subdirectory(tests)

