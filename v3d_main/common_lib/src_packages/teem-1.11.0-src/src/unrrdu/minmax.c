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

#define INFO "Print out min and max values in one or more nrrds"
static const char *_unrrdu_minmaxInfoL =
(INFO ". Unlike other commands, this doesn't produce a nrrd.  It only "
 "prints to standard out the min and max values found in the input nrrd(s), "
 "and it also indicates if there are non-existent values.\n "
 "* Uses nrrdRangeNewSet");

int
unrrdu_minmaxDoit(const char *me, char *inS, int blind8BitRange, FILE *fout) {
  Nrrd *nrrd;
  NrrdRange *range;
  airArray *mop;

  mop = airMopNew();
  airMopAdd(mop, nrrd=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdLoad(nrrd, inS, NULL)) {
    biffMovef(me, NRRD, "%s: trouble loading \"%s\"", me, inS);
    airMopError(mop); return 1;
  }

  range = nrrdRangeNewSet(nrrd, blind8BitRange);
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  airSinglePrintf(fout, NULL, "min: %.17g\n", range->min);
  airSinglePrintf(fout, NULL, "max: %.17g\n", range->max);
  if (0 == range->min && 0 == range->max) {
    fprintf(fout, "# min == max == 0.0 exactly\n");
  }
  if (range->hasNonExist) {
    fprintf(fout, "# has non-existent values\n");
  }

  airMopOkay(mop);
  return 0;
}

int
unrrdu_minmaxMain(int argc, const char **argv, const char *me,
                  hestParm *hparm) {
  hestOpt *opt = NULL;
  char *err, **inS;
  airArray *mop;
  int pret, blind8BitRange;
  unsigned int ni, ninLen;
#define B8DEF "false"

  mop = airMopNew();
  hestOptAdd(&opt, "blind8", "bool", airTypeBool, 1, 1, &blind8BitRange,
             B8DEF, /* NOTE: not using nrrdStateBlind8BitRange here
                       for consistency with previous behavior */
             "whether to blindly assert the range of 8-bit data, "
             "without actually going through the data values, i.e. "
             "uchar is always [0,255], signed char is [-128,127]. "
             "Note that even if you do not use this option, the default "
             "(" B8DEF ") is potentialy over-riding the effect of "
             "environment variable NRRD_STATE_BLIND_8_BIT_RANGE; "
             "see \"unu env\"");
  hestOptAdd(&opt, NULL, "nin1", airTypeString, 1, -1, &inS, NULL,
             "input nrrd(s)", &ninLen);
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_minmaxInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  for (ni=0; ni<ninLen; ni++) {
    if (ninLen > 1) {
      fprintf(stdout, "==> %s <==\n", inS[ni]);
    }
    if (unrrdu_minmaxDoit(me, inS[ni], blind8BitRange, stdout)) {
      airMopAdd(mop, err = biffGetDone(me), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble with \"%s\":\n%s",
              me, inS[ni], err);
      /* continue working on the remaining files */
    }
    if (ninLen > 1 && ni < ninLen-1) {
      fprintf(stdout, "\n");
    }
  }

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(minmax, INFO);
