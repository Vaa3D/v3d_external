set(V3D_SRCS
  main.cpp
  dialog_rotate.cpp
  dialog_watershed_para.cpp
  idrawmain.cpp
  import_filelistname.cpp
  import_tiffseries.cpp
  # dialog_pointcloudatlas_linkerloader.cpp
  glwidget.cpp # duplicate symbol in arthurwidget.cpp
  histogramsimple.cpp
  mdichild.cpp
  my4dimage.cpp
  rotate_image.cpp
  v3dimgproc_entry.cpp
  v3dimg_proc_neuron.cpp
  # vano_linker_loader_dialog.cpp
  atlas_viewer.cpp
  # atlas_window.cpp   looks for image_window.h : non-existing file : FIXME
  colormap.cpp
  mainwindow.cpp
  mainwindow_interface.cpp
  v3d_core.cpp
  dialog_keypoint_features.cpp
  landmark_property_dialog.cpp
  DownloadManager.cpp
  )

set(V3D_RCCS v3d.qrc)

QT4_ADD_RESOURCES(V3D_RCC_SRCS ${V3D_RCCS})

QT4_WRAP_UI(QT_UI_SRCS
  dialog_curve_trace.ui
  dialog_imagecrop_bbox.ui
  dialog_imageresample.ui
  dialog_keypoint_features.ui
  dialog_maskroi.ui
  # dialog_pointcloudatlas_linkerloader.ui
  # dialog_vano_linkerloader.ui
  FL_levelsetSegPara.ui
  FL_watershedSegPara.ui
  import_filelist_dialog.ui
  import_images_tool.ui
  dialog_url_entry.ui
  )

QT4_WRAP_CPP(QT_MOC_SRCS
  atlas_viewer.h
  dialog_rotate.h
  dialog_watershed_para.h
  dialog_imagecrop_bbox.h
  dialog_maskroi.h
  dialog_imageresample.h
  dialog_curve_trace_para.h
  dialog_keypoint_features.h
  DownloadManager.h
  glwidget.h
  idrawmain.h
  import_filelist_dialog.h
  import_images_tool_dialog.h
  landmark_property_dialog.h
  mdichild.h
  mainwindow.h
  xformwidget.h
  v3d_core.h
  v3d_global_preference_dialog.h
  )

include_directories(
  ${V3D_BINARY_DIR}/v3dbase
  ${V3D_BINARY_DIR}/v3d
  )

add_library(v3dbase ${QT_UI_SRCS} ${QT_MOC_SRCS})

target_link_libraries( v3dbase v3dbase1 jba newmat11 ${QT_LIBRARIES} )

