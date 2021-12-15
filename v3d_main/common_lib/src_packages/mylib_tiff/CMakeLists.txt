add_subdirectory(MY_TIFF)

set(MyLibTiff_SRCS
  array.c
  cdf.c
  fct.min.c
  fct.root.c
  hash.c
  histogram.c
  image.c
  linear.algebra.c
  utilities.c
  )

add_library( mylib_tiff ${MyLibTiff_SRCS} )

target_link_libraries( mylib_tiff my_tiff ${TIFF_LIBRARIES} )

if(UNIX AND NOT APPLE)
  if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    set_target_properties(mylib_tiff PROPERTIES COMPILE_FLAGS -fPIC)
  endif()
endif()

