set(MyTiff_SRCS
  app.color.c
  app.convert.c
  app.mrc2.c
  app.show.c
  app.tagger.c
  tiff.image.c
  tiff.io.c
  utilities.c
  )

add_library( my_tiff ${MyTiff_SRCS} )

target_link_libraries( my_tiff ${TIFF_LIBRARIES} )

if(UNIX AND NOT APPLE)
  if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    set_target_properties(my_tiff PROPERTIES COMPILE_FLAGS -fPIC)
  endif()
endif()

