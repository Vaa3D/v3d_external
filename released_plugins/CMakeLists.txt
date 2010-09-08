cmake_minimum_required(VERSION 2.4)
if(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
endif(COMMAND CMAKE_POLICY)

project(v3d_released_plugins)


find_package(Qt4 REQUIRED)
include( ${QT_USE_FILE} )

include(CTest)

include(CPack)


find_path(V3D_BINARY_DIR v3d)
set(INSTALLATION_DIRECTORY ${V3D_BINARY_DIR}/plugins)

find_path(V3D_BASIC_C_FUN_SOURCE_DIR v3d_interface.h)
if( V3D_BASIC_C_FUN_SOURCE_DIR )
  set( V3D_BASIC_C_FUN_BINARY_DIR  ${PROJECT_BINARY_DIR}/Source/basic_c_fun )
endif( V3D_BASIC_C_FUN_SOURCE_DIR )

if(NOT LIBRARY_OUTPUT_PATH)
  set (LIBRARY_OUTPUT_PATH
    ${INSTALLATION_DIRECTORY}
    CACHE INTERNAL "Output directory for the shared libraries.")
endif(NOT LIBRARY_OUTPUT_PATH)

if(NOT EXECUTABLE_OUTPUT_PATH)
  set (EXECUTABLE_OUTPUT_PATH
    ${INSTALLATION_DIRECTORY}
    CACHE INTERNAL "Output directory for the executables.")
endif(NOT EXECUTABLE_OUTPUT_PATH)


include_directories( ${V3D_BASIC_C_FUN_SOURCE_DIR} )

add_subdirectory( v3d_plugins )
add_subdirectory( special_v3d_plugins )

