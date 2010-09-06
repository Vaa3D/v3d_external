set(WormStraighten_SRCS
  # bdb_minus_bl.cpp
  bdb_minus.cpp
  bfs_1root.cpp
  main_worm_straightener.cpp
  mst_prim_c.cpp
  spline_cubic.cpp
  )

add_library( worm_straighten_c ${WormStraighten_SRCS} )

target_link_libraries( worm_straighten_c )

