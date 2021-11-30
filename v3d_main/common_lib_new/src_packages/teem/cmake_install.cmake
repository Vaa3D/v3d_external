# Install script for directory: /Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem-1.11.0-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
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
  set(CMAKE_OBJDUMP "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/bin/libteem.1.11.0.dylib"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/bin/libteem.1.dylib"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libteem.1.11.0.dylib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libteem.1.dylib"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      execute_process(COMMAND "/usr/bin/install_name_tool"
        -id "/usr/local/lib/libteem.1.dylib"
        "${file}")
      execute_process(COMMAND /usr/bin/install_name_tool
        -add_rpath "/usr/local/lib"
        "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -x "${file}")
      endif()
    endif()
  endforeach()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/bin/libteem.dylib")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libteem.dylib" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libteem.dylib")
    execute_process(COMMAND "/usr/bin/install_name_tool"
      -id "/usr/local/lib/libteem.1.dylib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libteem.dylib")
    execute_process(COMMAND /usr/bin/install_name_tool
      -add_rpath "/usr/local/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libteem.dylib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -x "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libteem.dylib")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/teem" TYPE FILE FILES
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/air.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/hest.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/biff.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/nrrd.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/nrrdDefines.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/nrrdMacros.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/nrrdEnums.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/ell.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/ellMacros.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/unrrdu.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/moss.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/gage.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/dye.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/limn.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/echo.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/hoover.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/seek.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/ten.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/tenMacros.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/pull.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/mite.h"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/include/teem/meet.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/CMake/TeemConfig.cmake"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem-1.11.0-src/CMake/TeemUse.cmake"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/TeemBuildSettings.cmake"
    "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/TeemLibraryDepends.cmake"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/src/bin/cmake_install.cmake")
  include("/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/Testing/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
