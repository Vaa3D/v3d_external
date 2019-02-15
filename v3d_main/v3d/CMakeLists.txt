set(V3D_SRCS
  # main.cpp
  dialog_rotate.cpp
  # dialog_watershed_para.cpp
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
  ../custom_toolbar/v3d_custom_toolbar.cpp
  mainwindow_interface.cpp
  v3d_core.cpp
  dialog_keypoint_features.cpp
  landmark_property_dialog.cpp
  DownloadManager.cpp
  CommandManager.cpp
  v3d_application.cpp
  ../cell_counter/CellCounter3D.cpp
  v3d_commandlineparser.cpp
  pluginfunchandler.cpp
  ChannelTable.cpp
  mapview.cpp
  # ../neuron_toolbox/vaa3d_neurontoolbox.cpp
  ../basic_c_fun/basic_4dimage_create.cpp
  ../neuron_annotator/analysis/AlignerUtils.cpp
  ../io/io_bioformats.cpp
  v3d_actions.cpp
  )

set(ARTHUR_SRCS "")

set(V3D_RCCS
    v3d.qrc
    ../3drenderer/3drenderer.qrc
    ../terafly/icons.qrc
    ../neuron_annotator/resources.qrc)

set(V3D_UI_SRCS
    v3d_global_preference.ui
    template_matching_cellseg.ui
    landmark_property.ui
    surface_obj_annotation.ui
    surfaceobj_geometry_dialog.ui
    dialog_update_v3d.ui
    dialog_update_list.ui
    dialog_update_options.ui
    dialog_update_downloading.ui
    dialog_update_checking.ui
    dialog_curve_trace.ui
    dialog_imagecrop_bbox.ui
    dialog_imageresample.ui
    dialog_keypoint_features.ui
    dialog_maskroi.ui
    # dialog_pointcloudatlas_linkerloader.ui
    # dialog_vano_linkerloader.ui
    # FL_levelsetSegPara.ui
    # FL_watershedSegPara.ui
    import_filelist_dialog.ui
    import_images_tool.ui
    dialog_url_entry.ui
)

set(V3D_MOC_SRCS
    atlas_viewer.h
    dialog_rotate.h
    # dialog_watershed_para.h
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
    v3d_version_info.h
    v3d_actions.h
    v3d_application.h
    ChannelTable.h
    ../custom_toolbar/v3d_custom_toolbar.h
  )

if(Qt5Core_FOUND)
  set(ARTHUR_MOC_HEADERS
      ${arthurwidgets_path}/arthurwidgets.h
      ${arthurwidgets_path}/arthurstyle.h
      ${arthurwidgets_path}/hoverpoints.h
  )
  set(ARTHUR_SRCS
      ${arthurwidgets_path}/arthurwidgets.cpp
      ${arthurwidgets_path}/arthurstyle.cpp
      ${arthurwidgets_path}/hoverpoints.cpp
  )
  set_source_files_properties( ${ARTHUR_SRCS} PROPERTIES GENERATED TRUE )
endif()

if(NOT Qt5Core_FOUND)
  QT4_WRAP_UI( QT_UI_SRCS ${V3D_UI_SRCS} )
  QT4_WRAP_CPP(QT_MOC_SRCS ${V3D_MOC_SRCS})
  QT4_ADD_RESOURCES(V3D_RCC_SRCS ${V3D_RCCS})
else()
  QT5_WRAP_UI( QT_UI_SRCS ${V3D_UI_SRCS} )
  QT5_WRAP_CPP(QT_MOC_SRCS ${V3D_MOC_SRCS})
  QT5_WRAP_CPP(ARTHUR_MOC_SRCS ${ARTHUR_MOC_HEADERS})
  QT5_ADD_RESOURCES(V3D_RCC_SRCS ${V3D_RCCS})
endif()

