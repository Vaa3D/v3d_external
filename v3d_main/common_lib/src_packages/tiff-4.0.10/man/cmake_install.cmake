# Install script for directory: F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man

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
   "C:/Program Files (x86)/tiff/share/man/man1/fax2ps.1;C:/Program Files (x86)/tiff/share/man/man1/fax2tiff.1;C:/Program Files (x86)/tiff/share/man/man1/pal2rgb.1;C:/Program Files (x86)/tiff/share/man/man1/ppm2tiff.1;C:/Program Files (x86)/tiff/share/man/man1/raw2tiff.1;C:/Program Files (x86)/tiff/share/man/man1/tiff2bw.1;C:/Program Files (x86)/tiff/share/man/man1/tiff2pdf.1;C:/Program Files (x86)/tiff/share/man/man1/tiff2ps.1;C:/Program Files (x86)/tiff/share/man/man1/tiff2rgba.1;C:/Program Files (x86)/tiff/share/man/man1/tiffcmp.1;C:/Program Files (x86)/tiff/share/man/man1/tiffcp.1;C:/Program Files (x86)/tiff/share/man/man1/tiffcrop.1;C:/Program Files (x86)/tiff/share/man/man1/tiffdither.1;C:/Program Files (x86)/tiff/share/man/man1/tiffdump.1;C:/Program Files (x86)/tiff/share/man/man1/tiffgt.1;C:/Program Files (x86)/tiff/share/man/man1/tiffinfo.1;C:/Program Files (x86)/tiff/share/man/man1/tiffmedian.1;C:/Program Files (x86)/tiff/share/man/man1/tiffset.1;C:/Program Files (x86)/tiff/share/man/man1/tiffsplit.1")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/tiff/share/man/man1" TYPE FILE FILES
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/fax2ps.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/fax2tiff.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/pal2rgb.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/ppm2tiff.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/raw2tiff.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiff2bw.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiff2pdf.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiff2ps.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiff2rgba.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiffcmp.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiffcp.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiffcrop.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiffdither.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiffdump.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiffgt.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiffinfo.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiffmedian.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiffset.1"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/tiffsplit.1"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/tiff/share/man/man3/libtiff.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFbuffer.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFClose.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFcodec.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFcolor.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFDataWidth.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFError.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFFieldDataType.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFFieldName.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFFieldPassCount.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFFieldReadCount.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFFieldTag.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFFieldWriteCount.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFFlush.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFGetField.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFmemory.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFOpen.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFPrintDirectory.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFquery.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFReadDirectory.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFReadEncodedStrip.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFReadEncodedTile.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFReadRawStrip.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFReadRawTile.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFReadRGBAImage.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFReadRGBAStrip.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFReadRGBATile.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFReadScanline.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFReadTile.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFRGBAImage.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFSetDirectory.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFSetField.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFsize.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFstrip.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFswab.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFtile.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFWarning.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFWriteDirectory.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFWriteEncodedStrip.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFWriteEncodedTile.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFWriteRawStrip.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFWriteRawTile.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFWriteScanline.3tiff;C:/Program Files (x86)/tiff/share/man/man3/TIFFWriteTile.3tiff")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/tiff/share/man/man3" TYPE FILE FILES
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/libtiff.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFbuffer.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFClose.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFcodec.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFcolor.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFDataWidth.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFError.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFFieldDataType.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFFieldName.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFFieldPassCount.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFFieldReadCount.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFFieldTag.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFFieldWriteCount.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFFlush.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFGetField.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFmemory.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFOpen.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFPrintDirectory.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFquery.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFReadDirectory.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFReadEncodedStrip.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFReadEncodedTile.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFReadRawStrip.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFReadRawTile.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFReadRGBAImage.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFReadRGBAStrip.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFReadRGBATile.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFReadScanline.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFReadTile.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFRGBAImage.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFSetDirectory.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFSetField.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFsize.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFstrip.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFswab.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFtile.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFWarning.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFWriteDirectory.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFWriteEncodedStrip.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFWriteEncodedTile.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFWriteRawStrip.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFWriteRawTile.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFWriteScanline.3tiff"
    "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/tiff-4.0.10/man/TIFFWriteTile.3tiff"
    )
endif()

