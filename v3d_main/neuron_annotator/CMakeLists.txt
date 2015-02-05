# Link neuron annotator functionality into Vaa3D
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../v3d)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../v3dbase)
include_directories(../v3d)
include_directories(../3drenderer)
include_directories(${Boost_INCLUDE_DIR})

file(GLOB NA_UI_FILES
    "gui/*.ui"
    "microCT/*.ui")
QT4_WRAP_UI(NA_UI_SRCS
    ${NA_UI_FILES}
    # Repeat some ui files to avoid circular dependencies
    ../v3d/v3d_global_preference.ui
    ../v3d/template_matching_cellseg.ui
)

# Look for "Q_OBJECT" in headers to determine which ones to MOC
# Begin with a list of all header file candidates
message(STATUS "Finding header files containing 'Q_OBJECT' macro...")
file(GLOB NA_MOC_CANDIDATES
    gui/*.h
    data_model/*.h
    gui/trees/*.h
    microCT/*.h
    utility/*.h)
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
if(USE_FFMPEG)
    set(WRAP_OPTIONS "-DUSE_FFMPEG")
endif()
QT4_WRAP_CPP(NA_MOC_SRCS
    ${NA_MOC_FILES}
    geometry/CameraModel.h
     utility/ConsoleObserver.h
    utility/DataThread.h
    utility/ImageLoader.h
    data_model/Fast3DTexture.h
    DataFlowModel.h
    ExportFile.h
    NeuronSelector.h
    ../webservice/impl/ConsoleObserverServiceImpl.h
    OPTIONS ${WRAP_OPTIONS}
)

file(GLOB NA_SRCS
    "*.cpp"
    # "*/*.cpp" # don't want "tests"
    "analysis/*.cpp"
    "animation/*.cpp"
    "data_model/*.cpp"
    "entity_model/*.cpp"
    "geometry/*.cpp"
    "gui/*.cpp"
    "microCT/*.cpp"
    "utility/*.cpp"
    "gui/trees/*.cpp"
    "../webservice/*/*.cpp"
    "render/*.cpp")

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

add_library(NeuronAnnotatorLib STATIC
    ${NA_UI_SRCS} ${NA_MOC_SRCS} ${NA_SRCS}
)
add_custom_target(NaGeneratedFiles DEPENDS ${NA_UI_SRCS})
add_dependencies(NeuronAnnotatorLib NaGeneratedFiles)

if(NA_ENABLE_3D_STEREO)
    if(APPLE)
        add_subdirectory(utility/mac_stereo)
        target_link_libraries(NeuronAnnotatorLib nanvstlib)
    endif(APPLE)
endif()

# Permit loading 3D volumes from movie files

if(USE_FFMPEG)
    add_dependencies(NeuronAnnotatorLib LibAvcodec-0-10-2-head2012Apr25)
    target_link_libraries(NeuronAnnotatorLib
        ${SWSCALE_LIBRARY}
        ${AVDEVICE_LIBRARY}
        ${AVFORMAT_LIBRARY}
        ${AVCODEC_LIBRARY}
        ${AVUTIL_LIBRARY}
        ${BZIP2_LIBRARIES}
        ${ZLIB_LIBRARY}
        ${ICONV_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
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

