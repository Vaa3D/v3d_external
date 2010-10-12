set(NeuronTracing_SRCS
  dij_bgl.cpp
  # stacksize.cpp
  )

# Boost headers might be unpacked here...
include_directories(${CMAKE_SOURCE_DIR}/common_lib/include)

# Or use system boost install
find_package(Boost)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

add_library( neuron_tracing ${NeuronTracing_SRCS} )

target_link_libraries( neuron_tracing dijkstra )

