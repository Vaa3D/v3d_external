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

#define INFO " Automatically crop axes based on given measure"
static const char *_unrrdu_acropInfoL =
  (INFO ". For the axes that are to be cropped, the slices perpendicular "
   "to that axis are projected down to a scalar with the specified measure. "
   "The resulting 1D array is analyzed by determining what portions at the "
   "beginning and end constitute less than some portion of the cumulative "
   "array sum; these ends are cropped off.  The cropping bounds determined "
   "here can be saved and applied to other arrays via the \"-b\" option.\n "
   "* Uses nrrdCropAuto");

int
unrrdu_acropMain(int argc, const char **argv, const char *me,
                 hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  int pret;
  airArray *mop;

  size_t min[NRRD_DIM_MAX], max[NRRD_DIM_MAX];
  unsigned int *axes, axesLen;
  double frac;
  int measr, offset;
  Nrrd *nbounds;
  char *boundsSave;

  hestOptAdd(&opt, "a,axes", "ax0", airTypeUInt, 0, -1, &axes, "",
             "the axes (if any) that should NOT be cropped", &axesLen);
  hestOptAdd(&opt, "m,measure", "measr", airTypeEnum, 1, 1, &measr, NULL,
             "How to measure slices (along axes to crop) as scalars, "
             "to form 1-D array analyzed to determine cropping extent. "
             "All the measures from \"unu project\" can be used, but "
             "those that make more sense here include:\n "
             "\b\bo \"max\", \"mean\", \"median\", "
             "\"variance\": (self-explanatory)\n "
             "\b\bo \"stdv\": standard deviation\n "
             "\b\bo \"cov\": coefficient of variation\n "
             "\b\bo \"product\", \"sum\": product or sum of all values\n "
             "\b\bo \"L1\", \"L2\", \"NL2\", \"RMS\", \"Linf\": "
             "different norms.", NULL, nrrdMeasure);
  hestOptAdd(&opt, "f,frac", "frac", airTypeDouble, 1, 1, &frac, "0.1",
             "threshold of cumulative sum of 1-D array at which to crop. "
             "Needs to be in interval [0.0,0.5).");
  hestOptAdd(&opt, "off,offset", "offset", airTypeInt, 1, 1, &offset, "1",
             "how much to offset the numerically determined cropping; "
             "positive offsets means expanding the interval of kept "
             "indices (less cropping)");
  hestOptAdd(&opt, "b,bounds", "filename", airTypeString, 1, 1,
             &boundsSave, "",
             "if a filename is given here, the automatically determined "
             "min and max bounds for cropping are saved to this file "
             "as a 2-D array; first scanline is for -min, second is for -max. "
             "Unfortunately nothing using the \"m\" and \"M\" semantics "
             "(above) can currently be saved in the bounds file.");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_acropInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdCropAuto(nout, nin, min, max,
                   axes, axesLen,
                   measr, frac, offset)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error cropping nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }
  if (airStrlen(boundsSave)) {
    unsigned int axi;
    airULLong *bounds;
    nbounds = nrrdNew();
    airMopAdd(mop, nbounds, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdMaybeAlloc_va(nbounds, nrrdTypeULLong, 2,
                          AIR_CAST(airULLong, nin->dim),
                          AIR_CAST(airULLong, 2))) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error allocating cropping bounds array:\n%s",
              me, err);
      airMopError(mop);
      return 1;
    }
    bounds = AIR_CAST(airULLong*, nbounds->data);
    for (axi=0; axi<nin->dim; axi++) {
      bounds[axi + 0*(nin->dim)] = AIR_CAST(airULLong, min[axi]);
      bounds[axi + 1*(nin->dim)] = AIR_CAST(airULLong, max[axi]);
    }
    if (nrrdSave(boundsSave, nbounds, NULL)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error saving cropping bounds array:\n%s",
              me, err);
      airMopError(mop);
      return 1;
    }
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(acrop, INFO);
