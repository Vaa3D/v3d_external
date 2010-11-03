#
#  This file provides the standard configuration
#  for a generic plugin
#
#
function(configure_v3d_plugin PLUGIN_NAME)

include_directories( ${CMAKE_SOURCE_DIR} )

set(QtITK_SRCS ${PLUGIN_SOURCES} )

qt4_wrap_cpp(QT_MOC_SRCS ${PLUGIN_HEADERS})

configure_v3d_plugin_common(${PLUGIN_NAME})

endfunction(configure_v3d_plugin)




#
#  Function equivalent to CONFIGURE_V3D_PLUGIN but for
#  plugins whose .h is trivial and can be generated
#  via C macros.
#
function(CONFIGURE_V3D_PLUGIN_SIMPLE PLUGIN_NAME)

include_directories( ${CMAKE_SOURCE_DIR} )

set(QtITK_SRCS ${PLUGIN_SOURCES} )

configure_v3d_plugin_common(${PLUGIN_NAME})

endfunction(CONFIGURE_V3D_PLUGIN_SIMPLE)




function(configure_v3d_plugin_common PLUGIN_NAME)

add_library(${PLUGIN_NAME} SHARED ${QtITK_SRCS} ${QT_MOC_SRCS})
target_link_libraries(${PLUGIN_NAME} ${QT_LIBRARIES} )
# CMB Nov-03-2010
# I apologize if I am doing this wrong...
# Several plugins yield link errors for method v3d_message without this
# link to the V3DInterface library
if (TARGET V3DInterface)
  target_link_libraries(${PLUGIN_NAME} V3DInterface)
endif()
# Install plugins below executable for cpack installer builds
if(V3D_INSTALL_DIR)
  install(TARGETS ${PLUGIN_NAME} DESTINATION ${V3D_INSTALL_DIR}/plugins)
endif()

if(NOT PLUGIN_DIRECTORY_NAME)
  set(PLUGIN_DIRECTORY_NAME ${PLUGIN_NAME})
endif()

set(PLUGIN_DESTINATION_DIR ${INSTALLATION_DIRECTORY}/${PLUGIN_DIRECTORY_NAME} )

file(MAKE_DIRECTORY ${PLUGIN_DESTINATION_DIR})

install(TARGETS ${PLUGIN_NAME}
  LIBRARY DESTINATION ${PLUGIN_DESTINATION_DIR} COMPONENT RuntimeLibraries
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
  )

endfunction(configure_v3d_plugin_common)
