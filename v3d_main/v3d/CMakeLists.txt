set(V3D_SRCS
  main.cpp
  vano_linker_loader_dialog.cpp
  atlas_viewer.cpp
  # atlas_window.cpp   looks for image_window.h : non-existing file : FIXME
  colormap.cpp
  dialog_keypoint_features.cpp
  v3d_version_info.cpp
# dialog_pointcloudatlas_linkerloader.cpp  # missing the pointcloud_atlas_io.h file.
  dialog_rotate.cpp
  dialog_watershed_para.cpp
  glwidget.cpp
  histogramsimple.cpp
  v3dimg_proc_neuron.cpp
  landmark_property_dialog.cpp
  mainwindow_interface.cpp
  idrawmain.cpp
  rotate_image.cpp
  v3dimgproc_entry.cpp
  import_filelistname.cpp
  import_tiffseries.cpp
  mainwindow.cpp
  mdichild.cpp
  my4dimage.cpp
  surfaceobj_geometry_dialog.cpp
  v3d_core.cpp
  )

QT4_WRAP_UI(QT_UI_SRCS
  dialog_curve_trace.ui
  dialog_imagecrop_bbox.ui
  dialog_imageresample.ui
  dialog_keypoint_features.ui
  dialog_maskroi.ui
  dialog_pointcloudatlas_linkerloader.ui
  dialog_vano_linkerloader.ui
  FL_levelsetSegPara.ui
  FL_watershedSegPara.ui
  import_filelist_dialog.ui
  import_images_tool.ui
  landmark_property.ui
  surface_obj_annotation.ui
  surfaceobj_geometry_dialog.ui
  template_matching_cellseg.ui
  v3d_global_preference.ui
  )

QT4_WRAP_CPP(QT_MOC_SRCS
  )

include_directories(${V3D_BINARY_DIR}/v3d)

add_library(v3dbase ${QT_UI_SRCS} ${QT_MOC_SRCS})
target_link_libraries( v3dbase newmat11 ${QT_LIBRARIES} )

add_executable( v3d ${V3D_SRCS} )

target_link_libraries( v3d v3dbase )
