# Top level cmake to build v3d, standard v3d plugins, and platform-specific installers
project(V3DPackage)
cmake_minimum_required(VERSION 2.8)

# It is important on Windows for both v3d and plugins to use same build type.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING 
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." 
        FORCE) # Default to Release
endif()

# I want to use my local version of the STGZ header input
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake")

# Build V3D
add_subdirectory(v3d_main)

# Build plugins
set(V3D_BASIC_C_FUN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/basic_c_fun)
add_subdirectory(released_plugins)

#############################################################
# Packaging custom installers for each platform using cpack #
#############################################################

set(CPACK_PACKAGE_NAME v3d)
# name of dmg file
set(CPACK_PACKAGE_FILE_NAME "InstallV3d-${V3D_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "V3D is a bioimage volume visualization and analysis application.")
set(CPACK_PACKAGE_VENDOR "Howard Hughes Medical Institute Janelia Farm Research Campus")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/LICENSE")
set(CPACK_PACKAGE_VERSION "${V3D_VERSION}")
set(CPACK_PACKAGE_CONTACT "brunsc at janelia.hhmi.org")
set(CPACK_PACKAGE_INSTALL_DIRECTORY v3d)
set(CPACK_PACKAGE_EXECUTABLES "v3d.exe" "v3d")

# Component arrangement in NSIS installer (and possible mac package installer, if we make one)
set(CPACK_COMPONENT_V3D_REQUIRED TRUE)
set(CPACK_COMPONENT_V3D_DESCRIPTION "V3D application for bioimage visualization and analysis")
set(CPACK_COMPONENT_PLUGINS_DEPENDS v3d)
set(CPACK_COMPONENT_PLUGINS_DESCRIPTION "Standard plugin extensions to V3D")
set(CPACK_COMPONENT_RUNTIMELIBRARIES_DEPENDS v3d)
set(CPACK_COMPONENT_RUNTIMELIBRARIES_DESCRIPTION "Qt, Tiff, and other dynamic libraries needed by V3D")
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE TRUE) # Just one stgz installer, please

# Mac installer is a .dmg disk image containing an application bundle,
# a link to the /Applications folder, and a background image encouraging
# the user to drag the app into /Applications.  A license dialog appears
# when the user tries to open the dmg file.
if(APPLE)
    # MACOSX_BUNDLE_WHATEVER parameters must be set before configure_file statement on Info.plist is called
    # MACOSX_BUNDLE_EXECUTABLE_NAME should match CPACK_BUNDLE_NAME
    set(MACOSX_BUNDLE_EXECUTABLE_NAME v3d)
    set(MACOSX_BUNDLE_INFO_STRING "v3d")
    set(MACOSX_BUNDLE_ICON_FILE "v3d.icns")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.hhmi.janelia.v3d")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${V3D_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_NAME v3d)
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${V3D_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PACKAGE_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT "(c) 2006-2010 Hanchuan Peng")
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/MacOSXBundleInfo.plist.in
      ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
      @ONLY
    )
    # Use a script to launch the v3d binary within the bundle
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/v3d_mac.sh"
        "${CMAKE_CURRENT_BINARY_DIR}/mac_script/v3d"
        COPYONLY)
    set(CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/mac_script/v3d")
    # Hope for recent version of cpack to pretty up dmg...
    set(CPACK_DMG_VOLUME_NAME "v3d-${V3D_VERSION}")
    set(CPACK_DMG_DS_STORE "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/dmg_DS_STORE")
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/v3d_dmg_bgd.png")
    #
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/V3D_Disk.icns")
    # name of Bundle in dmg file, and executable in bundle
    set(CPACK_BUNDLE_NAME ${MACOSX_BUNDLE_EXECUTABLE_NAME})
    set(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/v3d/v3d.icns")
    set(CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
    set(CPACK_GENERATOR Bundle)
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

    set(CPACK_NSIS_MENU_LINKS "bin/v3d.exe" "V3D") # OK
    SET(CPACK_NSIS_DISPLAY_NAME V3D)
    SET(CPACK_NSIS_URL_INFO_ABOUT  
        "http://penglab.janelia.org/proj/v3d/V3D/About_V3D.html")

# Linux installer is a self extracting shell archive
else() # Linux
    # If we are on a debian system (e.g. Ubuntu), make a deb installer
    find_program(DPKG dpkg)
    mark_as_advanced(DPKG)
    # Unfortunately cpack 2.8.2 creates archives that cannot be opened with 
    # stock gnu tar 1.23 on Ubuntu.  So disable DEB for now... Nov-3-2010
    # if(DPKG)
    if(FALSE)
      # Debian package.  Our eventual goal is "apt-get install v3d"
      set(CPACK_GENERATOR DEB)
      set(CPACK_PACKAGE_FILE_NAME v3d-${V3D_VERSION})
      set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "libtiff4 (>= 3.9.4-2), libqt4-opengl (>= 4.7.0), libqt4-network (>= 4.7.0), libqt4-xml (>= 4.7.0)"
      )
    else()
      # Self extracting shell archive if other installer options unavailable
      set(CPACK_GENERATOR STGZ)
    endif()
endif()

# include(CPack) must always be last!
include(CPack)
