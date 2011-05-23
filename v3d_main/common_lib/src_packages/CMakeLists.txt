#
#

if (NOT MSVC)
  add_subdirectory(mylib_tiff)
endif()

if(UNIX)
    # Build tiff library
    set(TIFF_TARFILE "${CMAKE_CURRENT_SOURCE_DIR}/tiff-3.8.2.1.tar.gz")
    # message(${TIFF_TARFILE})
    set(TIFF_BUILD_DIR_PARENT ${CMAKE_CURRENT_BINARY_DIR}/build_tiff)
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
    ### Configure tiff library ###
    set(TIFF_INSTALL_DIR "${TIFF_BUILD_DIR_PARENT}/install")
    file(MAKE_DIRECTORY "${TIFF_INSTALL_DIR}")
    add_custom_command(
        OUTPUT "${TIFF_BUILD_DIR}/config.log"
        COMMAND ./configure
        ARGS CFLAGS=\"-O3 -fPIC\" CXXFLAGS=\"-O3 -fPIC\" --disable-jpeg --disable-zlib --disable-pixarlog --enable-static --enable-cxx --prefix=\"${TIFF_INSTALL_DIR}\"
        DEPENDS "${TIFF_BUILD_DIR}/VERSION"
        WORKING_DIRECTORY "${TIFF_BUILD_DIR}"
        COMMENT "Configuring tiff library"
    )
    ### Make tiff library ###
    add_custom_command(
        OUTPUT "${TIFF_BUILD_DIR}/libtiff/.libs/libtiff.a"
        COMMAND make
        DEPENDS "${TIFF_BUILD_DIR}/config.log"
        WORKING_DIRECTORY "${TIFF_BUILD_DIR}"
        COMMENT "Building tiff library"
    )
    ### Install tiff library ###
    add_custom_command(
        OUTPUT 
            "${TIFF_INSTALL_DIR}/lib/libtiff.a"
            "${TIFF_INSTALL_DIR}/include/tiff.h"
        COMMAND make ARGS install
        DEPENDS "${TIFF_BUILD_DIR}/libtiff/.libs/libtiff.a"
        WORKING_DIRECTORY "${TIFF_BUILD_DIR}"
        COMMENT "Installing tiff library"
    )
    # set include, lib variables
    set(CUSTOM_TIFF_LIBRARY "${TIFF_INSTALL_DIR}/lib/libtiff.a")
    set(CUSTOM_TIFF_INCLUDE_DIR "${TIFF_INSTALL_DIR}/include")
    add_custom_target(CustomTiffLibrary
        DEPENDS
            "${CUSTOM_TIFF_LIBRARY}"
            "${CUSTOM_TIFF_INCLUDE_DIR}/tiff.h"
        COMMENT "Building custom tiff library"
    )
    if(V3D_USE_OWN_TIFF)
        set(TIFF_LIBRARY "${CUSTOM_TIFF_LIBRARY}" CACHE FILEPATH "Location of libtiff library file" FORCE)
        set(TIFF_INCLUDE_DIR "${CUSTOM_TIFF_INCLUDE_DIR}" CACHE PATH "Directory containing libtiff header files, such as tiff.h" FORCE)
    endif(V3D_USE_OWN_TIFF)
endif(UNIX)

