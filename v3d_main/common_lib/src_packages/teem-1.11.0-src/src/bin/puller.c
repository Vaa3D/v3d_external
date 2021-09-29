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

/*
** NOTE: all the "#ifdef DEFT" directives refer to an FLTK2-based GUI
** for some parts of Teem called "Deft".  Unfortunately FLTK2 has been
** abandoned, and Deft is not released or supported in any way.  The
** Deft-related code is preserved for legacy purposes.
*/

#ifdef DEFT
#include "Deft.h"
#include "Contour.h"
#include "Viewer.h"
#include "ViewerUI.h"
#include "TensorGlyph.h"
#include "TensorGlyphUI.h"
#include "Slider.h"
#include "TriPlane.h"
#include "TriPlaneUI.h"

#endif

#include <teem/pull.h>
#include <teem/meet.h>

static const char *info =
  ("Command-line interface to the \"pull\" library. "
   "Published research using this tool or the \"pull\" library "
   "should cite the paper: \n "
   "\t\tGordon L. Kindlmann, Ra{\\'u}l San Jos{\\'e} Est{\\'e}par, Stephen M. Smith,\n "
   "\t\tCarl-Fredrik Westin. Sampling and Visualizing Creases with Scale-Space\n "
   "\t\tParticles. IEEE Trans. on Visualization and Computer Graphics,\n "
   "\t\t15(6):1415-1424 (2009).");

#ifdef DEFT
typedef struct {
  fltk::FloatInput *scaleVecInput[3];
  fltk::ValueInput *glyphScaleRadInput;
  Deft::Slider *isoval;
  Deft::Slider *strength;
  Deft::Slider *quality;
  Deft::Slider *alpha, *beta, *cwell, *gamma;
  /* Deft::Slider *height; */
  Deft::Slider *ccSelect, *rho, *sclMean, *sclWind;
  fltk::CheckButton *ccSingle;
  Deft::Contour *contour;
  Deft::Scene *scene;
  Deft::Viewer *viewer;
  Deft::TensorGlyph *glyph, *hedge;
  fltk::IntInput *iters;
  fltk::FloatInput *radius;
  fltk::ValueInput *verbose;
  pullContext *pctx;
  Nrrd *nPosOut, *nTenOut, *nFrcOut, *nten, *ntmp, *nenr, *nscl,
    *nidcc, *nstrn, *nqual, *ncovar, *ntcovar, *nstab, *nintern,
    *nstuck, *nfrcOld, *nfrcNew, *nposOld, *nposNew, *nrgb, *nccrgb,
    *ncval, *ncmap, *ncmapOut, *nblur;
  NrrdResampleContext *rsmc;
  const Nrrd *norig;
  NrrdRange *cvalRange;
  limnPolyData *phistLine, *phistTube;
  Deft::PolyData *phistSurf;
  double icvalr[2], sclMin, sclMax, strnMin, qualMin,
    scaleVec[3], glyphScaleRad, energyIncreasePermitFrac;
} pullBag;

void
verbose_cb(fltk::Widget *widget, pullBag *bag) {
  fltk::ValueInput *val;

  val = (fltk::ValueInput *)widget;
  pullVerboseSet(bag->pctx, (int)val->value());
}

void
isovalue_cb(fltk::Widget *widget, pullBag *bag) {
  Deft::Slider *slider;

  slider = (Deft::Slider *)widget;
  if (bag->contour) {
    bag->contour->extract(slider->value());
  }
  bag->viewer->redraw();
}

void
outputGet(pullBag *bag) {
  char me[]="outputGet", *err;
  size_t cropMin[2], cropMax[2];

  if (pullOutputGet(bag->nPosOut, bag->nTenOut,
                    bag->nstrn, /* may be NULL */
                    bag->scaleVec, bag->glyphScaleRad,
                    bag->pctx)
      || pullPropGet(bag->nscl, pullPropScale, bag->pctx)
      || (bag->pctx->ispec[pullInfoQuality]
          ? pullInfoGet(bag->nqual, pullInfoQuality, bag->pctx)
          : 0)
      || pullPropGet(bag->nenr, pullPropEnergy, bag->pctx)
      || pullPropGet(bag->nidcc, pullPropIdCC, bag->pctx)
      || pullPropGet(bag->nstuck, pullPropStuck, bag->pctx)
      || pullPropGet(bag->ncovar, pullPropNeighCovar7Ten, bag->pctx)
#if PULL_TANCOVAR
      || pullPropGet(bag->ntcovar, pullPropNeighTanCovar, bag->pctx)
#endif
      || pullPropGet(bag->nstab, pullPropStability, bag->pctx)
      || pullPropGet(bag->nintern, pullPropNeighInterNum, bag->pctx)
      || pullPropGet(bag->nFrcOut, pullPropForce, bag->pctx)
      || (pullPhistEnabled
          ? pullPositionHistoryGet(bag->phistLine, bag->pctx)
          : 0)) {
    err = biffGetDone(PULL);
    fprintf(stderr, "%s: error getting pull output:\n%s\n", me, err);
    free(err);
    exit(1);
  }

  cropMin[0] = 0;
  cropMin[1] = 0;
  cropMax[0] = 2;
  cropMax[1] = bag->nPosOut->axis[1].size-1;
  if ((!bag->pctx->iter
       ? 0
       : (nrrdCopy(bag->nfrcOld, bag->nfrcNew)
          || nrrdCopy(bag->nposOld, bag->nposNew)))
      || nrrdConvert(bag->nten, bag->nTenOut, nrrdTypeFloat)
      /* hacks to visualize the (tan) covariance tensors
      || nrrdCopy(bag->nten, bag->ncovar)
      || nrrdCopy(bag->nten, bag->ntcovar)
      */
      || nrrdCrop(bag->ntmp, bag->nPosOut, cropMin, cropMax)
      || nrrdConvert(bag->nposNew, bag->ntmp, nrrdTypeFloat)
      || nrrdCrop(bag->ntmp, bag->nFrcOut, cropMin, cropMax)
      || nrrdConvert(bag->nfrcNew, bag->ntmp, nrrdTypeFloat)
      || (!bag->pctx->iter
          ? (nrrdCopy(bag->nfrcOld, bag->nfrcNew)
             || nrrdCopy(bag->nposOld, bag->nposNew))
          : 0)) {
    err = biffGetDone(NRRD);
    fprintf(stderr, "%s: another error 0\n%s\n", me, err);
    free(err);
    exit(1);
  }
}

