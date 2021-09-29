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

#include "unrrdu.h"
#include "privateUnrrdu.h"

#define INFO "Select subset of slices along an axis"
static const char *_unrrdu_sselectInfoL =
  (INFO ". The choice to keep or nix a slice is determined by whether the "
   "values in a given 1-D line of values is above or below a given "
   "threshold.\n "
   "* Uses nrrdSliceSelect");

int
unrrdu_sselectMain(int argc, const char **argv, const char *me,
                   hestParm *hparm) {
  hestOpt *opt = NULL;
  char *err;
  Nrrd *nin, *noutAbove, *noutBelow, *nline;
  unsigned int axis;
  double thresh;
  int pret;
  airArray *mop;
  char *outS[2];

  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_AXIS(axis, "axis to slice along");
  hestOptAdd(&opt, "s,selector", "nline", airTypeOther, 1, 1, &nline, NULL,
             "the 1-D nrrd of values to compare with threshold",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "th", "thresh", airTypeDouble, 1, 1, &thresh,
             NULL, "threshold on selector line");
  hestOptAdd(&opt, "o,output", "above below", airTypeString, 2, 2,
             outS, "- x",
             "outputs for slices corresponding to values "
             "above (first) and below (second) given threshold. "
             "Use \"x\" to say that no output is desired.");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_sselectInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  if (!strcmp(outS[0], "x") && !strcmp(outS[1], "x")) {
    fprintf(stderr, "%s: need to save either above or below slices "
            "(can't use \"x\" for both)\n", me);
    airMopError(mop);
    return 1;
  }
  if (strcmp(outS[0], "x")) {
    noutAbove = nrrdNew();
    airMopAdd(mop, noutAbove, (airMopper)nrrdNuke, airMopAlways);
  } else {
    noutAbove = NULL;
  }
  if (strcmp(outS[1], "x")) {
    noutBelow = nrrdNew();
    airMopAdd(mop, noutBelow, (airMopper)nrrdNuke, airMopAlways);
  } else {
    noutBelow = NULL;
  }

  if (nrrdSliceSelect(noutAbove, noutBelow, nin, axis,
                      nline, thresh)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error selecting slices nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  if (noutAbove) {
    SAVE(outS[0], noutAbove, NULL);
  }
  if (noutBelow) {
    SAVE(outS[1], noutBelow, NULL);
  }

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(sselect, INFO);
