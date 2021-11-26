# include useful cmake scripts
include(cmake/utils.cmake)

# enable TeraFly code in Vaa3D
# add_definitions(-D_ALLOW_TERAFLY_MENU_) # does not work since v3d CMakeLists is at the same level of the current CMakeLists

# enable TeraFly code in iomanager TeraFly's module
add_definitions(-D_VAA3D_TERAFLY_PLUGIN_MODE)


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
include_directories(../v3d)
include_directories(../3drenderer)
include_directories(../basic_c_fun)
include_directories(../common_lib/include)
include_directories(../basic_c_fun/customary_structs)
include_directories(..)
include_directories(${Boost_INCLUDE_DIR})

# add internal include directories
include_directories(./src/presentation)
include_directories(./src/control)
include_directories(./src/core/imagemanager)
include_directories(./src/core/volumeconverter)
include_directories(./src/core/volumemanager)
include_directories(./src/core/stitcher)
include_directories(./src/core/crossmips)
include_directories(./src/core/tinyxml)
include_directories(./src/core/iomanager)
include_directories(./src/core/iomanager/plugins/tiff2D)
include_directories(./src/core/iomanager/plugins/tiff3D)

# add internal header files
file(GLOB presentation_h 		./src/presentation/*.h)
file(GLOB control_h 			./src/control/*.h)
file(GLOB imagemanager_h 		./src/core/imagemanager/*.h)
file(GLOB volumeconverter_h 	./src/core/volumeconverter/*.h)
file(GLOB volumemanager_h 		./src/core/volumemanager/*.h)
file(GLOB stitcher_h 			./src/core/stitcher/*.h)
file(GLOB crossmips_h 			./src/core/crossmips/*.h)
file(GLOB tinyxml_h 			./src/core/tinyxml/*.h)
file(GLOB iomanager_h 			./src/core/iomanager/*.h)
file(GLOB plugin_tiff2D_h 		./src/core/iomanager/plugins/tiff2D/*.h)
file(GLOB plugin_tiff3D_h 		./src/core/iomanager/plugins/tiff3D/*.h)

# add internal source files
file(GLOB presentation_src 		./src/presentation/*.cpp)
file(GLOB control_src 			./src/control/*.cpp)
file(GLOB imagemanager_src 		./src/core/imagemanager/*.cpp)
file(GLOB volumeconverter_src 	./src/core/volumeconverter/*.cpp)
file(GLOB volumemanager_src 	./src/core/volumemanager/*.cpp)
file(GLOB stitcher_src 			./src/core/stitcher/*.cpp)
file(GLOB crossmips_src 		./src/core/crossmips/*.cpp)
file(GLOB tinyxml_src 			./src/core/tinyxml/*.cpp)
file(GLOB iomanager_src 		./src/core/iomanager/*.cpp)
file(GLOB plugin_tiff2D_src 	./src/core/iomanager/plugins/tiff2D/*.cpp)
file(GLOB plugin_tiff3D_src 	./src/core/iomanager/plugins/tiff3D/*.cpp)

# look for "Q_OBJECT" in headers to determine which ones to MOC
message(STATUS "TeraFly: finding header files containing 'Q_OBJECT' macro...")
file(GLOB TERAFLY_MOC_CANDIDATES
    src/presentation/*.h
    src/control/*.h)
foreach(TERAFLY_MOC_CAND ${TERAFLY_MOC_CANDIDATES})
    file(READ "${TERAFLY_MOC_CAND}" HEADER_TEXT LIMIT 10000)
    string(REGEX MATCH "Q_OBJECT" QOB_RESULT ${HEADER_TEXT})
    if(QOB_RESULT)
        set(TERAFLY_MOC_FILES ${TERAFLY_MOC_CANDIDATES} ${TERAFLY_MOC_CAND})
    endif()
endforeach()
message(STATUS "TeraFly: finished examining header files")

# MOC
if(NOT Qt5Core_FOUND)
  QT4_WRAP_CPP(TERAFLY_MOC_SRCS ${TERAFLY_MOC_FILES})
else()
  QT5_WRAP_CPP(TERAFLY_MOC_SRCS ${TERAFLY_MOC_FILES})
endif()

# create TeraFly lib
add_library(terafly STATIC
    ${TERAFLY_MOC_SRCS}
	${presentation_src}
	${control_src}
	${imagemanager_src}
	${volumeconverter_src}
	${volumemanager_src}
	${iomanager_src}
	${plugin_tiff2D_src}
	${plugin_tiff3D_src}
	${stitcher_src}
	${crossmips_src}
	${tinyxml_src}
	${presentation_h}
	${control_h}
	${imagemanager_h}
	${volumeconverter_h}
	${volumemanager_h}
	${iomanager_h}
	${plugin_tiff2D_h}
	${plugin_tiff3D_h}
	${stitcher_h}
	${crossmips_h}
	${tinyxml_h}
)

target_link_libraries(terafly ${HDF5_LIBRARY} ${z_LIBRARY} ${szip_LIBRARY})