void
outputShow(pullBag *bag) {
  char me[]="outputShow", *err;
  float *rgb;
  unsigned int ii, nn, *idcc;
  unsigned char *stuck;
  int first;
  double *cval, emean, estdv, *pos;

  /*
  if (limnPolyDataSpiralTubeWrap(bag->phistTube, bag->phistLine,
                                 (1 << limnPolyDataInfoRGBA)
                                 | (1 << limnPolyDataInfoNorm),
                                 NULL,
                                 8, 8, bag->glyph->glyphScale()/5)) {
    err = biffGetDone(LIMN);
    fprintf(stderr, "%s: another error 1\n%s\n", me, err);
    free(err);
    exit(1);
  }
  */

  if (pullPhistEnabled) {
    bag->phistSurf->changed();
  }

  bag->ncval = bag->nenr;
  /* bag->ncval = bag->nstrn; */
  /* bag->ncval = bag->nstuck; */
  /* bag->ncval = bag->nscl; */

  if (bag->ncval) {
    nrrdRangeSet(bag->cvalRange, bag->ncval, AIR_FALSE);
  } else {
    bag->cvalRange->min = AIR_NAN;
    bag->cvalRange->max = AIR_NAN;
  }
  if (bag->ncval) {
    cval = AIR_CAST(double *, bag->ncval->data);
  } else {
    cval = NULL;
  }
  if (cval) {
    nn = bag->ncval->axis[0].size;
    emean = 0;
    for (ii=0; ii<nn; ii++) {
      emean += cval[ii];
    }
    emean /= nn;
    estdv = 0;
    for (ii=0; ii<nn; ii++) {
      estdv += (emean - cval[ii])*(emean - cval[ii]);
    }
    estdv = sqrt(estdv/nn);
    if (bag->cvalRange->hasNonExist) {
      fprintf(stderr, "!%s: cval range %g -- %g (%s), mean %g, stdv %g\n", me,
              bag->cvalRange->min, bag->cvalRange->max,
              bag->cvalRange->hasNonExist ? "HAS non-exist" : "no non-exist",
              emean, estdv);
    }
    bag->cvalRange->min = AIR_LERP(0.7, bag->cvalRange->min, emean - 2*estdv);
    bag->cvalRange->max = AIR_LERP(0.7, bag->cvalRange->max, emean + 2*estdv);
  }
  float *cmapOut;
  if (bag->ncmap
      && bag->ncval
      && AIR_EXISTS(bag->cvalRange->min)
      && AIR_EXISTS(bag->cvalRange->max)) {
    /* double mmin, mmax; */
    fprintf(stderr, "!%s: cval cmap range %g %g ----------- \n", me,
            bag->cvalRange->min, bag->cvalRange->max);
    /*
    mmin = -0.0342937;
    mmax = -0.0105725;
    */
    /* */
    /*
    bag->cvalRange->min = AIR_LERP(0.05, mmin, mmax);
    bag->cvalRange->max = AIR_LERP(0.3, mmin, mmax);
    */
    /* */
    if (nrrdApply1DRegMap(bag->ncmapOut, bag->ncval, bag->cvalRange,
                          bag->ncmap, nrrdTypeFloat, AIR_TRUE)) {
      err = biffGetDone(NRRD);
      fprintf(stderr, "%s: cmap error\n%s\n", me, err);
      free(err);  exit(1);
    }
    cmapOut = AIR_CAST(float *, bag->ncmapOut->data);
  } else {
    cmapOut = NULL;
  }

  idcc = AIR_CAST(unsigned int *, bag->nidcc->data);
  stuck = AIR_CAST(unsigned char *, bag->nstuck->data);
  nn = bag->nPosOut->axis[1].size;
  pos = AIR_CAST(double *, bag->nPosOut->data);

  first = bag->nrgb->axis[1].size != bag->nPosOut->axis[1].size;
  /*
  fprintf(stderr, "!%s: %u %u -> %d\n", me,
          AIR_UINT(bag->nrgb->axis[1].size),
          AIR_UINT(bag->nPosOut->axis[1].size), first);
  */
  if (first) {
    if (nrrdMaybeAlloc_va(bag->nrgb, nrrdTypeFloat, 2,
                          AIR_CAST(size_t, 3),
                          bag->nPosOut->axis[1].size)) {
      err = biffGetDone(NRRD);
      fprintf(stderr, "%s: error creating RGB:\n%s\n", me, err);
      free(err);
      exit(1);
    }
  }
  bag->icvalr[0] = bag->cvalRange->min;
  bag->icvalr[1] = bag->cvalRange->max;

  double *strnOut;
  strnOut = (bag->nstrn
             ? AIR_CAST(double *, bag->nstrn->data)
             : NULL);
  double *qualOut;
  qualOut = (bag->nqual
             ? AIR_CAST(double *, bag->nqual->data)
             : NULL);
  rgb = (float*)bag->nrgb->data;
  for (ii=0; ii<nn; ii++) {
    float ee, *ccrgb;
    ccrgb = (float*)bag->nccrgb->data;
    /* ee = bag->cvalRange->min - (bag->icvalr[1] - bag->icvalr[0])/50;*/
    ee = bag->cvalRange->min;
    if (bag->pctx->CCNum && ccrgb) {
      rgb[0 + 3*ii] = ccrgb[0 + 3*idcc[ii]];
      rgb[1 + 3*ii] = ccrgb[1 + 3*idcc[ii]];
      rgb[2 + 3*ii] = ccrgb[2 + 3*idcc[ii]];
    } else if (cmapOut) {
      ELL_3V_COPY(rgb + 3*ii, cmapOut + 3*ii);
    } else {
      ELL_3V_SET(rgb + 3*ii, 0.95, 0.95, 0.95);
      /*
      if (AIR_EXISTS(cval[ii])) {
        rgb[1 + 3*ii] = AIR_AFFINE(ee, cval[ii], bag->cvalRange->max, 0, 1);
        rgb[1 + 3*ii] = sqrt(rgb[1 + 3*ii]);
        rgb[0 + 3*ii] = rgb[1 + 3*ii];
      } else {
        rgb[1 + 3*ii] = 0;
        rgb[0 + 3*ii] = 1;
      }
      rgb[2 + 3*ii] = stuck[ii];
      */
    }
  }
  if (1) {
    float *ten, *pos;
    double *posOut;
    ten = AIR_CAST(float *, bag->nten->data);
    posOut = AIR_CAST(double *, bag->nPosOut->data);
    pos = AIR_CAST(float *, bag->nposNew->data);
    for (ii=0; ii<nn; ii++) {
      if (!( AIR_IN_CL(bag->sclMin-0.00001, posOut[3],
                       bag->sclMax+0.00001) )) {
        ten[0] = 0;
      } else if (strnOut && strnOut[ii] < bag->strnMin) {
        ten[0] = 0;
      } else if (qualOut && qualOut[ii] < bag->qualMin-0.000001) {
        ten[0] = 0;
      } else if (bag->pctx->CCNum
                 && (bag->ccSingle->value()
                     ? idcc[ii] != bag->ccSelect->value()
                     : idcc[ii] > bag->ccSelect->value())) {
        ten[0] = 0;
      } else {
        ten[0] = 1;
      }
      /*
      if (4589 == ii) {
        fprintf(stderr, "!%s: point %u/%u at (%g,%g,%g)=(%g,%g,%g,%g) got ten[0] %g\n",
                me, ii, nn,
                pos[0], pos[1], pos[1],
                posOut[0], posOut[1], posOut[2], posOut[3],
                ten[0]);
      }
      */
      pos += 3;
      posOut += 4;
      ten += 7;
    }
  }
  if (bag->nPosOut->axis[1].size) {
    bag->glyph->dataSet(bag->nPosOut->axis[1].size,
                        (float*)bag->nten->data, 7,
                        (float*)bag->nposNew->data, 3, rgb, 3, NULL);
    bag->glyph->update();
    /*
    bag->hedge->dataSet(bag->nPosOut->axis[1].size,
                        (float*)bag->nfrcNew->data, 3,
                        (float*)bag->nposOld->data, 3, rgb, 3, NULL);
    bag->hedge->update();
    */
    bag->viewer->redraw();
    fltk::flush();
  } else {
    fprintf(stderr, "!%s: got zero tensors out!\n", me);
  }
  return;
}

void
iter_cb(void *_bag) {
  pullBag *bag;
  bag = AIR_CAST(pullBag *, _bag);
  outputGet(bag);
  outputShow(bag);
}

void
step_cb(fltk::Widget *, pullBag *bag) {
  /*  static double lastthresh = -42; */
  char me[]="step_cb", *err;
  static unsigned int itersTotal=0;

  unsigned int iters = bag->iters->ivalue();
  bag->pctx->iterParm.max += iters;
  if (pullRun(bag->pctx)) {
    err = biffGetDone(PULL);
    fprintf(stderr, "%s: error running pull:\n%s\n", me, err);
    free(err);
    exit(1);
  }
  itersTotal += iters;
  fprintf(stderr, "!%s: enr = %g; time = %g sec; %u iters (%g iters/sec)\n",
          me, bag->pctx->energy, bag->pctx->timeRun, itersTotal,
          itersTotal/bag->pctx->timeRun);
  outputGet(bag);
  outputShow(bag);
  for (unsigned int ci=pullCountUnknown+1; ci<pullCountLast; ci++) {
    if (bag->pctx->count[ci]) {
      fprintf(stderr, "  %u: %s\n", bag->pctx->count[ci],
              airEnumStr(pullCount, ci));
    }
  }
}

void
gammaSet_cb(fltk::Widget *, pullBag *bag) {
  char me[]="gammaSet_cb";

  if (pullGammaLearn(bag->pctx)) {
    char *err = biffGetDone(PULL);
    fprintf(stderr, "%s: problem learning gamma:\n%s", me, err);
    free(err);
  }
  if (bag->pctx->sysParm.gamma > bag->gamma->maximum()) {
    bag->gamma->maximum(2*bag->pctx->sysParm.gamma);
  }
  bag->gamma->value(bag->pctx->sysParm.gamma);
}

void
cc_cb(fltk::Widget *, pullBag *bag) {
  char me[]="cc_cb";
  unsigned int cc;
  float *rgb;

  if (pullCCFind(bag->pctx)
      || pullCCSort(bag->pctx,
                    (bag->pctx->ispec[pullInfoQuality]
                     ? pullInfoQuality
                     : 0), bag->rho->value())) {
    char *err = biffGetDone(PULL);
    fprintf(stderr, "%s: problem finding/sorting CCs:\n%s", me, err);
    free(err);
  }
  printf("%s: found %u CCs\n", me, bag->pctx->CCNum);
  bag->ccSelect->range(0, bag->pctx->CCNum-1);
  if (bag->nccrgb->axis[1].size != bag->pctx->CCNum) {
    airSrandMT(AIR_UINT(airTime()));
    if (nrrdMaybeAlloc_va(bag->nccrgb, nrrdTypeFloat, 2,
                          AIR_CAST(size_t, 3),
                          AIR_CAST(size_t, bag->pctx->CCNum))) {
      char *err = biffGetDone(NRRD);
      fprintf(stderr, "%s: problem alloc'ing cc rgb:\n%s", me, err);
      free(err);
    }
    rgb = (float*)bag->nccrgb->data;
    ELL_3V_SET(rgb + 0*3, 0.95, 0.95, 0.95);
    for (cc=0; cc<bag->pctx->CCNum; cc++) {
      rgb[0 + 3*cc] = AIR_AFFINE(0, airDrandMT(), 1, 0.3, 1.0);
      rgb[1 + 3*cc] = AIR_AFFINE(0, airDrandMT(), 1, 0.3, 1.0);
      rgb[2 + 3*cc] = AIR_AFFINE(0, airDrandMT(), 1, 0.3, 1.0);
    }
  }
  outputGet(bag);
  outputShow(bag);
}

void
ccSelect_cb(fltk::Widget *, pullBag *bag) {

  outputShow(bag);
}

void
scaleGlyph_cb(fltk::Widget *, pullBag *bag) {

  bag->scaleVec[0] = bag->scaleVecInput[0]->fvalue();
  bag->scaleVec[1] = bag->scaleVecInput[1]->fvalue();
  bag->scaleVec[2] = bag->scaleVecInput[2]->fvalue();
  bag->glyphScaleRad = bag->glyphScaleRadInput->value();
  outputGet(bag);
  outputShow(bag);
}

