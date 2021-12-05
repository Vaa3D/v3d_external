# Top level cmake to build v3d, standard v3d plugins, and platform-specific installers
project(Vaa3D)
cmake_minimum_required(VERSION 2.8)

include(CTest)

# Including two specific dirs to keep compilation going
include_directories( ${CMAKE_BINARY_DIR}/v3d_main/cellseg )
include_directories( ${CMAKE_BINARY_DIR}/v3d_main/v3d )

set (V3D_MAIN "${CMAKE_SOURCE_DIR}/v3d_main")

# Switch Qt versions
set(FORCE_QT4 ON CACHE BOOL "Build with Qt4")

# Disable Janelia version
set(JANELIA_BUILD OFF CACHE BOOL "Build for HHMI Janelia version")

# Sidestep compile trouble with boost 1.46 with MSVC11/2012
if(WIN32)
    add_definitions(-DBOOST_NO_0X_HDR_INITIALIZER_LIST)
endif()


set(V3D_PACKAGE_AS_NEURON_ANNOTATOR OFF CACHE BOOL "Whether to package custom NeuronAnnotator mode")
# V3D_PACKAGE_AS_NEURON_ANNOTATOR is not working well, at least on Windows
if(WIN32)
    mark_as_advanced(CLEAR V3D_PACKAGE_AS_NEURON_ANNOTATOR)
else()
    mark_as_advanced(CLEAR V3D_PACKAGE_AS_NEURON_ANNOTATOR)
endif()

# It is important on Windows for both v3d and plugins to use same build type.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE) # Default to Release
endif()

set(V3D_AUTO_NAG_FOR_SOFTWARE_UPDATE OFF CACHE BOOL "Whether to alert user at startup that a newer version of Vaa3D is available")
if (NOT V3D_AUTO_NAG_FOR_SOFTWARE_UPDATE)
    add_definitions(-DV3D_SKIP_AUTO_VERSION_CHECK=1) # Skip automatic nag to update software
endif()

# I want to use my local version of the STGZ header input
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake")

# There are a lot of platform flavors we want to distribute.  Establish the name here.
if(APPLE)
    set(PLATFORM_FLAVOR "Mac")
    # if you are using this CMake tree, architecture is fat 32+64
    set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_Fat)
elseif(WIN32)
    set(PLATFORM_FLAVOR "Windows")
    if(MSVC)
        if(MSVC10)
            set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_MSVC10)
        else()
            set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_MSVC)
        endif()
    else()
        set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_MSYS)
    endif()
    string(REGEX MATCH "Win64" BUILD_WIN64 "${CMAKE_GENERATOR}")
    if(BUILD_WIN64)
        set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_64)
    else()
        set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_32)
    endif()
else()
    # Assume for the time being that not-windows and not-mac means Linux
    set(PLATFORM_FLAVOR "Linux")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_64)
    else()
        set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_32)
    endif()
endif()

# Default to building 64 bit binaries
if(APPLE)
    if(NOT CMAKE_OSX_ARCHITECTURES)
         set(CMAKE_OSX_ARCHITECTURES x86_64)
    endif()
endif()

