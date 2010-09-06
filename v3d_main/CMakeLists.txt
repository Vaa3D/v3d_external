cmake_minimum_required(VERSION 2.8)

project(V3D)

if(NOT LIBRARY_OUTPUT_PATH)
  set (LIBRARY_OUTPUT_PATH ${V3D_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all libraries.")
endif(NOT LIBRARY_OUTPUT_PATH)

if(NOT EXECUTABLE_OUTPUT_PATH)
  set (EXECUTABLE_OUTPUT_PATH ${V3D_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all executables.")
endif(NOT EXECUTABLE_OUTPUT_PATH)

mark_as_advanced(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)


set(CMAKE_MODULE_PATH ${V3D_SOURCE_DIR}/CMake)


include(CTest)

include(CPack)


find_package(Qt4 REQUIRED)
include( ${QT_USE_FILE} )
include_directories( ${QT_INCLUDES} )

option( V3D_USE_OWN_TIFF "Use the TIFF library source code distributed with v3d" OFF)

if( NOT V3D_USE_OWN_TIFF )
  find_package(TIFF REQUIRED)
endif()

find_path(QT4_DEMOS_INCLUDE_DIR
  arthurwidgets.h
  PATHS
  /usr/lib/qt4/demos/shared/
  )

if(NOT QT4_DEMOS_INCLUDE_DIR)
  message(FATAL "You need to install the Qt4 demo files")
else()
  include_directories(${QT4_DEMOS_INCLUDE_DIR})
endif()

add_subdirectory( common_lib )

add_subdirectory( basic_c_fun )

add_subdirectory( v3dbase )

add_subdirectory( plugin_loader )

add_subdirectory( 3drenderer )

add_subdirectory( neuron_editing )

add_subdirectory( neuron_tracing )

add_subdirectory( worm_straighten_c )

add_subdirectory( jba )

add_subdirectory( v3d )

add_subdirectory( testing )