void
reblur_cb(fltk::Widget *, pullBag *bag) {
  static const char me[]="reblur_cb";
  double kparm[NRRD_KERNEL_PARMS_NUM], scl;
  int E;

  if (!bag->pctx->haveScale) {
    return;
  }
  scl = bag->sclMean->value();
  if (bag->pctx->flag.scaleIsTau) {
    kparm[0] = gageSigOfTau(scl);
    printf("!%s: tau = %g ---> sigma = %g\n", me, scl, kparm[0]);
  } else {
    kparm[0] = scl;
    printf("!%s: sigma = %g\n", me, kparm[0]);
  }
  kparm[1] = 3;
  E = 0;
  for (unsigned int axi=0; axi<3; axi++) {
    if (!E) E |= nrrdResampleKernelSet(bag->rsmc, axi,
                                       nrrdKernelDiscreteGaussian,
                                       kparm);
  }
  if (!E) E |= nrrdResampleExecute(bag->rsmc, bag->nblur);
  if (E) {
    char *err = biffGetDone(NRRD);
    fprintf(stderr, "%s: problem resampling to scale %g:\n%s",
            me, bag->sclMean->value(), err);
    free(err);
  }
  outputShow(bag);
  return;
}

void
scale_cb(fltk::Widget *, pullBag *bag) {
  double sclMean, sclWind;

  if (bag->pctx->haveScale) {
    sclMean = bag->sclMean->value();
    sclWind = bag->sclWind->value();
    bag->sclMin = sclMean - sclWind/2;
    bag->sclMax = sclMean + sclWind/2;
  } else {
    bag->sclMin = 0;
    bag->sclMax = 0;
  }
  outputShow(bag);
  return;
}

void
alpha_cb(fltk::Widget *, pullBag *bag) {

  pullSysParmSet(bag->pctx, pullSysParmAlpha, bag->alpha->value());
}

void
beta_cb(fltk::Widget *, pullBag *bag) {

  pullSysParmSet(bag->pctx, pullSysParmBeta, bag->beta->value());
}

void
cwell_cb(fltk::Widget *, pullBag *bag) {
  double *parm;

  parm = bag->pctx->energySpecR->parm;
  parm[1] = bag->cwell->value();
  pullSysParmSet(bag->pctx, pullSysParmEnergyIncreasePermit,
                 bag->energyIncreasePermitFrac*bag->cwell->value());
  {
    unsigned int ii, nn;
    double xx, yy, de;
    FILE *file;

    if ((file = fopen("eplot.txt", "w"))) {
      nn = 800;
      for (ii=0; ii<nn; ii++) {
        xx = AIR_AFFINE(0, ii, nn-1, 0.0, 1.0);
        yy = bag->pctx->energySpecR->energy->eval(&de, xx, parm);
        fprintf(file, "%f %f\n", xx, yy);
      }
      fclose(file);
    }
  }
}

void
gamma_cb(fltk::Widget *, pullBag *bag) {

  pullSysParmSet(bag->pctx, pullSysParmGamma, bag->gamma->value());
}

void
strength_cb(fltk::Widget *, pullBag *bag) {

  bag->strnMin = bag->strength->value();
  outputShow(bag);
}

void
quality_cb(fltk::Widget *, pullBag *bag) {

  bag->qualMin = bag->quality->value();
  outputShow(bag);
}

void
save_cb(fltk::Widget *, pullBag *bag) {
  static const char me[]="save_cb";
  unsigned int ii, nn, count;
  float *ten;
  Nrrd *nPosSel, *nStrnSel;
  double *posSel, *posAll, *strnSel, *strnAll;

  if (!( 0.0 == ELL_3V_LEN(bag->scaleVec) )) {
    fprintf(stderr, "%s: refusing to save with non-zero scaleVec %g %g %g\n",
            me, bag->scaleVec[0], bag->scaleVec[1], bag->scaleVec[2]);
    return;
  }

  nPosSel = nrrdNew();
  if (bag->nstrn) {
    nStrnSel = nrrdNew();
  } else {
    nStrnSel = NULL;
  }
  outputGet(bag);
  outputShow(bag); /* will exploit its masking of ten[0] */
  count = 0;
  nn = bag->nPosOut->axis[1].size;
  ten = AIR_CAST(float *, bag->nten->data);
  for (ii=0; ii<nn; ii++) {
    count += !!ten[0];
    ten += 7;
  }
  nrrdMaybeAlloc_va(nPosSel, nrrdTypeDouble, 2,
                    AIR_CAST(size_t, 4),
                    AIR_CAST(size_t, count));
  posAll = AIR_CAST(double *, bag->nPosOut->data);
  posSel = AIR_CAST(double *, nPosSel->data);
  if (bag->nstrn) {
    nrrdMaybeAlloc_va(nStrnSel, nrrdTypeDouble, 1,
                      AIR_CAST(size_t, count));
    strnAll = AIR_CAST(double *, bag->nstrn->data);
    strnSel = AIR_CAST(double *, nStrnSel->data);
  } else {
    strnSel = NULL;
    strnAll = NULL;
  }
  ten = AIR_CAST(float *, bag->nten->data);
  count = 0;
  for (ii=0; ii<nn; ii++) {
    if (ten[0]) {
      ELL_4V_COPY(posSel, posAll);
      if (strnSel && strnAll) {
        strnSel[count] = strnAll[ii];
      }
      posSel += 4;
      count++;
    }
    posAll += 4;
    ten += 7;
  }
  nrrdSave("pos-all.nrrd", bag->nPosOut, NULL);
  if (0) {
    nrrdSave("pos-sel.nrrd", nPosSel, NULL);
  } else {
    char fname[512];
    FILE *ff;
    unsigned int ii=0;

    for (ii=0, ff=NULL; AIR_TRUE; ii++) {
      ff = airFclose(ff);
      sprintf(fname, "pos-sel-%03u.nrrd", ii);
      if (!(ff = fopen(fname, "rb"))) {
        break;
      }
    }
    ff = airFclose(ff);
    nrrdSave(fname, nPosSel, NULL);
  }
  nrrdSave("covar-all.nrrd", bag->ncovar, NULL);
#if PULL_TANCOVAR
  nrrdSave("tcovar-all.nrrd", bag->ntcovar, NULL);
#endif
  nrrdSave("stab-all.nrrd", bag->nstab, NULL);
  nrrdSave("intern-all.nrrd", bag->nintern, NULL);
  if (bag->nstrn) {
    nrrdSave("strn-all.nrrd", bag->nstrn, NULL);
    nrrdSave("strn-sel.nrrd", nStrnSel, NULL);
  }
  nrrdNuke(nPosSel);
  if (bag->nstrn) {
    nrrdNuke(nStrnSel);
  }
  return;
}

#endif

int
main(int argc, const char **argv) {
  hestOpt *hopt=NULL;
  hestParm *hparm;
  airArray *mop;
  const char *me;

#ifdef DEFT
  float fr[3], at[3], up[3], fovy, neer, faar, dist, bg[3];
  int imgSize[2], ortho, rght, atrel, camkeep;

  float anisoThresh, anisoThreshMin, glyphScale, haloTraceBound,
    glyphHaloWidth, glyphNormPow, glyphEvalPow, sqdSharp;
  int glyphType, glyphFacetRes, aniso;

  pullBag bag;
#endif

  char *err, *outS, *extraOutBaseS, *addLogS, *cachePathSS;
  FILE *addLog;
  meetPullVol **vspec;
  meetPullInfo **idef;
  Nrrd *nPosIn=NULL, *nPosOut;
  pullEnergySpec *enspR, *enspS, *enspWin;
  NrrdKernelSpec *k00, *k11, *k22, *kSSrecon, *kSSblur;
  pullContext *pctx;
  int E=0, ret=0;
  unsigned int vsi, vspecNum, idefNum;
  double ssrange[2], scaleVec[3], glyphScaleRad;
  /* things that used to be set directly inside pullContext */
  int energyFromStrength, nixAtVolumeEdgeSpace, constraintBeforeSeedThresh,
    binSingle, liveThresholdOnInit, permuteOnRebin, noPopCntlWithZeroAlpha,
    useBetaForGammaLearn, restrictiveAddToBins, noAdd, unequalShapesAllow,
    popCntlEnoughTest;
  int verbose;
  int interType, allowCodimension3Constraints, scaleIsTau, useHalton,
    pointPerVoxel;
  unsigned int samplesAlongScaleNum, pointNumInitial,
    ppvZRange[2], snap, iterMax, stuckIterMax, constraintIterMax,
    popCntlPeriod, addDescent, iterCallback, rngSeed, progressBinMod,
    threadNum, eipHalfLife;
  double jitter, stepInitial, constraintStepMin, radiusSpace, binWidthSpace,
    radiusScale, alpha, beta, gamma, theta, wall, energyIncreasePermit,
    backStepScale, opporStepScale, energyDecreaseMin, energyDecreasePopCntlMin,
    neighborTrueProb, probeProb, fracNeighNixedMax;

  mop = airMopNew();
  hparm = hestParmNew();
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);

  nPosOut = nrrdNew();
  airMopAdd(mop, nPosOut, (airMopper)nrrdNuke, airMopAlways);

  hparm->respFileEnable = AIR_TRUE;
  me = argv[0];

