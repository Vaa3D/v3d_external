# Put test binaries in the build_cmake directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_executable(test_load_chk4 test_load_chk4.cpp)
target_link_libraries(test_load_chk4 NeuronAnnotatorLib)
# add_test(test_load_chk4 test_load_chk4)

if(USE_FFMPEG)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(CORE_VIDEO_FRAMEWORK CoreVideo)
    find_library(VIDEO_DECODE_ACCELERATION_FRAMEWORK VideoDecodeAcceleration)
    find_package(BZip2)
    add_executable(test_load_mp4
        test_load_mp4.cpp
        ../utility/loadV3dFFMpeg.cpp
        ../utility/FFMpegVideo.cpp
    )
    target_link_libraries(test_load_mp4
        ${AVUTIL_LIBRARY}
        ${AVFORMAT_LIBRARY}
        ${SWSCALE_LIBRARY}
        ${AVCODEC_LIBRARY}
        ${BZIP2_LIBRARIES}
        ${ZLIB_LIBRARY}
        # Below are required for My4DImage
        v3dbase
        v3dbase2
        V3DInterface
        v3d_plugin_loader
        3drenderer
        neuron_tracing
        NeuronAnnotatorLib
        ${QT_QTNETWORK_LIBRARY}
        ${QT4_DEMOS_LIBRARY}
        mylib_tiff
        worm_straighten_c
        cellseg
    )
    if(APPLE)
        target_link_libraries(test_load_mp4
            ${VIDEO_DECODE_ACCELERATION_FRAMEWORK}
            ${CORE_VIDEO_FRAMEWORK}
            ${CORE_FOUNDATION_FRAMEWORK}
        )
    endif()
endif()
