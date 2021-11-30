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

#define INFO "Ring removal for CT"
static const char *_unrrdu_deringInfoL =
(INFO
 ". Should be considered a work-in-progress. ");

int
unrrdu_deringMain(int argc, const char **argv, const char *me,
                  hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  airArray *mop;
  int pret;

  Nrrd *nmask;
  NrrdDeringContext *drc;
  double center[2], radScale, clampPerc[2], backval;
  int verbose, linterp, vertSeam;
  unsigned int thetaNum;
  NrrdKernelSpec *rkspec, *tkspec;

  /* HEY is this needed? (to display -rk and -tk kernels) */
  hparm->elideSingleOtherDefault = AIR_FALSE;

  hestOptAdd(&opt, "c,center", "x y", airTypeDouble, 2, 2, center, NULL,
             "center of rings, in index space of fastest two axes");
  hestOptAdd(&opt, "v,verbose", "v", airTypeInt, 1, 1, &verbose, "0",
             "verbosity level");
  hestOptAdd(&opt, "li,linterp", "bool", airTypeBool, 1, 1, &linterp, "false",
             "whether to use linear interpolation during polar transform");
  hestOptAdd(&opt, "vs,vertseam", "bool", airTypeBool, 1, 1, &vertSeam, "false",
             "whether to dering left and right sides separately "
             "(requires an even value for -tn thetanum)");
  hestOptAdd(&opt, "tn,thetanum", "# smpls", airTypeUInt, 1, 1, &thetaNum,
             "20", "# of theta samples");
  hestOptAdd(&opt, "rs,radscale", "scale", airTypeDouble, 1, 1, &radScale,
             "1.0", "scaling on radius in polar transform");
  hestOptAdd(&opt, "rk,radiuskernel", "kern", airTypeOther, 1, 1, &rkspec,
             "gauss:3,4",
             "kernel for high-pass filtering along radial direction",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&opt, "tk,thetakernel", "kern", airTypeOther, 1, 1, &tkspec,
             "box",
             "kernel for blurring along theta direction.",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&opt, "cp,clampperc", "lo hi", airTypeDouble, 2, 2, clampPerc,
             "0.0 0.0",
             "when clamping values as part of ring estimation, the "
             "clamping range is set to exclude this percent of values "
             "from the low and high end of the data range");
  hestOptAdd(&opt, "m,mask", "mask", airTypeOther, 1, 1, &nmask, "",
             "optional: after deringing, output undergoes a lerp, "
             "parameterized by this array, from the background value "
             "(via \"-b\") where mask=0 to the original deringing "
             "output where mask=1.  This lerp is effectively the same "
             "as a \"unu 3op lerp\", so this should either be match the "
             "input in size, or match its slices along the slowest axis.",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "b,back", "val", airTypeDouble, 1, 1, &backval, "0.0",
             "when using a mask (\"-m\"), the background value to "
             "lerp with.");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_deringInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (nmask) {
    if (!(2 == nmask->dim
          && nrrdTypeBlock != nmask->type
          && nmask->axis[0].size == nin->axis[0].size
          && nmask->axis[1].size == nin->axis[1].size)) {
      fprintf(stderr, "%s: given mask not 2-D %u-by-%u array of scalar type",
              me, AIR_CAST(unsigned int, nin->axis[0].size),
              AIR_CAST(unsigned int, nin->axis[1].size));
      airMopError(mop);
      return 1;
    }
  }

  drc = nrrdDeringContextNew();
  airMopAdd(mop, drc, (airMopper)nrrdDeringContextNix, airMopAlways);
  if (nrrdDeringVerboseSet(drc, verbose)
      || nrrdDeringLinearInterpSet(drc, linterp)
      || nrrdDeringVerticalSeamSet(drc, vertSeam)
      || nrrdDeringInputSet(drc, nin)
      || nrrdDeringCenterSet(drc, center[0], center[1])
      || nrrdDeringRadiusScaleSet(drc, radScale)
      || nrrdDeringThetaNumSet(drc, thetaNum)
      || nrrdDeringRadialKernelSet(drc, rkspec->kernel, rkspec->parm)
      || nrrdDeringThetaKernelSet(drc, tkspec->kernel, tkspec->parm)
      || nrrdDeringClampPercSet(drc, clampPerc[0], clampPerc[1])
      || nrrdDeringExecute(drc, nout)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error deringing:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  if (nmask) {
    NrrdIter *nitout, *nitmask, *nitback;
    Nrrd *ntmp;
    nitout = nrrdIterNew();
    airMopAdd(mop, nitout, (airMopper)nrrdIterNix, airMopAlways);
    nitmask = nrrdIterNew();
    airMopAdd(mop, nitmask, (airMopper)nrrdIterNix, airMopAlways);
    nitback = nrrdIterNew();
    airMopAdd(mop, nitback, (airMopper)nrrdIterNix, airMopAlways);
    nrrdIterSetValue(nitback, backval);

    ntmp = nrrdNew();
    airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);

    nrrdIterSetNrrd(nitout, nout);
    nrrdIterSetNrrd(nitmask, nmask);
    if (nrrdArithIterTernaryOpSelect(ntmp, nrrdTernaryOpLerp,
                                     nitmask, nitback, nitout, 2)
        || nrrdCopy(nout, ntmp)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error masking:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD_HIDE(dering, INFO);
