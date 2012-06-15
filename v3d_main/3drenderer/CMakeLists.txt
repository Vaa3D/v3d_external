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
  renderer_gl2.cpp
  renderer_hit.cpp
  renderer_labelfield.cpp
  renderer_obj.cpp
  renderer_tex.cpp
  test_main.cpp
  v3dr_colormapDialog.cpp
  # v3dr_control_signal.cpp
  v3dr_glwidget.cpp
  v3dr_mainwindow.cpp
  v3dr_surfaceDialog.cpp
  nstroke.cpp
  nstroke_tracing.cpp
  ../neuron_toolbox/vaa3d_neurontoolbox.cpp
  )

# QT4_ADD_RESOURCES(V3D_RENDERER_RCC_SRCS "3drenderer.qrc")

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
include_directories( ${CMAKE_BINARY_DIR}/cellseg )

add_library( 3drenderer 
    ${QT_MOC_SRCS}
    # ${V3D_RENDERER_RCC_SRCS} # no, this needs to be linked directly to v3d executable
    ${3DRenderer_SRCS})

target_link_libraries( 3drenderer ${QT_QTOPENGL_LIBRARY} ${OPENGL_LIBRARIES} )
target_link_libraries( 3drenderer imaging )

add_dependencies(3drenderer v3dbase)

# Avoid link error on Mac Snow Leopard
if(APPLE)
  target_link_libraries( 3drenderer "-framework CoreServices" )
endif()
