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

#include "gage.h"
#include "privateGage.h"

int
gageDeconvolve(Nrrd *_nout, double *lastDiffP,
               const Nrrd *nin, const gageKind *kind,
               const NrrdKernelSpec *ksp, int typeOut,
               unsigned int maxIter, int saveAnyway,
               double step, double epsilon, int verbose) {
  static const char me[]="gageDeconvolve";
  gageContext *ctx[2];
  gagePerVolume *pvl[2];
  double *out[2], *val[2], alpha, (*lup)(const void *, size_t), meandiff=0;
  const double *ans[2];
  Nrrd *nout[2];
  airArray *mop;
  unsigned int sx, sy, sz, xi, yi, zi, anslen, thiz=0, last, inIdx, iter;
  int E, valItem;

  if (!(_nout && lastDiffP && nin && kind && ksp)) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(nrrdTypeDefault == typeOut
        || !airEnumValCheck(nrrdType, typeOut))) {
    biffAddf(GAGE, "%s: typeOut %d not valid", me, typeOut);
    return 1;
  }
  if (!( maxIter >= 1 )) {
    biffAddf(GAGE, "%s: need maxIter >= 1 (not %u)", me, maxIter);
    return 1;
  }
  if (!( epsilon >= 0 )) {
    biffAddf(GAGE, "%s: need epsilon >= 0.0 (not %g)", me, epsilon);
    return 1;
  }

  /* this once changed from 0 to 1, but is unlikely to change again */
  valItem = 1;

  mop = airMopNew();
  for (iter=0; iter<2; iter++) {
    nout[iter] = nrrdNew();
    airMopAdd(mop, nout[iter], (airMopper)nrrdNuke, airMopAlways);
    if (nrrdConvert(nout[iter], nin, nrrdTypeDouble)) {
      biffMovef(GAGE, NRRD, "%s: couldn't allocate working buffer %u",
                me, iter);
      airMopError(mop); return 1;
    }
    ctx[iter] = gageContextNew();
    airMopAdd(mop, ctx[iter], (airMopper)gageContextNix, airMopAlways);
    E = 0;
    if (!E) E |= !(pvl[iter] = gagePerVolumeNew(ctx[iter], nout[iter], kind));
    if (!E) E |= gagePerVolumeAttach(ctx[iter], pvl[iter]);
    if (!E) E |= gageKernelSet(ctx[iter], gageKernel00,
                               ksp->kernel, ksp->parm);
    if (!E) E |= gageQueryItemOn(ctx[iter], pvl[iter], valItem);
    if (!E) E |= gageUpdate(ctx[iter]);
    if (E) {
      biffAddf(GAGE, "%s: trouble setting up context %u", me, iter);
      airMopError(mop); return 1;
    }
    out[iter] = AIR_CAST(double*, nout[iter]->data);
    ans[iter] = gageAnswerPointer(ctx[iter], pvl[iter], valItem);
  }

  anslen = kind->table[valItem].answerLength;
  lup = nrrdDLookup[nin->type];

  alpha = ksp->kernel->eval1_d(0.0, ksp->parm);
  sx = ctx[0]->shape->size[0];
  sy = ctx[0]->shape->size[1];
  sz = ctx[0]->shape->size[2];

  for (iter=0; iter<maxIter; iter++) {
    thiz = (iter+1) % 2;
    last = (iter+0) % 2;
    val[thiz] = out[thiz];
    val[last] = out[last];
    inIdx = 0;
    meandiff = 0;
    for (zi=0; zi<sz; zi++) {
      for (yi=0; yi<sy; yi++) {
        for (xi=0; xi<sx; xi++) {
          unsigned int ai;
          double in, aa;
          gageProbe(ctx[last], xi, yi, zi);
          for (ai=0; ai<anslen; ai++) {
            in = lup(nin->data, ai + anslen*inIdx);
            aa = ans[last][ai];
            val[thiz][ai] = val[last][ai] + step*(in - aa)/alpha;
            meandiff += 2*(in - aa)*(in - aa)/(DBL_EPSILON + in*in + aa*aa);
          }
          val[thiz] += anslen;
          val[last] += anslen;
          ++inIdx;
        }
      }
    }
    meandiff /= sx*sy*sz;
    if (verbose) {
      fprintf(stderr, "%s: iter %u meandiff = %g\n", me, iter, meandiff);
    }
    if (meandiff < epsilon) {
      /* we have indeed converged while iter < maxIter */
      break;
    }
  }
  if (iter == maxIter) {
    if (!saveAnyway) {
      biffAddf(GAGE, "%s: failed to converge in %u iterations, meandiff = %g",
               me, maxIter, meandiff);
      airMopError(mop); return 1;
    } else {
      if (verbose) {
        fprintf(stderr, "%s: at maxIter %u; err %g still > thresh %g\n", me,
                iter, meandiff, epsilon);
      }
    }
  }

  if (nrrdClampConvert(_nout, nout[thiz], (nrrdTypeDefault == typeOut
                                           ? nin->type
                                           : typeOut))) {
    biffAddf(GAGE, "%s: couldn't create output", me);
    airMopError(mop); return 1;
  }
  *lastDiffP = meandiff;

  airMopOkay(mop);
  return 0;
}

