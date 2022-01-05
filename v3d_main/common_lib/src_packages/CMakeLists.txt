
if (NOT MSVC)
  add_subdirectory(mylib_tiff)
endif()

# Build libtiff for windows, using cmake
# (Mac and linux use ExternalProject to build libtiff, in top level CMakeLists.txt file)

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
# Windows build of (previously unpacked) tiff library
# TODO -generalize this stanza for all platforms
if(MSVC AND V3D_USE_OWN_TIFF)
    # You might need to manually unpack the tiff source code before this will work
    find_path(TIFF_SOURCE_DIR 
        NAMES libtiff/tiffio.h
        PATHS 
            "${CMAKE_CURRENT_SOURCE_DIR}/tiff-4.0.1"
        NO_DEFAULT_PATH)
    mark_as_advanced(TIFF_SOURCE_DIR)
    # message("TIFF_SOURCE_DIR = ${TIFF_SOURCE_DIR}")
    if(TIFF_SOURCE_DIR)
        file(GLOB TIFF_HEADERS "${TIFF_SOURCE_DIR}/libtiff/*.h" "${TIFF_SOURCE_DIR}/libtiff/*.hxx")
        set(TIFF_SOURCES0
            tif_aux.c
            tif_close.c
            tif_codec.c
            tif_color.c
            tif_compress.c
            tif_dir.c
            tif_dirinfo.c
            tif_dirread.c
            tif_dirwrite.c
            tif_dumpmode.c
            tif_error.c
            tif_extension.c
            tif_fax3.c
            tif_fax3sm.c
            tif_getimage.c
            tif_jbig.c
            tif_jpeg.c
            tif_jpeg_12.c
            tif_ojpeg.c
            tif_flush.c
            tif_luv.c
            tif_lzw.c
            tif_next.c
            tif_open.c
            tif_packbits.c
            tif_pixarlog.c
            tif_predict.c
            tif_print.c
            tif_read.c
            tif_stream.cxx
            tif_swab.c
            tif_strip.c
            tif_thunder.c
            tif_tile.c
            tif_version.c
            tif_warning.c
            tif_write.c
            tif_zip.c
            )
		if(MSVC)
		    # Rules deduced from reading libtiff/Makefile.vc Mar 19 2012 CMB
			# Copy/rename tif_config.vc.h, tiffconf.vc.h
			# Include vc specific files BEFORE regular tiff includes
	        include_directories(${CMAKE_CURRENT_BINARY_DIR})
		    configure_file(${TIFF_SOURCE_DIR}/libtiff/tif_config.vc.h
			    ${CMAKE_CURRENT_BINARY_DIR}/tif_config.h)
			configure_file(${TIFF_SOURCE_DIR}/libtiff/tiffconf.vc.h
				${CMAKE_CURRENT_BINARY_DIR}/tiffconf.h)
			option(V3D_TIFF_USE_WIN_CRT_LIB "Build tiff library using windows common runtime" TRUE)
			if (V3D_TIFF_USE_WIN_CRT_LIB)
				set(TIFF_SOURCES0 ${TIFF_SOURCES0} tif_unix.c)
				add_definitions(-DAVOID_WIN32_FILEIO)
			else()
				set(TIFF_SOURCES0 ${TIFF_SOURCES0} tif_win32.c)
				add_definitions(-DUSE_WIN32_FILEIO)
			endif()
	        set(TIFF_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR};${TIFF_SOURCE_DIR}/libtiff" CACHE PATH "" FORCE)
		else(MSVC) # if(UNIX)?
			set(TIFF_SOURCES0 ${TIFF_SOURCES0} tif_unix.c)			
	        set(TIFF_INCLUDE_DIR "${TIFF_SOURCE_DIR}/libtiff" CACHE PATH "" FORCE)
		endif(MSVC)
		foreach(SRC ${TIFF_SOURCES0})
			set(TIFF_SOURCES ${TIFF_SOURCES} "${TIFF_SOURCE_DIR}/libtiff/${SRC}")
		endforeach()
		include_directories(${TIFF_INCLUDE_DIR})
        # To keep things simple, don't support JPEG, PACKBITS, ZIP etc, but do support LZW
        add_definitions(-DLZW_SUPPORT)
        add_library(libtiff STATIC ${TIFF_SOURCES} ${TIFF_HEADERS})
        set(TIFF_LIBRARY libtiff CACHE STRING "" FORCE)
    endif()
    add_dependencies(PluginPrerequisites libtiff)
endif()
