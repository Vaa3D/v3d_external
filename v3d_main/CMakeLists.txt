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


option( V3D_USE_OWN_NEWMAT "Use the NEWMAT library source code distributed with v3d" OFF)

if( NOT V3D_USE_OWN_NEWMAT )
   #  find_package(NEWMAT REQUIRED)   # Luis: to install newmat as ubuntu package
else( NOT V3D_USE_OWN_NEWMAT )
   add_subdirectory(jba)
endif()


add_subdirectory( common_lib )

add_subdirectory( basic_c_fun )

add_subdirectory( v3dbase )

add_subdirectory( plugin_loader )

add_subdirectory( 3drenderer )

add_subdirectory( v3d )


# add_subdirectory( cellseg )
# add_subdirectory( gmm )
# add_subdirectory( graph )
# add_subdirectory( issues )
# add_subdirectory( neuron_editing )
# add_subdirectory( neuron_matching )
# add_subdirectory( neuron_tracing )
# add_subdirectory( worm_straighten_c )

