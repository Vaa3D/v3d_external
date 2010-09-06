set(NeuronTracing_SRCS
  dij_bgl.cpp
  # stacksize.cpp
  )

add_library( neuron_tracing ${NeuronTracing_SRCS} )

target_link_libraries( neuron_tracing )