include_directories( BEFORE
  "${CMAKE_CURRENT_BINARY_DIR}/../v3dbase"
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}/../neuron_annotator"
  ../basic_c_fun
  ../basic_c_fun/customary_structs
  ../3drenderer
  ../neuron_annotator
  ../mozak/terafly/src/core/imagemanager
  ../mozak/terafly/src/presentation
  ../mozak/m_terafly/src/control
  "${CMAKE_CURRENT_SOURCE_DIR}"
  )

if(Qt5Core_FOUND)
  add_library(arthurwidgets ${ARTHUR_MOC_SRCS} ${ARTHUR_SRCS} )
  add_dependencies(arthurwidgets qt_demos)
endif()


add_library(v3dbase ${QT_UI_SRCS} ${QT_MOC_SRCS})

target_link_libraries( v3dbase v3dbase1 jba newmat11 multithreadimageIO ${QT_LIBRARIES} )


### boost
SET(Boost_ADDITIONAL_VERSIONS "1.46.0" "1.57.0" "1.56.1" "1.56.0" "1.54" "1.54.0")
find_package(Boost COMPONENTS system regex)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY_DIRS})
endif()



##########################
# NRRD IO support
include (ExternalProject)

add_library(v3dbase2 ${V3D_SRCS})

add_dependencies(v3dbase2 v3dbase)
add_dependencies(v3dbase2 NeuronAnnotatorLib)
add_dependencies(v3dbase NeuronAnnotatorLib)

