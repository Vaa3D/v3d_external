set(CMAKE_AUTOMOC ON)

include(src/terarepo/cmake/Utils.cmake)
include(cmake/TeraStitcher_Version.cmake)

# enable TeraFly code in Vaa3D
# add_definitions(-D_ALLOW_TERAFLY_MENU_) # does not work since v3d CMakeLists is at the same level of the current CMakeLists

# enable TeraFly code in terarepo/src
add_definitions(-D_VAA3D_TERAFLY_PLUGIN_MODE)
add_definitions(-DWITH_QT)
# add required definitions for terarepo
add_definitions(-DTERASTITCHER_MAJOR)
add_definitions(-DTERASTITCHER_MINOR)
add_definitions(-DTERASTITCHER_PATCH)
add_definitions(-DTERASTITCHER2_MAJOR)
add_definitions(-DTERASTITCHER2_MINOR)
add_definitions(-DTERASTITCHER2_PATCH)
add_definitions(-DTERACONVERTER_MAJOR)
add_definitions(-DTERACONVERTER_MINOR)
add_definitions(-DTERACONVERTER_PATCH)
# enable HDF5-dependent code
add_definitions(-DENABLE_BDV_HDF5)
add_definitions(-DENABLE_IMS_HDF5)


# disable annoying warnings on MSVC compilers
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	warnings_disable(CMAKE_CXX_FLAGS /wd4290) # vs2012: can't properly handle exceptions
	warnings_disable(CMAKE_CXX_FLAGS /wd4996) # vs2012: complains about unsafe standard C++ functions
endif()



# add hdf5 support for terafly
# HDF5
# temp solution to be consistent with the qmake builds, using the specific version of HDF5 lib.
# A better solution would be to use ExternalProjectAdd  to build HDF5 from source.
if (APPLE)
  # tmp solution. Note: as part of h5j we build HDF5, so skip these directories
  if(NOT USE_HDF5)
	  set (HDF5_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/lib_mac64/libhdf5.a")
	  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/include/hdf5")
	  link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/lib_mac64")
  endif()
  set (szip_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/lib_mac64/libszip.a")
  set (z_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/lib_mac64/libz.a")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  if(NOT USE_HDF5)
	  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/include/hdf5-win64")
	  link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/winlib64")
	  set (HDF5_LIBRARY
	      "${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/winlib64/libhdf5.lib")
  endif()

  set (szip_LIBRARY
      "${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/winlib64/libszip.lib")
  set (z_LIBRARY
      "${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/winlib64/libzlib.lib")

  endif()


# add external include directories

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../v3d)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3drenderer)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../basic_c_fun)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../basic_c_fun/customary_structs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories(${Boost_INCLUDE_DIR})

# add internal include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/stitcher)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/presentation)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/control)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/imagemanager)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/utils/volumeconverter)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/volumemanager)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/crossmips)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/3rdparty/tinyxml)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager/plugins/tiff2D)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager/plugins/tiff3D)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager/plugins/IMS_HDF5)

# add internal header files
file(GLOB presentation_h 		${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/*.h)
file(GLOB control_h 			${CMAKE_CURRENT_SOURCE_DIR}/src/control/*.h)
file(GLOB imagemanager_h 		${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/imagemanager/*.h)
list(REMOVE_ITEM imagemanager_h ${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/imagemanager/MappedVolume.h)
file(GLOB volumeconverter_h 	${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/utils/volumeconverter/*.h)
file(GLOB volumemanager_h 		${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/volumemanager/*.h)
file(GLOB stitcher_h 			${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/stitcher/*.h)
#list(REMOVE_ITEM stitcher_h     ${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/stitcher/resumer.h)
file(GLOB crossmips_h 			${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/crossmips/*.h)
file(GLOB tinyxml_h             ${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/3rdparty/tinyxml/*.h)
file(GLOB iomanager_h 			${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager/*.h)
file(GLOB plugin_tiff2D_h 		${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager/plugins/tiff2D/*.h)
file(GLOB plugin_tiff3D_h 		${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager/plugins/tiff3D/*.h)
file(GLOB plugin_dcimg_h 		${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager/plugins/dcimg/*.h)
file(GLOB plugin_IMS_HDF5_h 	${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager/plugins/IMS_HDF5/*.h)
file(GLOB common_h 				${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/common/*.h)

# add internal source files
file(GLOB presentation_src 		${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/*.cpp)
file(GLOB control_src 			${CMAKE_CURRENT_SOURCE_DIR}/src/control/*.cpp)
file(GLOB imagemanager_src 		${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/imagemanager/*.cpp)
list(REMOVE_ITEM imagemanager_src ${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/imagemanager/MappedVolume.cpp)
file(GLOB volumeconverter_src 	${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/utils/volumeconverter/*.cpp)
file(GLOB volumemanager_src 	${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/volumemanager/*.cpp)
file(GLOB stitcher_src 			${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/stitcher/*.cpp)
#list(REMOVE_ITEM stitcher_src   ${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/stitcher/resumer.cpp)
file(GLOB crossmips_src 		${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/crossmips/*.cpp)
file(GLOB tinyxml_src           ${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/3rdparty/tinyxml/*.cpp)
file(GLOB iomanager_src 		${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager/*.cpp)
file(GLOB plugin_tiff2D_src 	${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager/plugins/tiff2D/*.cpp)
file(GLOB plugin_tiff3D_src 	${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager/plugins/tiff3D/*.cpp)
file(GLOB plugin_dcimg_src 	${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager/plugins/dcimg/*.cpp)
file(GLOB plugin_IMS_HDF5_src 	${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/iomanager/plugins/IMS_HDF5/*.cpp)
file(GLOB common_src 			${CMAKE_CURRENT_SOURCE_DIR}/src/terarepo/src/common/*.cpp)

# look for "Q_OBJECT" in headers to determine which ones to MOC
message(STATUS "TeraFly: finding header files containing 'Q_OBJECT' macro...")

# create TeraFly lib
add_library(terafly STATIC
    ${TERAFLY_MOC_SRCS}
	${stitcher_h}
	${presentation_src}
	${control_src}
	${imagemanager_src}
	${volumeconverter_src}
	${volumemanager_src}
	${iomanager_src}
	${plugin_tiff2D_src}
	${plugin_tiff3D_src}
	${plugin_dcimg_src}
	${plugin_IMS_HDF5_src}
	${stitcher_src}
	${crossmips_src}
	${tinyxml_src}
	${common_src}
	${presentation_h}
	${control_h}
	${imagemanager_h}
	${volumeconverter_h}
	${volumemanager_h}
	${iomanager_h}
	${plugin_tiff2D_h}
	${plugin_tiff3D_h}
	${plugin_dcimg_h}
	${plugin_IMS_HDF5_h}
	${crossmips_h}
	${tinyxml_h}
	${common_h}
)

target_link_libraries(terafly ${HDF5_LIBRARY} ${z_LIBRARY} ${szip_LIBRARY})

