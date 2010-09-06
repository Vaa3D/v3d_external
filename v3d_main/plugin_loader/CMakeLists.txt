set(PluginLoader_SRCS
  pluginDialog.cpp
  v3d_plugin_loader.cpp
  )

QT4_WRAP_UI(QT_UI_SRCS
  )

QT4_WRAP_CPP(QT_MOC_SRCS
  pluginDialog.h
  v3d_plugin_loader.h
  )

include_directories( ${V3D_BINARY_DIR}/v3dbase )

add_library(v3d_plugin_loader ${QT_UI_SRCS} ${QT_MOC_SRCS} ${PluginLoader_SRCS})

