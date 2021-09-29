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

#define INFO "Converts masked non-redundant tensor images to redundant"
static const char *_tend_expandInfoL =
  (INFO
   ". For images of 3D tensors, this converts from a 7-value tensor "
   "starting with the confidence/mask value "
   "(conf, Dxx, Dxy, Dxz, Dyy, Dyz, Dzz) to "
   "a 9-value tensor with the full matrix "
   "(Dxx, Dxy, Dxz, Dxy, Dyy, Dyz, Dxz, Dyz, Dzz). "
   "This is set to all zeros when the confidence is below the given "
   "threshold. For images of 2D tensors, the conversion is from "
   "(conf, Dxx, Dxy, Dyy) to (Dxx, Dxy, Dxy, Dyy). " );

int
tend_expandMain(int argc, const char **argv, const char *me,
                hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  Nrrd *nin, *nout;
  char *outS;
  int orientRed, orientRedWithOrigin, mfRed;
  float scale, thresh;

  hestOptAdd(&hopt, "t", "thresh", airTypeFloat, 1, 1, &thresh, "0.5",
             "confidence level to threshold output tensors at.  Should "
             "be between 0.0 and 1.0.");
  hestOptAdd(&hopt, "s", "scale", airTypeFloat, 1, 1, &scale, "1.0",
             "how to scale values before saving as 9-value tensor.  Useful "
             "for visualization tools which assume certain characteristic "
             "ranges of eigenvalues");
  hestOptAdd(&hopt, "unmf", NULL, airTypeInt, 0, 0, &mfRed, NULL,
             "apply and remove the measurement frame, if it exists");
  hestOptAdd(&hopt, "ro", NULL, airTypeInt, 0, 0, &orientRed, NULL,
             "reduce general image orientation to axis-aligned spacings");
  hestOptAdd(&hopt, "roo", NULL, airTypeInt, 0, 0,
             &orientRedWithOrigin, NULL,
             "reduce general image orientation to axis-aligned spacings, "
             "while also making some effort to set axis mins from "
             "space origin");
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, "-",
             "input diffusion tensor volume, with 7 values per sample",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, NULL,
             "output tensor volume, with the 9 matrix components per sample");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_expandInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (mfRed
      && 3 == nin->spaceDim
      && AIR_EXISTS(nin->measurementFrame[0][0])) {
    if (tenMeasurementFrameReduce(nin, nin)) {
      airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble with measurement frame:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  }
  if (4 == nin->axis[0].size
      ? tenExpand2D(nout, nin, scale, thresh)
      : tenExpand(nout, nin, scale, thresh)) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble expanding tensors:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  if (orientRedWithOrigin || orientRed) {
    if (nrrdOrientationReduce(nout, nout,
                              orientRedWithOrigin
                              ? AIR_TRUE
                              : AIR_FALSE)) {
      airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble unorienting:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  }

  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
TEND_CMD(expand, INFO);
