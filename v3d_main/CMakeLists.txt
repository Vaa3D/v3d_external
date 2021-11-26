cmake_minimum_required(VERSION 2.6)

project(V3D)

SET(Boost_USE_STATIC_LIBS ON)
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

if(APPLE)
  if(DARWIN_MAJOR_VERSION GREATER_EQUAL 18)
    #message("Mac OS Version = Mojave")
    add_definitions(-DGL_SILENCE_DEPRECATION)
  endif()
endif()

find_package(OpenGL REQUIRED)

option( V3D_USE_OWN_TIFF "Use the TIFF library source code distributed with v3d" ON )

# Movie loading required libavcodec from ffmpeg
# FFMpeg requires YASM
set(USE_FFMPEG OFF CACHE BOOL "Whether to load 3D volumes from movies using libavcodec")
if(USE_FFMPEG)
    add_definitions(-DUSE_FFMPEG)
    set(USE_X265 OFF CACHE BOOL "Whether to build FFMeg with x265 codec")
endif()

# HDF5
set(USE_HDF5 OFF CACHE BOOL "Build HDF5 loader")
if(USE_HDF5)
    add_definitions(-DUSE_HDF5)
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

if(APPLE)
    if(NOT V3D_INITIALIZED_MAX_OSX_TO_DEFAULT) # Only force it the first time.
        set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING
            "Build architectures for OSX" FORCE)
          #set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8" CACHE STRING
          #  "Minimum OS X version to target for deployment (at runtime); newer APIs weak linked. Set to empty string for default value." FORCE)
        set(V3D_INITIALIZED_MAX_OSX_TO_DEFAULT TRUE CACHE INTERNAL "Whether default v3d values of Mac compatibility has been set.")
    endif()
endif()

