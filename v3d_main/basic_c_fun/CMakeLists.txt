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

