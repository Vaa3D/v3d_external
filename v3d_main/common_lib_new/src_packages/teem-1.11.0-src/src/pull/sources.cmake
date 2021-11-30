# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(PULL_SOURCES
  actionPull.c
  binningPull.c
  constraints.c
  contextPull.c
  parmPull.c
  initPull.c
  corePull.c
  defaultsPull.c
  energy.c
  infoPull.c
  pointPull.c
  privatePull.h
  pull.h
  taskPull.c
  volumePull.c
  popcntl.c
  ccPull.c
  enumsPull.c
  trace.c
  )

ADD_TEEM_LIBRARY(pull ${PULL_SOURCES})
