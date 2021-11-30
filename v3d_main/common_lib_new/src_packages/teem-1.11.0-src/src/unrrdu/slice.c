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

#define INFO "Slice along one or more axes at given positions"
static const char *_unrrdu_sliceInfoL =
  (INFO
   ". Output nrrd dimension is less than input nrrd "
   "dimension by the number of slice axes (except when the "
   "input is or gets down to 1-D). Can slice on all axes "
   "in order to sample a single value from the array. "
   "Per-axis information is preserved.\n "
   "* Uses nrrdSlice (possibly called multiple times)");

int
unrrdu_sliceMain(int argc, const char **argv, const char *me,
                 hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  unsigned int *axis;
  int pret, axi, axisNum, posNum;
  size_t pos[NRRD_DIM_MAX];
  long int *_pos;
  airArray *mop;

  hestOptAdd(&opt, "a,axis", "axis", airTypeUInt, 1, -1, &axis, NULL,
             "single axis or multiple axes to slice along.  "
             "Giving multiple axes here leads to doing multiple slices "
             "(at the corresponding positions "
             "given with \"-p\"). Multiple axes should be identified "
             "in terms of the axis numbering of the original nrrd; as "
             "the slices are done (in the given ordering) the actual "
             "slice axis will be different if previous slices were on "
             "lower-numbered (faster) axes.", &axisNum);
  hestOptAdd(&opt, "p,position", "pos", airTypeOther, 1, -1, &_pos, NULL,
             "position(s) to slice at:\n "
             "\b\bo <int> gives 0-based index\n "
             "\b\bo M-<int> give index relative "
             "to the last sample on the axis (M == #samples-1).",
             &posNum, NULL, &unrrduHestPosCB);
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_sliceInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);
  if (axisNum != posNum) {
    fprintf(stderr, "%s: # axes %u != # positions %u\n", me, axisNum, posNum);
    airMopError(mop); return 1;
  }
  if (axisNum > NRRD_DIM_MAX) {
    fprintf(stderr, "%s: got more slice axes %u than max nrrd dimension %u\n",
            me, axisNum, NRRD_DIM_MAX);
    airMopError(mop); return 1;
  }
  for (axi=0; axi<axisNum; axi++) {
    char stmp[AIR_STRLEN_SMALL];
    if (axisNum > 1) {
      sprintf(stmp, "[%d]", axi);
    } else {
      strcpy(stmp, "");
    }
    if (!( axis[axi] < nin->dim )) {
      fprintf(stderr, "%s: axis%s %d not in range [0,%d]\n",
              me, stmp, axis[axi], nin->dim-1);
      airMopError(mop); return 1;
    }
    if (_pos[0 + 2*axi] == -1) {
      fprintf(stderr, "%s: pos%s m+<int> spec not meaningful here\n",
              me, stmp);
      airMopError(mop); return 1;
    }
    pos[axi] = (_pos[0 + 2*axi]*(nin->axis[axis[axi]].size-1)
                + _pos[1 + 2*axi]);
    /*
    printf("%s: [%d] axis = %u, pos = %u\n", me, axi, axis[axi],
           AIR_CAST(unsigned int, pos[axi]));
    */
  }
  /* check on possibly adjust slice axes downward */
  if (axisNum > 1) {
    int axj;
    for (axi=0; axi<axisNum-1; axi++) {
      for (axj=axi+1; axj<axisNum; axj++) {
        if (axis[axi] == axis[axj]) {
          fprintf(stderr, "%s: can't repeat axis: axis[%d] = axis[%d] = %u\n",
                  me, axi, axj, axis[axj]);
          airMopError(mop); return 1;
        }
      }
    }
    for (axi=0; axi<axisNum-1; axi++) {
      for (axj=axi+1; axj<axisNum; axj++) {
        axis[axj] -= (axis[axj] > axis[axi]);
      }
    }
    /*
    for (axi=0; axi<axisNum; axi++) {
      printf("%s: axis[%d] = %u\n", me, axi, axis[axi]);
    }
    */
  }

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (1 == axisNum) {
    /* old single axis code */
    if (nrrdSlice(nout, nin, axis[0], pos[0])) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error slicing nrrd:\n%s", me, err);
      airMopError(mop); return 1;
    }
  } else {
    /* have to do multiple slices */
    Nrrd *ntmp[2];
    unsigned int tidx = 0;
    ntmp[0] = nrrdNew();
    airMopAdd(mop, ntmp[0], (airMopper)nrrdNuke, airMopAlways);
    ntmp[1] = nrrdNew();
    airMopAdd(mop, ntmp[1], (airMopper)nrrdNuke, airMopAlways);
    for (axi=0; axi<axisNum; axi++) {
      if (nrrdSlice((axi < axisNum-1
                     ? ntmp[1-tidx] /* use an ntmp for all but last output */
                     : nout),       /* and use nout for last output */
                    (0 == axi
                     ? nin          /* use nin for only first input */
                     : ntmp[tidx]), /* use an ntmp for all but first input */
                    axis[axi], pos[axi])) {
        airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
        fprintf(stderr, "%s: error with slice %d of %d:\n%s", me,
                axi+1, axisNum, err);
        airMopError(mop); return 1;
      }
      tidx = 1 - tidx;
    }
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(slice, INFO);
