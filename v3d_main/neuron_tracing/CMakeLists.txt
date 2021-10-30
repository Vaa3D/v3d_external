set(NeuronTracing_SRCS
  dij_bgl.cpp
  # stacksize.cpp
  )

# Boost headers might be unpacked here...
if(UNIX)
  include_directories(/usr/local/include)
endif()

#include_directories(${CMAKE_SOURCE_DIR}/common_lib/include)

SET(Boost_ADDITIONAL_VERSIONS "1.46.0" "1.57.0" "1.56.1" "1.56.0" "1.54" "1.54.0")
find_package(Boost COMPONENTS)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    message("${Boost_INCLUDE_DIR}")
    message("${Boost_LIBRARY_DIRS}")
    link_directories(${Boost_LIBRARY_DIRS})
endif()



add_library( neuron_tracing ${NeuronTracing_SRCS} )

target_link_libraries( neuron_tracing dijkstra )
# target_link_libraries( neuron_tracing ${Boost_SERIALIZATION_LIBRARY} "")
