set(WormStraighten_SRCS
  # bdb_minus_bl.cpp
  bdb_minus.cpp
  bfs_1root.cpp
  mst_prim_c.cpp
  spline_cubic.cpp
  )

# main_worm_straightener.cpp uses unistd/getopt, which might not work on Windows
if (NOT MSVC)
  set(WormStraighten_SRCS ${WormStraighten_SRCS} main_worm_straightener.cpp)
endif()

add_library( worm_straighten_c ${WormStraighten_SRCS} )

target_link_libraries( worm_straighten_c )

add_dependencies( worm_straighten_c ${TIFF_LIBRARY} )

