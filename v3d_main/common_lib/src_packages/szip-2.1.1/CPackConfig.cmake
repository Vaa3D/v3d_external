# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


set(CPACK_ALL_INSTALL_TYPES "Full;Developer;User")
set(CPACK_BUILD_SOURCE_DIRS "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/szip-2.1.1;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/szip-2.1.1")
set(CPACK_CMAKE_GENERATOR "MinGW Makefiles")
set(CPACK_COMPONENTS_ALL "libraries;headers;documents;configinstall")
set(CPACK_COMPONENTS_ALL_SET_BY_USER "TRUE")
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_FILE "F:/Cmake/share/cmake-3.18/Templates/CPack.GenericDescription.txt")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_SUMMARY "SZIP built using CMake")
set(CPACK_GENERATOR "ZIP")
set(CPACK_INSTALL_CMAKE_PROJECTS "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/szip-2.1.1;SZIP;configinstall;/")
set(CPACK_INSTALL_PREFIX "C:/Program Files (x86)/SZIP")
set(CPACK_INSTALL_TYPE_FULL_DISPLAY_NAME "Everything")
set(CPACK_MODULE_PATH "")
set(CPACK_NSIS_CONTACT "help@hdfgroup.org")
set(CPACK_NSIS_DISPLAY_NAME "szip 2.1")
set(CPACK_NSIS_DISPLAY_NAME_SET "TRUE")
set(CPACK_NSIS_INSTALLER_ICON_CODE "")
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
set(CPACK_NSIS_MODIFY_PATH "ON")
set(CPACK_NSIS_PACKAGE_NAME "szip 2.1")
set(CPACK_NSIS_UNINSTALL_NAME "Uninstall")
set(CPACK_OUTPUT_CONFIG_FILE "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/szip-2.1.1/CPackConfig.cmake")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/")
set(CPACK_PACKAGE_DESCRIPTION_FILE "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/szip-2.1.1/RELEASE.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "szip Installation")
set(CPACK_PACKAGE_FILE_NAME "szip--win64")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "HDF_Group\\szip\\")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "szip-")
set(CPACK_PACKAGE_NAME "szip")
set(CPACK_PACKAGE_RELOCATABLE "true")
set(CPACK_PACKAGE_VENDOR "HDF_Group")
set(CPACK_PACKAGE_VERSION "")
set(CPACK_PACKAGE_VERSION_MAJOR "2.1")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "")
set(CPACK_RESOURCE_FILE_LICENSE "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/szip-2.1.1/COPYING.txt")
set(CPACK_RESOURCE_FILE_README "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/szip-2.1.1/RELEASE.txt")
set(CPACK_RESOURCE_FILE_WELCOME "F:/Cmake/share/cmake-3.18/Templates/CPack.GenericWelcome.txt")
set(CPACK_SET_DESTDIR "OFF")
set(CPACK_SOURCE_7Z "ON")
set(CPACK_SOURCE_GENERATOR "7Z;ZIP")
set(CPACK_SOURCE_OUTPUT_CONFIG_FILE "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/szip-2.1.1/CPackSourceConfig.cmake")
set(CPACK_SOURCE_ZIP "ON")
set(CPACK_SYSTEM_NAME "win64")
set(CPACK_TOPLEVEL_TAG "win64")
set(CPACK_WIX_ROOT "")
set(CPACK_WIX_SIZEOF_VOID_P "8")
set(CPACK_WIX_UNINSTALL "1")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/szip-2.1.1/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()

# Configuration for component group "Runtime"

# Configuration for component group "Documents"
set(CPACK_COMPONENT_GROUP_DOCUMENTS_DESCRIPTION "Release notes for zlib")
set(CPACK_COMPONENT_GROUP_DOCUMENTS_EXPANDED TRUE)

# Configuration for component group "Development"
set(CPACK_COMPONENT_GROUP_DEVELOPMENT_DESCRIPTION "All of the tools you'll need to develop applications")
set(CPACK_COMPONENT_GROUP_DEVELOPMENT_EXPANDED TRUE)

# Configuration for component "libraries"
set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "SZIP Libraries")
set(CPACK_COMPONENT_LIBRARIES_GROUP Runtime)
set(CPACK_COMPONENT_LIBRARIES_INSTALL_TYPES Full Developer User)
set(CPACK_COMPONENT_LIBRARIES_REQUIRED TRUE)

# Configuration for component "headers"
set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "SZIP Headers")
set(CPACK_COMPONENT_HEADERS_GROUP Development)
set(CPACK_COMPONENT_HEADERS_DEPENDS libraries)
set(CPACK_COMPONENT_HEADERS_INSTALL_TYPES Full Developer)

# Configuration for component "documents"
set(CPACK_COMPONENT_DOCUMENTS_DISPLAY_NAME "SZIP Documents")
set(CPACK_COMPONENT_DOCUMENTS_GROUP Documents)
set(CPACK_COMPONENT_DOCUMENTS_INSTALL_TYPES Full Developer)

# Configuration for component "configinstall"
set(CPACK_COMPONENT_CONFIGINSTALL_DISPLAY_NAME "SZIP CMake files")
set(CPACK_COMPONENT_CONFIGINSTALL_GROUP Development)
set(CPACK_COMPONENT_CONFIGINSTALL_DEPENDS libraries)
set(CPACK_COMPONENT_CONFIGINSTALL_INSTALL_TYPES Full Developer User)
