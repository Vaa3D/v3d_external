#
#

if (NOT MSVC)
  add_subdirectory(mylib_tiff)
endif()

# install_custom_tiff_library sets variables
# CUSTOM_TIFF_LIBRARY and CUSTOM_TIFF_INCLUDE_DIR
macro(install_custom_tiff_library TIFF_TARFILE CFLAGS SUFFIX)
    set(TIFF_BUILD_DIR_PARENT ${CMAKE_CURRENT_BINARY_DIR}/build_tiff_${SUFFIX})
    file(MAKE_DIRECTORY "${TIFF_BUILD_DIR_PARENT}")
    set(TIFF_BUILD_DIR "${TIFF_BUILD_DIR_PARENT}/tiff-3.8.2.1")
    ### Unzip tiff file ###
    add_custom_command(
        OUTPUT "${TIFF_BUILD_DIR}/VERSION"
        COMMAND tar ARGS xf "${TIFF_TARFILE}"
        DEPENDS "${TIFF_TARFILE}"
        WORKING_DIRECTORY "${TIFF_BUILD_DIR_PARENT}"
        COMMENT "Unpacking libtiff source code"
    )
    set(TIFF_INSTALL_DIR "${TIFF_BUILD_DIR_PARENT}/install")
    file(MAKE_DIRECTORY "${TIFF_INSTALL_DIR}")
    # Note use of VERBATIM and the fact that CFLAGS and TIFF_INSTALL_DIR already contain quote marks.
    add_custom_command(
        OUTPUT "${TIFF_BUILD_DIR}/config.log"
        COMMAND ./configure
        ARGS
            CFLAGS=${CFLAGS}
            CXXFLAGS=${CFLAGS}
            --disable-jpeg --disable-zlib --disable-pixarlog --enable-static --disable-cxx --disable-shared
            --prefix=${TIFF_INSTALL_DIR}
        DEPENDS "${TIFF_BUILD_DIR}/VERSION"
        WORKING_DIRECTORY "${TIFF_BUILD_DIR}"
        COMMENT "Configuring tiff library"
        VERBATIM
    )
    ### Make tiff library ###
    add_custom_command(
        OUTPUT "${TIFF_BUILD_DIR}/libtiff/.libs/libtiff.a"
        COMMAND make clean
        COMMAND make
        DEPENDS "${TIFF_BUILD_DIR}/config.log"
        WORKING_DIRECTORY "${TIFF_BUILD_DIR}"
        COMMENT "Building tiff library"
    )
    ### Install tiff library ###
    add_custom_command(
        OUTPUT
            "${TIFF_INSTALL_DIR}/lib/libtiff.a"
            # "${TIFF_INSTALL_DIR}/include/tiff.h"
        COMMAND make ARGS install
        DEPENDS "${TIFF_BUILD_DIR}/libtiff/.libs/libtiff.a"
        WORKING_DIRECTORY "${TIFF_BUILD_DIR}"
        COMMENT "Installing tiff library"
    )
    # set include, lib variables
    set(CUSTOM_TIFF_LIBRARY "${TIFF_INSTALL_DIR}/lib/libtiff.a")
    set(CUSTOM_TIFF_INCLUDE_DIR "${TIFF_INSTALL_DIR}/include")
endmacro()

