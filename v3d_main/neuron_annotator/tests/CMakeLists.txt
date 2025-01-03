# Put test binaries in the build_cmake directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# add_executable(test_load_chk4 test_load_chk4.cpp)
# target_link_libraries(test_load_chk4 NeuronAnnotatorLib)
# add_test(test_load_chk4 test_load_chk4)

if(USE_FFMPEG)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(CORE_VIDEO_FRAMEWORK CoreVideo)
    # find_library(VIDEO_DECODE_ACCELERATION_FRAMEWORK VideoDecodeAcceleration)
    find_package(BZip2)
    set(WAY_TOO_MANY_LINK_LIBRARIES
        # due to way too much tight coupling of source files
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

    # These tests use My4DImage, and therefore must link to all of Vaa3D
    if (APPLE)
        set(test_progs
            # test_load_mp4
            # test_write_mp4
        )
    else()
        set(test_progs "")
    endif()
    find_package(Threads)
    foreach(test_prog ${test_progs})
        add_executable(${test_prog}
            ${test_prog}.cpp
            ../utility/loadV3dFFMpeg.cpp
            ../utility/FFMpegVideo.cpp
            # ${TEST_MPEG_MOC_SRCS}
        )
        target_link_libraries(${test_prog}
            ${WAY_TOO_MANY_LINK_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
        )
        if(APPLE)
            target_link_libraries(${test_prog}
                # ${VIDEO_DECODE_ACCELERATION_FRAMEWORK}
                ${CORE_VIDEO_FRAMEWORK}
                ${CORE_FOUNDATION_FRAMEWORK}
            )
        endif()
    endforeach()

endif()
