cmake_minimum_required(VERSION 2.8)

project(V3D)

if(NOT LIBRARY_OUTPUT_PATH)
  set (LIBRARY_OUTPUT_PATH ${V3D_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all libraries.")
endif(NOT LIBRARY_OUTPUT_PATH)

if(NOT EXECUTABLE_OUTPUT_PATH)
  set (EXECUTABLE_OUTPUT_PATH ${V3D_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all executables.")
endif(NOT EXECUTABLE_OUTPUT_PATH)

file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/plugins)

mark_as_advanced(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)


set(CMAKE_MODULE_PATH ${V3D_SOURCE_DIR}/CMake)


include(CTest)

include(CPack)

# Help find proper version of Qt by finding qmake first
# Qt version 4.5.2 is required for compatibility with published plugins
# October 2010 --cmb
set(QT_DESIRED_VERSION 4.5.2)
find_program(QT_QMAKE_EXECUTABLE qmake
  PATHS
    /usr/local/Trolltech/Qt-${QT_DESIRED_VERSION}/bin
  NO_DEFAULT_PATH
)

find_package(Qt4 REQUIRED)
set(QT_USE_NETWORK TRUE) # Oct-07-2010 CMB

include( ${QT_USE_FILE} )
include_directories( ${QT_INCLUDES} )

find_package(OpenGL REQUIRED)

option( V3D_USE_OWN_TIFF "Use the TIFF library source code distributed with v3d" OFF)

if( V3D_USE_OWN_TIFF )
  mark_as_advanced(TIFF_LIBRARY)
  mark_as_advanced(TIFF_INCLUDE_DIR)
else()
  # CMake 2.8 FindTiff is very simple.  I need to give it some help on windows
  FIND_PATH(TIFF_INCLUDE_DIR tiff.h
	# PATHS "$ENV{PROGRAMFILES}/tiff-3.8.2"
	PATHS "$ENV{PROGRAMFILES}/GnuWin32"
	PATH_SUFFIXES include libtiff "")
  SET(TIFF_NAMES ${TIFF_NAMES} tiff libtiff libtiff3 libtiff_i)
  FIND_LIBRARY(TIFF_LIBRARY NAMES ${TIFF_NAMES}
	# PATHS "$ENV{PROGRAMFILES}/tiff-3.8.2"
	PATHS "$ENV{PROGRAMFILES}/GnuWin32"
	PATH_SUFFIXES lib libtiff "")
  # The unadorned FindTiff below
  find_package(TIFF REQUIRED)
  include_directories(${TIFF_INCLUDE_DIR})
  mark_as_advanced(CLEAR TIFF_LIBRARY)
  mark_as_advanced(CLEAR TIFF_INCLUDE_DIR)
  # When building with MSYS on Windows, need to copy libtiff dlls
  # message("CMake generator = ${CMAKE_GENERATOR}")
  if(${CMAKE_GENERATOR} MATCHES "MSYS")
    find_file(ZLIB_DLL
      NAMES zlib1.dll
      PATHS ${TIFF_INCLUDE_DIR}/../bin)
    find_file(JPEG_DLL
      NAMES jpeg62.dll
      PATHS ${TIFF_INCLUDE_DIR}/../bin)
    find_file(TIFF_DLL
      NAMES libtiff3.dll
      PATHS ${TIFF_INCLUDE_DIR}/../bin)
  endif()
endif()

# message("QTVERSION = ${QTVERSION}")
find_path(QT4_DEMOS_INCLUDE_DIR
  arthurwidgets.h
  PATHS
        ${QT_LIBRARY_DIR}/..
	/usr/lib/qt4
	/usr/local/Trolltech/Qt-${QTVERSION}
	"C:/Qt/${QTVERSION}"
  PATH_SUFFIXES
	demos/shared
   )

find_library(QT4_DEMOS_LIBRARY
  demo_shared
  HINTS 
        ${QT_LIBRARY_DIR}/..
	/usr/lib/qt4
	/usr/local/Trolltech/Qt-${QTVERSION}
	"C:/Qt/${QTVERSION}"
  PATH_SUFFIXES
	demos/shared
	demos/shared/release # Windows
  )

if(NOT QT4_DEMOS_INCLUDE_DIR)
  message(FATAL "You need to install the Qt4 demo files")
else()
  include_directories(${QT4_DEMOS_INCLUDE_DIR})
endif()

add_subdirectory( common_lib )

add_subdirectory( graph )

add_subdirectory( basic_c_fun )

add_subdirectory( neuron_editing )

add_subdirectory( neuron_tracing )

add_subdirectory( v3dbase )

add_subdirectory( plugin_loader )

add_subdirectory( imaging ) # CMB Oct-11-2010

add_subdirectory( 3drenderer )

add_subdirectory( worm_straighten_c )

add_subdirectory( jba )

add_subdirectory( cellseg )

add_subdirectory( v3d )

add_subdirectory( testing )


if(ZLIB_DLL)
    install(PROGRAMS ${ZLIB_DLL}
        DESTINATION ".")
endif()
if(JPEG_DLL)
    install(PROGRAMS ${JPEG_DLL}
        DESTINATION ".")
endif()
if(TIFF_DLL)
    install(PROGRAMS ${TIFF_DLL}
        DESTINATION ".")
endif()
