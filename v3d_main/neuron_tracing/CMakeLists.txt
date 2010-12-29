set(NeuronTracing_SRCS
  dij_bgl.cpp
  # stacksize.cpp
  )

# Boost headers might be unpacked here...
if(UNIX)
  include_directories(/usr/local/include)
endif()
include_directories(${CMAKE_SOURCE_DIR}/common_lib/include)

# Or use system boost install
# FindBoost has its own idiosyncracies.  Add your boost version here
SET(Boost_ADDITIONAL_VERSIONS "1.44" "1.44.0")
# Note: with Boost 1.44.0 and MSVC, you need to patch boost/serialization/serialization.hpp
# comment out line 12: // #include <boost/archive/basic_archive.hpp>  // link bug
# to avoid auto link error to serialization library
find_package(Boost COMPONENTS)
mark_as_advanced(Boost_LIB_DIAGNOSTIC_DEFINITIONS)
mark_as_advanced(CLEAR Boost_INCLUDE_DIR)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY_DIRS})
endif()
# message("Boost serialization library = ${Boost_SERIALIZATION_LIBRARY}")

add_library( neuron_tracing ${NeuronTracing_SRCS} )

target_link_libraries( neuron_tracing dijkstra )
# target_link_libraries( neuron_tracing ${Boost_SERIALIZATION_LIBRARY} "")