/*
*******************************
** all the following functionality should at some point be
** pushed down to nrrd . . .
*/

static void
deconvLine(double *line, size_t llen, const NrrdKernelSpec *ksp) {

  /* Add as many other parameters to this as you want,
     like number and location of poles, or whatever other
     buffers you think you need (they should be passed,
     not allocated and freed on a per-line basis) */

  /* comment these out when there is a real function body */
  AIR_UNUSED(line);
  AIR_UNUSED(llen);
  AIR_UNUSED(ksp);

  return;
}

static int
deconvTrivial(const NrrdKernelSpec *ksp) {
  int ret;

  /* HEY this will be much easier once kernels have a way of
     advertising whether or not they interpolate */
  if (1 == ksp->parm[0] &&
      (ksp->kernel == nrrdKernelHann ||
       ksp->kernel == nrrdKernelBlackman ||
       ksp->kernel == nrrdKernelBox ||
       ksp->kernel == nrrdKernelCheap ||
       ksp->kernel == nrrdKernelTent)) {
    ret = AIR_TRUE;
  } else {
    ret = AIR_FALSE;
  }
  return ret;
}

int
gageDeconvolveSeparableKnown(const NrrdKernelSpec *ksp) {
  int ret;

  if (!ksp) {
    ret = 0;
  } else if (deconvTrivial(ksp)
             || nrrdKernelBSpline3 == ksp->kernel
             || nrrdKernelBSpline5 == ksp->kernel) {
    ret = 1;
  } else {
    ret = 0;
  }
  return ret;
}