if (WIN32) # Windows MSYS build
  # All this MYS stuff is just to compile in a program icon for windows
  set(CMAKE_RC_COMPILER_INIT windres)
  enable_language(RC)
  set(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> -O coff -i <SOURCE> -o <OBJECT>")
  add_executable( v3d # *not* WIN32, which causes MSVC to call WinMain() instead of main()
      main.cpp
      ${V3D_RCC_SRCS}
      v3d_win.rc)
else()

  add_executable( v3d
      main.cpp
      ${V3D_RCC_SRCS})
endif()

if(V3D_PACKAGE_AS_NEURON_ANNOTATOR)
    set_target_properties(v3d PROPERTIES
        OUTPUT_NAME NeuronAnnotator)
else()
    set_target_properties(v3d PROPERTIES
        OUTPUT_NAME vaa3d)
endif()

target_link_libraries( v3d
  v3dbase2
  v3dbase
  V3DInterface
  v3d_plugin_loader
  3drenderer
  neuron_tracing
  worm_straighten_c
  terafly
  # mylib_tiff
  cellseg
  ${OPENGL_glu_LIBRARY}
  )

if(MSVC)
  target_link_libraries(v3d ${TIFF_LIBRARY})
else()
  target_link_libraries(v3d mylib_tiff)
endif()
add_dependencies(v3d v3dbase2)


if(UNIX)
  find_package(ZLIB)
  find_package(JPEG)
  mark_as_advanced(CLEAR ZLIB_LIBRARY JPEG_LIBRARY)
  target_link_libraries(v3d ${ZLIB_LIBRARY} ) #${JPEG_LIBRARY})
endif()

# CMB 11-Oct-2010
if(NOT Qt5Core_FOUND)
  target_link_libraries(v3d ${QT4_DEMOS_LIBRARY})
  target_link_libraries(v3d ${QT_QTNETWORK_LIBRARY})
  target_link_libraries(v3d ${QT_QTXML_LIBRARY})
  target_link_libraries(v3d ${QT_QTGUI_LIBRARY})
  target_link_libraries(v3d ${QT_QTCORE_LIBRARY})
else()
  target_link_libraries(v3d arthurwidgets)
  target_link_libraries(v3d Qt5::Widgets Qt5::Core Qt5::Network Qt5::Xml
                            Qt5::OpenGL Qt5::Concurrent)
endif()

if(APPLE)
    # There are two ways to build V3D on Mac.
    # Either as an application bundle in directory "v3d.app",
    # or as a command line tool in directory "v3d".
    # Redefine V3D_MAC_CREATE_BUNDLE variable, in case this is not built
    # from very top trunk directory.
    set(V3D_MAC_CREATE_BUNDLE TRUE CACHE BOOL
        "Create V3D.app application bundle, instead of v3d command line tool")
    if (V3D_MAC_CREATE_BUNDLE)
        # Build app bundle in place at build time
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/v3d")
        if(V3D_PACKAGE_AS_NEURON_ANNOTATOR)
            set(BUNDLE_APP_NAME NeuronAnnotator.app)
        else()
            set(BUNDLE_APP_NAME Vaa3d.app)
        endif()
        if(V3D_STAGING_BASE_DIR)
            set(BUNDLE_BUILD_DIR "${V3D_STAGING_BASE_DIR}/${BUNDLE_APP_NAME}" CACHE PATH "Where to build Mac OS X application bundle" FORCE)
        else()
            set(BUNDLE_BUILD_DIR "${CMAKE_BINARY_DIR}/v3d/${BUNDLE_APP_NAME}" CACHE PATH "Where to build Mac OS X application bundle" FORCE)
        endif()
        set(V3D_BUILD_BINARY_DIR "${BUNDLE_BUILD_DIR}/Contents/MacOS" CACHE INTERNAL "" FORCE)
        # file(REMOVE_RECURSE "${BUNDLE_BUILD_DIR}/Contents/Frameworks") # Dangerous...
        # file(REMOVE_RECURSE "${BUNDLE_BUILD_DIR}/Contents/Plugins") # Dangerous...
        file(MAKE_DIRECTORY "${BUNDLE_BUILD_DIR}"
                            "${BUNDLE_BUILD_DIR}/Contents"
                            "${BUNDLE_BUILD_DIR}/Contents/Frameworks"
                            "${BUNDLE_BUILD_DIR}/Contents/MacOS"
                            "${BUNDLE_BUILD_DIR}/Contents/Resources"
        )
        # MACOSX_BUNDLE_WHATEVER parameters must be set before configure_file statement on Info.plist is called
        # MACOSX_BUNDLE_EXECUTABLE_NAME should match CPACK_BUNDLE_NAME
        # Launch script is called v3d_script (not just 'v3d'), because...
        # ...actual binary must be called "v3d", so macdeployqt tool can find it.
        set(MACOSX_BUNDLE_EXECUTABLE_NAME vaa3d_script)
        set(MACOSX_BUNDLE_INFO_STRING "vaa3d")
        set(MACOSX_BUNDLE_ICON_FILE "vaa3d.icns")
        set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.hhmi.janelia.vaa3d")
        set(MACOSX_BUNDLE_LONG_VERSION_STRING ${V3D_VERSION})
        set(MACOSX_BUNDLE_BUNDLE_NAME vaa3d)
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${V3D_VERSION})
        set(MACOSX_BUNDLE_BUNDLE_VERSION ${PACKAGE_VERSION})
        set(MACOSX_BUNDLE_COPYRIGHT "(c) 2006-2012 Hanchuan Peng")
        configure_file(
          ${CMAKE_CURRENT_SOURCE_DIR}/../CMake/package/MacOSXBundleInfo.plist.in
          ${BUNDLE_BUILD_DIR}/Contents/Info.plist
          @ONLY
        )
        if(V3D_PACKAGE_AS_NEURON_ANNOTATOR)
            configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/../CMake/package/neuron_annotator_mac.sh"
                "${BUNDLE_BUILD_DIR}/Contents/MacOS/${MACOSX_BUNDLE_EXECUTABLE_NAME}"
                COPYONLY)
        else()
            configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/../CMake/package/v3d_mac.sh"
                "${BUNDLE_BUILD_DIR}/Contents/MacOS/${MACOSX_BUNDLE_EXECUTABLE_NAME}"
                COPYONLY)
        endif()
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/../v3d/vaa3d.icns"
            "${BUNDLE_BUILD_DIR}/Contents/Resources/vaa3d.icns"
            COPYONLY)
        install(DIRECTORY ${BUNDLE_BUILD_DIR}
            DESTINATION "."
            USE_SOURCE_PERMISSIONS)
    else(V3D_MAC_CREATE_BUNDLE)
        # Don't create a bundle, just a v3d folder containing v3d and plugins
        # Place v3d in a folder called "v3d"
        set(V3D_INSTALL_DIR "vaa3d" CACHE INTERNAL "Relative path where V3D will be installed")
        install(TARGETS v3d
            DESTINATION ${V3D_INSTALL_DIR}
            COMPONENT v3d)
    endif(V3D_MAC_CREATE_BUNDLE)

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
      if(Qt5Core_FOUND)
        set(SHLIBS jpeg62.dll libtiff3.dll Qt5Core.dll Qt5Gui.dll Qt5Network.dll
                    Qt5OpenGL.dll Qt5Xml.dll )
      else()
        set(SHLIBS jpeg62.dll libtiff3.dll QtCore4.dll QtGui4.dll QtNetwork4.dll
                    QtOpenGL4.dll QtXml4.dll )
      endif()

      # Install Qt plugins for image formats, at least on Windows
      if (QT_PLUGINS_DIR)
          file(GLOB QT_IMAGEFORMAT_PLUGINS "${QT_PLUGINS_DIR}/imageformats/*.dll")
          install(PROGRAMS ${QT_IMAGEFORMAT_PLUGINS}
          DESTINATION "${V3D_INSTALL_DIR}/imageformats"
          COMPONENT v3d)
      endif()
      if(USE_FFMPEG)
          file(GLOB FFMPEG_DLLS "${av_dir}/bin/*.dll")
          install(PROGRAMS ${FFMPEG_DLLS}
          DESTINATION "${V3D_INSTALL_DIR}"
          COMPONENT v3d)
      endif()
      if(USE_HDF5)
          file(GLOB HDF5_DLLS "${hdf_dir}/install/bin/*.dll")
          install(PROGRAMS ${HDF5_DLLS}
          DESTINATION "${V3D_INSTALL_DIR}"
          COMPONENT v3d)
      endif()
    endif()
    if(UNIX AND NOT APPLE) # Linux
        set(LIB_PATHS "")
        set(SHLIBS "")
        # On second thought, don't install any libraries with linux.
        # We should document dependencies.
        # set(SHLIBS libtiff.so.4 libz.so.1 libjpeg.so.62
        #     libQtCore.so.4 libQtGui.so.4 libQtNetwork.so.4 libQtOpenGL.so.4 libQtXml.so.4)
        message("installing ffmpeg libraries into ${V3D_BUILD_BINARY_DIR}")
        add_custom_command( TARGET v3d POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E tar cfz "${V3D_BUILD_BINARY_DIR}/libs.tgz" "lib/lib*${CMAKE_SHARED_LIBRARY_SUFFIX}*"
                            WORKING_DIRECTORY "${av_dir}/install")
        add_custom_command( TARGET v3d POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E tar xfz "${V3D_BUILD_BINARY_DIR}/libs.tgz"
                            COMMAND ${CMAKE_COMMAND} -E remove "${V3D_BUILD_BINARY_DIR}/libs.tgz"
                            WORKING_DIRECTORY "${V3D_BUILD_BINARY_DIR}")
        add_custom_command( TARGET v3d POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E tar cfz "${V3D_BUILD_BINARY_DIR}/libs.tgz" "lib*${CMAKE_SHARED_LIBRARY_SUFFIX}*"
                            WORKING_DIRECTORY "${x265_dir}/install/lib"
                            COMMENT "Inserting X265 libraries into v3d.app")
        add_custom_command( TARGET v3d POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E tar xfz "${V3D_BUILD_BINARY_DIR}/libs.tgz"
                            COMMAND ${CMAKE_COMMAND} -E remove "${V3D_BUILD_BINARY_DIR}/libs.tgz"
                            WORKING_DIRECTORY "${V3D_BUILD_BINARY_DIR}/lib")
    endif()
    # message("LIB_PATHS = ${LIB_PATHS}")
    foreach(SHLIB ${SHLIBS})
        set(lib_var_name "${SHLIB}_LIB_FILE")
        find_file(${lib_var_name}
            NAMES ${SHLIB}
            PATHS "${LIB_PATHS}"
            )
        if(${lib_var_name}_NOT_FOUND)
           find_file(${lib_var_name}
           NAMES ${SHLIB}
           PATHS /usr/lib /lib
           "${TIFF_INCLUDE_DIR}/../bin" # ${LIB_PATHS} does not work if spaces are in name...
           # Yes default paths, if above find_library failed
           )
        endif()

        set(lib_name "${${lib_var_name}}")
	#message("set ${lib_name} = ${${lib_var_name}} ")
        # Follow symlinks to the real file
        # So we don't just install symlinks
        if(lib_name)
            # message("Found ${lib_name}")
        else()
            # message("${SHLIB} (${lib_name}) not found")

        endif()
        get_filename_component(real_lib ${lib_name} REALPATH)
        get_filename_component(lib_name ${lib_name} NAME) # Strip path

        if(real_lib)
            #message("install  ${real_lib}")
            install(PROGRAMS
                ${real_lib}
                DESTINATION ${V3D_INSTALL_DIR}
                COMPONENT RuntimeLibraries
                RENAME ${lib_name}
        )
        endif()
	#mark_as_advanced(${lib_var_name})
    endforeach()