# Build libtiff library
if(UNIX)
    set(TIFF_TARFILE "${CMAKE_CURRENT_SOURCE_DIR}/tiff-3.8.2.1.tar.gz")
    set(TIFF_FLAGS -O3)
    # Mac needs fat binaries
    if(APPLE)
        # Tiff library build system does not work well with multiple "-arch" arguments,
        # So make multiple libraries and "lipo" them together.
        foreach(OSX_ARCH ${CMAKE_OSX_ARCHITECTURES})
            set(ARCH_TIFF_FLAGS "${TIFF_FLAGS} -arch ${OSX_ARCH}")
            set(TIFF_INSTALL_DIR "${TIFF_BUILD_DIR_PARENT}/install_${OSX_ARCH}")
            # message(${ARCH_TIFF_FLAGS})
            install_custom_tiff_library("${TIFF_TARFILE}" "${ARCH_TIFF_FLAGS}" ${OSX_ARCH})
            set(ARCH_LIBS ${ARCH_LIBS} ${CUSTOM_TIFF_LIBRARY})
        endforeach()
        # Leave CUSTOM_TIFF_INCLUDE_DIR to whatever the final arch value was.
        # but create a new CUSTOM_TIFF_LIBRARY using lipo.
        set(TIFF_BUILD_DIR_PARENT ${CMAKE_CURRENT_BINARY_DIR}/build_tiff)
        set(TIFF_INSTALL_DIR "${TIFF_BUILD_DIR_PARENT}/install")
        file(MAKE_DIRECTORY "${TIFF_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${TIFF_INSTALL_DIR}/lib")
        set(CUSTOM_TIFF_LIBRARY "${TIFF_INSTALL_DIR}/lib/libtiff.a")
        add_custom_command(
            OUTPUT "${CUSTOM_TIFF_LIBRARY}"
            COMMAND lipo
            ARGS ${ARCH_LIBS} -create -output ${CUSTOM_TIFF_LIBRARY}
            DEPENDS ${ARCH_LIBS}
            WORKING_DIRECTORY "${TIFF_INSTALL_DIR}/lib"
            COMMENT "Combining tiff libraries into a fat binary"
            VERBATIM
        )
    else(APPLE) # Linux
        # Linux, especially 64-bit, need -fPIC
        set(TIFF_FLAGS "${TIFF_FLAGS} -fPIC")
        install_custom_tiff_library("${TIFF_TARFILE}" "${TIFF_FLAGS}" "")
    endif(APPLE)

    add_custom_target(CustomTiffLibrary
        DEPENDS
            "${CUSTOM_TIFF_LIBRARY}"
            # "${CUSTOM_TIFF_INCLUDE_DIR}/tiff.h"
        COMMENT "Using custom tiff library"
    )
    if(V3D_USE_OWN_TIFF)
        set(TIFF_LIBRARY "${CUSTOM_TIFF_LIBRARY}" CACHE FILEPATH "Location of libtiff library file" FORCE)
        set(TIFF_INCLUDE_DIR "${CUSTOM_TIFF_INCLUDE_DIR}" CACHE PATH "Directory containing libtiff header files, such as tiff.h" FORCE)
    endif(V3D_USE_OWN_TIFF)
endif(UNIX)

# Windows build of (previously unpacked) tiff library
if(MSVC AND V3D_USE_OWN_TIFF)
    # You might need to manually unpack the tiff source code before this will work
    find_path(TIFF_SOURCE_DIR 
        NAMES libtiff/tiffio.h
        PATHS 
            "${CMAKE_CURRENT_SOURCE_DIR}/tiff-4.0.1"
        NO_DEFAULT_PATH)
    # message("TIFF_SOURCE_DIR = ${TIFF_SOURCE_DIR}")
    if(TIFF_SOURCE_DIR)
        # Rules deduced from reading libtiff/Makefile.vc Mar 19 2012 CMB
        # Copy/rename tif_config.vc.h, tiffconf.vc.h
        # Include vc specific files BEFORE regular tiff includes
        include_directories(${CMAKE_CURRENT_BINARY_DIR})
        configure_file(${TIFF_SOURCE_DIR}/libtiff/tif_config.vc.h
            ${CMAKE_CURRENT_BINARY_DIR}/tif_config.h)
        configure_file(${TIFF_SOURCE_DIR}/libtiff/tiffconf.vc.h
            ${CMAKE_CURRENT_BINARY_DIR}/tiffconf.h)
        include_directories(${TIF_SOURCE_DIR}/libtiff)
        set(TIFF_SOURCES
            ${TIFF_SOURCE_DIR}/libtiff/tif_aux.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_close.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_codec.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_color.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_compress.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_dir.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_dirinfo.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_dirread.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_dirwrite.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_dumpmode.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_error.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_extension.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_fax3.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_fax3sm.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_getimage.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_jbig.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_jpeg.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_jpeg_12.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_ojpeg.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_flush.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_luv.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_lzw.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_next.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_open.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_packbits.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_pixarlog.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_predict.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_print.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_read.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_stream.cxx
            ${TIFF_SOURCE_DIR}/libtiff/tif_swab.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_strip.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_thunder.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_tile.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_version.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_warning.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_write.c
            ${TIFF_SOURCE_DIR}/libtiff/tif_zip.c
            )
        option(V3D_TIFF_USE_WIN_CRT_LIB TRUE)
        if (V3D_TIFF_USE_WIN_CRT_LIB)
            set(TIFF_SOURCES ${TIFF_SOURCES} ${TIFF_SOURCE_DIR}/libtiff/tif_unix.c)
            add_definitions(-DAVOID_WIN32_FILEIO)
        else()
            set(TIFF_SOURCES ${TIFF_SOURCES} ${TIFF_SOURCE_DIR}/libtiff/tif_win32.c)
            add_definitions(-DUSE_WIN32_FILEIO)
        endif()
        set(TIFF_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR};${TIFF_SOURCE_DIR}/libtiff" CACHE PATH "" FORCE)
        # To keep things simple, don't support JPEG, PACKBITS, ZIP etc, but do support LZW
        add_definitions(-DLZW_SUPPORT)
        add_library(libtiff STATIC ${TIFF_SOURCES})
        set(TIFF_LIBRARY libtiff CACHE STRING "" FORCE)
    endif()
endif()