set(V3D_PLATFORM_NAME "${PLATFORM_FLAVOR}" CACHE STRING "Platform name describing this particular Vaa3D binary type.")
add_definitions(-DV3D_PLATFORM_NAME=\"${V3D_PLATFORM_NAME}\") # " # so we can access platform name from running code.

# I want to have a runnable arrangement of Vaa3D and plugins in the build area
set(V3D_STAGING_BASE_DIR "${CMAKE_BINARY_DIR}/v3d/${V3D_PLATFORM_NAME}" CACHE PATH "Build location for v3d and plugin binaries.")
set(V3D_BUILD_BINARY_DIR ${V3D_STAGING_BASE_DIR} CACHE INTERNAL "Location of v3d executable and plugins." FORCE)
# Plugins use "V3D_BINARY_DIR"
set(V3D_BINARY_DIR "${V3D_BUILD_BINARY_DIR}" CACHE PATH "Where to put plugins directory")

add_custom_target(PluginPrerequisites)

# Both plugins and v3d depend on tiff library, so build it here
if (NOT WIN32) # see src_packages/CMakeLists.txt for windows tiff build
    # Build TIFF as an external project
    set(tiff_dir "${CMAKE_CURRENT_BINARY_DIR}/tiff")
    include(ExternalProject)
    # AMD 64 requires -fPIC to link static libraries on linux
    set(tiff_cflags "")
    if(UNIX)
      set(tiff_cflags ${tiff_cflags} -O3)
    endif()
    if(APPLE)
      foreach(OSX_ARCH ${CMAKE_OSX_ARCHITECTURES})
        set(tiff_cflags "${tiff_cflags} -arch ${OSX_ARCH}")
      endforeach()
    endif()
    if(UNIX AND NOT APPLE)
      if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
        set(tiff_cflags "-fPIC ${tiff_cflags}")
      endif()
    endif()
    # TODO - windows build rules from src_packages/CMakeLists.txt
    ExternalProject_Add(
        LibTIFF
        URL "${CMAKE_SOURCE_DIR}/v3d_main/common_lib/src_packages/tiff-3.8.2.1.tar.gz"
        SOURCE_DIR ${tiff_dir}/src
        BINARY_DIR ${tiff_dir}/build
        INSTALL_DIR ${tiff_dir}/install
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND <SOURCE_DIR>/configure
            --prefix=<INSTALL_DIR>
            --disable-jpeg
            --disable-zlib
            --disable-pixarlog
            --enable-static
            --disable-cxx
            --disable-shared
            CFLAGS=${tiff_cflags}
            CXXFLAGS=${tiff_cflags}
    )
    set(TIFF_INCLUDE_DIR "${tiff_dir}/install/include" CACHE PATH "" FORCE)
    include_directories("${TIFF_INCLUDE_DIR}")
    set(TIFF_LIBRARY "${tiff_dir}/install/lib/libtiff.a" CACHE PATH "" FORCE)

    add_dependencies(PluginPrerequisites LibTIFF)
endif()

# Build V3D
add_subdirectory(v3d_main)

if(V3D_PACKAGE_AS_NEURON_ANNOTATOR)
    find_file(V3D_COMPARTMENT_MAP_FILE
        flybraincompartmentmap.v3ds
        PATHS "${CMAKE_SOURCE_DIR}/v3d"
    )
    if(V3D_COMPARTMENT_MAP_FILE AND BUNDLE_BUILD_DIR)
        configure_file(${V3D_COMPARTMENT_MAP_FILE} "${BUNDLE_BUILD_DIR}/Contents/MacOS/flybraincompartmentmap.v3ds" COPYONLY)
    endif()
endif()

# Some plugins might be binary only
# Place them in the BUILD directory under binary_plugins
set(BINARY_PLUGIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/binary_plugins/")
if(EXISTS "${BINARY_PLUGIN_DIR}")
    set(PLUGIN_DESTINATION_DIR "./bin/plugins")
    # Don't install plugins separate from app bundle on apple
    if(V3D_MAC_CREATE_BUNDLE AND BUNDLE_BUILD_DIR AND APPLE)
        set(PLUGIN_DESTINATION_DIR "./Vaa3d.app/Contents/MacOS/plugins")
    endif()
    install(DIRECTORY "${BINARY_PLUGIN_DIR}"
        DESTINATION "${PLUGIN_DESTINATION_DIR}"
        USE_SOURCE_PERMISSIONS
        COMPONENT Plugins
        FILES_MATCHING
        PATTERN "*"
        PATTERN ".svn" EXCLUDE
        PATTERN "*.exp" EXCLUDE
        PATTERN "*.lib" EXCLUDE
        PATTERN "*.exe" EXCLUDE
        PATTERN "*.jar" EXCLUDE
        )
    if(WIN32)  # package all the extra exes that are required to run plugins
	file(GLOB programs "${BINARY_PLUGIN_DIR}/../DEPENDENTS/windows_ffmpeg.exe")
        install(PROGRAMS ${programs}
            DESTINATION "${V3D_INSTALL_DIR}"
            COMPONENT RuntimeLibraries
            )
        install(DIRECTORY "${BINARY_PLUGIN_DIR}/../DEPENDENTS/bioformats_tools"
	    DESTINATION "${V3D_INSTALL_DIR}"
            COMPONENT RuntimeLibraries
	    )
    endif()
endif()



# Build plugins
set(V3D_BASIC_C_FUN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/basic_c_fun)
add_subdirectory(released_plugins_more)

# Build jba application itself, if the directory has been checked out here
# (at the same level as v3d_main)
if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/jba)
    include_directories(
        v3d_main/basic_c_fun
        ${TIFF_INCLUDE_DIR}
        v3d_main/jba/newmat11/
    )
    add_executable(brainaligner
       jba/c++/main_jba.cpp
       jba/c++/jba_mainfunc.cpp
       jba/c++/jba_match_landmarks.cpp
       jba/c++/wkernel.cpp
       jba/c++/convert_type2uint8.cpp
       jba/c++/remove_nonaffine_points.cpp
       jba/c++/jba_affine_xform.cpp
       jba/c++/histeq.cpp
       v3d_main/basic_c_fun/mg_utilities.cpp
       v3d_main/basic_c_fun/mg_image_lib.cpp
       v3d_main/basic_c_fun/stackutil.cpp
    )
    target_link_libraries(brainaligner
         newmat11 ${TIFF_LIBRARY})
    set_target_properties(brainaligner PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    install(TARGETS brainaligner DESTINATION bin)

    add_dependencies(brainaligner LibTIFF)
endif()

#############################################################
# Packaging custom installers for each platform using cpack #
#############################################################

if(V3D_PACKAGE_AS_NEURON_ANNOTATOR)
    set(CPACK_PACKAGE_NAME NeuronAnnotator)
    set(CPACK_PACKAGE_FILE_NAME "InstallNeuronAnnotator-${V3D_VERSION}-${V3D_PLATFORM_NAME}")
    set(CPACK_PACKAGE_EXECUTABLES "NeuronAnnotator.exe" "NeuronAnnotator")
else()
    set(CPACK_PACKAGE_NAME Vaa3D-${V3D_VERSION})
    set(CPACK_PACKAGE_FILE_NAME "InstallVaa3D-${V3D_VERSION}-${V3D_PLATFORM_NAME}")
    # only for NSIS and for older CMake versions, not necessary when
    # CMAKE_NSIS_MENU_LINKS is used after CMake 2.8.4+
    set(CPACK_PACKAGE_EXECUTABLES "vaa3d;vaa3d-${V3D_VERSION}")
endif()



set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Vaa3D is a bioimage volume visualization and analysis application.")
set(CPACK_PACKAGE_VENDOR "Howard Hughes Medical Institute Janelia Farm Research Campus")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION "${V3D_VERSION}")
set(CPACK_PACKAGE_CONTACT "brunsc at janelia.hhmi.org")

if(WIN32)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY Vaa3D-${V3D_VERSION}) # Upper case folder looks better on Windows
else()
    set(CPACK_PACKAGE_INSTALL_DIRECTORY vaa3d)
