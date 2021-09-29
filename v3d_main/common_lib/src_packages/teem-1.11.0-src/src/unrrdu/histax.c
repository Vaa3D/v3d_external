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

#define INFO "Replace each scanline along an axis with its histogram"
static const char *_unrrdu_histaxInfoL =
  (INFO
   ".\n "
   "* Uses nrrdHistoAxis");

int
unrrdu_histaxMain(int argc, const char **argv, const char *me,
                  hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  char *minStr, *maxStr;
  int type, pret, blind8BitRange;
  unsigned int axis, bins;
  airArray *mop;
  NrrdRange *range;

  OPT_ADD_AXIS(axis, "axis to histogram along");
  hestOptAdd(&opt, "b,bin", "bins", airTypeUInt, 1, 1, &bins, NULL,
             "# of bins in histogram");
  OPT_ADD_TYPE(type, "output type", "uchar");
  /* HEY copy and paste from unrrdu/quantize.c */
  hestOptAdd(&opt, "min,minimum", "value", airTypeString, 1, 1,
             &minStr, "nan",
             "The value to map to zero, given explicitly as a regular number, "
             "*or*, if the number is given with a \"" NRRD_MINMAX_PERC_SUFF
             "\" suffix, this "
             "minimum is specified in terms of the percentage of samples in "
             "input that are lower. "
             "\"0" NRRD_MINMAX_PERC_SUFF "\" means the "
             "lowest input value is used, "
             "\"1" NRRD_MINMAX_PERC_SUFF "\" means that the "
             "1% of the lowest values are all mapped to zero. "
             "By default (not using this option), the lowest input value is "
             "used.");
  hestOptAdd(&opt, "max,maximum", "value", airTypeString, 1, 1,
             &maxStr, "nan",
             "The value to map to the highest unsigned integral value, given "
             "explicitly as a regular number, "
             "*or*, if the number is given with "
             "a \"" NRRD_MINMAX_PERC_SUFF "\" suffix, "
             "this maximum is specified "
             "in terms of the percentage of samples in input that are higher. "
             "\"0" NRRD_MINMAX_PERC_SUFF "\" means the highest input value is "
             "used, which is also the default "
             "behavior (same as not using this option).");
  hestOptAdd(&opt, "blind8", "bool", airTypeBool, 1, 1, &blind8BitRange,
             nrrdStateBlind8BitRange ? "true" : "false",
             "Whether to know the range of 8-bit data blindly "
             "(uchar is always [0,255], signed char is [-128,127]).");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_histaxInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  range = nrrdRangeNew(AIR_NAN, AIR_NAN);
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdRangePercentileFromStringSet(range, nin, minStr, maxStr,
                                       10*bins /* HEY magic */,
                                       blind8BitRange)
      || nrrdHistoAxis(nout, nin, range, axis, bins, type)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error doing axis histogramming:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(histax, INFO);
