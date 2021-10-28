# Install script for directory: F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Teem")
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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/libteem.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/teem" TYPE FILE FILES
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/air.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/hest.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/biff.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/nrrd.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/nrrdDefines.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/nrrdMacros.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/nrrdEnums.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/ell.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/ellMacros.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/unrrdu.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/moss.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/gage.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/dye.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/limn.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/echo.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/hoover.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/seek.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/ten.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/tenMacros.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/pull.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/mite.h"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/include/teem/meet.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/CMake/TeemConfig.cmake"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/CMake/TeemUse.cmake"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/TeemBuildSettings.cmake"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/TeemLibraryDepends.cmake"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/src/bin/cmake_install.cmake")
  include("F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
