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

#define INFO "Crop along each axis to make a smaller nrrd"
static const char *_unrrdu_cropInfoL =
  (INFO ".\n "
   "* Uses nrrdCrop");

int
unrrdu_cropMain(int argc, const char **argv, const char *me,
                hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  unsigned int ai;
  int minLen, maxLen, pret;
  long int *minOff, *maxOff;
  size_t min[NRRD_DIM_MAX], max[NRRD_DIM_MAX];
  airArray *mop;
  Nrrd *_nbounds;

  OPT_ADD_BOUND("min,minimum", 0, minOff, "0",
                "low corner of bounding box.\n "
                "\b\bo <int> gives 0-based index\n "
                "\b\bo M, M+<int>, M-<int> give index relative "
                "to the last sample on the axis (M == #samples-1).",
                minLen);
  OPT_ADD_BOUND("max,maximum", 0, maxOff, "0",
                "high corner of bounding box.  "
                "Besides the specification styles described above, "
                "there's also:\n "
                "\b\bo m+<int> give index relative to minimum.",
                maxLen);
  hestOptAdd(&opt, "b,bounds", "filename", airTypeOther, 1, 1,
             &_nbounds, "",
             "a filename given here overrides the -min and -max "
             "options (they don't need to be used) and provides the "
             "cropping bounds as a 2-D array; first scanline is for "
             "-min, second is for -max. Unfortunately the "
             "\"m\" and \"M\" semantics (above) are currently not "
             "supported in the bounds file.",
             NULL, NULL, nrrdHestNrrd);
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_cropInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  if (!_nbounds) {
    if (!( minLen == (int)nin->dim && maxLen == (int)nin->dim )) {
      fprintf(stderr,
              "%s: # min coords (%d) or max coords (%d) != nrrd dim (%d)\n",
              me, minLen, maxLen, nin->dim);
      airMopError(mop);
      return 1;
    }
    for (ai=0; ai<nin->dim; ai++) {
      if (-1 == minOff[0 + 2*ai]) {
        fprintf(stderr, "%s: can't use m+<int> specification for axis %d min\n",
                me, ai);
        airMopError(mop);
        return 1;
      }
    }
    for (ai=0; ai<nin->dim; ai++) {
      min[ai] = minOff[0 + 2*ai]*(nin->axis[ai].size-1) + minOff[1 + 2*ai];
      if (-1 == maxOff[0 + 2*ai]) {
        max[ai] = min[ai] + maxOff[1 + 2*ai];
      } else {
        max[ai] = maxOff[0 + 2*ai]*(nin->axis[ai].size-1) + maxOff[1 + 2*ai];
      }
      /*
        fprintf(stderr, "%s: ai %2d: min = %4d, max = %4d\n",
        me, ai, min[ai], max[ai]);
      */
    }
  } else {
    Nrrd *nbounds;
    airULLong *bounds;
    unsigned int axi;
    if (!(2 == _nbounds->dim
          && nin->dim == AIR_CAST(unsigned int, _nbounds->axis[0].size)
          && 2 == _nbounds->axis[1].size)) {
      char stmp1[AIR_STRLEN_SMALL], stmp2[AIR_STRLEN_SMALL];
      if (_nbounds->dim >= 2) {
        airSprintSize_t(stmp1, _nbounds->axis[1].size);
      } else {
        strcpy(stmp1, "");
      }
      fprintf(stderr, "%s: expected 2-D %u-by-2 array of cropping bounds, "
              "not %u-D %s%s%s%s\n", me, nin->dim, _nbounds->dim,
              airSprintSize_t(stmp2, _nbounds->axis[0].size),
              _nbounds->dim >= 2 ? "-by-" : "-long",
              _nbounds->dim >= 2 ? stmp1 : "",
              _nbounds->dim > 2 ? "-by-X" : "");
      airMopError(mop);
      return 1;
    }
    nbounds = nrrdNew();
    airMopAdd(mop, nbounds, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdConvert(nbounds, _nbounds, nrrdTypeULLong)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error converting bounds array:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
    bounds = AIR_CAST(airULLong*, nbounds->data);
    for (axi=0; axi<nin->dim; axi++) {
      min[axi] = AIR_CAST(size_t, bounds[axi + 0*(nin->dim)]);
      max[axi] = AIR_CAST(size_t, bounds[axi + 1*(nin->dim)]);
    }
  }

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdCrop(nout, nin, min, max)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error cropping nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(crop, INFO);