int
gageDeconvolveSeparable(Nrrd *nout, const Nrrd *nin,
                        const gageKind *kind,
                        const NrrdKernelSpec *ksp,
                        int typeOut) {
  static const char me[]="gageDeconvolveSeparable";
  double *line, (*lup)(const void *, size_t),
    (*ins)(void *, size_t, double);
  airArray *mop;
  size_t lineLen, sx, sy, sz, idx, ii, jj;
  unsigned int vi, valLen;

  if (!(nout && nin && kind && ksp)) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(nrrdTypeDefault == typeOut
        || !airEnumValCheck(nrrdType, typeOut))) {
    biffAddf(GAGE, "%s: typeOut %d not valid", me, typeOut);
    return 1;
  }
  if (!gageDeconvolveSeparableKnown(ksp)) {
    biffAddf(GAGE, "%s: separable deconv not known for %s kernel",
             me, ksp->kernel->name);
    return 1;
  }
  if (gageKindVolumeCheck(kind, nin)) {
    biffAddf(GAGE, "%s: given volume doesn't fit %s kind",
             me, kind->name);
    return 1;
  }
  if (nrrdTypeDefault == typeOut
      ? nrrdCopy(nout, nin)
      : nrrdConvert(nout, nin, typeOut)) {
    biffMovef(GAGE, NRRD, "%s: problem allocating output", me);
    return 1;
  }
  if (deconvTrivial(ksp)) {
    /* if there's no real work for the deconvolution, then by
       copying the values we're already done; bye */
    return 0;
  }

  valLen = kind->valLen;
  sx = nin->axis[kind->baseDim + 0].size;
  sy = nin->axis[kind->baseDim + 1].size;
  sz = nin->axis[kind->baseDim + 2].size;
  lineLen = sx;
  lineLen = AIR_MAX(lineLen, sy);
  lineLen = AIR_MAX(lineLen, sz);
  lup = nrrdDLookup[nin->type];
  ins = nrrdDInsert[nout->type];

  mop = airMopNew();
  line = AIR_CALLOC(lineLen*valLen, double);
  airMopAdd(mop, line, airFree, airMopAlways);

  /* process along X scanlines */
  for (jj=0; jj<sy*sz; jj++) {
    /* xi = 0, yi = jj%sy, zi = jj/sy
       ==> xi + sx*(yi + sy*zi)
       == 0 + sx*(jj%sy + sy*(jj/sy)) == 0 + sx*jj */
    idx = 0 + valLen*(0 + sx*jj);
    for (ii=0; ii<sx; ii++) {
      for (vi=0; vi<valLen; vi++) {
        line[ii + sx*vi] = lup(nin->data, idx + vi + valLen*ii);
      }
    }
    for (vi=0; vi<valLen; vi++) {
      deconvLine(line + sx*vi, sx, ksp);
    }
    for (ii=0; ii<sx; ii++) {
      for (vi=0; vi<valLen; vi++) {
        ins(nout->data, idx + vi + valLen*ii, line[ii + sx*vi]);
      }
    }
  }

  /* process along Y scanlines */
  for (jj=0; jj<sx*sz; jj++) {
    /* xi = jj%sx, yi = 0, zi = jj/sx
       ==> xi + sx*(yi + sy*zi)
       == jj%sx + sx*(0 + sy*jj/sx) */
    idx = 0 + valLen*((jj%sx) + sx*(0 + sy*(jj/sx)));
    for (ii=0; ii<sy; ii++) {
      for (vi=0; vi<valLen; vi++) {
        line[ii + sy*vi] = lup(nin->data, idx + vi + valLen*sx*ii);
      }
    }
    for (vi=0; vi<valLen; vi++) {
      deconvLine(line + sy*vi, sy, ksp);
    }
    for (ii=0; ii<sx; ii++) {
      for (vi=0; vi<valLen; vi++) {
        ins(nout->data, idx + vi + valLen*sx*ii, line[ii + sy*vi]);
      }
    }
  }

  /* process along Z scanlines */
  for (jj=0; jj<sx*sy; jj++) {
    /* xi = jj%sx, yi = jj/sx, zi = 0
       ==> xi + sx*(yi + sy*zi)
       == jj%sx + sx*(jj/sx + sy*0)
       == jj%sx + sx*(jj/sx) == jj */
    idx = 0 + valLen*jj;
    for (ii=0; ii<sz; ii++) {
      for (vi=0; vi<valLen; vi++) {
        line[ii + sz*vi] = lup(nin->data, idx + vi + valLen*sx*sy*ii);
      }
    }
    for (vi=0; vi<valLen; vi++) {
      deconvLine(line + sz*vi, sz, ksp);
    }
    for (ii=0; ii<sx; ii++) {
      for (vi=0; vi<valLen; vi++) {
        ins(nout->data, idx + vi + valLen*sx*sy*ii, line[ii + sz*vi]);
      }
    }
  }

  airMopOkay(mop);
  return 0;
}
