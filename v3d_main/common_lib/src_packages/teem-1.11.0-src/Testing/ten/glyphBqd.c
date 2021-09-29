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

#include "teem/ten.h"
#include <testDataPath.h>

/*
** Tests:
*/

int
main(int argc, const char **argv) {
  airArray *mop;

  Nrrd *nabc, *nref;
  double *abc;
  unsigned int sz, ui, vi, bi, betaMaxNum;
  double uv[2], betaMax[2] = {2.5555555555, 3.888888888888};
  char *err, *refname, explain[AIR_STRLEN_LARGE];
  int differ, ret;

  mop = airMopNew();
  nabc = nrrdNew();
  airMopAdd(mop, nabc, (airMopper)nrrdNuke, airMopAlways);
  sz = 100;
  betaMaxNum = AIR_CAST(unsigned int, sizeof(betaMax)/sizeof(double));
  if (nrrdMaybeAlloc_va(nabc, nrrdTypeDouble, 3,
                        AIR_CAST(size_t, 3),
                        AIR_CAST(size_t, sz),
                        AIR_CAST(size_t, sz*betaMaxNum))) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "trouble allocating:\n%s", err);
    airMopError(mop); return 1;
  }

  abc = AIR_CAST(double *, nabc->data);
  for (bi=0; bi<betaMaxNum; bi++) {
    for (vi=0; vi<sz; vi++) {
      uv[1] = NRRD_CELL_POS(0.0, 1.0, sz, vi);
      for (ui=0; ui<sz; ui++) {
        uv[0] = NRRD_CELL_POS(0.0, 1.0, sz, ui);
        tenGlyphBqdAbcUv(abc + 3*(ui + sz*(vi + sz*bi)), uv, betaMax[bi]);
      }
    }
  }

  refname = testDataPathPrefix("test/tenGlyphBqdAbcUv.nrrd");
  airMopAdd(mop, refname, airFree, airMopAlways);
  nref = nrrdNew();
  airMopAdd(mop, nref, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdLoad(nref, refname, NULL)
      || nrrdCompare(nref, nabc, AIR_FALSE /* onlyData */,
                     5e-15 /* epsilon */, &differ, explain)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "trouble loading or comparing with ref:\n%s", err);
    airMopError(mop); return 1;
  }

  if (differ) {
    printf("generated and reference arrays differ:\n%s\n", explain);
    if (2 == argc) {
      if (nrrdSave(argv[1], nabc, NULL)) {
        airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
        fprintf(stderr, "oops, can't save generated array:\n%s", err);
      } else {
        printf("%s: saved generated (and different) array as %s\n",
               argv[0], argv[1]);
      }
    }
    ret = 1;
  } else {
    printf("All ok.\n");
    ret = 0;
  }

  airMopOkay(mop);
  return ret;
}
