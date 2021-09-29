/*
  Teem: Tools to process and visualize scientific data and images             .
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

#include "../gage.h"

char *optsigInfo = ("Computes tables of optimal sigmas for hermite-spline "
                    "reconstruction of scale space");

int
main(int argc, const char *argv[]) {
  const char *me;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;

  char *err, *outS;
  double sigmaMax, convEps, cutoff;
  int measr[2], tentRecon;
  unsigned int sampleNumMax, dim, measrSampleNum, maxIter, num, ii;
  gageOptimSigParm *osparm;
  double *scalePos, *out, info[512];
  Nrrd *nout;
  double *plotPos; unsigned int plotPosNum;

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "num", "# samp", airTypeUInt, 1, 1, &sampleNumMax, NULL,
             "maximum number of samples to optimize");
  hestOptAdd(&hopt, "dim", "dimension", airTypeUInt, 1, 1, &dim, "3",
             "dimension of image to work with");
  hestOptAdd(&hopt, "max", "sigma", airTypeDouble, 1, 1, &sigmaMax, NULL,
             "sigma to use for top sample (using 0 for bottom sample)");
  hestOptAdd(&hopt, "cut", "cut", airTypeDouble, 1, 1, &cutoff, "4",
             "at how many sigmas to cut-off discrete gaussian");
  hestOptAdd(&hopt, "mi", "max", airTypeUInt, 1, 1, &maxIter, "1000",
             "maximum # iterations");
  hestOptAdd(&hopt, "N", "# samp", airTypeUInt, 1, 1, &measrSampleNum, "300",
             "number of samples in the measurement of error across scales");
  hestOptAdd(&hopt, "eps", "eps", airTypeDouble, 1, 1, &convEps, "0.0001",
             "convergence threshold for optimization");
  hestOptAdd(&hopt, "m", "m1 m2", airTypeEnum, 2, 2, measr, "l2 l2",
             "how to measure error across image, and across scales",
             NULL, nrrdMeasure);
  hestOptAdd(&hopt, "p", "s0 s1", airTypeDouble, 2, -1, &plotPos, "nan nan",
             "hack: dont do optimization; just plot the recon error given "
             "these samples along scale", &plotPosNum);
  hestOptAdd(&hopt, "tent", NULL, airTypeInt, 0, 0, &tentRecon, NULL,
             "same hack: plot error with tent recon, not hermite");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, NULL,
             "output array");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, optsigInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, AIR_CAST(airMopper, nrrdNuke), airMopAlways);

  osparm = gageOptimSigParmNew(sampleNumMax);
  airMopAdd(mop, osparm, AIR_CAST(airMopper, gageOptimSigParmNix),
            airMopAlways);
  scalePos = AIR_CAST(double *, calloc(sampleNumMax, sizeof(double)));
  airMopAdd(mop, scalePos, airFree, airMopAlways);

  osparm->plotting = (AIR_EXISTS(plotPos[0]) && AIR_EXISTS(plotPos[1]));
  if (gageOptimSigTruthSet(osparm, dim, sigmaMax, cutoff, measrSampleNum)) {
    airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s", me, err);
    airMopError(mop); return 1;
  }

  if (osparm->plotting) {
    if (gageOptimSigPlot(osparm, nout, plotPos, plotPosNum,
                         measr[0], tentRecon)) {
      airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble:\n%s", me, err);
      airMopError(mop); return 1;
    }
  } else {
    /* do sample position optimization */
    if (nrrdMaybeAlloc_va(nout, nrrdTypeDouble, 2,
                          AIR_CAST(size_t, sampleNumMax+1),
                          AIR_CAST(size_t, sampleNumMax+1))) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble allocating output:\n%s", me, err);
      airMopError(mop); return 1;
    }
    out = AIR_CAST(double *, nout->data);
    /* hacky way of saving some of the computation information */
    info[0] = cutoff;
    info[1] = measrSampleNum;
    info[2] = measr[0];
    info[3] = measr[1];
    info[4] = convEps;
    info[5] = maxIter;
    for (ii=0; ii<sampleNumMax+1; ii++) {
      out[ii] = info[ii];
    }
    for (num=2; num<=sampleNumMax; num++) {
      printf("\n%s: ======= optimizing %u/%u samples (sigmaMax %g) \n\n",
             me, num, sampleNumMax, sigmaMax);
      if (gageOptimSigCalculate(osparm, scalePos, num,
                                measr[0], measr[1],
                                convEps, maxIter)) {
        airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
        fprintf(stderr, "%s: trouble:\n%s", me, err);
        airMopError(mop); return 1;
      }
      out[sampleNumMax + (sampleNumMax+1)*num] = osparm->finalErr;
      for (ii=0; ii<num; ii++) {
        out[ii + (sampleNumMax+1)*num] = scalePos[ii];
      }
    }
  }
  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving output:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }

  airMopOkay(mop);
  exit(0);
}
