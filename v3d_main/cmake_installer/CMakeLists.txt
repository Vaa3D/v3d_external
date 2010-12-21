# Build an installer based on a preexisting install directory.
# This only works for V3D right now, due to specific files

project(ProgramInstaller NONE)
cmake_minimum_required(VERSION 2.8)

# TODO make this into a cmake module
# possible variables needed
#     INSTALLER_TEMPLATE_DIR
#     INSTALLER_PROGRAM_NAME
#     INSTALLER_PROGRAM_VERSION
# optional
#      INSTALLER_PLATFORM_NAME

# Specify the directory where the final install files are.
# This directory must have been populated beforehand.
set(DEFAULT_TEMPLATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../v3d")
get_filename_component(DEFAULT_TEMPLATE_DIR "${DEFAULT_TEMPLATE_DIR}" REALPATH)
set(INSTALLER_TEMPLATE_DIR "${DEFAULT_TEMPLATE_DIR}" CACHE PATH "Directory containing files that will be installed by the installer.")

# Just one install rule: copy contents of that directory to the install area.
# Notice trailing slash ("/"), to install just the *contents* of the directory
install(DIRECTORY "${INSTALLER_TEMPLATE_DIR}/"
    DESTINATION "."
    USE_SOURCE_PERMISSIONS
    PATTERN ".svn" EXCLUDE
)

# I want to use my local version of the STGZ header input
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../CMake")

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
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_64)
    else()
        set(PLATFORM_FLAVOR ${PLATFORM_FLAVOR}_32)
    endif()
endif()

set(INSTALLER_PROGRAM_NAME "V3D" CACHE STRING "Name of the program being installed.")
set(INSTALLER_PLATFORM_NAME "${PLATFORM_FLAVOR}" CACHE STRING "Platform name describing this particular program binary type.")
set(INSTALLER_PROGRAM_VERSION "0.0.0" CACHE STRING "Version number of this release of the program.")

#############################################################
# Packaging custom installers for each platform using cpack #
#############################################################

set(CPACK_PACKAGE_NAME ${INSTALLER_PROGRAM_NAME})
# name of dmg file
set(CPACK_PACKAGE_FILE_NAME "Install_${INSTALLER_PROGRAM_NAME}_${INSTALLER_PROGRAM_VERSION}_${INSTALLER_PLATFORM_NAME}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "V3D is a bioimage volume visualization and analysis application.")
set(CPACK_PACKAGE_VENDOR "Howard Hughes Medical Institute Janelia Farm Research Campus")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
set(CPACK_PACKAGE_VERSION "${INSTALLER_PROGRAM_VERSION}")
set(CPACK_PACKAGE_CONTACT "brunsc at janelia.hhmi.org")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${INSTALLER_PROGRAM_NAME})
set(CPACK_PACKAGE_EXECUTABLES "" "")

# Component arrangement in NSIS installer (and possible mac package installer, if we make one)
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE TRUE) # Just one stgz installer, please

# Mac installer is a .dmg disk image containing an application bundle,
# a link to the /Applications folder, and a background image encouraging
# the user to drag the app into /Applications.  A license dialog appears
# when the user tries to open the dmg file.
if(APPLE)
    # name of Bundle in dmg file, and executable in bundle
    set(CPACK_BUNDLE_NAME ${INSTALLER_PROGRAM_NAME})
    # Whether or not we create a bundle, we create a dmg disk image.
    # Hope for recent version of cpack to pretty up dmg...
    set(CPACK_DMG_VOLUME_NAME "${INSTALLER_PROGRAM_NAME}_${INSTALLER_PROGRAM_VERSION}")
    set(CPACK_DMG_DS_STORE "${CMAKE_CURRENT_SOURCE_DIR}/../CMake/package/dmg_DS_STORE")
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/../CMake/package/v3d_dmg_bgd.png")
    #
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../CMake/package/V3D_Disk.icns")
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
      "${CMAKE_CURRENT_SOURCE_DIR}/../CMake/package/windows\\\\v3d.ico")
    set(CPACK_NSIS_MUI_UNIICON
       "${CMAKE_CURRENT_SOURCE_DIR}/../CMake/package/windows\\\\v3d.ico")
    set(CPACK_PACKAGE_ICON
       "${CMAKE_CURRENT_SOURCE_DIR}/../CMake/package/windows\\\\v3dInBox128.png")

    # Start menu link to program
    find_program(PROGRAM_BINARY
        NAMES v3d
        PATHS "${INSTALLER_TEMPLATE_DIR}/bin" "${INSTALLER_TEMPLATE_DIR}"
        NO_DEFAULT_PATH
        "The local location of the program executable that should be linked from the start menu"
    )
    if (PROGRAM_BINARY)
        file(RELATIVE_PATH RELATIVE_BINARY "${INSTALLER_TEMPLATE_DIR}" ${PROGRAM_BINARY})
        set(CPACK_NSIS_MENU_LINKS "${RELATIVE_BINARY}" "${INSTALLER_PROGRAM_NAME}")
    endif()
    SET(CPACK_NSIS_DISPLAY_NAME ${INSTALLER_PROGRAM_NAME})
    SET(CPACK_NSIS_URL_INFO_ABOUT  
        "http://penglab.janelia.org/proj/v3d/V3D/About_V3D.html")

# Linux installer is a self extracting shell archive
else() # Linux
    # Self extracting shell archive if other installer options unavailable
    set(CPACK_GENERATOR STGZ)
endif()

# Clean up cmake variable display
mark_as_advanced(CMAKE_BUILD_TYPE)
mark_as_advanced(CMAKE_OSX_ARCHITECTURES)
mark_as_advanced(CMAKE_OSX_DEPLOYMENT_TARGET)
mark_as_advanced(CMAKE_OSX_SYSROOT)
mark_as_advanced(CMAKE_INSTALL_PREFIX)
mark_as_advanced(CMAKE_CONFIGURATION_TYPES)

# include(CPack) must always be last!
include(CPack)

