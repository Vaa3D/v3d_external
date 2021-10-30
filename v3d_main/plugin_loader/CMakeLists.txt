set(PluginLoader_SRCS
  pluginDialog.cpp
  v3d_plugin_loader.cpp
  )

set(PLUGIN_UI_SRCS
  )

set(PLUGIN_MOC_SRCS
  pluginDialog.h
  v3d_plugin_loader.h
  )

if(NOT Qt5Core_FOUND)
  QT4_WRAP_UI( QT_UI_SRCS ${PLUGIN_UI_SRCS} )
  QT4_WRAP_CPP(QT_MOC_SRCS ${PLUGIN_MOC_SRCS})
else()
  QT5_WRAP_UI( QT_UI_SRCS ${PLUGIN_UI_SRCS} )
  QT5_WRAP_CPP(QT_MOC_SRCS ${PLUGIN_MOC_SRCS})
endif()

include_directories( ${V3D_BINARY_DIR}/v3dbase )
include_directories( ../basic_c_fun )

add_library(v3d_plugin_loader ${QT_UI_SRCS} ${QT_MOC_SRCS} ${PluginLoader_SRCS})
add_dependencies(v3d_plugin_loader v3dbase1)

# Command line tool for parsing out plugin version information
add_executable(get_plugin_versions
    get_plugin_versions.cpp)

if(NOT Qt5Core_FOUND)
  target_link_libraries(get_plugin_versions ${QT_QTCORE_LIBRARY})
else()
  target_link_libraries(get_plugin_versions Qt5::Core )
endif()

add_dependencies(get_plugin_versions v3d)