#ifdef DEFT
  unsigned int baryRes;
  int saveAndQuit, fog;

  hestOptAdd(&hopt, "csqvmm", "min max", airTypeDouble, 2, 2,
             Deft::colorSclQuantityValueMinMax, "nan nan",
             "min/max values for cutting planes of scalar values");
  hestOptAdd(&hopt, "saq", "save & quit", airTypeInt, 0, 0, &saveAndQuit,
             NULL, "save image and quit, for batch processing");
  hestOptAdd(&hopt, "cmap", "nin", airTypeOther, 1, 1, &(bag.ncmap), "",
             "colormap for particles", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "fr", "from point", airTypeFloat, 3, 3, fr, "3 4 5",
             "position of camera, used to determine view vector");
  hestOptAdd(&hopt, "at", "at point", airTypeFloat, 3, 3, at, "0 0 0",
             "camera look-at point, used to determine view vector");
  hestOptAdd(&hopt, "up", "up vector", airTypeFloat, 3, 3, up, "0 0 1",
             "camera pseudo-up vector, used to determine view coordinates");
  hestOptAdd(&hopt, "rh", NULL, airTypeInt, 0, 0, &rght, NULL,
             "normally, use a right-handed UVN frame (V points down), "
             "but in Deft this is always true");
  hestOptAdd(&hopt, "fv", "fov", airTypeFloat, 1, 1, &fovy, "20",
             "vertical field-of-view, in degrees");
  hestOptAdd(&hopt, "or", NULL, airTypeInt, 0, 0, &ortho, NULL,
             "use orthogonal projection instead of perspective");
  hestOptAdd(&hopt, "dn", "near clip", airTypeFloat, 1, 1, &neer, "-2",
             "position of near clipping plane, relative to look-at point");
  hestOptAdd(&hopt, "di", "image", airTypeFloat, 1, 1, &dist, "0.0",
             "normally, distance to image plane, "
             "but in Deft this is always 0.0");
  hestOptAdd(&hopt, "df", "far clip", airTypeFloat, 1, 1, &faar, "2",
             "position of far clipping plane, relative to look-at point");
  hestOptAdd(&hopt, "ar", NULL, airTypeInt, 0, 0, &atrel, NULL,
             "normally: near, image, and far plane distances are relative to "
             "the *at* point, instead of the eye point, "
             "but for Deft, this is always true");
  hestOptAdd(&hopt, "usecam", NULL, airTypeInt, 0, 0, &camkeep, NULL,
             "hack: by default, a camera reset is done to put the volume "
             "in view. Use this to say that the camera specified by the "
             "flags above should be preserved and used");
  hestOptAdd(&hopt, "bg", "R G B", airTypeFloat, 3, 3, bg, "0.2 0.3 0.4",
             "background color");
  hestOptAdd(&hopt, "fog", NULL, airTypeInt, 0, 0, &fog, NULL,
             "hack: turn on fog");
  hestOptAdd(&hopt, "is", "su sv", airTypeInt, 2, 2, imgSize, "640 480",
             "initial window size");

  /* this tensor stuff is here because we're hijacking the tensor glyph
     object for doing the particle display ... */
  hestOptAdd(&hopt, "a", "aniso", airTypeEnum, 1, 1, &aniso, NULL,
             "anisotropy metric to make volume of",
             NULL, tenAniso);
  hestOptAdd(&hopt, "atr", "aniso thresh", airTypeFloat, 1, 1,
             &anisoThresh, "0.85",
             "Glyphs will be drawn only for tensors with anisotropy "
             "greater than this threshold");
  hestOptAdd(&hopt, "atrm", "aniso thresh min", airTypeFloat, 1, 1,
             &anisoThreshMin, "0.4",
             "lower bound on aniso thresh");
  hestOptAdd(&hopt, "g", "glyph shape", airTypeEnum, 1, 1, &glyphType, "sqd",
             "shape of glyph to use for display.  Possibilities "
             "include \"box\", \"sphere\"=\"sph\", \"cylinder\"=\"cyl\", and "
             "\"superquad\"=\"sqd\"", NULL, tenGlyphType);
  hestOptAdd(&hopt, "gsc", "scale", airTypeFloat, 1, 1, &glyphScale,
             "0.25", "over-all glyph size");
  hestOptAdd(&hopt, "htb", "trace", airTypeFloat, 1, 1, &haloTraceBound,
             "1.0", "halo trace bound");
  hestOptAdd(&hopt, "ghw", "hwidth", airTypeFloat, 1, 1, &glyphHaloWidth,
             "0.0", "glyph halo width");
  hestOptAdd(&hopt, "gnp", "npow", airTypeFloat, 1, 1, &glyphNormPow,
             "1.0", "pow() exponent for compressing range of norms");
  hestOptAdd(&hopt, "gep", "epow", airTypeFloat, 1, 1, &glyphEvalPow,
             "1.0", "pow() exponent for compressing single eigenvalues");
  hestOptAdd(&hopt, "br", "barycentric res", airTypeInt, 1, 1, &baryRes, "50",
             "resolution of sampling of tensor shape palette");
  hestOptAdd(&hopt, "gr", "glyph res", airTypeInt, 1, 1, &glyphFacetRes, "7",
             "resolution of polygonalization of glyphs (other than box)");
  hestOptAdd(&hopt, "sh", "sharpness", airTypeFloat, 1, 1, &sqdSharp, "2.5",
             "for superquadric glyphs, how much to sharp edges form as a "
             "function of differences between eigenvalues.  Higher values "
             "mean that edges form more easily");

