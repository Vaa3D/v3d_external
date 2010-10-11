find_package(OpenGL REQUIRED)

set(3DRenderer_SRCS
  GLee_r.c
  barFigureDialog.cpp
  # glsl_r.cpp
  gradients.cpp
  hoverpoints.cpp
  ItemEditor.cpp
  marchingcubes.cpp
  renderer.cpp
  Renderer_gl2.cpp
  renderer_hit2.cpp
  renderer_labelfield.cpp
  renderer_obj2.cpp
  renderer_tex2.cpp
  test_main.cpp
  v3dr_colormapDialog.cpp
  # v3dr_control_signal.cpp
  v3dr_glwidget.cpp
  v3dr_mainwindow.cpp
  v3dr_surfaceDialog.cpp
  )

QT4_WRAP_CPP(QT_MOC_SRCS
  gradients.h
  hoverpoints.h
  ItemEditor.h
  v3dr_colormapDialog.h
  v3dr_glwidget.h
  v3dr_mainwindow.h
  v3dr_surfaceDialog.h
  )

include_directories( ${V3D_BINARY_DIR}/v3dbase )

add_library( 3drenderer ${QT_MOC_SRCS} ${3DRenderer_SRCS} )

target_link_libraries( 3drenderer ${QT_QTOPENGL_LIBRARY} ${OPENGL_LIBRARIES} )
target_link_libraries( 3drenderer imaging )

# Avoid link error on Mac Snow Leopard
if(APPLE)
  target_link_libraries( 3drenderer "-framework CoreServices" )
endif()
