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


#include <stdio.h>

#include <teem/biff.h>
#include <teem/hest.h>
#include <teem/nrrd.h>
#include <teem/gage.h>
#include <teem/ten.h>
#include <teem/meet.h>

#define SPACING(spc) (AIR_EXISTS(spc) ? spc: nrrdDefaultSpacing)

/* copied this from ten.h; I don't want gage to depend on ten */
#define PROBE_MAT2LIST(l, m) ( \
   (l)[1] = (m)[0],          \
   (l)[2] = (m)[3],          \
   (l)[3] = (m)[6],          \
   (l)[4] = (m)[4],          \
   (l)[5] = (m)[7],          \
   (l)[6] = (m)[8] )

static const char *deconvInfo = ("Does deconvolution. ");

int
main(int argc, const char *argv[]) {
  gageKind *kind;
  const char *me;
  char *outS, *err;
  hestParm *hparm;
  hestOpt *hopt = NULL;
  NrrdKernelSpec *ksp;
  int otype, separ, ret;
  unsigned int maxIter;
  double epsilon, lastDiff, step;
  Nrrd *nin, *nout;
  airArray *mop;

  mop = airMopNew();
  me = argv[0];
  hparm = hestParmNew();
  airMopAdd(mop, hparm, AIR_CAST(airMopper, hestParmFree), airMopAlways);
  hparm->elideSingleOtherType = AIR_TRUE;
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "k", "kind", airTypeOther, 1, 1, &kind, NULL,
             "\"kind\" of volume (\"scalar\", \"vector\", "
             "\"tensor\", or \"dwi\")",
             NULL, NULL, meetHestGageKind);
  hestOptAdd(&hopt, "k00", "kernel", airTypeOther, 1, 1, &ksp, NULL,
             "convolution kernel",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "mi", "max # iters", airTypeUInt, 1, 1, &maxIter, "100",
             "maximum number of iterations with which to compute the "
             "deconvolution");
  hestOptAdd(&hopt, "e", "epsilon", airTypeDouble, 1, 1, &epsilon,
             "0.00000001", "convergence threshold");
  hestOptAdd(&hopt, "s", "step", airTypeDouble, 1, 1, &step, "1.0",
             "scaling of value update");
  hestOptAdd(&hopt, "t", "type", airTypeOther, 1, 1, &otype, "default",
             "type to save output as. By default (not using this option), "
             "the output type is the same as the input type",
             NULL, NULL, &unrrduHestMaybeTypeCB);
  hestOptAdd(&hopt, "sep", "bool", airTypeBool, 1, 1, &separ, "false",
             "use fast separable deconvolution instead of brain-dead "
             "brute-force iterative method");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output volume");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, deconvInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, AIR_CAST(airMopper, hestOptFree), airMopAlways);
  airMopAdd(mop, hopt, AIR_CAST(airMopper, hestParseFree), airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, AIR_CAST(airMopper, nrrdNuke), airMopAlways);

  if (separ) {
    ret = gageDeconvolveSeparable(nout, nin, kind, ksp, otype);
  } else {
    ret = gageDeconvolve(nout, &lastDiff,
                         nin, kind,
                         ksp, otype,
                         maxIter, AIR_TRUE,
                         step, epsilon, 1);
  }
  if (ret) {
    airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }

  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving output:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }

  airMopOkay(mop);
  return 0;
}
