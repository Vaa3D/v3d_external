# Include hacked support for nvidia's 3d-vision stereo emitter
if(APPLE)
    include(ExternalProject)
    #
    set(LIBUSB_DIR "${CMAKE_CURRENT_BINARY_DIR}/libusb")
    ExternalProject_Add(
        LibUsb-1-0-9
        URL http://sourceforge.net/projects/libusb/files/libusb-1.0/libusb-1.0.9/libusb-1.0.9.tar.bz2
        PREFIX "${LIBUSB_DIR}"
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND <SOURCE_DIR>/configure
            --prefix=<INSTALL_DIR> --enable-static --disable-shared
    )
    include_directories(${LIBUSB_DIR}/include/libusb-1.0)
    set(LIBUSB_LIBRARY ${LIBUSB_DIR}/lib/libusb-1.0.a)
    #
    find_package(X11)
    include_directories(${X11_INCLUDE_DIR})
    find_library(OPENGL_X11_LIBRARY GL
        PATHS /Developer/SDKs/MacOSX10.6.sdk/usr/X11/lib/GL
    )
    find_path(OPENGL_X11_INCLUDE_DIR
        GL/glx.h
        PATHS /usr/X11/include
    )
    include_directories(${OPENGL_X11_INCLUDE_DIR})
    #
    add_library(nanvstlib STATIC 
            nvstusb.c nvstusb.h
            usb_libusb.c usb.h
            nanvst.cpp)
    target_link_libraries(nanvstlib
        ${LIBUSB_LIBRARY}
        ${OPENGL_X11_LIBRARY}
        ${X11_LIBRARIES}
    )
    add_dependencies(nanvstlib LibUsb-1-0-9)
    add_dependencies(NeuronAnnotatorLib nanvstlib)
endif(APPLE)