if (MSYS) # Windows MSYS build
  # All this MYS stuff is just to compile in a program icon for windows
  set(CMAKE_RC_COMPILER_INIT windres)
  enable_language(RC)
  set(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> -O coff -i <SOURCE> -o <OBJECT>")
  add_executable( v3d WIN32
      ${V3D_SRCS} ${V3D_RCC_SRCS}
      v3d_win.rc)
else()
  add_executable( v3d ${V3D_SRCS} ${V3D_RCC_SRCS} )
endif()

target_link_libraries( v3d
  v3dbase
  V3DInterface
  v3d_plugin_loader
  3drenderer
  neuron_tracing
  worm_straighten_c
  # mylib_tiff
  cellseg
  ${OPENGL_glu_LIBRARY}
  ${QT4_DEMOS_LIBRARY}
  )
  
if (MSVC)
  target_link_libraries(v3d ${TIFF_LIBRARY})
else()
  target_link_libraries(v3d mylib_tiff)
endif()

# CMB 11-Oct-2010
target_link_libraries(v3d ${QT_QTNETWORK_LIBRARY})
target_link_libraries(v3d ${QT_QTXML_LIBRARY})

if(APPLE)
    # Build app bundle in place at build time
    set(BUNDLE_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/v3d.app" CACHE PATH "Where to build Mac OS X application bundle")
    file(REMOVE_RECURSE "${BUNDLE_BUILD_DIR}/Contents/Frameworks") # Dangerous...
    file(REMOVE_RECURSE "${BUNDLE_BUILD_DIR}/Contents/Plugins") # Dangerous...
    file(MAKE_DIRECTORY "${BUNDLE_BUILD_DIR}"
                        "${BUNDLE_BUILD_DIR}/Contents"
                        "${BUNDLE_BUILD_DIR}/Contents/MacOS"
                        "${BUNDLE_BUILD_DIR}/Contents/Resources"
    )
    # Build v3d directly into nascent app bundle
    # Call binary v3d_bin, so "v3d" can be the shell script that launches it.
    set_target_properties(v3d PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY "${BUNDLE_BUILD_DIR}/Contents/MacOS"
        OUTPUT_NAME v3d_bin)
    # MACOSX_BUNDLE_WHATEVER parameters must be set before configure_file statement on Info.plist is called
    # MACOSX_BUNDLE_EXECUTABLE_NAME should match CPACK_BUNDLE_NAME
    set(MACOSX_BUNDLE_EXECUTABLE_NAME v3d)
    set(MACOSX_BUNDLE_INFO_STRING "v3d")
    set(MACOSX_BUNDLE_ICON_FILE "v3d.icns")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.hhmi.janelia.v3d")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${V3D_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_NAME v3d)
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${V3D_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PACKAGE_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT "(c) 2006-2010 Hanchuan Peng")
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/../CMake/package/MacOSXBundleInfo.plist.in
      ${BUNDLE_BUILD_DIR}/Contents/Info.plist
      @ONLY
    )
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/../CMake/package/v3d_mac.sh"
        "${BUNDLE_BUILD_DIR}/Contents/MacOS/v3d"
        COPYONLY)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/../v3d/v3d.icns"
        "${BUNDLE_BUILD_DIR}/Contents/Resources/v3d.icns"
        COPYONLY)
    install(DIRECTORY ${BUNDLE_BUILD_DIR}
        DESTINATION "."
        USE_SOURCE_PERMISSIONS)

else(APPLE)

    # On non-apple platforms, install executable, libraries, and plugins individually
    set(V3D_INSTALL_DIR "bin" CACHE INTERNAL "Relative path where V3D will be installed")
    install(TARGETS v3d 
        DESTINATION ${V3D_INSTALL_DIR}
        COMPONENT v3d)
    
    # Also install shared libraries that are needed by v3d
    if(WIN32) # well, MSYS, mostly
        get_filename_component(CXX_BINDIR "${CMAKE_CXX_COMPILER}" PATH)
        set(LIB_PATHS
            "${TIFF_INCLUDE_DIR}/../bin"
            "${QT_LIBRARY_DIR}/../bin"
            "${CXX_BINDIR}")
        set(SHLIBS zlib1 jpeg62 libtiff3
            QtCore4.dll QtGui4.dll QtNetwork4.dll QtOpenGL4.dll QtXml4.dll
            mingwm10.dll "libgcc_s_dw2-1.dll" "libstdc++-6.dll")
    endif()
    if(UNIX AND NOT APPLE) # Linux
        set(LIB_PATHS "")
        set(SHLIBS libtiff.so.4 libz.so.1 libjpeg.so.62
            libQtCore.so.4 libQtGui.so.4 libQtNetwork.so.4 libQtOpenGL.so.4 libQtXml.so.4)
    endif()
    foreach(SHLIB ${SHLIBS})
        set(lib_var_name "${SHLIB}_LIB_FILE")
        find_file(${lib_var_name}
            NAMES 
                ${SHLIB}
            PATHS ${LIB_PATHS}
            NO_DEFAULT_PATH)
        find_file(${lib_var_name}
            NAMES ${SHLIB}
            PATHS /usr/lib /lib
            # Yes default paths, if above find_library failed
            )
        set(lib_name "${${lib_var_name}}")
        # Follow symlinks to the real file
        # So we don't just install symlinks
        get_filename_component(real_lib ${lib_name} REALPATH)
        get_filename_component(lib_name ${lib_name} NAME) # Strip path
        if(real_lib)
            install(PROGRAMS
                ${real_lib}
                DESTINATION ${V3D_INSTALL_DIR}
                COMPONENT RuntimeLibraries
                RENAME ${lib_name}
        )
        endif()
        mark_as_advanced(${lib_var_name})
    endforeach()
    
endif(APPLE)