endif()

# Component arrangement in NSIS installer (and possible mac package installer, if we make one)
set(CPACK_COMPONENT_V3D_REQUIRED TRUE)
set(CPACK_COMPONENT_V3D_DESCRIPTION "Vaa3D application for bioimage visualization and analysis")
set(CPACK_COMPONENT_PLUGINS_DEPENDS v3d)
set(CPACK_COMPONENT_PLUGINS_DESCRIPTION "Standard plugin extensions to Vaa3D")
set(CPACK_COMPONENT_RUNTIMELIBRARIES_DEPENDS v3d)
set(CPACK_COMPONENT_RUNTIMELIBRARIES_DESCRIPTION "Qt, Tiff, and other dynamic libraries needed by Vaa3D")
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE TRUE) # Just one stgz installer, please
if(BUILD_WIN64)
    # Default install to "Program Files", not "Program Files(86)" on 64-bit machine
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    # Install MSVC2010 runtime on machines lacking MSVC2010
    if(MSVC11)
        set(MS_REDIST_EXE vcredist_x64.exe)
        set(MS_REDIST_FOLDER msvc11)
    else()
        set(MS_REDIST_EXE vcredist_x64_msvc10sp1.exe)
        set(MS_REDIST_FOLDER msvc10)
    endif()
    install(PROGRAMS
        v3d_main/cmake_installer/${MS_REDIST_FOLDER}/${MS_REDIST_EXE}
        DESTINATION ${V3D_INSTALL_DIR}
        COMPONENT RuntimeLibraries)
    # quiet version
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS ${CPACK_NSIS_EXTRA_INSTALL_COMMANDS} "ExecWait \\\"$INSTDIR\\\\bin\\\\${MS_REDIST_EXE} /passive /norestart\\\" ")
endif()

