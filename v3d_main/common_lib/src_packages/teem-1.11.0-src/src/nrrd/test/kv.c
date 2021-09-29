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


#include "../nrrd.h"

void
usage(char *me) {
  /*                       0     1 */
  fprintf(stderr, "usage: %s <nin>\n", me);
  exit(1);
}

int
main(int argc, char **argv) {
  char *me, *err, *key="strong bad", *value;
  Nrrd *nrrd;
  NrrdIoState *io;

  me = argv[0];
  if (2 != argc)
    usage(me);

  io = nrrdIoStateNew();
  nrrdStateVerboseIO = 10;

  if (nrrdLoad(nrrd=nrrdNew(), argv[1], NULL)) {
    fprintf(stderr, "%s: trouble loading \"%s\":\n%s",
            me, argv[1], err = biffGetDone(NRRD));
    free(err);
    exit(1);
  }

  if ((value = nrrdKeyValueGet(nrrd, key))) {
    fprintf(stderr, "%s: '%s':='%s' (%d)\n", me, key, value,
            (int)strlen(value));
  } else {
    fprintf(stderr, "%s: value not found for key: %s\n", me, key);
  }

  nrrdIoStateNix(io);
  nrrdNuke(nrrd);

  exit(0);
}
