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

#define INFO "Try to create the look of early 80s analog B+W video"
static const char *_unrrdu_vidiconInfoL =
  (INFO
   ". Does various things, some more justified than others.\n "
   "* (as yet there's no single nrrd function which does all this)");

int
unrrdu_vidiconMain(int argc, const char **argv, const char *me,
                   hestParm *hparm) {
  hestOpt *opt = NULL;
  airArray *mop, *submop=NULL;

  unsigned int vsize[2], vpadding[2], rpadding[2];
  double rescale, rperc;
  Nrrd *nin, *nrescale, *npad, *nvbase, *ntmp, *nout;
  char *out, *err, *stpfx, stname[AIR_STRLEN_SMALL];
  int pret;
  NrrdResampleContext *rsmc;
  NrrdKernelSpec *rescaleKsp, *vdsmp[2];
  NrrdRange *b8range;

  hparm->elideSingleOtherDefault = AIR_FALSE;

  hestOptAdd(&opt, "i", "input", airTypeOther, 1, 1, &nin, NULL,
             "input image. Should be grayscale PNG.",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "rs", "rescale", airTypeDouble, 1, 1, &rescale, "0.75",
             "how to rescale (downsample) the image prior to processing, "
             "just to get a better representation of the floating-point "
             "range of image values (overcoming 8-bit quantization effects)");
  hestOptAdd(&opt, "rsk", "kern", airTypeOther, 1, 1, &rescaleKsp, "hann:5",
             "kernel for rescaling.",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&opt, "rsp", "percentile", airTypeDouble, 1, 1, &rperc, "1.5",
             "after rescaling, the highest and lowest percentiles are mapped "
             "to 0.0 and 255.0, just to have a uniform range of intensities "
             "in subsequent processing. This option determines how big those "
             "percentiles are.");
  hestOptAdd(&opt, "vs", "sx sy", airTypeUInt, 2, 2, vsize, "550 525",
             "the lowest (\"video\") resolution to which the image is "
             "down-sampled, reflecting the limited resolution of the "
             "vidicon tubes");
  hestOptAdd(&opt, "pad", "padX padY", airTypeUInt, 2, 2, vpadding, "10 10",
             "at the lowest resolution, there should be this much padding "
             "by black, to reflect the fact the signal outside the tube "
             "(e.g. between scanlines is black)");
  hestOptAdd(&opt, "vk", "kernX kernY", airTypeOther, 2, 2, vdsmp,
             "hann:1,4 cubic:0,0.5",
             "kernels for downsampling to video resolution; the horizontal "
             "and vertical kernels are different",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&opt, "stp", "prefix", airTypeString, 1, 1, &stpfx, "",
             "if a string is given here, a series of images are saved, "
             "representing the various stages of processing");
  hestOptAdd(&opt, "o", "output", airTypeString, 1, 1, &out, NULL,
             "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_unrrdu_vidiconInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);
  ntmp = nrrdNew();
  airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  b8range = nrrdRangeNew(0.0, 255.0);
  airMopAdd(mop, b8range, (airMopper)nrrdRangeNix, airMopAlways);

  if (!( 2 == nin->dim && nrrdTypeBlock != nin->type )) {
    fprintf(stderr, "%s: need input as 2D grayscale image (not %u-d %s)\n",
            me, nin->dim, airEnumStr(nrrdType, nin->type));
    airMopError(mop); return 1;
  }
  nrescale = nrrdNew();
  airMopAdd(mop, nrescale, (airMopper)nrrdNuke, airMopAlways);

  fprintf(stderr, "%s: rescaling by %g ... \n", me, rescale);
  rsmc = nrrdResampleContextNew();
  airMopAdd(mop, rsmc, (airMopper)nrrdResampleContextNix, airMopAlways);
  if (nrrdResampleDefaultCenterSet(rsmc, nrrdCenterCell)
      || nrrdResampleInputSet(rsmc, nin)
      || nrrdResampleKernelSet(rsmc, 0, rescaleKsp->kernel, rescaleKsp->parm)
      || nrrdResampleKernelSet(rsmc, 1, rescaleKsp->kernel, rescaleKsp->parm)
      || nrrdResampleSamplesSet(rsmc, 0, rescale*nin->axis[0].size)
      || nrrdResampleSamplesSet(rsmc, 1, rescale*nin->axis[1].size)
      || nrrdResampleRangeFullSet(rsmc, 0)
      || nrrdResampleRangeFullSet(rsmc, 1)
      || nrrdResampleTypeOutSet(rsmc, nrrdTypeFloat)
      || nrrdResampleRenormalizeSet(rsmc, AIR_TRUE)
      || nrrdResampleExecute(rsmc, nrescale)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: problem rescaling:\n%s", me, err);
    airMopError(mop); return 1;
  }

#define SAVE_TMP(name, nrrd)                                            \
  if (airStrlen(stpfx)) {                                               \
    sprintf(stname, "%s-" #name ".png", stpfx);                         \
    if (nrrdQuantize(ntmp, nrrd, b8range, 8)                            \
        || nrrdSave(stname, ntmp, 0)) {                                 \
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);   \
      fprintf(stderr, "%s: problem saving %s:\n%s", me, stname, err);   \
      airMopError(mop); return 1;                                       \
    }                                                                   \
  }
  SAVE_TMP(rescale, nrescale);

  /* rescaling values to 0.0 -- 255.0 based on percentile rperc */
  {
    Nrrd *nhist;
    double *hist, sum, total, minval, maxval;
    unsigned int hi, hbins;
    float *rescaled;
    size_t ii, nn;

    submop = airMopNew();
    nhist = nrrdNew();
    hbins = 3000;
    airMopAdd(submop, nhist, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdHisto(nhist, nrescale, NULL, NULL, hbins, nrrdTypeDouble)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble making histogram:\n%s", me, err);
      airMopError(submop); airMopError(mop); return 1;
    }
    hist = AIR_CAST(double *, nhist->data);
    total = AIR_CAST(double, nrrdElementNumber(nrescale));
    minval = AIR_NAN;
    sum = 0;
    for (hi=0; hi<hbins; hi++) {
      sum += hist[hi];
      if (sum >= rperc*total/100.0) {
        minval = AIR_AFFINE(0, hi, hbins-1,
                            nhist->axis[0].min, nhist->axis[0].max);
        break;
      }
    }
    if (hi == hbins || !AIR_EXISTS(minval)) {
      fprintf(stderr, "%s: failed to find lower %g-percentile value",
              me, rperc);
      airMopError(submop); airMopError(mop); return 1;
    }
    maxval = AIR_NAN;
    sum = 0;
    for (hi=hbins; hi; hi--) {
      sum += hist[hi-1];
      if (sum >= rperc*total/100.0) {
        maxval = AIR_AFFINE(0, hi-1, hbins-1,
                            nhist->axis[0].min, nhist->axis[0].max);
        break;
      }
    }
    if (!hi || !AIR_EXISTS(maxval)) {
      fprintf(stderr, "%s: failed to find upper %g-percentile value",
              me, rperc);
      airMopError(submop); airMopError(mop); return 1;
    }
    fprintf(stderr, "%s: min %g --> 0, max %g --> 255\n", me, minval, maxval);
    nn = nrrdElementNumber(nrescale);
    rescaled = AIR_CAST(float *, nrescale->data);
    for (ii=0; ii<nn; ii++) {
      rescaled[ii] = AIR_AFFINE(minval, rescaled[ii], maxval, 0.0, 255.0);
    }
    airMopOkay(submop);
    submop = NULL;
  }

  /* padding rescaled image with black */
  rpadding[0] = AIR_ROUNDUP(AIR_CAST(double, vpadding[0])
                            * nrescale->axis[0].size / vsize[0]);
  rpadding[1] = AIR_ROUNDUP(AIR_CAST(double, vpadding[1])
                            * nrescale->axis[1].size / vsize[1]);
  fprintf(stderr, "%s: padding in rescaled image: %u x %u\n",
          me, rpadding[0], rpadding[1]);
  npad = nrrdNew();
  airMopAdd(mop, npad, (airMopper)nrrdNuke, airMopAlways);
  {
    ptrdiff_t pmin[2], pmax[2];
    pmin[0] = -AIR_CAST(ptrdiff_t, rpadding[0]);
    pmin[1] = -AIR_CAST(ptrdiff_t, rpadding[1]);
    pmax[0] = nrescale->axis[0].size + rpadding[0];
    pmax[1] = nrescale->axis[1].size + rpadding[1];
    if (nrrdPad_nva(npad, nrescale, pmin, pmax, nrrdBoundaryPad, 0.0)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: problem padding:\n%s", me, err);
      airMopError(mop); return 1;
    }
  }

  /* rescaling down to "video" resolution */
  fprintf(stderr, "%s: downsampling to %u x %u\n", me,
          AIR_CAST(unsigned int, vsize[0] + 2*vpadding[0]),
          AIR_CAST(unsigned int, vsize[1] + 2*vpadding[1]));
  nvbase = nrrdNew();
  airMopAdd(mop, nvbase, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdResampleDefaultCenterSet(rsmc, nrrdCenterCell)
      || nrrdResampleInputSet(rsmc, npad)
      || nrrdResampleKernelSet(rsmc, 0, vdsmp[0]->kernel, vdsmp[0]->parm)
      || nrrdResampleKernelSet(rsmc, 1, vdsmp[1]->kernel, vdsmp[1]->parm)
      || nrrdResampleSamplesSet(rsmc, 0, vsize[0] + 2*vpadding[0])
      || nrrdResampleSamplesSet(rsmc, 1, vsize[1] + 2*vpadding[1])
      || nrrdResampleRangeFullSet(rsmc, 0)
      || nrrdResampleRangeFullSet(rsmc, 1)
      || nrrdResampleTypeOutSet(rsmc, nrrdTypeFloat)
      || nrrdResampleRenormalizeSet(rsmc, AIR_TRUE)
      || nrrdResampleExecute(rsmc, nvbase)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: problem downsampling to video resolution:\n%s", me, err);
    airMopError(mop); return 1;
  }

  /* halo, lowfilt, windowing, noise, filt, interlace, noise, fuzz, upsample */

  nrrdCopy(nout, nvbase);

  SAVE(out, nout, NULL);
  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD_HIDE(vidicon, INFO);