# Mac installer is a .dmg disk image containing an application bundle,
# a link to the /Applications folder, and a background image encouraging
# the user to drag the app into /Applications.  A license dialog appears
# when the user tries to open the dmg file.
if(APPLE)
    set(V3D_MAC_CREATE_BUNDLE TRUE CACHE BOOL
        "Create Vaa3D.app application bundle, instead of v3d command line tool")
    if(V3D_MAC_CREATE_BUNDLE)
        # But macdeployqt does not fix up plugins properly
        find_package(Perl)
        add_custom_TARGET(DeployPlugins ALL
            COMMAND "${PERL_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/mac/mac_deploy_cleanup.pl"
                "${BUNDLE_BUILD_DIR}"
            COMMENT "Adjusting link libraries for plugins")
        # name of Bundle in dmg file, and executable in bundle
        if(V3D_PACKAGE_AS_NEURON_ANNOTATOR)
            set(CPACK_BUNDLE_NAME NeuronAnnotator)
        else()
            set(CPACK_BUNDLE_NAME "Vaa3D-${V3D_VERSION}")
        endif()
        set(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/v3d/v3d.icns")
        set(CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
    else(V3D_MAC_CREATE_BUNDLE)
    endif(V3D_MAC_CREATE_BUNDLE)

    # Whether or not we create a bundle, we create a dmg disk image.
    # Hope for recent version of cpack to pretty up dmg...
    if(V3D_PACKAGE_AS_NEURON_ANNOTATOR)
        set(CPACK_DMG_VOLUME_NAME "NeuronAnnotator-${V3D_VERSION}")
    else()
        set(CPACK_DMG_VOLUME_NAME "Vaa3D-${V3D_VERSION}")
    endif()
    if(V3D_PACKAGE_AS_NEURON_ANNOTATOR)
        set(CPACK_DMG_DS_STORE "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/dmg_DS_Store_na")
    else()
        set(CPACK_DMG_DS_STORE "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/dmg_DS_STORE")
    endif()
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/v3d_dmg_bgd.png")
    #
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/V3D_Disk.icns")
    set(CPACK_GENERATOR DragNDrop)
##################
# Windows installer uses NSIS #
##################
elseif(WIN32)
    set(CPACK_GENERATOR NSIS)
    # There is a bug in NSI that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backslashes.

    # Believe it or not, it seems all three of these icon settings are required to set the installer icon
    set(CPACK_NSIS_MUI_ICON
      "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/windows\\\\v3d.ico")
    set(CPACK_NSIS_MUI_UNIICON
       "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/windows\\\\v3d.ico")
    set(CPACK_PACKAGE_ICON
       "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/windows\\\\v3dInBox128.png")

    if(V3D_PACKAGE_AS_NEURON_ANNOTATOR)
        set(CPACK_NSIS_MENU_LINKS "bin/NeuronAnnotator.exe" "NeuronAnnotator")
        SET(CPACK_NSIS_DISPLAY_NAME NeuronAnnotator)
    else()
        set(CPACK_NSIS_MENU_LINKS "bin/vaa3d.exe" "Vaa3D-${V3D_VERSION}")
        SET(CPACK_NSIS_DISPLAY_NAME Vaa3D-${V3D_VERSION})
    endif()
    SET(CPACK_NSIS_URL_INFO_ABOUT
        "http://penglab.janelia.org/proj/v3d/V3D/About_V3D.html")

# Linux installer is a self extracting shell archive
else() # Linux
    # If we are on a debian system (e.g. Ubuntu), make a deb installer
    find_program(DPKG dpkg)
    mark_as_advanced(DPKG)
    # Unfortunately cpack 2.8.2 creates archives that cannot be opened with
    # stock gnu tar 1.23 on Ubuntu.  So disable DEB for now... Nov-3-2010
    # cpack 2.8.3 is better, but it turns out that cpack/deb is inadequate to make a world class deb distribution.
    # if(DPKG)
    if(FALSE)
      # Debian package.  Our eventual goal is "apt-get install vaa3d"
      set(CPACK_GENERATOR DEB)
      set(CPACK_PACKAGE_FILE_NAME vaa3d-${V3D_VERSION})
      set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "libtiff4 (>= 3.9.4-2), libqt4-opengl (>= 4.7.0), libqt4-network (>= 4.7.0), libqt4-xml (>= 4.7.0)"
      )
    else()
      # Self extracting shell archive if other installer options unavailable
      set(CPACK_GENERATOR STGZ)
    endif()
endif()

# add_subdirectory(doc/api)

# include(CPack) must always be last!
include(CPack)

