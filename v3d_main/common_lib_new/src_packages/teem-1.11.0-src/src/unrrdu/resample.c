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

#define INFO "Filtering and {up,down}sampling with a separable kernel"
static const char *_unrrdu_resampleInfoL =
(INFO
 ". Simplifies access to the NrrdResampleContext functions "
 "by assuming (among other things) that the same kernel "
 "is used for resampling "
 "every axis that is being resampled.  Only required option is "
 "\"-s\" to specify which axes to resample and how many "
 "output samples to generate.  Resampling kernel \"-k\" defaults "
 "to an interpolating cubic, but many other choices are available. "
 "By default, resampling an axis resamples the full extent of its "
 "samples, but it is possible to offset this range via \"-off\", "
 "or to crop and/or pad via \"-min\" and \"-max\". "
 "The resampling respects the difference between cell- and "
 "node-centered data, but you can over-ride known centering "
 "with \"-co\".\n "
 "* Uses the many nrrdResample* functions operating on a nrrdResampleContext");

int
unrrdu_resampleMain(int argc, const char **argv, const char *me,
                    hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  int type, bb, pret, norenorm, neb, older, E, defaultCenter,
    verbose, overrideCenter, minSet=AIR_FALSE, maxSet=AIR_FALSE,
    offSet=AIR_FALSE;
  unsigned int scaleLen, ai, samplesOut, minLen, maxLen, offLen,
    aspRatNum, nonAspRatNum, nonAspRatIdx;
  airArray *mop;
  double *scale;
  double padVal, *min, *max, *off, aspRatScl=AIR_NAN;
  NrrdResampleInfo *info;
  NrrdResampleContext *rsmc;
  NrrdKernelSpec *unuk;

  mop = airMopNew();
  info = nrrdResampleInfoNew();
  airMopAdd(mop, info, (airMopper)nrrdResampleInfoNix, airMopAlways);
  hparm->elideSingleOtherDefault = AIR_FALSE;
  hestOptAdd(&opt, "old", NULL, airTypeInt, 0, 0, &older, NULL,
             "instead of using the new nrrdResampleContext implementation, "
             "use the old nrrdSpatialResample implementation");
  hestOptAdd(&opt, "s,size", "sz0", airTypeOther, 1, -1, &scale, NULL,
             "For each axis, information about how many samples in output:\n "
             "\b\bo \"=\": leave this axis completely untouched: no "
             "resampling whatsoever\n "
             "\b\bo \"x<float>\": multiply the number of input samples by "
             "<float>, and round to the nearest integer, to get the number "
             "of output samples.  Use \"x1\" to resample the axis but leave "
             "the number of samples unchanged\n "
             "\b\bo \"/<float>\": divide number of samples by <float>\n "
             "\b\bo \"+=<uint>\", \"-=<uint>\": add <uint> to or subtract "
             "<uint> from number input samples to get number output samples\n "
             "\b\bo \"<uint>\": exact number of output samples\n "
             "\b\bo \"a\": resample this axis to whatever number of samples "
             "preserves the aspect ratio of other resampled axes. Currently "
             "needs to be used on all but one of the resampled axes, "
             "if at all. ",
             &scaleLen, NULL, &unrrduHestScaleCB);
  hestOptAdd(&opt, "off,offset", "off0", airTypeDouble, 0, -1, &off, "",
             "For each axis, an offset or shift to the position (in index "
             "space) of the lower end of the sampling domain. "
             "Either -off can be used, or -min and -max "
             "together, or none of these (so that, by default, the full "
             "domain of the axis is resampled).",  &offLen);
  hestOptAdd(&opt, "min,minimum", "min0", airTypeDouble, 0, -1, &min, "",
             "For each axis, the lower end (in index space) of the domain "
             "of the resampling. Either -off can be used, or -min and -max "
             "together, or none of these (so that, by default, the full "
             "domain of the axis is resampled).",  &minLen);
  hestOptAdd(&opt, "max,maximum", "max0", airTypeDouble, 0, -1, &max, "",
             "For each axis, the upper end (in index space) of the domain "
             "of the resampling. Either -off can be used, or -min and -max "
             "together, or none of these, so that (by default), the full "
             "domain of the axis is resampled.",  &maxLen);
  hestOptAdd(&opt, "k,kernel", "kern", airTypeOther, 1, 1, &unuk,
             "cubic:0,0.5",
             "The kernel to use for resampling.  "
             "Kernels logically live in the input index space for upsampling, "
             "and in the output index space for downsampling.  "
             "Possibilities include:\n "
             "\b\bo \"box\": nearest neighbor interpolation on upsampling, "
             "and uniform averaging on downsampling\n "
             "\b\bo \"cheap\": nearest neighbor interpolation for upsampling, "
             "and non-blurring sub-sampling (pick subset of input samples) "
             "on downsampling\n "
             "\b\bo \"tent\": linear interpolation\n "
             "\b\bo \"cubic:B,C\": Mitchell/Netravali BC-family of "
             "cubics:\n "
             "\t\t\"cubic:1,0\": B-spline; maximal blurring\n "
             "\t\t\"cubic:0,0.5\": Catmull-Rom; good interpolating kernel\n "
             "\b\bo \"c4h\": 6-sample-support, C^4 continuous, accurate\n "
             "\b\bo \"c4hai\": discrete pre-filter to make c4h interpolate\n "
             "\b\bo \"bspl3\", \"bspl5\", \"bspl7\": cubic (same as cubic:1,0), "
             "quintic, and 7th order B-spline\n "
             "\b\bo \"bspl3ai\", \"bspl5ai\", \"bspl7ai\": discrete pre-filters to make "
             "bspl3, bspl5, bspl7 interpolate\n "
             "\b\bo \"hann:R\": Hann (cosine bell) windowed sinc, radius R\n "
             "\b\bo \"black:R\": Blackman windowed sinc, radius R\n "
             "\b\bo \"gauss:S,C\": Gaussian blurring, with standard deviation "
             "S and cut-off at C standard deviations\n "
             "\b\bo \"dgauss:S,C\": Lindeberg's discrete Gaussian.",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&opt, "nrn", NULL, airTypeInt, 0, 0, &norenorm, NULL,
             "do NOT do per-pass kernel weight renormalization. "
             "Doing the renormalization is not a performance hit (hence is "
             "enabled by default), and the renormalization is sometimes "
             "needed to avoid \"grating\" on non-integral "
             "down-sampling.  Disabling the renormalization is needed for "
             "correct results with artificially narrow kernels. ");
  hestOptAdd(&opt, "ne,nonexistent", "behavior", airTypeEnum, 1, 1,
             &neb, "noop",
             "When resampling floating-point values, how to handle "
             "non-existent values within kernel support:\n "
             "\b\bo \"noop\": do nothing; let them pollute result\n "
             "\b\bo \"renorm\": ignore them and renormalize weights of "
             "existent values\n "
             "\b\bo \"wght\": ignore them and simply use weights of "
             "existent values",
             NULL, nrrdResampleNonExistent);
  hestOptAdd(&opt, "b,boundary", "behavior", airTypeEnum, 1, 1, &bb, "bleed",
             "How to handle samples beyond the input bounds:\n "
             "\b\bo \"pad\": use some specified value\n "
             "\b\bo \"bleed\": extend border values outward\n "
             "\b\bo \"mirror\": repeated reflections\n "
             "\b\bo \"wrap\": wrap-around to other side",
             NULL, nrrdBoundary);
  hestOptAdd(&opt, "v,value", "value", airTypeDouble, 1, 1, &padVal, "0.0",
             "for \"pad\" boundary behavior, pad with this value");
  hestOptAdd(&opt, "t,type", "type", airTypeOther, 1, 1, &type, "default",
             "type to save OUTPUT as. By default (not using this option), "
             "the output type is the same as the input type",
             NULL, NULL, &unrrduHestMaybeTypeCB);
  hestOptAdd(&opt, "cheap", NULL, airTypeInt, 0, 0, &(info->cheap), NULL,
             "[DEPRECATED: the \"-k cheap\" option is the new (and more "
             "reliable) way to access this functionality. \"-cheap\" is "
             "only here for legacy use in combination with \"-old\".]\n "
             "When downsampling (reducing number of samples), don't "
             "try to do correct filtering by scaling kernel to match "
             "new (stretched) index space; keep it in old index space. "
             "When used in conjunction with \"-k box\", this can implement "
             "subsampling which chooses every Nth value. ");
  hestOptAdd(&opt, "c,center", "center", airTypeEnum, 1, 1, &defaultCenter,
             (nrrdCenterCell == nrrdDefaultCenter
              ? "cell"
              : "node"),
             "(not available with \"-old\") "
             "default centering of axes when input nrrd "
             "axes don't have a known centering: \"cell\" or \"node\" ",
             NULL, nrrdCenter);
  hestOptAdd(&opt, "co,center-override", NULL, airTypeInt, 0, 0,
             &overrideCenter, NULL,
             "(not available with \"-old\") "
             "centering info specified via \"-c\" should *over-ride* "
             "known centering, rather than simply be used when centering "
             "is unknown.");
  hestOptAdd(&opt, "verbose", "v", airTypeInt, 1, 1, &verbose, "0",
             "(not available with \"-old\") verbosity level");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_resampleInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (scaleLen != nin->dim) {
    fprintf(stderr, "%s: # sampling sizes %d != input nrrd dimension %d\n",
            me, scaleLen, nin->dim);
    airMopError(mop);
    return 1;
  }
  if (!older) {
    if (offLen >= 1) {
      /* seems to want to set off[] */
      if (offLen != scaleLen) {
        fprintf(stderr, "%s: offLen %u != scaleLen %u\n", me,
                offLen, scaleLen);
        airMopError(mop);
        return 1;
      }
      for (ai=0; ai<offLen; ai++) {
        if (unrrduScaleNothing != (int)(scale[0 + 2*ai])
            && !AIR_EXISTS(off[ai])) {
          fprintf(stderr, "%s: off[%u] %g doesn't exist\n", me,
                  ai, off[ai]);
          airMopError(mop);
          return 1;
        }
      }
      offSet = AIR_TRUE;
    } else {
      offSet = AIR_FALSE;
    }
    if (minLen >= 1 && AIR_EXISTS(min[0])) { /* HEY copy and paste */
      /* seems to want to set min[] */
      if (minLen != scaleLen) {
        fprintf(stderr, "%s: minLen %u != scaleLen %u\n", me,
                minLen, scaleLen);
        airMopError(mop);
        return 1;
      }
      for (ai=0; ai<minLen; ai++) {
        if (unrrduScaleNothing != (int)(scale[0 + 2*ai])
            && !AIR_EXISTS(min[ai])) {
          fprintf(stderr, "%s: min[%u] %g doesn't exist\n", me,
                  ai, min[ai]);
          airMopError(mop);
          return 1;
        }
      }
      minSet = AIR_TRUE;
    } else {
      minSet = AIR_FALSE;
    }
    if (maxLen >= 1 && AIR_EXISTS(max[0])) { /* HEY copy and paste */
      /* seems to want to set max[] */
      if (maxLen != scaleLen) {
        fprintf(stderr, "%s: maxLen %u != scaleLen %u\n", me,
                maxLen, scaleLen);
        airMopError(mop);
        return 1;
      }
      for (ai=0; ai<maxLen; ai++) {
        if (unrrduScaleNothing != (int)(scale[0 + 2*ai])
            && !AIR_EXISTS(max[ai])) {
          fprintf(stderr, "%s: max[%u] %g doesn't exist\n", me,
                  ai, max[ai]);
          airMopError(mop);
          return 1;
        }
      }
      maxSet = AIR_TRUE;
    } else {
      maxSet = AIR_FALSE;
    }
    if (!( (minSet && maxSet) || (!minSet && !maxSet) )) {
      fprintf(stderr, "%s: need -min and -max to be set consistently\n", me);
      airMopError(mop);
      return 1;
    }
    if (minSet && offSet) {
      fprintf(stderr, "%s: can't use -off with -min and -max\n", me);
      airMopError(mop);
      return 1;
    }
    aspRatNum = nonAspRatNum = 0;
    for (ai=0; ai<nin->dim; ai++) {
      int dowhat = AIR_CAST(int, scale[0 + 2*ai]);
      if (!(unrrduScaleNothing == dowhat)) {
        if (unrrduScaleAspectRatio == dowhat) {
          aspRatNum++;
        } else {
          nonAspRatNum++;
          nonAspRatIdx = ai;
        }
      }
    }
    if (aspRatNum) {
      if (1 != nonAspRatNum) {
        fprintf(stderr, "%s: sorry, aspect-ratio-preserving "
                "resampling must currently be used on all but one "
                "(not %u) resampled axis, if any\n", me,
                nonAspRatNum);
        airMopError(mop);
        return 1;
      }
    }

    rsmc = nrrdResampleContextNew();
    rsmc->verbose = verbose;
    airMopAdd(mop, rsmc, (airMopper)nrrdResampleContextNix, airMopAlways);
    E = AIR_FALSE;
    if (!E) E |= nrrdResampleDefaultCenterSet(rsmc, defaultCenter);
    if (!E) E |= nrrdResampleInputSet(rsmc, nin);
    for (ai=0; ai<nin->dim; ai++) {
      int dowhat = AIR_CAST(int, scale[0 + 2*ai]);
      switch(dowhat) {
      case unrrduScaleNothing:
        /* no resampling */
        if (!E) E |= nrrdResampleKernelSet(rsmc, ai, NULL, NULL);
        break;
      case unrrduScaleMultiply:
      case unrrduScaleDivide:
      case unrrduScaleAdd:
      case unrrduScaleSubtract:
        /* scaling of input # samples */
        if (defaultCenter && overrideCenter) {
          if (!E) E |= nrrdResampleOverrideCenterSet(rsmc, ai, defaultCenter);
        }
        if (!E) E |= nrrdResampleKernelSet(rsmc, ai, unuk->kernel, unuk->parm);
        switch(dowhat) {
          unsigned int incr;
          char stmp[AIR_STRLEN_SMALL];
        case unrrduScaleMultiply:
          samplesOut = AIR_ROUNDUP(nin->axis[ai].size*scale[1 + 2*ai]);
          break;
        case unrrduScaleDivide:
          samplesOut = AIR_ROUNDUP(nin->axis[ai].size/scale[1 + 2*ai]);
          break;
        case unrrduScaleAdd:
          samplesOut = nin->axis[ai].size + AIR_CAST(unsigned int, scale[1 + 2*ai]);
          break;
        case unrrduScaleSubtract:
          incr = AIR_CAST(unsigned int, scale[1 + 2*ai]);
          if (nin->axis[ai].size - 1 < incr) {
            fprintf(stderr, "%s: can't subtract %u from axis size %s\n",
                    me, incr, airSprintSize_t(stmp, nin->axis[ai].size));
            airMopError(mop);
            return 1;
          }
          samplesOut = nin->axis[ai].size - incr;
          break;
        }
        aspRatScl = AIR_CAST(double, samplesOut)/nin->axis[ai].size;
        if (!E) E |= nrrdResampleSamplesSet(rsmc, ai, samplesOut);
        break;
      case unrrduScaleExact:
        /* explicit # of samples */
        if (defaultCenter && overrideCenter) {
          if (!E) E |= nrrdResampleOverrideCenterSet(rsmc, ai, defaultCenter);
        }
        if (!E) E |= nrrdResampleKernelSet(rsmc, ai, unuk->kernel, unuk->parm);
        samplesOut = (size_t)scale[1 + 2*ai];
        aspRatScl = AIR_CAST(double, samplesOut)/nin->axis[ai].size;
        if (!E) E |= nrrdResampleSamplesSet(rsmc, ai, samplesOut);
        break;
      case unrrduScaleAspectRatio:
        /* wants aspect-ratio preserving, but may not know # samples yet */
        if (defaultCenter && overrideCenter) {
          if (!E) E |= nrrdResampleOverrideCenterSet(rsmc, ai, defaultCenter);
        }
        if (!E) E |= nrrdResampleKernelSet(rsmc, ai, unuk->kernel, unuk->parm);
        /* will set samples later, after aspRatScl has been set */
        break;
      default:
        fprintf(stderr, "%s: sorry, unrecognized unrrduScale value %d\n",
                me, dowhat);
        airMopError(mop);
        return 1;
      }
      if (minSet && maxSet) {
        if (!E) E |= nrrdResampleRangeSet(rsmc, ai, min[ai], max[ai]);
      } else {
        if (!E) E |= nrrdResampleRangeFullSet(rsmc, ai);
        if (offSet) {
          /* HEY: this is a hack; We're reading out the information from
             determined by nrrdResampleRangeFullSet, and benefitting from
             the fact that it set one of the flags that are processed by
             nrrdResampleExecute() */
          rsmc->axis[ai].min += off[ai];
          rsmc->axis[ai].max += off[ai];
        }
      }
    }
    if (!E && aspRatNum) {
      if (!AIR_EXISTS(aspRatScl)) {
        fprintf(stderr, "%s: confusion, should have learned scaling "
                "of aspect-ratio-preserving resampling by now", me);
        airMopError(mop);
        return 1;
      }
      for (ai=0; ai<nin->dim; ai++) {
        int dowhat = AIR_CAST(int, scale[0 + 2*ai]);
        if (unrrduScaleAspectRatio == dowhat) {
          samplesOut = AIR_ROUNDUP(nin->axis[ai].size*aspRatScl);
          if (!E) E |= nrrdResampleSamplesSet(rsmc, ai, samplesOut);
        }
      }
    }
    if (!E) E |= nrrdResampleBoundarySet(rsmc, bb);
    if (!E) E |= nrrdResampleTypeOutSet(rsmc, type);
    if (!E) E |= nrrdResamplePadValueSet(rsmc, padVal);
    if (!E) E |= nrrdResampleRenormalizeSet(rsmc, !norenorm);
    if (!E) E |= nrrdResampleNonExistentSet(rsmc, neb);
    if (!E) E |= nrrdResampleExecute(rsmc, nout);
    if (E) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error resampling nrrd:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
  } else {
    for (ai=0; ai<nin->dim; ai++) {
      int dowhat = AIR_CAST(int, scale[0 + 2*ai]);
      /* this may be over-written below */
      info->kernel[ai] = unuk->kernel;
      switch(dowhat) {
      case unrrduScaleNothing:
        /* no resampling */
        info->kernel[ai] = NULL;
        break;
      case unrrduScaleMultiply:
        /* scaling of input # samples */
        info->samples[ai] = AIR_ROUNDUP(scale[1 + 2*ai]*nin->axis[ai].size);
        break;
      case unrrduScaleExact:
        /* explicit # of samples */
        info->samples[ai] = (size_t)scale[1 + 2*ai];
        break;
      default:
        fprintf(stderr, "%s: sorry, unrecognized unrrduScale value %d\n",
                me, dowhat);
        airMopError(mop);
        return 1;
      }
      memcpy(info->parm[ai], unuk->parm,
             NRRD_KERNEL_PARMS_NUM*sizeof(*unuk->parm));
      if (info->kernel[ai] &&
          (!( AIR_EXISTS(nin->axis[ai].min)
              && AIR_EXISTS(nin->axis[ai].max))) ) {
        nrrdAxisInfoMinMaxSet(nin, ai,
                              (nin->axis[ai].center
                               ? nin->axis[ai].center
                               : nrrdDefaultCenter));
      }
      info->min[ai] = nin->axis[ai].min;
      info->max[ai] = nin->axis[ai].max;
    }
    info->boundary = bb;
    info->type = type;
    info->padValue = padVal;
    info->renormalize = !norenorm;
    if (nrrdSpatialResample(nout, nin, info)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error resampling nrrd:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
  }


  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(resample, INFO);
