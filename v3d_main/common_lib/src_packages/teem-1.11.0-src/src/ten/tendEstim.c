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

#define INFO "Estimate tensors from a set of DW images"
static const char *_tend_estimInfoL =
  (INFO
   ". The tensor coefficient weightings associated with "
   "each of the DWIs, the B-matrix, is given either as a separate array, "
   "(see \"tend bmat\" usage info for details), or by the key-value pairs "
   "in the DWI nrrd header.  A \"confidence\" value is computed with the "
   "tensor, based on a soft thresholding of the sum of all the DWIs, "
   "according to the threshold and softness parameters. ");

int
tend_estimThresholdFind(double *threshP, Nrrd *nbmat, Nrrd *nin4d) {
  static const char me[]="tend_estimThresholdFind";
  Nrrd **ndwi;
  airArray *mop;
  unsigned int slIdx, slNum, dwiAx, dwiNum,
    rangeAxisNum, rangeAxisIdx[NRRD_DIM_MAX];
  double *bmat, bten[7], bnorm;
  int dwiIdx;

  mop = airMopNew();

  if (!(threshP && nbmat && nin4d)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    airMopError(mop); return 1;
  }
  if (tenBMatrixCheck(nbmat, nrrdTypeDouble, 6)) {
    biffAddf(TEN, "%s: problem within given b-matrix", me);
    airMopError(mop); return 1;
  }

  /* HEY: copied from tenEpiRegister4D() */
  rangeAxisNum = nrrdRangeAxesGet(nin4d, rangeAxisIdx);
  if (0 == rangeAxisNum) {
    /* we fall back on old behavior */
    dwiAx = 0;
  } else if (1 == rangeAxisNum) {
    /* thankfully there's exactly one range axis */
    dwiAx = rangeAxisIdx[0];
  } else {
    biffAddf(TEN, "%s: have %u range axes instead of 1, don't know which "
             "is DWI axis", me, rangeAxisNum);
    airMopError(mop); return 1;
  }

  slNum = nin4d->axis[dwiAx].size;
  bmat = AIR_CAST(double *, nbmat->data);
  dwiNum = 0;
  for (slIdx=0; slIdx<slNum; slIdx++) {
    TEN_T_SET(bten, 1.0,
              bmat[0], bmat[1], bmat[2],
              bmat[3], bmat[4],
              bmat[5]);
    bnorm = TEN_T_NORM(bten);
    dwiNum += bnorm > 0.0;
    bmat += 6;
  }
  if (0 == dwiNum) {
    biffAddf(TEN, "%s: somehow got zero DWIs", me);
    airMopError(mop); return 1;
  }
  ndwi = AIR_CALLOC(dwiNum, Nrrd *);
  airMopAdd(mop, ndwi, (airMopper)airFree, airMopAlways);
  bmat = AIR_CAST(double *, nbmat->data);
  dwiIdx = -1;
  for (slIdx=0; slIdx<slNum; slIdx++) {
    TEN_T_SET(bten, 1.0,
              bmat[0], bmat[1], bmat[2],
              bmat[3], bmat[4],
              bmat[5]);
    bnorm = TEN_T_NORM(bten);
    if (bnorm > 0.0) {
      dwiIdx++;
      ndwi[dwiIdx] = nrrdNew();
      airMopAdd(mop, ndwi[dwiIdx], (airMopper)nrrdNuke, airMopAlways);
      if (nrrdSlice(ndwi[dwiIdx], nin4d, dwiAx, slIdx)) {
        biffMovef(TEN, NRRD,
                  "%s: trouble slicing DWI at index %u", me, slIdx);
        airMopError(mop); return 1;
      }
    }
    bmat += 6;
  }
  if (_tenEpiRegThresholdFind(threshP, ndwi, dwiNum, AIR_FALSE, 1.5)) {
    biffAddf(TEN, "%s: trouble finding thresh", me);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}

int
tend_estimMain(int argc, const char **argv, const char *me,
               hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  Nrrd **nin, *nin4d, *nbmat, *nterr, *nB0, *nout;
  char *outS, *terrS, *bmatS, *eb0S;
  float soft, scale, sigma;
  int dwiax, EE, knownB0, oldstuff, estmeth, verbose, fixneg;
  unsigned int ninLen, axmap[4], wlsi, *skip, skipNum, skipIdx;
  double valueMin, thresh;

  Nrrd *ngradKVP=NULL, *nbmatKVP=NULL;
  double bKVP, bval;

  tenEstimateContext *tec;

  hestOptAdd(&hopt, "old", NULL, airTypeInt, 0, 0, &oldstuff, NULL,
             "instead of the new tenEstimateContext code, use "
             "the old tenEstimateLinear code");
  hestOptAdd(&hopt, "sigma", "sigma", airTypeFloat, 1, 1, &sigma, "nan",
             "Rician noise parameter");
  hestOptAdd(&hopt, "v", "verbose", airTypeInt, 1, 1, &verbose, "0",
             "verbosity level");
  hestOptAdd(&hopt, "est", "estimate method", airTypeEnum, 1, 1, &estmeth,
             "lls",
             "estimation method to use. \"lls\": linear-least squares",
             NULL, tenEstimate1Method);
  hestOptAdd(&hopt, "wlsi", "WLS iters", airTypeUInt, 1, 1, &wlsi, "1",
             "when using weighted-least-squares (\"-est wls\"), how "
             "many iterations to do after the initial weighted fit.");
  hestOptAdd(&hopt, "fixneg", NULL, airTypeInt, 0, 0, &fixneg, NULL,
             "after estimating the tensor, ensure that there are no negative "
             "eigenvalues by adding (to all eigenvalues) the amount by which "
             "the smallest is negative (corresponding to increasing the "
             "non-DWI image value).");
  hestOptAdd(&hopt, "ee", "filename", airTypeString, 1, 1, &terrS, "",
             "Giving a filename here allows you to save out the tensor "
             "estimation error: a value which measures how much error there "
             "is between the tensor model and the given diffusion weighted "
             "measurements for each sample.  By default, no such error "
             "calculation is saved.");
  hestOptAdd(&hopt, "eb", "filename", airTypeString, 1, 1, &eb0S, "",
             "In those cases where there is no B=0 reference image given "
             "(\"-knownB0 false\"), "
             "giving a filename here allows you to save out the B=0 image "
             "which is estimated from the data.  By default, this image value "
             "is estimated but not saved.");
  hestOptAdd(&hopt, "t", "thresh", airTypeDouble, 1, 1, &thresh, "nan",
             "value at which to threshold the mean DWI value per pixel "
             "in order to generate the \"confidence\" mask.  By default, "
             "the threshold value is calculated automatically, based on "
             "histogram analysis.");
  hestOptAdd(&hopt, "soft", "soft", airTypeFloat, 1, 1, &soft, "0",
             "how fuzzy the confidence boundary should be.  By default, "
             "confidence boundary is perfectly sharp");
  hestOptAdd(&hopt, "scale", "scale", airTypeFloat, 1, 1, &scale, "1",
             "After estimating the tensor, scale all of its elements "
             "(but not the confidence value) by this amount.  Can help with "
             "downstream numerical precision if values are very large "
             "or small.");
  hestOptAdd(&hopt, "mv", "min val", airTypeDouble, 1, 1, &valueMin, "1.0",
             "minimum plausible value (especially important for linear "
             "least squares estimation)");
  hestOptAdd(&hopt, "B", "B-list", airTypeString, 1, 1, &bmatS, NULL,
             "6-by-N list of B-matrices characterizing "
             "the diffusion weighting for each "
             "image.  \"tend bmat\" is one source for such a matrix; see "
             "its usage info for specifics on how the coefficients of "
             "the B-matrix are ordered. "
             "An unadorned plain text file is a great way to "
             "specify the B-matrix.\n  **OR**\n "
             "Can say just \"-B kvp\" to try to learn B matrices from "
             "key/value pair information in input images.");
  hestOptAdd(&hopt, "b", "b", airTypeDouble, 1, 1, &bval, "nan",
             "\"b\" diffusion-weighting factor (units of sec/mm^2)");
  hestOptAdd(&hopt, "knownB0", "bool", airTypeBool, 1, 1, &knownB0, NULL,
             "Indicates if the B=0 non-diffusion-weighted reference image "
             "is known, or if it has to be estimated along with the tensor "
             "elements.\n "
             "\b\bo if \"true\": in the given list of diffusion gradients or "
             "B-matrices, there are one or more with zero norm, which are "
             "simply averaged to find the B=0 reference image value\n "
             "\b\bo if \"false\": there may or may not be diffusion-weighted "
             "images among the input; the B=0 image value is going to be "
             "estimated along with the diffusion model");
  hestOptAdd(&hopt, "i", "dwi0 dwi1", airTypeOther, 1, -1, &nin, "-",
             "all the diffusion-weighted images (DWIs), as separate 3D nrrds, "
             "**OR**: One 4D nrrd of all DWIs stacked along axis 0",
             &ninLen, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output tensor volume");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_estimInfoL);
  JUSTPARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  nbmat = nrrdNew();
  airMopAdd(mop, nbmat, (airMopper)nrrdNuke, airMopAlways);

  /* figure out B-matrix */
  if (strcmp("kvp", airToLower(bmatS))) {
    /* its NOT coming from key/value pairs */
    if (!AIR_EXISTS(bval)) {
      fprintf(stderr, "%s: need to specify scalar b-value\n", me);
      airMopError(mop); return 1;
    }
    if (nrrdLoad(nbmat, bmatS, NULL)) {
      airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble loading B-matrix:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
    nin4d = nin[0];
    skip = NULL;
    skipNum = 0;
  } else {
    /* it IS coming from key/value pairs */
    if (1 != ninLen) {
      fprintf(stderr, "%s: require a single 4-D DWI volume for "
              "key/value pair based calculation of B-matrix\n", me);
      airMopError(mop); return 1;
    }
    if (oldstuff) {
      if (knownB0) {
        fprintf(stderr, "%s: sorry, key/value-based DWI info not compatible "
                "with older implementation of knownB0\n", me);
        airMopError(mop); return 1;
      }
    }
    if (tenDWMRIKeyValueParse(&ngradKVP, &nbmatKVP, &bKVP,
                              &skip, &skipNum, nin[0])) {
      airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble parsing DWI info:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
    if (AIR_EXISTS(bval)) {
      fprintf(stderr, "%s: WARNING: key/value pair derived b-value %g "
              "over-riding %g from command-line", me, bKVP, bval);
    }
    bval = bKVP;
    if (ngradKVP) {
      airMopAdd(mop, ngradKVP, (airMopper)nrrdNuke, airMopAlways);
      if (tenBMatrixCalc(nbmat, ngradKVP)) {
        airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
        fprintf(stderr, "%s: trouble finding B-matrix:\n%s\n", me, err);
        airMopError(mop); return 1;
      }
    } else {
      airMopAdd(mop, nbmatKVP, (airMopper)nrrdNuke, airMopAlways);
      if (nrrdConvert(nbmat, nbmatKVP, nrrdTypeDouble)) {
        airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
        fprintf(stderr, "%s: trouble converting B-matrix:\n%s\n", me, err);
        airMopError(mop); return 1;
      }
    }
    /* this will work because of the impositions of tenDWMRIKeyValueParse */
    dwiax = ((nrrdKindList == nin[0]->axis[0].kind ||
              nrrdKindVector == nin[0]->axis[0].kind)
             ? 0
             : ((nrrdKindList == nin[0]->axis[1].kind ||
                 nrrdKindVector == nin[0]->axis[1].kind)
                ? 1
                : ((nrrdKindList == nin[0]->axis[2].kind ||
                    nrrdKindVector == nin[0]->axis[2].kind)
                   ? 2
                   : 3)));
    if (0 == dwiax) {
      nin4d = nin[0];
    } else {
      axmap[0] = dwiax;
      axmap[1] = 1 > dwiax ? 1 : 0;
      axmap[2] = 2 > dwiax ? 2 : 1;
      axmap[3] = 3 > dwiax ? 3 : 2;
      nin4d = nrrdNew();
      airMopAdd(mop, nin4d, (airMopper)nrrdNuke, airMopAlways);
      if (nrrdAxesPermute(nin4d, nin[0], axmap)) {
        airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
        fprintf(stderr, "%s: trouble creating DWI volume:\n%s\n", me, err);
        airMopError(mop); return 1;
      }
    }
  }

  nterr = NULL;
  nB0 = NULL;
  if (!oldstuff) {
    if (1 != ninLen) {
      fprintf(stderr, "%s: sorry, currently need single 4D volume "
              "for new implementation\n", me);
      airMopError(mop); return 1;
    }
    if (!AIR_EXISTS(thresh)) {
      if (tend_estimThresholdFind(&thresh, nbmat, nin4d)) {
        airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
        fprintf(stderr, "%s: trouble finding threshold:\n%s\n", me, err);
        airMopError(mop); return 1;
      }
      /* HACK to lower threshold a titch */
      thresh *= 0.93;
      fprintf(stderr, "%s: using mean DWI threshold %g\n", me, thresh);
    }
    tec = tenEstimateContextNew();
    tec->progress = AIR_TRUE;
    airMopAdd(mop, tec, (airMopper)tenEstimateContextNix, airMopAlways);
    EE = 0;
    if (!EE) tenEstimateVerboseSet(tec, verbose);
    if (!EE) tenEstimateNegEvalShiftSet(tec, fixneg);
    if (!EE) EE |= tenEstimateMethodSet(tec, estmeth);
    if (!EE) EE |= tenEstimateBMatricesSet(tec, nbmat, bval, !knownB0);
    if (!EE) EE |= tenEstimateValueMinSet(tec, valueMin);
    for (skipIdx=0; skipIdx<skipNum; skipIdx++) {
      /* fprintf(stderr, "%s: skipping %u\n", me, skip[skipIdx]); */
      if (!EE) EE |= tenEstimateSkipSet(tec, skip[skipIdx], AIR_TRUE);
    }
    switch(estmeth) {
    case tenEstimate1MethodLLS:
      if (airStrlen(terrS)) {
        tec->recordErrorLogDwi = AIR_TRUE;
        /* tec->recordErrorDwi = AIR_TRUE; */
      }
      break;
    case tenEstimate1MethodNLS:
      if (airStrlen(terrS)) {
        tec->recordErrorDwi = AIR_TRUE;
      }
      break;
    case tenEstimate1MethodWLS:
      if (!EE) tec->WLSIterNum = wlsi;
      if (airStrlen(terrS)) {
        tec->recordErrorDwi = AIR_TRUE;
      }
      break;
    case tenEstimate1MethodMLE:
      if (!(AIR_EXISTS(sigma) && sigma > 0.0)) {
        fprintf(stderr, "%s: can't do %s w/out sigma > 0 (not %g)\n",
                me, airEnumStr(tenEstimate1Method, tenEstimate1MethodMLE),
                sigma);
        airMopError(mop); return 1;
      }
      if (!EE) EE |= tenEstimateSigmaSet(tec, sigma);
      if (airStrlen(terrS)) {
        tec->recordLikelihoodDwi = AIR_TRUE;
      }
      break;
    }
    if (!EE) EE |= tenEstimateThresholdSet(tec, thresh, soft);
    if (!EE) EE |= tenEstimateUpdate(tec);
    if (EE) {
      airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble setting up estimation:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
    if (tenEstimate1TensorVolume4D(tec, nout, &nB0,
                                   airStrlen(terrS)
                                   ? &nterr
                                   : NULL,
                                   nin4d, nrrdTypeFloat)) {
      airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble doing estimation:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
    if (airStrlen(terrS)) {
      airMopAdd(mop, nterr, (airMopper)nrrdNuke, airMopAlways);
    }
  } else {
    EE = 0;
    if (1 == ninLen) {
      EE = tenEstimateLinear4D(nout, airStrlen(terrS) ? &nterr : NULL, &nB0,
                               nin4d, nbmat, knownB0, thresh, soft, bval);
    } else {
      EE = tenEstimateLinear3D(nout, airStrlen(terrS) ? &nterr : NULL, &nB0,
                               (const Nrrd*const*)nin, ninLen, nbmat,
                               knownB0, thresh, soft, bval);
    }
    if (EE) {
      airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble making tensor volume:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  }
  if (nterr) {
    /* it was allocated by tenEstimate*, we have to clean it up */
    airMopAdd(mop, nterr, (airMopper)nrrdNuke, airMopAlways);
  }
  if (nB0) {
    /* it was allocated by tenEstimate*, we have to clean it up */
    airMopAdd(mop, nB0, (airMopper)nrrdNuke, airMopAlways);
  }
  if (1 != scale) {
    if (tenSizeScale(nout, nout, scale)) {
      airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble doing scaling:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  }
  if (nterr) {
    if (nrrdSave(terrS, nterr, NULL)) {
      airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble writing error image:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  }
  if (!knownB0 && airStrlen(eb0S)) {
    if (nrrdSave(eb0S, nB0, NULL)) {
      airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble writing estimated B=0 image:\n%s\n",
              me, err);
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
/* TEND_CMD(estim, INFO); */
unrrduCmd tend_estimCmd = { "estim", INFO, tend_estimMain };
