# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(MEET_SOURCES
  enumall.c
  meetNrrd.c
  meetGage.c
  meetPull.c
  meet.h
  )

ADD_TEEM_LIBRARY(meet ${MEET_SOURCES})
