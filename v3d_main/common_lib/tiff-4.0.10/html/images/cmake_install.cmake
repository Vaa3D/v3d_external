# Install script for directory: D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images

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
   "C:/Program Files (x86)/tiff/share/doc/tiff/html/images/back.gif;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/bali.jpg;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/cat.gif;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/cover.jpg;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/cramps.gif;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/dave.gif;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/info.gif;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/jello.jpg;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/jim.gif;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/note.gif;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/oxford.gif;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/quad.jpg;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/ring.gif;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/smallliz.jpg;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/strike.gif;C:/Program Files (x86)/tiff/share/doc/tiff/html/images/warning.gif")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/tiff/share/doc/tiff/html/images" TYPE FILE FILES
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/back.gif"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/bali.jpg"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/cat.gif"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/cover.jpg"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/cramps.gif"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/dave.gif"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/info.gif"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/jello.jpg"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/jim.gif"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/note.gif"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/oxford.gif"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/quad.jpg"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/ring.gif"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/smallliz.jpg"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/strike.gif"
    "D:/Jazzyuancheng/v3d_external-master/v3d_main/common_lib/tiff-4.0.10/html/images/warning.gif"
    )
endif()

