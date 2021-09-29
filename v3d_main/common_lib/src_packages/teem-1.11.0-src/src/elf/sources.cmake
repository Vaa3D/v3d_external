# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(ELF_SOURCES
  ballStickElf.c
  ESHEstimElf.c
  glyphElf.c
  maximaElf.c
  elf.h
  )

ADD_TEEM_LIBRARY(elf ${ELF_SOURCES})
