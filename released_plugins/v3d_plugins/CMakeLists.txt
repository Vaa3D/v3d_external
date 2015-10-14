# Visit plugin subdirectories if and only if they have a CMakeLists.txt file
file(GLOB CMAKE_DIRS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*/CMakeLists.txt")
# message("CMAKE_DIRS = ${CMAKE_DIRS}")
# if(NOT WIN32)
foreach(CMAKE_DIR ${CMAKE_DIRS})
    string(REPLACE "//CMakeLists.txt" "" CMAKE_DIR "${CMAKE_DIR}")
    string(REPLACE "/CMakeLists.txt" "" CMAKE_DIR "${CMAKE_DIR}")
    # message(${CMAKE_DIR})
    add_subdirectory(${CMAKE_DIR})
endforeach()
# endif()

# If ireg and imath directories exist, build those plugins too
if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ireg)
    set(PLUGIN_HEADERS ireg/ireg.h) # Q_OBJECT macro containing files only
    set(PLUGIN_DIRECTORY_NAME image_registration/ireg)
    set(PLUGIN_SOURCES
        ireg/ireg.cpp
        ireg/y_NiftiImageIO.cpp
        ireg/niftilib/nifti1_io.c
        ireg/niftilib/znzlib.c
    )
    include_directories(
        "${TIFF_INCLUDE_DIR}"
    )
    configure_v3d_plugin(ireg)
    find_library(ZLIB z)
    target_link_libraries(ireg mylib_tiff ${ZLIB})

    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/imath)
        set(PLUGIN_HEADERS imath/imath.h) # Q_OBJECT macro containing files only
        set(PLUGIN_DIRECTORY_NAME image_registration/imath)
        set(PLUGIN_SOURCES imath/imath.cpp)
        include_directories(
            ireg
            "${TIFF_INCLUDE_DIR}"
        )
        configure_v3d_plugin(imath)
        target_link_libraries(imath mylib_tiff)
    endif()
endif()