endif(APPLE)


# Enforce a particular location for v3d.exe at build time.  So it can be next to plugins, for testing  before install.
# This must be defined after above check for BUNDLE_BUILD_DIR
if (V3D_BUILD_BINARY_DIR)
    # Build v3d directly into nascent app bundle
    set_target_properties(v3d PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${V3D_BUILD_BINARY_DIR}")
    if (MSVC)
        # hack to get around the "Debug" and "Release" directories cmake tries to add on Windows
        # But the PREFIX trick does not work with MSVC 2010
        # set_target_properties (v3d PROPERTIES PREFIX "../")
        if(V3D_BUILD_BINARY_DIR)
            set_target_properties(v3d PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${V3D_BUILD_BINARY_DIR})
            set_target_properties(v3d PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${V3D_BUILD_BINARY_DIR})
            set_target_properties(v3d PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${V3D_BUILD_BINARY_DIR})
            set_target_properties(v3d PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${V3D_BUILD_BINARY_DIR})
            set_target_properties(v3d PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${V3D_BUILD_BINARY_DIR})
            set_target_properties(v3d PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${V3D_BUILD_BINARY_DIR})
        endif()
    endif()
endif()

target_link_libraries(v3d NeuronAnnotatorLib)
if(USE_X265)
  target_link_libraries(v3d ${X265_LIBRARY})
endif(USE_X265)

if(V3D_MAC_CREATE_BUNDLE)
    # Install Qt libraries into bundle
    find_program(MAC_DEPLOY_QT macdeployqt)
    add_custom_command(TARGET v3d POST_BUILD
        COMMAND "${MAC_DEPLOY_QT}" "${BUNDLE_BUILD_DIR}" 
        DEPENDS v3d
        COMMENT "Inserting Qt libraries into v3d.app")
    add_custom_command( TARGET v3d POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E tar cfz "${V3D_BUILD_BINARY_DIR}/libs.tgz" "lib*dylib"
                        WORKING_DIRECTORY "${x265_dir}/install/lib"
                        COMMENT "Inserting X265 libraries into v3d.app")
    add_custom_command( TARGET v3d POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E tar xfz "${V3D_BUILD_BINARY_DIR}/libs.tgz"
                        COMMAND ${CMAKE_COMMAND} -E remove "${V3D_BUILD_BINARY_DIR}/libs.tgz"
                        WORKING_DIRECTORY "${BUNDLE_BUILD_DIR}/Contents/Frameworks")
endif()


