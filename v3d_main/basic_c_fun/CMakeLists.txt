include_directories(${TIFF_INCLUDE_DIR})

set(V3DInterface_SRCS
  basic_4dimage.cpp
  basic_memory.cpp
  basic_surf_objs.cpp
  imageio_mylib.cpp
  mg_image_lib.cpp
  mg_utilities.cpp
  stackutil.cpp
  v3d_message.cpp
  vcdiff.cpp
  )

add_library(V3DInterface ${V3DInterface_SRCS})

target_link_libraries(V3DInterface ${TIFF_LIBRARY})
target_link_libraries(V3DInterface ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
