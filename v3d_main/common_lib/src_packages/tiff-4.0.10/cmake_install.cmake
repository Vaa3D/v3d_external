# Install script for directory: F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/tiff")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/Program Files/mingw-w64/x86_64-8.1.0-win32-sjlj-rt_v6-rev0/mingw64/bin/objdump.exe")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/tiff/lib/pkgconfig/libtiff-4.pc")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/tiff/lib/pkgconfig" TYPE FILE FILES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/libtiff-4.pc")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/port/cmake_install.cmake")
  include("F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/libtiff/cmake_install.cmake")
  include("F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/tools/cmake_install.cmake")
  include("F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/test/cmake_install.cmake")
  include("F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/contrib/cmake_install.cmake")
  include("F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/build/cmake_install.cmake")
  include("F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/cmake_install.cmake")
  include("F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/html/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
