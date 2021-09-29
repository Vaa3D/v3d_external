# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(TIJK_SOURCES
  2dTijk.c
  3dTijk.c
  approxTijk.c
  enumsTijk.c
  fsTijk.c
  miscTijk.c
  nrrdTijk.c
  shTijk.c
  privateTijk.h
  convertQuietPush.h
  convertQuietPop.h
  shtables.h
  tijk.h
  )

ADD_TEEM_LIBRARY(tijk ${TIJK_SOURCES})
