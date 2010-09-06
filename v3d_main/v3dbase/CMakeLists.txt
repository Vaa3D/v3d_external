set(V3DBASE_SRCS
  ../v3d/v3d_version_info.cpp
  )

QT4_WRAP_UI(QT_UI_SRCS
  ../v3d/v3d_global_preference.ui
  ../v3d/template_matching_cellseg.ui
  ../v3d/landmark_property.ui
  ../v3d/surface_obj_annotation.ui
  ../v3d/surfaceobj_geometry_dialog.ui
  )

QT4_WRAP_CPP(QT_MOC_SRCS
  )

include_directories(${V3D_BINARY_DIR}/v3d)

add_library(v3dbase1 ${QT_UI_SRCS} ${QT_MOC_SRCS} ${V3DBASE_SRCS})
