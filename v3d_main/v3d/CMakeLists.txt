set(V3D_SRCS
  main.cpp
  dialog_rotate.cpp
  dialog_watershed_para.cpp
  idrawmain.cpp
  import_filelistname.cpp
  import_tiffseries.cpp
  # dialog_pointcloudatlas_linkerloader.cpp
  # glwidget.cpp # duplicate symbol in arthurwidget.cpp
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
  # glwidget.h
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

add_executable( v3d ${V3D_SRCS} ${V3D_RCC_SRCS} )

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
# message("QtNetwork library = ${QT_QTNETWORK_LIBRARY}")
if(QT_QTNETWORK_LIBRARY)
  target_link_libraries(v3d ${QT_QTNETWORK_LIBRARY})
endif()
