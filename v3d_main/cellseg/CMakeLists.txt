set(CellSeg_SRCS
  template_matching_seg.cpp
  )

QT4_WRAP_UI(QT_UI_SRCS
  ../v3d/template_matching_cellseg.ui
  )

QT4_WRAP_CPP(QT_MOC_SRCS
  #  bwlabel.h
  )

include_directories(${V3D_BINARY_DIR}/cellseg)
include_directories(${V3D_BINARY_DIR}/v3dbase)

add_library( cellseg ${CellSeg_SRCS} ${QT_UI_SRCS} ${QT_MOC_SRCS} )

target_link_libraries( cellseg )

