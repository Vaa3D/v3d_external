/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "../air.h"

FILE *
myopen(char *name) {

  if (!strcmp(name, "-")) {
    return stdin;
  } else {
    return fopen(name, "r");
  }
}

void
myclose(FILE *file) {

  if (file != stdin) {
    fclose(file);
  }
  return;
}


int
main(int argc, char *argv[]) {
  char *me, *fileS, *line;
  FILE *file;
  int size, maxed, ret;

  me = argv[0];
  if (2 != argc) {
    /*                       0   1   (2) */
    fprintf(stderr, "usage: %s <file>\n", me);
    exit(1);
  }
  fileS = argv[1];
  if (!( file = myopen(fileS) )) {
    fprintf(stderr, "%s: couldn't open \"%s\" for reading\n", me, fileS);
    exit(1);
  }
  myclose(file);

  size = 30000;
  do {
    maxed = 0;
    file = myopen(fileS);
    line = (char *)calloc(size, sizeof(char));
    printf(" ----------------- size = %d\n", size);
    do {
      ret = airOneLine(file, line, size);
      if (ret) {
        printf("%2d   |%s|\n", ret, line);
        maxed |= (ret == size+1);
      }
    } while(ret > 0);
    myclose(file);
    free(line);
    size++;
  } while(maxed);

  printf("\n\nsize = %d\n", size);

  exit(0);
}
