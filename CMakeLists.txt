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

# There are a lot of platform flavors we want to distribute.  Establish the name here.
if(APPLE)
    set(PLATFORM_FLAVOR "Mac")
    # if you are using this CMake tree, architecture is fat 32+64
    set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_Fat)
elseif(WIN32)
    set(PLATFORM_FLAVOR "Windows")
    if(MSVC)
		set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_MSVC)
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
	if(CMAKE_SIZEDOF_VOID_P EQUAL 8)
	    set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_64)
	else()
	    set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_32)
	endif()
endif()

set(V3D_PLATFORM_NAME "${PLATFORM_FLAVOR}" CACHE STRING "Platform name describing this particular V3D binary type.")
add_definitions(-DV3D_PLATFORM_NAME=\"${V3D_PLATFORM_NAME}\") # so we can access platform name from running code.

# I want to have a runnable arrangement of V3D and plugins in the build area
set(V3D_STAGING_BASE_DIR "${CMAKE_BINARY_DIR}/v3d/${V3D_PLATFORM_NAME}" CACHE PATH "Build location for v3d and plugin binaries.")
set(V3D_BUILD_BINARY_DIR ${V3D_STAGING_BASE_DIR})

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
    set(V3D_MAC_CREATE_BUNDLE TRUE CACHE BOOL
        "Create V3D.app application bundle, instead of v3d command line tool")
    if(V3D_MAC_CREATE_BUNDLE)
        # Install Qt libraries into bundle
        find_program(MAC_DEPLOY_QT macdeployqt)
        add_custom_target(DeployQtLibraries ALL
            COMMAND "${MAC_DEPLOY_QT}" "${BUNDLE_BUILD_DIR}"
            COMMENT "Inserting Qt libraries into v3d.app")
        add_dependencies(DeployQtLibraries v3d)
        add_dependencies(DeployQtLibraries FinishedPlugins)
        # But macdeployqt does not fix up plugins properly
        find_package(Perl)
        add_custom_target(DeployPlugins ALL
            COMMAND "${PERL_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/mac/mac_deploy_cleanup.pl"
                "${BUNDLE_BUILD_DIR}"
            COMMENT "Adjusting link libraries for plugins")
        add_dependencies(DeployPlugins DeployQtLibraries)

        # name of Bundle in dmg file, and executable in bundle
        set(CPACK_BUNDLE_NAME v3d)
        set(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/v3d/v3d.icns")
        set(CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
    else(V3D_MAC_CREATE_BUNDLE)
    endif(V3D_MAC_CREATE_BUNDLE)

    # Whether or not we create a bundle, we create a dmg disk image.
    # Hope for recent version of cpack to pretty up dmg...
    set(CPACK_DMG_VOLUME_NAME "v3d-${V3D_VERSION}")
    set(CPACK_DMG_DS_STORE "${CMAKE_CURRENT_SOURCE_DIR}/v3d_main/CMake/package/dmg_DS_STORE")
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

    set(CPACK_NSIS_MENU_LINKS "bin/v3d.exe" "V3D")
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
    # cpack 2.8.3 is better, but it turns out that cpack/deb is inadequate to make a world class deb distribution.
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
