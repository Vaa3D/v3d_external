cmake_minimum_required(VERSION 2.8)

project(V3D)

# Parse version number from source file
file(STRINGS 
    "${CMAKE_CURRENT_SOURCE_DIR}/v3d/v3d_version_info.cpp"
    V3D_VERSION_TMP
    REGEX "VersionInfo thisVersionOfV3D(.*)")
string(REGEX MATCH "[0-9+]\\.[0-9.]+" V3D_VERSION_TMP ${V3D_VERSION_TMP})
set(V3D_VERSION ${V3D_VERSION_TMP} CACHE INTERNAL "Current version of V3D" FORCE)
# message("V3D version = " ${V3D_VERSION})

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

# Help find proper version of Qt by finding qmake first
# Qt version 4.5.2 is required for compatibility with published plugins
# October 2010 --cmb
set(QT_DESIRED_VERSION 4.5.2)
find_program(QT_QMAKE_EXECUTABLE qmake
  PATHS
    # /usr/local/Trolltech/Qt-${QT_DESIRED_VERSION}/bin
  NO_DEFAULT_PATH
)

find_package(Qt4 REQUIRED)
set(QT_USE_NETWORK TRUE) # Oct-07-2010 CMB
set(QT_USE_XML TRUE) # Oct-22-2010 CMB

include( ${QT_USE_FILE} )
include_directories( ${QT_INCLUDES} )

find_package(OpenGL REQUIRED)

option( V3D_USE_OWN_TIFF "Use the TIFF library source code distributed with v3d" OFF)

if( V3D_USE_OWN_TIFF )
  mark_as_advanced(TIFF_LIBRARY)
  mark_as_advanced(TIFF_INCLUDE_DIR)
  include_directories(BEFORE "${V3D_SOURCE_DIR}/common_lib/include")
else()
  # CMake 2.8 FindTiff is very simple.  I need to give it some help on windows
  FIND_PATH(TIFF_INCLUDE_DIR tiff.h
	# PATHS "$ENV{PROGRAMFILES}/tiff-3.8.2"
	PATHS "$ENV{PROGRAMFILES}/GnuWin32"
		  "$ENV{PROGRAMFILES}/libtiff-3.9.4"
	PATH_SUFFIXES include libtiff "")
  SET(TIFF_NAMES ${TIFF_NAMES}  libv3dtiff.a v3dtiff libv3dtiff tiff libtiff libtiff3 libtiff_i)
  FIND_LIBRARY(TIFF_LIBRARY NAMES ${TIFF_NAMES}
	# PATHS "$ENV{PROGRAMFILES}/tiff-3.8.2"
	PATHS "$ENV{PROGRAMFILES}/GnuWin32"
		  "$ENV{PROGRAMFILES}/libtiff-3.9.4"
          /usr/local/lib
	PATH_SUFFIXES lib libtiff "")
  # The unadorned FindTiff below
  find_package(TIFF REQUIRED)
  include_directories(${TIFF_INCLUDE_DIR})
  mark_as_advanced(CLEAR TIFF_LIBRARY)
  mark_as_advanced(CLEAR TIFF_INCLUDE_DIR)
endif()

# message("QTVERSION = ${QTVERSION}")
find_path(QT4_DEMOS_INCLUDE_DIR
  arthurwidgets.h
  PATHS
        ${QT_LIBRARY_DIR}/..
        /Developer/Examples/Qt
	/usr/lib/qt4
	/usr/local/Trolltech/Qt-${QTVERSION}
	"C:/Qt/${QTVERSION}"
  PATH_SUFFIXES
	demos/shared
	Demos/shared
   )

find_library(QT4_DEMOS_LIBRARY
  demo_shared
  HINTS 
        ${QT_LIBRARY_DIR}/..
        /Developer/Examples/Qt
	/usr/lib/qt4
	/usr/local/Trolltech/Qt-${QTVERSION}
	"C:/Qt/${QTVERSION}"
  PATH_SUFFIXES
	demos/shared
	Demos/shared
	demos/shared/release # Windows
  )

if(NOT QT4_DEMOS_INCLUDE_DIR)
  message(FATAL "You need to install the Qt4 demo files")
else()
  include_directories(${QT4_DEMOS_INCLUDE_DIR})
endif()

# Keep Mac compatibility to OSX 10.5 (Leopard)
# even when building on Snow Leopard
find_path(TMP_OSX_SYSROOT MacOSX10.5.sdk PATHS /Developer/SDKs NO_DEFAULT_PATH)
mark_as_advanced(TMP_OSX_SYSROOT)
if(APPLE AND TMP_OSX_SYSROOT)
    if(NOT V3D_INITIALIZED_MAX_OSX_TO_DEFAULT) # Only force it the first time.
        set(CMAKE_OSX_ARCHITECTURES "i386;x86_64" CACHE STRING 
            "Build architectures for OSX" FORCE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.5" CACHE STRING 
            "Minimum OS X version to target for deployment (at runtime); newer APIs weak linked. Set to empty string for default value." FORCE)
        set(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.5.sdk" CACHE PATH
            "The product will be built against the headers and libraries located inside the indicated SDK." FORCE)
        set(V3D_INITIALIZED_MAX_OSX_TO_DEFAULT TRUE CACHE INTERNAL "Whether default v3d values of Mac compatibility has been set.")
    endif()
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