#endif

  hestOptAdd(&hopt, "int", "int", airTypeEnum, 1, 1, &interType,
             "justr", "inter-particle energy type", NULL, pullInterType);
  hestOptAdd(&hopt, "enr", "spec", airTypeOther, 1, 1, &enspR, "cotan",
             "inter-particle energy, radial component",
             NULL, NULL, pullHestEnergySpec);
  hestOptAdd(&hopt, "ens", "spec", airTypeOther, 1, 1, &enspS, "zero",
             "inter-particle energy, scale component",
             NULL, NULL, pullHestEnergySpec);
  hestOptAdd(&hopt, "enw", "spec", airTypeOther, 1, 1, &enspWin,
             "butter:16,0.8", "windowing to create locality with additive "
             "scale-space interaction (\"-int add\")",
             NULL, NULL, pullHestEnergySpec);
  hestOptAdd(&hopt, "efs", "bool", airTypeBool, 1, 1,
             &energyFromStrength, "false",
             "whether or not strength contributes to particle-image energy");
  hestOptAdd(&hopt, "nave", "bool", airTypeBool, 1, 1,
             &nixAtVolumeEdgeSpace, "false",
             "whether or not to nix points at edge of volume, where gage had "
             "to invent values for kernel support");
  hestOptAdd(&hopt, "cbst", "bool", airTypeBool, 1, 1,
             &constraintBeforeSeedThresh, "false",
             "during initialization, try constraint satisfaction before "
             "testing seedThresh");
  hestOptAdd(&hopt, "noadd", NULL, airTypeBool, 0, 0,
             &noAdd, NULL, "turn off adding during population control");
  hestOptAdd(&hopt, "usa", "bool", airTypeBool, 1, 1,
             &unequalShapesAllow, "false",
             "allow volumes to have different shapes (false is safe as "
             "different volume sizes are often accidental)");
  hestOptAdd(&hopt, "pcet", "bool", airTypeBool, 1, 1, &popCntlEnoughTest,
             "true", "use neighbor-counting \"enough\" heuristic to "
             "bail out of pop cntl");
  hestOptAdd(&hopt, "nobin", NULL, airTypeBool, 0, 0,
             &binSingle, NULL,
             "turn off spatial binning (which prevents multi-threading "
             "from being useful), for debugging or speed-up measurement");
  hestOptAdd(&hopt, "lti", "bool", airTypeBool, 1, 1,
             &liveThresholdOnInit, "true",
             "impose liveThresh on initialization");
  hestOptAdd(&hopt, "por", "bool", airTypeBool, 1, 1,
             &permuteOnRebin, "true",
             "permute points during rebinning");
  hestOptAdd(&hopt, "npcwza", "bool", airTypeBool, 1, 1,
             &noPopCntlWithZeroAlpha, "false",
             "no pop cntl with zero alpha");
  hestOptAdd(&hopt, "ubfgl", "bool", airTypeBool, 1, 1,
             &useBetaForGammaLearn, "false",
             "use beta for gamma learning");
  hestOptAdd(&hopt, "ratb", "bool", airTypeBool, 1, 1,
             &restrictiveAddToBins, "true",
             "be choosy when adding points to bins to avoid overlap");
  hestOptAdd(&hopt, "svec", "vec", airTypeDouble, 3, 3, scaleVec, "0 0 0",
             "if non-zero (length), vector to use for displaying scale "
             "in 3-space");
  hestOptAdd(&hopt, "gssr", "rad", airTypeDouble, 1, 1, &glyphScaleRad, "0.0",
             "if non-zero (length), scaling of scale to cylindrical tensors");
  hestOptAdd(&hopt, "v", "verbosity", airTypeInt, 1, 1, &verbose, "1",
             "verbosity level");
  hestOptAdd(&hopt, "vol", "vol0 vol1", airTypeOther, 1, -1, &vspec, NULL,
             "input volumes, in format <filename>:<kind>:<volname>",
             &vspecNum, NULL, meetHestPullVol);
  hestOptAdd(&hopt, "info", "info0 info1", airTypeOther, 1, -1, &idef, NULL,
             "info definitions, in format "
             "<info>[-c]:<volname>:<item>[:<zero>:<scale>]",
             &idefNum, NULL, meetHestPullInfo);

  hestOptAdd(&hopt, "k00", "kern00", airTypeOther, 1, 1, &k00,
             "cubic:1,0", "kernel for gageKernel00",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k11", "kern11", airTypeOther, 1, 1, &k11,
             "cubicd:1,0", "kernel for gageKernel11",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k22", "kern22", airTypeOther, 1, 1, &k22,
             "cubicdd:1,0", "kernel for gageKernel22",
             NULL, NULL, nrrdHestKernelSpec);

  hestOptAdd(&hopt, "sscp", "path", airTypeString, 1, 1, &cachePathSS, "./",
             "path (without trailing /) for where to read/write "
             "pre-blurred volumes for scale-space");
  hestOptAdd(&hopt, "kssb", "kernel", airTypeOther, 1, 1, &kSSblur,
             "ds:1,5", "blurring kernel, to sample scale space",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "kssr", "kernel", airTypeOther, 1, 1, &kSSrecon,
             "hermite", "kernel for reconstructing from scale space samples",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "nss", "# scl smpls", airTypeUInt, 1, 1,
             &samplesAlongScaleNum, "1",
             "if using \"-ppv\", number of samples along scale axis "
             "for each spatial position");

  hestOptAdd(&hopt, "np", "# points", airTypeUInt, 1, 1,
             &pointNumInitial, "1000",
             "number of points to start in system");
  hestOptAdd(&hopt, "halton", NULL, airTypeInt, 0, 0,
             &useHalton, NULL,
             "use Halton sequence initialization instead of "
             "uniform random");
  hestOptAdd(&hopt, "ppv", "# pnts/vox", airTypeInt, 1, 1,
             &pointPerVoxel, "0",
             "number of points per voxel to start in simulation "
             "(need to have a seed thresh vol, overrides \"-np\")");
  hestOptAdd(&hopt, "ppvzr", "z range", airTypeUInt, 2, 2,
             ppvZRange, "1 0",
             "range of Z slices (1st num < 2nd num) to do ppv in, or, "
             "\"1 0\" for whole volume");
  hestOptAdd(&hopt, "jit", "jitter", airTypeDouble, 1, 1,
             &jitter, "0",
             "amount of jittering to do with ppv");
  hestOptAdd(&hopt, "pi", "npos", airTypeOther, 1, 1, &nPosIn, "",
             "4-by-N array of positions to start at (overrides \"-np\")",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "step", "step", airTypeDouble, 1, 1,
             &stepInitial, "1",
             "initial step size for gradient descent");
  hestOptAdd(&hopt, "csm", "step", airTypeDouble, 1, 1,
             &constraintStepMin, "0.0001",
             "convergence criterion for constraint satisfaction");
  hestOptAdd(&hopt, "snap", "# iters", airTypeUInt, 1, 1,
             &snap, "0",
             "if non-zero, # iters between saved snapshots");
  hestOptAdd(&hopt, "maxi", "# iters", airTypeUInt, 1, 1,
             &iterMax, "0",
             "if non-zero, max # iterations to run whole system");
  hestOptAdd(&hopt, "stim", "# iters", airTypeUInt, 1, 1,
             &stuckIterMax, "5",
             "if non-zero, max # iterations to allow a particle "
             " to be stuck before nixing");
  hestOptAdd(&hopt, "maxci", "# iters", airTypeUInt, 1, 1,
             &constraintIterMax, "15",
             "if non-zero, max # iterations for contraint enforcement");
  hestOptAdd(&hopt, "irad", "scale", airTypeDouble, 1, 1,
             &radiusSpace, "1",
             "particle radius in spatial domain");
  hestOptAdd(&hopt, "srad", "scale", airTypeDouble, 1, 1,
             &radiusScale, "1",
             "particle radius in scale domain");
  hestOptAdd(&hopt, "bws", "bin width", airTypeDouble, 1, 1,
             &binWidthSpace, "1.001",
             "spatial bin width as multiple of spatial radius");
  hestOptAdd(&hopt, "alpha", "alpha", airTypeDouble, 1, 1,
             &alpha, "0.5",
             "blend between particle-image (alpha=0) and "
             "inter-particle (alpha=1) energies");
  hestOptAdd(&hopt, "beta", "beta", airTypeDouble, 1, 1,
             &beta, "1.0",
             "when using Phi2 energy, blend between pure "
             "space repulsion (beta=0) and "
             "scale attraction (beta=1)");
  hestOptAdd(&hopt, "gamma", "gamma", airTypeDouble, 1, 1,
             &gamma, "1.0",
             "scaling factor on energy from strength");
  hestOptAdd(&hopt, "theta", "theta", airTypeDouble, 1, 1,
             &theta, "0.0",
             "slope of increasing livethresh wrt scale");
  hestOptAdd(&hopt, "wall", "k", airTypeDouble, 1, 1,
             &wall, "0.0",
             "spring constant on walls");
  hestOptAdd(&hopt, "eip", "k", airTypeDouble, 1, 1,
             &energyIncreasePermit, "0.0",
             "amount by which its okay for *per-particle* energy to increase "
             "during gradient descent process");
  hestOptAdd(&hopt, "ess", "scl", airTypeDouble, 1, 1,
             &backStepScale, "0.5",
             "when energy goes up instead of down, scale step "
             "size by this");
  hestOptAdd(&hopt, "oss", "scl", airTypeDouble, 1, 1,
             &opporStepScale, "1.0",
             "opportunistic scaling (hopefully up, >1) of step size "
             "on every iteration");
  hestOptAdd(&hopt, "edmin", "frac", airTypeDouble, 1, 1,
             &energyDecreaseMin, "0.0001",
             "convergence threshold: stop when fractional improvement "
             "(decrease) in energy dips below this");
  hestOptAdd(&hopt, "edpcmin", "frac", airTypeDouble, 1, 1,
             &energyDecreasePopCntlMin, "0.01",
             "population control is triggered when energy improvement "
             "goes below this threshold");
  hestOptAdd(&hopt, "fnnm", "frac", airTypeDouble, 1, 1,
             &fracNeighNixedMax, "0.25",
             "don't nix if this fraction (or more) of neighbors "
             "have been nixed");
  hestOptAdd(&hopt, "pcp", "period", airTypeUInt, 1, 1,
             &popCntlPeriod, "20",
             "# iters to wait between attempts at population control");
  hestOptAdd(&hopt, "iad", "# iters", airTypeUInt, 1, 1,
             &addDescent, "10",
             "# iters to run descent on tentative new points during PC");
  hestOptAdd(&hopt, "icb", "# iters", airTypeUInt, 1, 1,
             &iterCallback, "1",
             "periodicity of calling rendering callback");

  hestOptAdd(&hopt, "ac3c", "ac3c", airTypeBool, 1, 1,
             &allowCodimension3Constraints, "false",
             "allow codimensions 3 constraints");
  hestOptAdd(&hopt, "sit", "sit", airTypeBool, 1, 1, &scaleIsTau, "false",
             "scale is tau");
  hestOptAdd(&hopt, "rng", "seed", airTypeUInt, 1, 1,
             &rngSeed, "42",
             "base seed value for RNGs");
  hestOptAdd(&hopt, "pbm", "mod", airTypeUInt, 1, 1,
             &progressBinMod, "50",
             "progress bin mod");
  hestOptAdd(&hopt, "eiphl", "hl", airTypeUInt, 1, 1, &eipHalfLife, "0",
             "half-life of energyIncreasePermute (\"-eip\")");
  hestOptAdd(&hopt, "nt", "# threads", airTypeInt, 1, 1,
             &threadNum, "1",
             (airThreadCapable
              ? "number of threads hoover should use"
              : "if threads where enabled in this Teem build, this is how "
              "you would control the number of threads to use"));
  hestOptAdd(&hopt, "nprob", "prob", airTypeDouble, 1, 1,
             &neighborTrueProb, "1.0",
             "do full neighbor discovery with this probability");
  hestOptAdd(&hopt, "pprob", "prob", airTypeDouble, 1, 1,
             &probeProb, "1.0",
             "probe local image values with this probability");

  hestOptAdd(&hopt, "addlog", "fname", airTypeString, 1, 1, &addLogS, "",
             "name of file in which to log all particle additions");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "filename for saving computed positions");
  hestOptAdd(&hopt, "eob", "base", airTypeString, 1, 1, &extraOutBaseS, "",
             "save extra info (besides position), and use this string as "
             "the base of the filenames.  Not using this means the extra "
             "info is not saved.");

  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

#ifdef DEFT
  if (const char *envS = getenv("DEFT_HOME")) {
    strcpy(Deft::homeDir, envS);
    strcat(Deft::homeDir, "/");
  } else {
    fprintf(stderr, "%s: WARNING: \"DEFT_HOME\" environment variable "
            "not set; assuming \".\"\n", me);
    strcpy(Deft::homeDir, "./");
  }