if(USE_FFMPEG)
  if(NOT WIN32)
    include(ExternalProject)

    ExternalProject_Add(
        PKG-CONFIG-0-28
        URL https://pkgconfig.freedesktop.org/releases/pkg-config-0.28.tar.gz
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/pkg-config/src
        INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/pkg-config/install
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND <SOURCE_DIR>/configure
            --prefix=<INSTALL_DIR>
	    --with-internal-glib
        BUILD_IN_SOURCE 1
    )
    ExternalProject_Add(
        Nasm-2-13-03
        GIT_REPOSITORY https://repo.or.cz/nasm.git
        GIT_TAG nasm-2.13.03
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/nasm/src
        INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/nasm/install
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND "<SOURCE_DIR>/autogen.sh"
        COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
        BUILD_COMMAND make
        BUILD_IN_SOURCE 1
    )

    set(av_dir "${CMAKE_CURRENT_BINARY_DIR}/libavcodec")
    set(x265_dir "${CMAKE_CURRENT_BINARY_DIR}/X265")
    set(extra_cflags "")
    set(extra_ldflags "")
    if(USE_X265)
      set(X265_FLAGS "--enable-libx265")
      ExternalProject_Add(
          X265-2.7
          DEPENDS Nasm-2-13-03
          URL ${CMAKE_CURRENT_SOURCE_DIR}/neuron_annotator/utility/x265-2.7.tar.gz
          SOURCE_DIR ${x265_dir}/src
          BINARY_DIR ${x265_dir}/build
          INSTALL_DIR ${x265_dir}/install
          UPDATE_COMMAND ""
          PATCH_COMMAND patch -p1 -t -N < ${CMAKE_CURRENT_SOURCE_DIR}/neuron_annotator/utility/x265.patch
          CONFIGURE_COMMAND ""
          BUILD_COMMAND ""
      )
      ExternalProject_Add_Step(X265-2.7 multilib
        DEPENDEES           build
        COMMAND           <SOURCE_DIR>/build/linux/multilib.sh ${CMAKE_CURRENT_BINARY_DIR}/nasm/install/bin/nasm${CMAKE_EXECUTABLE_SUFFIX}
        WORKING_DIRECTORY <SOURCE_DIR>/build/linux
      )
      ExternalProject_Add_StepTargets(X265-2.7 multilib)

      set(X265_LIBRARY ${x265_dir}/install/lib/libx265_main${CMAKE_SHARED_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")

    endif()
    if(APPLE)
        # Avoid ___sincos_stret link error on Mac
        set(extra_cflags "-D__STDC_CONSTANT_MACROS")
      	set(extra_ldflags "-Wl,-headerpad_max_install_names ")
    endif()
    set(extra_ldflags "")
    set(FFMPEG_PATCH_FILE "ffmpeg-4.0.patch")
    if(NOT WIN32)
      set(extra_libs "-lstdc++ -ldl")
    else()
      set(FFMPEG_PATCH_FILE "ffmpeg-2.5.4-windows.patch")
    endif()

    ExternalProject_Add(
        FFMPEG-4.0
        DEPENDS Nasm-2-13-03 X265-2.7 PKG-CONFIG-0-28
        URL ${CMAKE_CURRENT_SOURCE_DIR}/neuron_annotator/utility/ffmpeg-4.0.tgz
        SOURCE_DIR ${av_dir}/src
        BINARY_DIR ${av_dir}/build
        INSTALL_DIR ${av_dir}/install
        UPDATE_COMMAND ""
        #PATCH_COMMAND patch -p1 -t -N < ${CMAKE_CURRENT_SOURCE_DIR}/neuron_annotator/utility/${FFMPEG_PATCH_FILE}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND sh "${CMAKE_CURRENT_SOURCE_DIR}/neuron_annotator/utility/build_ffmpeg.sh"
                          --source_dir=<SOURCE_DIR> --install_dir=<INSTALL_DIR>
                          --cmake_current_binary_dir=${CMAKE_CURRENT_BINARY_DIR}
                          --x265_flags=${X265_FLAGS} --extra_cflags=${extra_cflags}
                          --extra_ldflags=${extra_ldflags} --extra_libs=${extra_libs}
                          --pkg_config_path=${x265_dir}/install/lib/pkgconfig
                          --asm_exe=${CMAKE_CURRENT_BINARY_DIR}/nasm/install/bin/nasm${CMAKE_EXECUTABLE_SUFFIX}
    )
    set(AVCODEC_INCLUDE_DIR "${av_dir}/install/include")
    set(AVDEVICE_LIBRARY ${av_dir}/install/lib/libavdevice${CMAKE_SHARED_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
    set(AVFORMAT_LIBRARY ${av_dir}/install/lib/libavformat${CMAKE_SHARED_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
    set(AVFILTER_LIBRARY ${av_dir}/install/lib/libavfilter${CMAKE_SHARED_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
    set(AVCODEC_LIBRARY ${av_dir}/install/lib/libavcodec${CMAKE_SHARED_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
    set(SWSCALE_LIBRARY ${av_dir}/install/lib/libswscale${CMAKE_SHARED_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
    set(SWRESAMPLE_LIBRARY ${av_dir}/install/lib/libswresample${CMAKE_SHARED_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
    set(AVUTIL_LIBRARY ${av_dir}/install/lib/libavutil${CMAKE_SHARED_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")

    find_package(ZLIB)
    find_package(BZip2)
    find_package(Iconv)

    find_package(Threads)
  else()
    set(AVCODEC_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win64/include")
    set(av_dir "${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win64/")
    if (MINGW)
      set(AVDEVICE_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win32/lib/libavdevice${CMAKE_SHARED_LIBRARY_SUFFIX}.a CACHE INTERNAL "path to a library")
      set(AVFORMAT_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win32/lib/libavformat${CMAKE_SHARED_LIBRARY_SUFFIX}.a CACHE INTERNAL "path to a library")
      set(AVFILTER_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win32/lib/libavfilter${CMAKE_SHARED_LIBRARY_SUFFIX}.a CACHE INTERNAL "path to a library")
      set(AVCODEC_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win32/lib/libavcodec${CMAKE_SHARED_LIBRARY_SUFFIX}.a CACHE INTERNAL "path to a library")
      set(SWSCALE_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win32/lib/libswscale${CMAKE_SHARED_LIBRARY_SUFFIX}.a CACHE INTERNAL "path to a library")
      set(SWRESAMPLE_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win32/lib/libswresample${CMAKE_SHARED_LIBRARY_SUFFIX}.a CACHE INTERNAL "path to a library")
      set(AVUTIL_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win32/lib/libavutil${CMAKE_SHARED_LIBRARY_SUFFIX}.a CACHE INTERNAL "path to a library")
    else()
      set(AVDEVICE_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win64/lib/avdevice.lib CACHE INTERNAL "path to a library")
      set(AVFORMAT_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win64/lib/avformat.lib CACHE INTERNAL "path to a library")
      set(AVFILTER_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win64/lib/avfilter.lib CACHE INTERNAL "path to a library")
      set(AVCODEC_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win64/lib/avcodec.lib CACHE INTERNAL "path to a library")
      set(SWSCALE_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win64/lib/swscale.lib CACHE INTERNAL "path to a library")
      set(SWRESAMPLE_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win64/lib/swresample.lib CACHE INTERNAL "path to a library")
      set(AVUTIL_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/common_lib/ffmpeg-2.5.2-win64/lib/avutil.lib CACHE INTERNAL "path to a library")
    endif()
  endif()

  include_directories(${AVCODEC_INCLUDE_DIR})
  add_definitions(-D__STDC_CONSTANT_MACROS)
endif()

if(USE_HDF5)
    include(ExternalProject)
    set(hdf_dir "${CMAKE_CURRENT_BINARY_DIR}/hdf5")
    ExternalProject_Add(
        HDF5
        #URL ${CMAKE_CURRENT_SOURCE_DIR}/neuron_annotator/utility/hdf5-1.10.1.tar.gz
        URL https://www.hdfgroup.org/package/gzip/?wpdmdl=4301&refresh=59dd1c8be460b1507662987
        DOWNLOAD_NAME hdf-1.8.20.tar.gz
        CMAKE_ARGS -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} -DBUILD_SHARED_LIBS:BOOL=OFF
                   -DCMAKE_INSTALL_PREFIX:PATH=${hdf_dir}/install -DHDF5_BUILD_CPP_LIB:BOOL=ON
                   -DHDF5_BUILD_HL_LIB:BOOL=ON -DHDF5_BUILD_TOOLS:BOOL=ON
                   -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
        UPDATE_COMMAND ""
    )

    set(HDF5_INCLUDE_DIR "${hdf_dir}/install/include")
    if (WIN32)
      # MINGW mangles the librariy names to be doubly prefixed with lib
      set(HDF5_LIBRARY ${hdf_dir}/install/lib/libhdf5$<$<CONFIG:Debug>:_D>${CMAKE_STATIC_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
      set(HDF5_HL_LIBRARY ${hdf_dir}/install/lib/libhdf5_hl$<$<CONFIG:Debug>:_D>${CMAKE_STATIC_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
      set(HDF5_CPP_LIBRARY ${hdf_dir}/install/lib/libhdf5_cpp$<$<CONFIG:Debug>:_D>${CMAKE_STATIC_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
      set(HDF5_HL_CPP_LIBRARY ${hdf_dir}/install/lib/libhdf5_hl_cpp$<$<CONFIG:Debug>:_D>${CMAKE_STATIC_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
    else()
      set(HDF5_LIBRARY ${hdf_dir}/install/lib/libhdf5$<$<CONFIG:Debug>:_debug>${CMAKE_STATIC_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
      set(HDF5_HL_LIBRARY ${hdf_dir}/install/lib/libhdf5_hl$<$<CONFIG:Debug>:_debug>${CMAKE_STATIC_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
      set(HDF5_CPP_LIBRARY ${hdf_dir}/install/lib/libhdf5_cpp$<$<CONFIG:Debug>:_debug>${CMAKE_STATIC_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
      set(HDF5_HL_CPP_LIBRARY ${hdf_dir}/install/lib/libhdf5_hl_cpp$<$<CONFIG:Debug>:_debug>${CMAKE_STATIC_LIBRARY_SUFFIX} CACHE INTERNAL "path to a library")
    endif()

    include_directories(${HDF5_INCLUDE_DIR})
endif()

# search for Qt
IF (DESIRED_QT_VERSION MATCHES 5)
  find_package(Qt5 COMPONENTS Core Widgets Xml Network OpenGL Concurrent REQUIRED)

   macro(qt_use_modules)
        qt5_use_modules(${ARGN})
   endmacro()

   macro(qt_wrap_ui)
       qt5_wrap_ui(${ARGN})
   endmacro()

   macro(qt_add_resources)
       qt5_add_resources(${ARGN})
   endmacro()

   macro(qt_add_translation)
       qt5_add_translation(${ARGN})
   endmacro()

   add_definitions(-DUSE_Qt5)
   include_directories( ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS}
                        ${Qt5Xml_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS}
                        ${Qt5Concurrent_INCLUDE_DIRS})
endif()

if(DESIRED_QT_VERSION MATCHES 4 OR NOT Qt5Core_FOUND)
  find_package(Qt4 REQUIRED)
  set(QT_USE_NETWORK TRUE) # Oct-07-2010 CMB
  set(QT_USE_XML TRUE) # Oct-22-2010 CMB

   macro(qt_use_modules)
        qt4_use_modules(${ARGN})
   endmacro()

   macro(qt_wrap_ui)
        qt4_wrap_ui(${ARGN})
   endmacro()

   macro(qt_add_resources)
        qt4_add_resources(${ARGN})
   endmacro()

   macro(qt_add_translation)
        qt4_add_translation(${ARGN})
   endmacro()

  include( ${QT_USE_FILE} )
  include_directories( ${QT_INCLUDES} )
endif()

set(BUILD_QT_DEMO_LIBRARY ON CACHE BOOL "Build rather than search for the qt shared demo library")

if (BUILD_QT_DEMO_LIBRARY)
  set(arthurwidgets_path "${CMAKE_CURRENT_BINARY_DIR}/qt_shared/install")

  if (Qt5Core_FOUND)
    set( QT_DEMO_LIBRARY_SOURCE "qt5_shared_demos.tar.gz")
  else()
    set( QT_DEMO_LIBRARY_SOURCE "qt_shared_demos.tar.gz")
  endif()

  find_program(QT_QMAKE_EXECUTABLE qmake)
  ExternalProject_Add(
      ARTHURWIDGETS
      URL ${CMAKE_CURRENT_SOURCE_DIR}/common_lib/src_packages/${QT_DEMO_LIBRARY_SOURCE}
      SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/qt_shared/src
      INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/qt_shared/install
      UPDATE_COMMAND ""
      CONFIGURE_COMMAND ${QT_QMAKE_EXECUTABLE}
      BUILD_IN_SOURCE 1
  )
  include_directories(${arthurwidgets_path})
  set(QT4_DEMOS_LIBRARY ${arthurwidgets_path}/libdemo_shared${CMAKE_STATIC_LIBRARY_SUFFIX} CACHE INTERNAL "path")
else()
  find_program(QT_QMAKE_EXECUTABLE qmake)
  execute_process( COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_DEMOS
                   OUTPUT_VARIABLE QT_DEMO_PREFIX
                   OUTPUT_STRIP_TRAILING_WHITESPACE )
  if (Qt5Core_FOUND)
    message("QTVERSION = ${Qt5_VERSION_MAJOR}.${Qt5_VERSION_MINOR}")
  else()
    message("QTVERSION = ${QTVERSION}")
  endif()
  message("QT_DEMO_PREFIX = ${QT_DEMO_PREFIX}")

  find_path(QT_DEMOS_INCLUDE_DIR
    arthurwidgets.h
    PATHS
          ${QT_DEMO_PREFIX}
    PATH_SUFFIXES
        	shared
        	widgets/painting/shared
    )

  if(NOT Qt5Core_FOUND)
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
  else()
    set(arthurwidgets_path "${QT_DEMOS_INCLUDE_DIR}/")
  endif()

  add_custom_target( qt_demos DEPENDS ${QT4_DEMOS_LIBRARY} )

  message("QT_DEMOS_INCLUDE_DIR = ${QT_DEMOS_INCLUDE_DIR}")

  if(NOT QT_DEMOS_INCLUDE_DIR)
    message(FATAL "You need to install the Qt demo files")
  else()
    include_directories(${QT_DEMOS_INCLUDE_DIR} ${QT_DEMO_PREFIX})
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

include_directories(${Boost_INCLUDE_DIR})
add_subdirectory( 3drenderer )

add_subdirectory( worm_straighten_c )

add_subdirectory( jba )

add_subdirectory( cellseg )


add_subdirectory( terafly )

add_subdirectory( v3d )

add_subdirectory( testing )

