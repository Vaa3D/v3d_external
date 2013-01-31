include_directories(${TIFF_INCLUDE_DIR})

QT4_WRAP_CPP(QT_INTERFACE_MOC_SRCS
  basic_thread.h
)

set(V3DInterface_SRCS
  ${QT_INTERFACE_MOC_SRCS}
  basic_4dimage.cpp
  # basic_memory.cpp
  basic_surf_objs.cpp
  imageio_mylib.cpp
  mg_image_lib.cpp
  mg_utilities.cpp
  stackutil.cpp
  v3d_message.cpp
  # vcdiff.cpp
  ../neuron_annotator/utility/ImageLoaderBasic.cpp
  basic_4dimage_create.cpp
  )
  
if (WIN32)
    set(V3DInterface_SRCS ${V3DInterface_SRCS} vcdiff.cpp)
else()
    set(V3DInterface_SRCS ${V3DInterface_SRCS} imageio_mylib.cpp)
endif()

add_library(V3DInterface ${V3DInterface_SRCS})

target_link_libraries(V3DInterface ${TIFF_LIBRARY})
target_link_libraries(V3DInterface ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
if (NOT WIN32)
    target_link_libraries(V3DInterface mylib_tiff)
endif()

if(V3D_USE_OWN_TIFF)
    add_dependencies(V3DInterface CustomTiffLibrary)
endif(V3D_USE_OWN_TIFF)

# 64 bit AMD Linux requires -fPIC flag for linking static libraries --CMB
if(UNIX AND NOT APPLE)
  if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    set_target_properties(V3DInterface PROPERTIES COMPILE_FLAGS -fPIC)
  endif()
endif()

