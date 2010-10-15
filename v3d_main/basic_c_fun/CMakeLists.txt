if (MSVC)
  include_directories(${TIFF_INCLUDE_DIR})
endif()

SET(V3DInterface_SRCS
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

ADD_LIBRARY(V3DInterface ${V3DInterface_SRCS})

if (MSVC)
  target_link_libraries(V3DInterface ${TIFF_LIBRARY})
else()
  target_link_libraries(V3DInterface mylib_tiff)
endif()
