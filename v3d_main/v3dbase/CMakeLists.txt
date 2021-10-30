set(V3DBASE_SRCS
  ../v3d/v3d_version_info.cpp
  ../v3d/surfaceobj_geometry_dialog.cpp
  )

set(V3DBASE_UI_SRCS
  ../v3d/v3d_global_preference.ui
  ../v3d/template_matching_cellseg.ui
  ../v3d/landmark_property.ui
  ../v3d/surface_obj_annotation.ui
  ../v3d/surfaceobj_geometry_dialog.ui
  ../v3d/dialog_update_v3d.ui
  ../v3d/dialog_update_list.ui
  ../v3d/dialog_update_options.ui
  ../v3d/dialog_update_downloading.ui
  ../v3d/dialog_update_checking.ui
  ../v3d/v3d_global_preference.ui
  ../v3d/dialog_curve_trace.ui
  )

set(V3DBASE_MOC_SRCS
  ../v3d/surfaceobj_geometry_dialog.h
  ../v3d/surfaceobj_annotation_dialog.h
  ../v3d/template_matching_cellseg_dialog.h
  ../v3d/v3d_version_info.h
  )

if(NOT Qt5Core_FOUND)
  QT4_WRAP_UI( QT_UI_SRCS ${V3DBASE_UI_SRCS} )
  QT4_WRAP_CPP(QT_MOC_SRCS ${V3DBASE_MOC_SRCS})
else()
  QT5_WRAP_UI( QT_UI_SRCS ${V3DBASE_UI_SRCS} )
  QT5_WRAP_CPP(QT_MOC_SRCS ${V3DBASE_MOC_SRCS})
endif()

include_directories(BEFORE ../basic_c_fun )
include_directories(BEFORE ${V3D_BINARY_DIR}/v3d)
include_directories(BEFORE ${V3D_BINARY_DIR}/v3dbase)
include_directories(${TIFF_INCLUDE_DIR})

add_library(v3dbase1 ${QT_UI_SRCS} ${QT_MOC_SRCS} ${V3DBASE_SRCS})

target_link_libraries( v3dbase1 neuron_editing )
target_link_libraries( v3dbase1 ${QT_QTXML_LIBRARY} )

if(V3D_USE_OWN_TIFF)
    add_dependencies(v3dbase1 CustomTiffLibrary)
endif(V3D_USE_OWN_TIFF)

# Dependency of interface on some overly included ui headers (cellseg)
add_dependencies(V3DInterface v3dbase1)

