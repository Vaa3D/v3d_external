# Install script for directory: D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html

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
  set(CMAKE_OBJDUMP "D:/mingw64/mingw64/bin/objdump.exe")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/tiff/share/doc/tiff/html/addingtags.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/bugs.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/build.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/contrib.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/document.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/images.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/index.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/internals.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/intro.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/libtiff.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/misc.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/support.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/TIFFTechNote2.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/tools.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.4beta007.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.4beta016.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.4beta018.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.4beta024.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.4beta028.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.4beta029.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.4beta031.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.4beta032.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.4beta033.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.4beta034.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.4beta035.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.4beta036.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.5.1.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.5.2.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.5.3.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.5.4.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.5.5.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.5.6-beta.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.5.7.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.6.0.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.6.1.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.7.0alpha.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.7.0beta.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.7.0beta2.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.7.0.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.7.1.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.7.2.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.7.3.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.7.4.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.8.0.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.8.1.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.8.2.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.9.0beta.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.9.1.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v3.9.2.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v4.0.0.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v4.0.1.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v4.0.2.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v4.0.3.html;C:/Program Files (x86)/tiff/share/doc/tiff/html/v4.0.4beta.html")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/tiff/share/doc/tiff/html" TYPE FILE FILES
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/addingtags.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/bugs.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/build.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/contrib.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/document.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/index.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/internals.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/intro.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/libtiff.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/misc.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/support.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/TIFFTechNote2.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/tools.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.4beta007.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.4beta016.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.4beta018.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.4beta024.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.4beta028.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.4beta029.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.4beta031.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.4beta032.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.4beta033.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.4beta034.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.4beta035.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.4beta036.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.5.1.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.5.2.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.5.3.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.5.4.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.5.5.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.5.6-beta.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.5.7.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.6.0.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.6.1.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.7.0alpha.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.7.0beta.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.7.0beta2.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.7.0.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.7.1.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.7.2.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.7.3.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.7.4.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.8.0.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.8.1.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.8.2.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.9.0beta.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.9.1.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v3.9.2.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v4.0.0.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v4.0.1.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v4.0.2.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v4.0.3.html"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/v4.0.4beta.html"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/cmake_install.cmake")
  include("D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/man/cmake_install.cmake")

endif()

