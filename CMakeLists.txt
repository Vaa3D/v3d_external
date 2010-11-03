# Top level cmake to build v3d, standard v3d plugins, and platform-specific installers
project(V3DPackage)
cmake_minimum_required(VERSION 2.8)

# Build V3D
add_subdirectory(v3d_main)

# Build plugins
set(V3D_BASIC_C_FUN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/basic_c_fun)
add_subdirectory(released_plugins)

#############################################################
# Packaging custom installers for each platform using cpack #
#############################################################

# TODO - Only Mac installer is well-configured here.

set(CPACK_PACKAGE_NAME InstallV3d-${V3D_VERSION})
# name of dmg file
set(CPACK_PACKAGE_FILE_NAME "InstallV3d-${V3D_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "V3D is a bioimage volume visualization and analysis application.")
set(CPACK_PACKAGE_VENDOR "Howard Hughes Medical Institute Janelia Farm Research Campus")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/LICENSE")
set(CPACK_PACKAGE_VERSION "${V3D_VERSION}")

# Mac installer is a .dmg disk image containing an application bundle,
# a link to the /Applications folder, and a background image encouraging
# the user to drag the app into /Applications.  A license dialog appears
# when the user tries to open the dmg file.
if(APPLE)
    # MACOSX_BUNDLE_WHATEVER parameters must be set before configure_file statement on Info.plist is called
    # MACOSX_BUNDLE_EXECUTABLE_NAME should match CPACK_BUNDLE_NAME
    set(MACOSX_BUNDLE_EXECUTABLE_NAME v3d)
    set(MACOSX_BUNDLE_INFO_STRING "v3d")
    set(MACOSX_BUNDLE_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/v3d/v3d.icns")
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
# Windows installer uses NSIS
elseif(WIN32)
    set(CPACK_GENERATOR NSIS)
# Linux installer is a self extracting shell archive
# TODO - create deb file for ubuntu
else() # Linux?
    set(CPACK_GENERATOR STGZ)
endif()

# include(CPack) must always be last!
include(CPack)