#endif
  /*
  airEnumPrint(stderr, gageScl);
  exit(0);
  */
  if (airStrlen(addLogS)) {
    if (!(addLog = airFopen(addLogS, stdout, "w"))) {
      fprintf(stderr, "%s: couldn't open %s for writing", me, addLogS);
      airMopError(mop); return 1;
    }
    airMopAdd(mop, addLog, (airMopper)airFclose, airMopAlways);
  } else {
    addLog = NULL;
  }

  pctx = pullContextNew();
  airMopAdd(mop, pctx, (airMopper)pullContextNix, airMopAlways);
  if (pullVerboseSet(pctx, verbose)
      || pullFlagSet(pctx, pullFlagEnergyFromStrength, energyFromStrength)
      || pullFlagSet(pctx, pullFlagNixAtVolumeEdgeSpace, nixAtVolumeEdgeSpace)
      || pullFlagSet(pctx, pullFlagConstraintBeforeSeedThresh,
                     constraintBeforeSeedThresh)
      || pullFlagSet(pctx, pullFlagPopCntlEnoughTest, popCntlEnoughTest)
      || pullFlagSet(pctx, pullFlagBinSingle, binSingle)
      || pullFlagSet(pctx, pullFlagNoAdd, noAdd)
      || pullFlagSet(pctx, pullFlagPermuteOnRebin, permuteOnRebin)
      || pullFlagSet(pctx, pullFlagNoPopCntlWithZeroAlpha,
                     noPopCntlWithZeroAlpha)
      || pullFlagSet(pctx, pullFlagUseBetaForGammaLearn,
                     useBetaForGammaLearn)
      || pullFlagSet(pctx, pullFlagRestrictiveAddToBins,
                     restrictiveAddToBins)
      || pullFlagSet(pctx, pullFlagAllowCodimension3Constraints,
                     allowCodimension3Constraints)
      || pullFlagSet(pctx, pullFlagScaleIsTau, scaleIsTau)
      || pullInitUnequalShapesAllowSet(pctx, unequalShapesAllow)
      || pullIterParmSet(pctx, pullIterParmSnap, snap)
      || pullIterParmSet(pctx, pullIterParmMax, iterMax)
      || pullIterParmSet(pctx, pullIterParmStuckMax, stuckIterMax)
      || pullIterParmSet(pctx, pullIterParmConstraintMax, constraintIterMax)
      || pullIterParmSet(pctx, pullIterParmPopCntlPeriod, popCntlPeriod)
      || pullIterParmSet(pctx, pullIterParmAddDescent, addDescent)
      || pullIterParmSet(pctx, pullIterParmCallback, iterCallback)
      || pullIterParmSet(pctx, pullIterParmEnergyIncreasePermitHalfLife,
                         eipHalfLife)
      || pullSysParmSet(pctx, pullSysParmStepInitial, stepInitial)
      || pullSysParmSet(pctx, pullSysParmConstraintStepMin, constraintStepMin)
      || pullSysParmSet(pctx, pullSysParmRadiusSpace, radiusSpace)
      || pullSysParmSet(pctx, pullSysParmRadiusScale, radiusScale)
      || pullSysParmSet(pctx, pullSysParmBinWidthSpace, binWidthSpace)
      || pullSysParmSet(pctx, pullSysParmAlpha, alpha)
      || pullSysParmSet(pctx, pullSysParmBeta, beta)
      || pullSysParmSet(pctx, pullSysParmGamma, gamma)
      || pullSysParmSet(pctx, pullSysParmTheta, theta)
      || pullSysParmSet(pctx, pullSysParmWall, wall)
      || pullSysParmSet(pctx, pullSysParmEnergyIncreasePermit,
                        energyIncreasePermit)
      || pullSysParmSet(pctx, pullSysParmEnergyDecreaseMin,
                        energyDecreaseMin)
      || pullSysParmSet(pctx, pullSysParmFracNeighNixedMax,
                        fracNeighNixedMax)
      || pullSysParmSet(pctx, pullSysParmEnergyDecreasePopCntlMin,
                        energyDecreasePopCntlMin)
      || pullSysParmSet(pctx, pullSysParmBackStepScale, backStepScale)
      || pullSysParmSet(pctx, pullSysParmOpporStepScale, opporStepScale)
      || pullSysParmSet(pctx, pullSysParmNeighborTrueProb,
                        neighborTrueProb)
      || pullSysParmSet(pctx, pullSysParmProbeProb, probeProb)
      || pullRngSeedSet(pctx, rngSeed)
      || pullProgressBinModSet(pctx, progressBinMod)
      || pullThreadNumSet(pctx, threadNum)
      || pullInterEnergySet(pctx, interType, enspR, enspS, enspWin)
      || pullInitLiveThreshUseSet(pctx, liveThresholdOnInit)
      || pullLogAddSet(pctx, addLog)) {
    airMopAdd(mop, err = biffGetDone(PULL), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble with flags:\n%s", me, err);
    airMopError(mop); return 1;
  }

  if (nPosIn) {
    E = pullInitGivenPosSet(pctx, nPosIn);
  } else if (pointPerVoxel) {
    E = pullInitPointPerVoxelSet(pctx, pointPerVoxel,
                                 ppvZRange[0], ppvZRange[1],
                                 samplesAlongScaleNum, jitter);
  } else if (useHalton) {
    E = pullInitHaltonSet(pctx, pointNumInitial, 0);
  } else {
    E = pullInitRandomSet(pctx, pointNumInitial);
  }
  if (E) {
    airMopAdd(mop, err = biffGetDone(PULL), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble with flags:\n%s", me, err);
    airMopError(mop); return 1;
  }
  if (meetPullVolLoadMulti(vspec, vspecNum, cachePathSS,
                           kSSblur, nrrdBoundaryBleed, AIR_NAN,
                           verbose)
      || meetPullVolAddMulti(pctx, vspec, vspecNum,
                             k00, k11, k22, kSSrecon)
      || meetPullInfoAddMulti(pctx, idef, idefNum)) {
    airMopAdd(mop, err = biffGetDone(MEET), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble with volumes or infos:\n%s", me, err);
    airMopError(mop); return 1;
  }

  if (pullStart(pctx)) {
    airMopAdd(mop, err = biffGetDone(PULL), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble starting system:\n%s", me, err);
    airMopError(mop); return 1;
  }

  /* -------------------------------------------------- */

  /* not sure when this table was created, don't have heart to nix it
   *
   *                 hght scl   tang1    tang2   mode scl  strength
   *  ridge surface:    -1      evec2      -        -       -eval2
   *     ridge line:    -1      evec2    evec1      -       -eval1
   *     all ridges:    -1      evec2    evec1     +1        ??
   * valley surface:    +1      evec0      -        -        eval0
   *    valley line:    +1      evec0    evec1      -        eval1
   *      all lines:    +1      evec0    evec1     -1
   */

  ssrange[0] = FLT_MAX;
  ssrange[1] = -FLT_MAX;
  for (vsi=0; vsi<vspecNum; vsi++) {
    meetPullVol *vol;
    vol = vspec[vsi];
    if (vol->numSS) {
      ssrange[0] = AIR_MIN(ssrange[0], vol->rangeSS[0]);
      ssrange[1] = AIR_MAX(ssrange[1], vol->rangeSS[1]);
    }
  }
  if (pctx->flag.scaleIsTau) {
    ssrange[0] = gageTauOfSig(ssrange[0]);
    ssrange[1] = gageTauOfSig(ssrange[1]);
  }

#ifdef DEFT
  /* -------------------------------------------------- */
  /* initialize bag and its UI */
  bag.pctx = pctx;
  bag.nPosOut = nrrdNew();
  bag.nTenOut = nrrdNew();
  bag.nFrcOut = nrrdNew();
  bag.nposOld = nrrdNew();
  bag.nposNew = nrrdNew();
  bag.nten = nrrdNew();
  bag.ntmp = nrrdNew();
  bag.nenr = nrrdNew();
  bag.nscl = nrrdNew();
  bag.nidcc = nrrdNew();
  bag.ncovar = nrrdNew();
#if PULL_TANCOVAR
  bag.ntcovar = nrrdNew();
#endif
  bag.nstab = nrrdNew();
  bag.nintern = nrrdNew();
  if (pctx->ispec[pullInfoStrength]) {
    printf("!%s: trouble creating strength nrrd\n", me);
    bag.nstrn = nrrdNew();
  } else {
    bag.nstrn = NULL;
  }
  if (pctx->ispec[pullInfoQuality]) {
    printf("!%s: trouble creating quality nrrd\n", me);
    bag.nqual = nrrdNew();
  } else {
    bag.nqual = NULL;
  }
  bag.nstuck = nrrdNew();
  bag.nfrcOld = nrrdNew();
  bag.nfrcNew = nrrdNew();
  bag.nrgb = nrrdNew();
  bag.nccrgb = nrrdNew();
  bag.ncmapOut = nrrdNew();
  bag.nblur = nrrdNew();
  bag.norig = vspec[0]->nin;
  nrrdCopy(bag.nblur, bag.norig);
  bag.rsmc = nrrdResampleContextNew();
  E = 0;
  if (!E) E |= nrrdResampleDefaultCenterSet(bag.rsmc, nrrdDefaultCenter);
  if (!E) E |= nrrdResampleInputSet(bag.rsmc, bag.norig);
  for (unsigned int axi=0; axi<3; axi++) {
    if (!E) E |= nrrdResampleSamplesSet(bag.rsmc, axi,
                                        bag.norig->axis[axi].size);
    if (!E) E |= nrrdResampleRangeFullSet(bag.rsmc, axi);
  }
  if (!E) E |= nrrdResampleBoundarySet(bag.rsmc, nrrdBoundaryBleed);
  if (!E) E |= nrrdResampleTypeOutSet(bag.rsmc, nrrdTypeDefault);
  if (!E) E |= nrrdResampleRenormalizeSet(bag.rsmc, AIR_TRUE);
  if (E) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble setting up resampler:\n%s", me, err);
    airMopError(mop); return 1;
  }
  /* bag.ncval is just a pointer to other nrrds */
  bag.cvalRange = nrrdRangeNew(AIR_NAN, AIR_NAN);
  ELL_3V_COPY(bag.scaleVec, scaleVec);
  bag.glyphScaleRad = glyphScaleRad;

  bag.scene = new Deft::Scene();
  bag.scene->bgColor(bg[0], bg[1], bg[2]);

  int winy = 10;
  int incy;
  fltk::Window *win = new fltk::Window(400, 600, "pull UI");
  win->begin();
  fltk::Button *stepButton = new fltk::Button(10, winy, 50, incy=20, "step");
  stepButton->callback((fltk::Callback*)step_cb, &bag);

  bag.verbose = new fltk::ValueInput(100, winy, 30, 20, "verb");
  bag.verbose->value(pctx->verbose);
  bag.verbose->callback((fltk::Callback*)verbose_cb, &bag);

  bag.iters = new fltk::IntInput(200, winy, 50, 20, "# iters");
  bag.iters->value(1);

  if (ssrange[1] > ssrange[0]) {
    fltk::Button *gamButton = new fltk::Button(260, winy,
                                               50, 20, "gamma");
    gamButton->callback((fltk::Callback*)gammaSet_cb, &bag);
  }
  fltk::Button *ccButton = new fltk::Button(360, winy,
                                            30, 20, "CC");
  ccButton->callback((fltk::Callback*)cc_cb, &bag);

  winy += incy + 5;
  fltk::Button *saveButton = new fltk::Button(10, winy, 50, 20, "save");
  saveButton->callback((fltk::Callback*)save_cb, &bag);

  bag.scaleVecInput[0] = new fltk::FloatInput(120, winy, 35, 20, "scaleVec");
  bag.scaleVecInput[1] = new fltk::FloatInput(160, winy, 35, 20, "");
  bag.scaleVecInput[2] = new fltk::FloatInput(200, winy, 35, 20, "");
  bag.scaleVecInput[0]->value(scaleVec[0]);
  bag.scaleVecInput[1]->value(scaleVec[1]);
  bag.scaleVecInput[2]->value(scaleVec[2]);
  bag.scaleVecInput[0]->callback((fltk::Callback*)scaleGlyph_cb, &bag);
  bag.scaleVecInput[1]->callback((fltk::Callback*)scaleGlyph_cb, &bag);
  bag.scaleVecInput[2]->callback((fltk::Callback*)scaleGlyph_cb, &bag);

  bag.glyphScaleRadInput = new fltk::ValueInput(300, winy, 45, 20, "gssr");
  bag.glyphScaleRadInput->range(0.0, 100.0);
  bag.glyphScaleRadInput->step(0.1);
  bag.glyphScaleRadInput->linesize(0.1);
  bag.glyphScaleRadInput->value(glyphScaleRad);
  bag.glyphScaleRadInput->callback((fltk::Callback*)scaleGlyph_cb, &bag);

  winy += incy;
  bag.alpha = new Deft::Slider(0, winy, win->w(), incy=55, "alpha");
  bag.alpha->align(fltk::ALIGN_LEFT);
  bag.alpha->range(0, 1);
  bag.alpha->value(pctx->sysParm.alpha);
  bag.alpha->fastUpdate(1);
  bag.alpha->callback((fltk::Callback*)alpha_cb, &bag);

  if (pullInterTypeAdditive == pctx->interType) {
    winy += incy;
    bag.beta = new Deft::Slider(0, winy, win->w(), incy=55, "beta");
    bag.beta->align(fltk::ALIGN_LEFT);
    bag.beta->range(0, 1);
    bag.beta->value(pctx->sysParm.beta);
    bag.beta->fastUpdate(1);
    bag.beta->callback((fltk::Callback*)beta_cb, &bag);
  }

  if (pullEnergyCubicWell == pctx->energySpecR->energy
      || pullEnergyBetterCubicWell == pctx->energySpecR->energy) {
    winy += incy;
    bag.cwell = new Deft::Slider(0, winy, win->w(), incy=55, "well depth");
    bag.cwell->align(fltk::ALIGN_LEFT);
    bag.cwell->range(-0.04, 0);
    bag.cwell->value(bag.pctx->energySpecR->parm[1]);
    bag.cwell->fastUpdate(1);
    bag.cwell->callback((fltk::Callback*)cwell_cb, &bag);
    /* remember eip as fraction of well depth */
    bag.energyIncreasePermitFrac =
      energyIncreasePermit/bag.pctx->energySpecR->parm[1];
  } else {
    bag.energyIncreasePermitFrac = AIR_NAN;
  }

  winy += incy;
  bag.gamma = new Deft::Slider(0, winy, win->w(), incy=55, "gamma");
  bag.gamma->align(fltk::ALIGN_LEFT);
  bag.gamma->range(0, 2*pctx->sysParm.gamma);
  bag.gamma->value(pctx->sysParm.gamma);
  bag.gamma->fastUpdate(1);
  bag.gamma->callback((fltk::Callback*)gamma_cb, &bag);

  winy += incy;
  bag.ccSelect = new Deft::Slider(0, winy, win->w(), incy=55, "CC Select");
  bag.ccSelect->align(fltk::ALIGN_LEFT);
  bag.ccSelect->range(0, 0);
  bag.ccSelect->step(1);
  bag.ccSelect->value(0);
  bag.ccSelect->fastUpdate(1);
  bag.ccSelect->callback((fltk::Callback*)ccSelect_cb, &bag);

  bag.ccSingle = new fltk::CheckButton(130, winy+4, 50, 20, "Single");
  bag.ccSingle->value(0);
  bag.ccSingle->callback((fltk::Callback*)ccSelect_cb, &bag);

  winy += incy;
  bag.rho = new Deft::Slider(0, winy, win->w(), incy=55, "rho");
  bag.rho->align(fltk::ALIGN_LEFT);
  bag.rho->range(0, 1.0);
  bag.rho->value(0.5);
  bag.rho->fastUpdate(1);
  bag.rho->callback((fltk::Callback*)cc_cb, &bag);

  if (ssrange[1] > ssrange[0]) {
    winy += incy;
    bag.sclMean = new Deft::Slider(0, winy, win->w(), incy=55, "scale mean");
    bag.sclMean->align(fltk::ALIGN_LEFT);
    bag.sclMean->range(ssrange[0], ssrange[1]);
    bag.sclMean->value((ssrange[0] + ssrange[1])/2);
    bag.sclMean->fastUpdate(1);
    bag.sclMean->callback((fltk::Callback*)scale_cb, &bag);

    fltk::Button *reblurButton = new fltk::Button(130, winy+4, 50, 20, "reblur");
    reblurButton->callback((fltk::Callback*)reblur_cb, &bag);

    winy += incy;
    bag.sclWind = new Deft::Slider(0, winy, win->w(), incy=55, "scale window");
    bag.sclWind->align(fltk::ALIGN_LEFT);
    bag.sclWind->range(0, ssrange[1] - ssrange[0]);
    bag.sclWind->value(ssrange[1] - ssrange[0]);
    bag.sclWind->fastUpdate(1);
    bag.sclWind->callback((fltk::Callback*)scale_cb, &bag);
    scale_cb(NULL, &bag);
  } else {
    bag.sclMin = bag.sclMax = 0;
  }

  if (pctx->ispec[pullInfoStrength]) {
    winy += incy;
    bag.strength = new Deft::Slider(0, winy, win->w(), incy=55, "strength");
    bag.strength->align(fltk::ALIGN_LEFT);
    bag.strength->range(0, 1);
    bag.strength->callback((fltk::Callback*)strength_cb, &bag);
    bag.strength->fastUpdate(1);
    bag.strength->value(0);
  } else {
    bag.strnMin = 0;
  }
  if (pctx->ispec[pullInfoQuality]) {
    winy += incy;
    bag.quality = new Deft::Slider(0, winy, win->w(), incy=55, "quality");
    bag.quality->align(fltk::ALIGN_LEFT);
    bag.quality->range(-0.1, 1);
    bag.quality->callback((fltk::Callback*)quality_cb, &bag);
    bag.quality->fastUpdate(1);
    bag.quality->value(-0.1);
  } else {
    bag.qualMin = 0;
  }

  /*
  winy += incy;
  bag.height = new Deft::Slider(0, winy, win->w(), incy=55, "height");
  bag.height->align(fltk::ALIGN_LEFT);
  bag.height->range(0, 10);
  bag.height->value(10);
  bag.height->fastUpdate(1);
  bag.height->callback((fltk::Callback*)strength_cb, &bag);
  */

  win->end();
  win->show();

  /* -------------------------------------------------- */
  bag.viewer = new Deft::Viewer(bag.scene, imgSize[0], imgSize[1]);
  if (camkeep) {
    bag.viewer->camera(fr[0], fr[1], fr[2],
                       at[0], at[1], at[2],
                       up[0], up[1], up[2],
                       fovy, neer, faar);
  }
  bag.viewer->resizable(bag.viewer);
  bag.viewer->end();
  const char *fakeArgv[2] = {"Deft_pull", NULL};
  bag.viewer->show(1, (char**)fakeArgv);
  if (ortho) {
    /* total hack */
    bag.viewer->keyboard('p', 0, 0);
  }
  /* bag.viewer->helpPrint(); */
  Deft::ViewerUI *viewerUI = new Deft::ViewerUI(bag.viewer);
  viewerUI->show();
  fltk::flush();

  /* -------------------------------------------------- */
  if (gageKindScl == vspec[0]->kind) {
    bag.contour = new Deft::Contour();
    bag.contour->volumeSet(bag.nblur);
    bag.contour->twoSided(true);
    bag.scene->objectAdd(bag.contour);
  } else {
    bag.contour = NULL;
  }

  /* -------------------------------------------------- */
  Deft::TensorGlyph *glyph = new Deft::TensorGlyph();
  glyph->dynamic(true);
  glyph->twoSided(true);
  glyph->anisoType(aniso);
  glyph->anisoThreshMin(anisoThreshMin);
  glyph->anisoThresh(anisoThresh);
  glyph->glyphType(glyphType);
  glyph->superquadSharpness(sqdSharp);
  glyph->glyphResolution(glyphFacetRes);
  if (tenGlyphTypeBetterquad) {
    glyph->barycentricResolution(baryRes);
  } else {
    glyph->barycentricResolution(20);
  }
  glyph->glyphScale(glyphScale);
  glyph->glyphHaloWidth(glyphHaloWidth);
  glyph->haloTraceBound(haloTraceBound);
  glyph->glyphNormPow(glyphNormPow);
  glyph->glyphEvalPow(glyphEvalPow);
  glyph->rgbEvecParmSet(tenAniso_Cl2, 0, 0.7, 1.0, 2.3, 1.0);
  glyph->rgbEvecParmSet(tenAniso_Cl2, 0, 0, 1.0, 1, 0.0);
  glyph->maskThresh(0.5);
  /*
  void rgbParmSet(int aniso, unsigned int evec,
                  double maxSat, double isoGray,
                  double gamma, double modulate);
  */

  bag.scene->objectAdd(glyph);
  bag.glyph = glyph;

  Deft::TensorGlyphUI *glyphUI = new Deft::TensorGlyphUI(bag.glyph,
                                                         bag.viewer);
  glyphUI->show();

  /* -------------------------------------------------- */
  /*
  Deft::TensorGlyph *hedge = new Deft::TensorGlyph();
  hedge->dynamic(true);
  hedge->anisoType(aniso);
  hedge->anisoThreshMin(anisoThreshMin);
  hedge->anisoThresh(anisoThresh);
  hedge->glyphType(glyphType);
  hedge->superquadSharpness(sqdSharp);
  hedge->glyphResolution(glyphFacetRes);
  hedge->glyphScale(glyphScale/4);
  hedge->rgbParmSet(tenAniso_Cl2, 0, 0.7, 1.0, 2.3, 1.0);
  hedge->rgbParmSet(tenAniso_Cl2, 0, 0, 1.0, 1, 0.0);
  hedge->maskThresh(0.0);
  bag.hedge = hedge;
  bag.scene->objectAdd(hedge);
  Deft::TensorGlyphUI *hedgeUI = new Deft::TensorGlyphUI(bag.hedge,
                                                         bag.viewer);
  hedgeUI->show();
  */

  /* -------------------------------------------------- */
  Deft::Volume *vol = new Deft::Volume(vspec[0]->kind, bag.nblur);
  fprintf(stderr, "!%s: vol = %p *********************\n", me, vol);
  Deft::TriPlane *triplane = new Deft::TriPlane(vol);
  /*
  ** HEY: you can eventually segfault if this isn't set here
  ** shouldn't doing so be optional?
  */
  if (gageKindScl == vspec[0]->kind) {
    triplane->alphaMaskQuantity(Deft::alphaMaskSclQuantityValue);
    triplane->alphaMaskThreshold(0);
    triplane->colorQuantity(Deft::colorSclQuantityValue);
  } else if (tenGageKind == vspec[0]->kind) {
    triplane->alphaMaskQuantity(Deft::alphaMaskTenQuantityConfidence);
    triplane->alphaMaskThreshold(0);
    triplane->colorQuantity(Deft::colorTenQuantityRgbLinear);
  }

  NrrdKernelSpec *ksp = nrrdKernelSpecNew();
  double kparm[10];

  kparm[0] = 1.0;
  nrrdKernelSpecSet(ksp, nrrdKernelTent, kparm);
  triplane->kernel(gageKernel00, ksp);
  ELL_3V_SET(kparm, 1, 1, 0);
  nrrdKernelSpecSet(ksp, nrrdKernelBCCubicD, kparm);
  triplane->kernel(gageKernel11, ksp);
  nrrdKernelSpecSet(ksp, nrrdKernelBCCubicDD, kparm);
  triplane->kernel(gageKernel22, ksp);
  triplane->visible(false);

  bag.scene->groupAdd(triplane);

  Deft::TriPlaneUI *planeUI = new Deft::TriPlaneUI(triplane, bag.viewer);
  planeUI->show();

  /* -------------------------------------------------- */

  if (gageKindScl == vspec[0]->kind) {
    fltk::Window *window = new fltk::Window(400, 80, "isovalue");
    window->begin();
    window->resizable(window);

    winy = 0;
    bag.isoval = new Deft::Slider(0, winy, window->w(), incy=55, "isovalue");
    winy += incy;
    bag.isoval->align(fltk::ALIGN_LEFT);
    bag.isoval->range(bag.contour->minimum(), bag.contour->maximum());
    bag.isoval->value(bag.contour->maximum());
    bag.contour->wireframe(false);
    bag.isoval->fastUpdate(0);
    bag.isoval->callback((fltk::Callback*)isovalue_cb, &bag);
    /* bag.isoval->value(1.0); */
    window->end();
    window->show(argc,(char**)argv);
  }

  if (pullPhistEnabled) {
    bag.phistLine = limnPolyDataNew();
    /* bag.phistTube = limnPolyDataNew(); */
    bag.phistSurf = new Deft::PolyData(bag.phistLine, false);
    bag.phistSurf->wireframe(false);
    bag.phistSurf->normalsUse(true);
    bag.phistSurf->colorUse(true);
    bag.phistSurf->visible(true);
    bag.phistSurf->changed();
    bag.scene->objectAdd(bag.phistSurf);
  }

  fltk::flush();
  fltk::redraw();

  /* this is to get "output" prior to any iterations,
     but after seeding and initialization */
  outputGet(&bag);
  outputShow(&bag);

  if (!camkeep) {
    bag.viewer->cameraReset();
  }

  if (0) {
    Nrrd *nplot;
    nplot = nrrdNew();
    if (pullEnergyPlot(pctx, nplot, 1, 0, 0, 601)) {
      airMopAdd(mop, err = biffGetDone(PULL), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble plotting:\n%s", me, err);
      airMopError(mop); return 1;
    }
    nrrdSave("eplot.nrrd", nplot, NULL);
    nplot = nrrdNuke(nplot);
#if PULL_HINTER
    pctx->nhinter = nrrdNew();
#endif
  }

  if (fog) {
    bag.viewer->fog(true);
    bag.viewer->redraw();
    ret = fltk::wait();
  }
  if (saveAndQuit) {
    bag.scene->draw();
    bag.viewer->screenDump();
  } else {

    /* set up callbacks in pull */
    pullCallbackSet(pctx, iter_cb, &bag);

    /* this returns when the user quits */
    ret = fltk::run();
  }

#else
  /* else not running as Deft, but as command-line */
  if (!E) E |= pullRun(pctx);
  if (E) {
    airMopAdd(mop, err = biffGetDone(PULL), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble 3:\n%s", me, err);
    airMopError(mop); return 1;
  }
  if (pullOutputGet(nPosOut, NULL, NULL, NULL, 0.0, pctx)) {
    airMopAdd(mop, err = biffGetDone(PULL), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble 3.1:\n%s", me, err);
    airMopError(mop); return 1;
  }
  nrrdSave(outS, nPosOut, NULL);
  if (airStrlen(extraOutBaseS)) {
    Nrrd *nstrn, *nstab, *nintern;
    char fname[3][AIR_STRLEN_MED];
    nstrn = nrrdNew();
    nstab = nrrdNew();
    nintern = nrrdNew();
    airMopAdd(mop, nstrn, (airMopper)nrrdNuke, airMopAlways);
    airMopAdd(mop, nstab, (airMopper)nrrdNuke, airMopAlways);
    airMopAdd(mop, nintern, (airMopper)nrrdNuke, airMopAlways);
    if (pullInfoGet(nstrn, pullInfoStrength, pctx)
        || pullPropGet(nstab, pullPropStability, pctx)
        || pullPropGet(nintern, pullPropNeighInterNum, pctx)) {
      airMopAdd(mop, err = biffGetDone(PULL), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble 3.2:\n%s", me, err);
      airMopError(mop); return 1;
    }
    sprintf(fname[0], "%s-strn.nrrd", extraOutBaseS);
    sprintf(fname[1], "%s-stab.nrrd", extraOutBaseS);
    sprintf(fname[2], "%s-intern.nrrd", extraOutBaseS);
    if (nrrdSave(fname[0], nstrn, NULL)
        || nrrdSave(fname[1], nstab, NULL)
        || nrrdSave(fname[2], nintern, NULL)) {
      airMopAdd(mop, err = biffGetDone(PULL), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble 3.3:\n%s", me, err);
      airMopError(mop); return 1;
    }
  }
#endif

  pullFinish(pctx);
  airMopOkay(mop);
  return ret;
}
