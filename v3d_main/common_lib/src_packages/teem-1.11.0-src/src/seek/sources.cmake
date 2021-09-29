# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(SEEK_SOURCES
  descend.c
  enumsSeek.c
  extract.c
  methodsSeek.c
  privateSeek.h
  seek.h
  setSeek.c
  tables.c
  textract.c
  updateSeek.c
  )

ADD_TEEM_LIBRARY(seek ${SEEK_SOURCES})
