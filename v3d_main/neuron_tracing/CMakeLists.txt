set(NeuronTracing_SRCS
  dij_bgl.cpp
  # stacksize.cpp
  )

include_directories(${CMAKE_SOURCE_DIR}/common_lib/include)
add_library( neuron_tracing ${NeuronTracing_SRCS} )

target_link_libraries( neuron_tracing dijkstra )

