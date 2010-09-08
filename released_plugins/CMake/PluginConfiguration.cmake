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

set(PLUGIN_DESTINATION_DIR ${INSTALLATION_DIRECTORY}/${PLUGIN_NAME} )

file(MAKE_DIRECTORY ${PLUGIN_DESTINATION_DIR})

install(TARGETS ${PLUGIN_NAME}
  LIBRARY DESTINATION ${PLUGIN_DESTINATION_DIR} COMPONENT RuntimeLibraries
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
  )


if(NOT APPLE)
if(NOT SOLVED_RPATH_ISSUE)

  SET(LIBRARY_BUILD_DIR ${LIBRARY_OUTPUT_PATH})

  ADD_CUSTOM_COMMAND(
    SOURCE ${PLUGIN_NAME}
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy ${LIBRARY_BUILD_DIR}/${CMAKE_SHARED_MODULE_PREFIX}${PLUGIN_NAME}${CMAKE_SHARED_MODULE_SUFFIX} ${PLUGIN_DESTINATION_DIR}/${PLUGIN_NAME}${CMAKE_SHARED_MODULE_SUFFIX}
    TARGET ${PLUGIN_NAME}
    OUTPUTS ${LIBRARY_DESTINATION_DIR}/${PLUG}${CMAKE_SHARED_MODULE_SUFFIX}
    )

endif(NOT SOLVED_RPATH_ISSUE)
endif(NOT APPLE)

endfunction(configure_v3d_plugin_common)
