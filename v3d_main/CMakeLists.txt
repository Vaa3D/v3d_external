cmake_minimum_required(VERSION 2.6)

project(V3D)

# Parse version number from source file
file(STRINGS 
    "${CMAKE_CURRENT_SOURCE_DIR}/v3d/v3d_version_info.cpp"
    V3D_VERSION_TMP
    REGEX "VersionInfo thisVersionOfV3D(.*)")
string(REGEX MATCH "[0-9+]\\.[0-9.]+" V3D_VERSION_TMP ${V3D_VERSION_TMP})
set(V3D_PATCH_VERSION "" CACHE STRING "Additional patch version string on main V3D version")
set(V3D_VERSION "${V3D_VERSION_TMP}${V3D_PATCH_VERSION}" CACHE STRING "Current version of V3D" FORCE)
mark_as_advanced(V3D_VERSION)
# message("V3D version = " ${V3D_VERSION})

if(NOT LIBRARY_OUTPUT_PATH)
  set (LIBRARY_OUTPUT_PATH ${V3D_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all libraries.")
endif(NOT LIBRARY_OUTPUT_PATH)

if(NOT EXECUTABLE_OUTPUT_PATH)
  set (EXECUTABLE_OUTPUT_PATH ${V3D_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all executables.")
endif(NOT EXECUTABLE_OUTPUT_PATH)

file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/plugins)

mark_as_advanced(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

include(CTest)

find_package(Qt4 4.7.0 REQUIRED)
set(QT_USE_NETWORK TRUE) # Oct-07-2010 CMB
set(QT_USE_XML TRUE) # Oct-22-2010 CMB

include( ${QT_USE_FILE} )
include_directories( ${QT_INCLUDES} )

find_package(OpenGL REQUIRED)

option( V3D_USE_OWN_TIFF "Use the TIFF library source code distributed with v3d" ON )

# Movie loading required libavcodec from ffmpeg
# FFMpeg requires YASM
set(USE_FFMPEG FALSE CACHE BOOL "Whether to load 3D volumes from movies using libavcodec")
if(USE_FFMPEG)
    add_definitions(-DUSE_FFMPEG)
endif()

add_subdirectory( common_lib )

if( NOT V3D_USE_OWN_TIFF )
  # mark_as_advanced(TIFF_LIBRARY)
  # mark_as_advanced(TIFF_INCLUDE_DIR)
  # include_directories(BEFORE "${CMAKE_CURRENT_SOURCE_DIR}/common_lib/include")
else()
  # CMake 2.8 FindTiff is very simple.  I need to give it some help on windows
  SET(TIFF_NAMES ${TIFF_NAMES}  libtiff.lib libv3dtiff.a libtiff_v3d.a v3dtiff libv3dtiff libtiff.a tiff libtiff libtiff3 libtiff_i)
  # message("PROGRAMFILES = $ENV{PROGRAMFILES}")
  FIND_LIBRARY(TIFF_LIBRARY NAMES ${TIFF_NAMES}
	# PATHS "$ENV{PROGRAMFILES}/tiff-3.8.2"
	PATHS 
        "$ENV{ProgramW6432}/libtiff64"
        "$ENV{ProgramW6432}/libtiff"
        # "C:/Program Files/libtiff64"
        "$ENV{PROGRAMFILES}/V3DTiff"
        "$ENV{PROGRAMFILES}/GnuWin32"
        "$ENV{PROGRAMFILES}/libtiff-3.9.4"
        /usr/local/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/common_lib/src_packages/tiff-3.8.2.1/libtiff/.libs
	PATH_SUFFIXES lib libtiff "")
  FIND_PATH(TIFF_INCLUDE_DIR tiff.h
	# PATHS "$ENV{PROGRAMFILES}/tiff-3.8.2"
	PATHS 
        "$ENV{ProgramW6432}/libtiff64"
        "$ENV{ProgramW6432}/libtiff"
        "${CMAKE_CURRENT_SOURCE_DIR}/common_lib/include"
        "$ENV{PROGRAMFILES}/V3DTiff"
        "$ENV{PROGRAMFILES}/GnuWin32"
        "$ENV{PROGRAMFILES}/libtiff-3.9.4"
        ${CMAKE_CURRENT_SOURCE_DIR}/common_lib/src_packages/tiff-3.8.2.1/libtiff
	PATH_SUFFIXES include libtiff "")
  # The unadorned FindTiff below
  find_package(TIFF REQUIRED)
  # include_directories(${TIFF_INCLUDE_DIR})
  mark_as_advanced(CLEAR TIFF_LIBRARY)
  mark_as_advanced(CLEAR TIFF_INCLUDE_DIR)
endif()

include_directories(${TIFF_INCLUDE_DIR})

# message("QTVERSION = ${QTVERSION}")
find_path(QT4_DEMOS_INCLUDE_DIR
  arthurwidgets.h
  PATHS
        ${QT_LIBRARY_DIR}/..
        /Developer/Examples/Qt
	/usr/lib64/qt4
	/usr/lib/qt4
	/usr/local/Trolltech/Qt-${QTVERSION}
	"C:/Qt/${QTVERSION}"
  PATH_SUFFIXES
	demos/shared
	Demos/shared
   )

find_library(QT4_DEMOS_LIBRARY_RELEASE
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
find_library(QT4_DEMOS_LIBRARY_DEBUG
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
	demos/shared/debug # Windows
  )
set(QT4_DEMOS_LIBRARY "${QT4_DEMOS_LIBRARY_RELEASE}" CACHE PATH "")
if (MSVC AND QT4_DEMOS_LIBRARY_RELEASE AND QT4_DEMOS_LIBRARY_DEBUG)
    set(QT4_DEMOS_LIBRARY optimized "${QT4_DEMOS_LIBRARY_RELEASE}" debug "${QT4_DEMOS_LIBRARY_DEBUG}" CACHE PATH "" FORCE)
endif()

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
        set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING 
            "Build architectures for OSX" FORCE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.5" CACHE STRING 
            "Minimum OS X version to target for deployment (at runtime); newer APIs weak linked. Set to empty string for default value." FORCE)
        set(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.5.sdk" CACHE PATH
            "The product will be built against the headers and libraries located inside the indicated SDK." FORCE)
        set(V3D_INITIALIZED_MAX_OSX_TO_DEFAULT TRUE CACHE INTERNAL "Whether default v3d values of Mac compatibility has been set.")
    endif()
endif()

# Define __NAWEBSERVICE__ symbol -D__NAWEBSERVICE__
set(V3D_USE_FLYWORKSTATION_WEB_SERVICES ON CACHE BOOL "Enable web services for fly workstation")
if(V3D_USE_FLYWORKSTATION_WEB_SERVICES)
    add_definitions(-D__NAWEBSERVICE__)
endif()

# enable NeuronAnnotator mode
add_definitions(-D_ALLOW_WORKMODE_MENU_)

add_subdirectory( graph )

# V3DInterface needs some generated ui headers
include_directories(${CMAKE_CURRENT_BINARY_DIR}/v3dbase)
add_subdirectory( basic_c_fun )

add_subdirectory( neuron_editing )

add_subdirectory( neuron_tracing )

add_subdirectory( neuron_annotator )

include_directories(v3d)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/neuron_annotator)
add_subdirectory( v3dbase )

add_subdirectory( plugin_loader )

add_subdirectory( imaging ) # CMB Oct-11-2010
add_subdirectory( multithreadimageIO ) # CMB Dec-16-2010

add_subdirectory( 3drenderer )

add_subdirectory( worm_straighten_c )

add_subdirectory( jba )

add_subdirectory( cellseg )

add_subdirectory( v3d )

add_subdirectory( testing )
