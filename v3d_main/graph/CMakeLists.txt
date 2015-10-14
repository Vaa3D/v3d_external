set(Dijkstra_SRCS
  dijk.cpp
  )

add_library( dijkstra ${Dijkstra_SRCS} )

target_link_libraries( dijkstra )

