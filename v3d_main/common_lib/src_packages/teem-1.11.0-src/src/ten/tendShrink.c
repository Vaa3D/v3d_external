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

#include "ten.h"
#include "privateTen.h"

#define INFO "Converts a 9-value DT volume to a 7-value DT volume"
static const char *_tend_shrinkInfoL =
  (INFO
   ". The confidence value is set to 1.0 everwhere.  You can \"unu splice\" "
   "or nrrdSplice() something else in its place later.");

int
tend_shrinkMain(int argc, const char **argv, const char *me,
                hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  Nrrd *nin, *nout;
  char *outS;

  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, "-",
             "input diffusion tensor volume, with 9 matrix components "
             "per sample",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, NULL,
             "output tensor volume, with the 7 values per sample");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_shrinkInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (tenShrink(nout, NULL, nin)) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble shrinking tensors:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
TEND_CMD(shrink, INFO);
