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
            --disable-jpeg --disable-zlib --disable-pixarlog --enable-static --enable-cxx
            --prefix=${TIFF_INSTALL_DIR}
        DEPENDS "${TIFF_BUILD_DIR}/VERSION"
        WORKING_DIRECTORY "${TIFF_BUILD_DIR}"
        COMMENT "Configuring tiff library"
        VERBATIM
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
            "${CUSTOM_TIFF_INCLUDE_DIR}/tiff.h"
        COMMENT "Using custom tiff library"
    )
    if(V3D_USE_OWN_TIFF)
        set(TIFF_LIBRARY "${CUSTOM_TIFF_LIBRARY}" CACHE FILEPATH "Location of libtiff library file" FORCE)
        set(TIFF_INCLUDE_DIR "${CUSTOM_TIFF_INCLUDE_DIR}" CACHE PATH "Directory containing libtiff header files, such as tiff.h" FORCE)
    endif(V3D_USE_OWN_TIFF)
endif(UNIX)

