set(CellSeg_SRCS
  template_matching_seg.cpp
  )

set(CELLSEQ_UI_SRCS
  ../v3d/template_matching_cellseg.ui
  )

set(CELLSEQ_MOC_SRCS
  #  bwlabel.h
  )

if(NOT Qt5Core_FOUND)
  QT4_WRAP_UI( QT_UI_SRCS ${CELLSEQ_UI_SRCS} )
  QT4_WRAP_CPP(QT_MOC_SRCS ${CELLSEQ_MOC_SRCS})
else()
  QT5_WRAP_UI( QT_UI_SRCS ${CELLSEQ_UI_SRCS} )
  QT5_WRAP_CPP(QT_MOC_SRCS ${CELLSEQ_MOC_SRCS})
endif()


include_directories(${V3D_BINARY_DIR}/cellseg)
include_directories(${V3D_BINARY_DIR}/v3dbase)

add_library( cellseg ${CellSeg_SRCS} ${QT_UI_SRCS} ${QT_MOC_SRCS} )

target_link_libraries( cellseg )

