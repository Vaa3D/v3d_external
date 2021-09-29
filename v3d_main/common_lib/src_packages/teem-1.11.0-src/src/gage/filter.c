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

/*
** sets the filter sample location (fsl) array based on fractional
** probe location ctx->point->frac
**
** One possible rare surpise: if a filter is not continuous with 0
** at the end of its support, and if the sample location is at the
** highest possible point (xi == N-2, xf = 1.0), then the filter
** weights may not be the desired ones.  Forward differencing (via
** nrrdKernelForwDiff) is a good example of this.
*/
void
_gageFslSet(gageContext *ctx) {
  int fr, i;
  double *fslx, *fsly, *fslz;
  double xf, yf, zf;

  fr = ctx->radius;
  fslx = ctx->fsl + 0*2*fr;
  fsly = ctx->fsl + 1*2*fr;
  fslz = ctx->fsl + 2*2*fr;
  xf = ctx->point.frac[0];
  yf = ctx->point.frac[1];
  zf = ctx->point.frac[2];
  switch (fr) {
  case 1:
    fslx[0] = xf; fslx[1] = xf-1;
    fsly[0] = yf; fsly[1] = yf-1;
    fslz[0] = zf; fslz[1] = zf-1;
    break;
  case 2:
    fslx[0] = xf+1; fslx[1] = xf; fslx[2] = xf-1; fslx[3] = xf-2;
    fsly[0] = yf+1; fsly[1] = yf; fsly[2] = yf-1; fsly[3] = yf-2;
    fslz[0] = zf+1; fslz[1] = zf; fslz[2] = zf-1; fslz[3] = zf-2;
    break;
  default:
    /* filter radius bigger than 2 */
    for (i=-fr+1; i<=fr; i++) {
      fslx[i+fr-1] = xf-i;
      fsly[i+fr-1] = yf-i;
      fslz[i+fr-1] = zf-i;
    }
    break;
  }
  return;
}

/*
** renormalize weights of a reconstruction kernel with
** constraint: the sum of the weights must equal the continuous
** integral of the kernel
*/
void
_gageFwValueRenormalize(gageContext *ctx, int wch) {
  double integral, sumX, sumY, sumZ, *fwX, *fwY, *fwZ;
  int i, fd;

  fd = 2*ctx->radius;
  fwX = ctx->fw + 0 + fd*(0 + 3*wch);
  fwY = ctx->fw + 0 + fd*(1 + 3*wch);
  fwZ = ctx->fw + 0 + fd*(2 + 3*wch);
  integral = ctx->ksp[wch]->kernel->integral(ctx->ksp[wch]->parm);
  sumX = sumY = sumZ = 0;
  for (i=0; i<fd; i++) {
    sumX += fwX[i];
    sumY += fwY[i];
    sumZ += fwZ[i];
  }
  for (i=0; i<fd; i++) {
    fwX[i] *= integral/sumX;
    fwY[i] *= integral/sumY;
    fwZ[i] *= integral/sumZ;
  }
  return;
}

/*
** renormalize weights of a derivative kernel with
** constraint: the sum of the weights must be zero, but
** sign of individual weights must be preserved
*/
void
_gageFwDerivRenormalize(gageContext *ctx, int wch) {
  char me[]="_gageFwDerivRenormalize";
  double negX, negY, negZ, posX, posY, posZ, fixX, fixY, fixZ,
    *fwX, *fwY, *fwZ;
  int i, fd;

  fd = 2*ctx->radius;
  fwX = ctx->fw + 0 + fd*(0 + 3*wch);
  fwY = ctx->fw + 0 + fd*(1 + 3*wch);
  fwZ = ctx->fw + 0 + fd*(2 + 3*wch);
  negX = negY = negZ = 0;
  posX = posY = posZ = 0;
  for (i=0; i<fd; i++) {
    if (fwX[i] <= 0) { negX += -fwX[i]; } else { posX += fwX[i]; }
    if (fwY[i] <= 0) { negY += -fwY[i]; } else { posY += fwY[i]; }
    if (fwZ[i] <= 0) { negZ += -fwZ[i]; } else { posZ += fwZ[i]; }
  }
  /* fix is the sqrt() of factor by which the positive values
     are too big.  negative values are scaled up by fix;
     positive values are scaled down by fix */
  fixX = sqrt(posX/negX);
  fixY = sqrt(posY/negY);
  fixZ = sqrt(posZ/negZ);
  if (ctx->verbose > 2) {
    fprintf(stderr, "%s: fixX = % 10.4f, fixY = % 10.4f, fixX = % 10.4f\n",
            me, (float)fixX, (float)fixY, (float)fixZ);
  }
  for (i=0; i<fd; i++) {
    if (fwX[i] <= 0) { fwX[i] *= fixX; } else { fwX[i] /= fixX; }
    if (fwY[i] <= 0) { fwY[i] *= fixY; } else { fwY[i] /= fixY; }
    if (fwZ[i] <= 0) { fwZ[i] *= fixZ; } else { fwZ[i] /= fixZ; }
  }
  return;
}

void
_gageFwSet(gageContext *ctx, unsigned int sidx, double sfrac) {
  char me[]="_gageFwSet";
  int kidx;
  unsigned int fd;

  fd = 2*ctx->radius;
  for (kidx=gageKernelUnknown+1; kidx<gageKernelLast; kidx++) {
    if (!ctx->needK[kidx] || kidx==gageKernelStack) {
      continue;
    }
    /* we evaluate weights for all three axes with one call */
    ctx->ksp[kidx]->kernel->evalN_d(ctx->fw + fd*3*kidx, ctx->fsl,
                                    fd*3, ctx->ksp[kidx]->parm);
  }

  if (ctx->verbose > 2) {
    fprintf(stderr, "%s: filter weights after kernel evaluation:\n", me);
    _gagePrint_fslw(stderr, ctx);
  }
  if (ctx->parm.renormalize) {
    for (kidx=gageKernelUnknown+1; kidx<gageKernelLast; kidx++) {
      if (!ctx->needK[kidx] || kidx==gageKernelStack) {
        continue;
      }
      switch (kidx) {
      case gageKernel00:
      case gageKernel10:
      case gageKernel20:
        _gageFwValueRenormalize(ctx, kidx);
        break;
      default:
        _gageFwDerivRenormalize(ctx, kidx);
        break;
      }
    }
    if (ctx->verbose > 2) {
      fprintf(stderr, "%s: filter weights after renormalization:\n", me);
      _gagePrint_fslw(stderr, ctx);
    }
  }

  if (ctx->parm.stackUse && ctx->parm.stackNormalizeDeriv) {
    unsigned int jj;
    double scl, norm, *fwX, *fwY, *fwZ;

    scl = AIR_AFFINE(0.0, sfrac, 1.0,
                     ctx->stackPos[sidx],
                     ctx->stackPos[sidx+1]);
#if 0
    double (*dgeval)(double x, const double *parm),
      dgparm[2] = {0, 3};
    dgeval = nrrdKernelDiscreteGaussian->eval1_d;
    dgparm[0] = scl;
    /* from Eq. (120) in T. Lindeberg. "Feature Detection with Automatic
       Scale Selection." International Journal of Computer Vision,
       1998, 30, 77-116 */
    /* 0.7978845608 ~= sqrt(2)/sqrt(pi) */
    norm = 0.7978845608/(dgeval(0.0, dgparm) + dgeval(1.0, dgparm));
#endif
    /* really simple; no lindeberg normalization, possible bias */
    norm = scl + ctx->parm.stackNormalizeDerivBias;

    fd = 2*ctx->radius;
    kidx = gageKernel11;
    fwX = ctx->fw + 0 + fd*(0 + 3*kidx);
    fwY = ctx->fw + 0 + fd*(1 + 3*kidx);
    fwZ = ctx->fw + 0 + fd*(2 + 3*kidx);
    for (jj=0; jj<fd; jj++) {
      fwX[jj] *= norm;
      fwY[jj] *= norm;
      fwZ[jj] *= norm;
    }
    kidx = gageKernel22;
    fwX = ctx->fw + 0 + fd*(0 + 3*kidx);
    fwY = ctx->fw + 0 + fd*(1 + 3*kidx);
    fwZ = ctx->fw + 0 + fd*(2 + 3*kidx);
    for (jj=0; jj<fd; jj++) {
      fwX[jj] *= norm*norm;
      fwY[jj] *= norm*norm;
      fwZ[jj] *= norm*norm;
    }
  }

  return;
}

/*
** _gageLocationSet
**
** updates probe location in general context, and things which
** depend on it:
** fsl, fw
**
** (_xi,_yi,_zi) is *index* space position in the volume
** _si is the index-space position in the stack, the value is ignored
** if there is no stack behavior
**
** does NOT use biff, but returns 1 on error and 0 if all okay
** Currently only error is probing outside volume, which sets
** ctx->errNum and sprints message into ctx->errStr.
*/
int
_gageLocationSet(gageContext *ctx,
                 double xif, double yif, double zif, double sif) {
  char me[]="_gageProbeLocationSet";
  unsigned int top[3],  /* "top" x, y, z: highest valid index in volume */
    idx[4];
  int sdiff;      /* computed integral positions in volume */
  double frac[4], min, max[3];

  /* **** bounds checking **** */
  top[0] = ctx->shape->size[0] - 1;
  top[1] = ctx->shape->size[1] - 1;
  top[2] = ctx->shape->size[2] - 1;
  if (nrrdCenterNode == ctx->shape->center) {
    min = 0;
    max[0] = top[0];
    max[1] = top[1];
    max[2] = top[2];
  } else {
    min = -0.5;
    max[0] = AIR_CAST(double, top[0]) + 0.5;
    max[1] = AIR_CAST(double, top[1]) + 0.5;
    max[2] = AIR_CAST(double, top[2]) + 0.5;
  }
  if (!( AIR_IN_CL(min, xif, max[0]) &&
         AIR_IN_CL(min, yif, max[1]) &&
         AIR_IN_CL(min, zif, max[2]) )) {
    if (ctx->parm.generateErrStr) {
      sprintf(ctx->errStr, "%s: position (%g,%g,%g) outside (%s-centered) "
              "bounds [%g,%g]x[%g,%g]x[%g,%g]",
              me, xif, yif, zif,
              airEnumStr(nrrdCenter, ctx->shape->center),
              min, max[0], min, max[1], min, max[2]);
    } else {
      strcpy(ctx->errStr, _GAGE_NON_ERR_STR);
    }
    ctx->errNum = gageErrBoundsSpace;
    return 1;
  }
  if (ctx->parm.stackUse) {
    if (!( AIR_IN_CL(0, sif, ctx->pvlNum-2) )) {
      if (ctx->parm.generateErrStr) {
        sprintf(ctx->errStr, "%s: stack position %g outside (%s-centered) "
                "bounds [0,%u]", me, sif,
                airEnumStr(nrrdCenter, nrrdCenterNode), ctx->pvlNum-2);
      } else {
        strcpy(ctx->errStr, _GAGE_NON_ERR_STR);
      }
      ctx->errNum = gageErrBoundsStack;
      return 1;
    }
  }

  /* **** computing integral and fractional sample locations **** */
  /* Thu Jan 14 19:46:53 CST 2010: detected that along the low edge
     (next to sample 0) in cell centered, the rounding behavior of
     AIR_CAST(unsigned int, xif), namely [-0.5,0] --> 0, meant that
     the low edge was not treated symmetrically with the high edge.
     This motivated the change from using idx to store the lower
     corner of the containing voxel, to the upper corner.  So, the new
     "idx" is always +1 of what the old idx was.  Code here and in
     ctx.c (since idx is saved into ctx->point.idx) has been changed
     accordingly */
  ELL_3V_SET(idx,
             AIR_CAST(unsigned int, xif+1), /* +1: see above */
             AIR_CAST(unsigned int, yif+1),
             AIR_CAST(unsigned int, zif+1));
  if (ctx->verbose > 5) {
    fprintf(stderr, "%s: (%g,%g,%g,%g) -%s-> mm [%g, %g/%g/%g]\n"
            "        --> idx %u %u %u\n",
            me, xif, yif, zif, sif,
            airEnumStr(nrrdCenter, ctx->shape->center),
            min, max[0], max[1], max[2], idx[0], idx[1], idx[2]);
  }
  /* these can only can kick in for node-centered, because that's when
     max[] has an integral value */
  idx[0] -= (idx[0]-1 == max[0]);
  idx[1] -= (idx[1]-1 == max[1]);
  idx[2] -= (idx[2]-1 == max[2]);
  if (ctx->verbose > 5) {
    fprintf(stderr, "%s:        ----> idx %u %u %u\n",
            me, idx[0], idx[1], idx[2]);
  }
  ELL_3V_SET(frac,
             xif - (AIR_CAST(float, idx[0])-1),
             yif - (AIR_CAST(float, idx[1])-1),
             zif - (AIR_CAST(float, idx[2])-1));
  ELL_3V_COPY(ctx->point.idx, idx);  /* not idx[3], yet */
  if (ctx->parm.stackUse) {
    idx[3] = AIR_CAST(unsigned int, sif);
    idx[3] -= (idx[3] == ctx->pvlNum-2);
    frac[3] = sif - idx[3];
    sdiff = (ctx->point.idx[3] + ctx->point.frac[3] != sif);
  } else {
    idx[3] = 0;
    frac[3] = 0;
    sdiff = AIR_FALSE;
  }
  if (ctx->verbose > 2) {
    fprintf(stderr, "%s: \n"
            "        pos (% 15.7f,% 15.7f,% 15.7f,% 15.7f) \n"
            "        -> i(%5d,%5d,%5d,%5d) \n"
            "         + f(% 15.7f,% 15.7f,% 15.7f,% 15.7f) \n",
            me, xif, yif, zif, sif, idx[0], idx[1], idx[2], idx[3],
            frac[0], frac[1], frac[2], frac[3]);
  }

  /* **** compute *spatial* fsl and fw ****
     these have to be reconsidered if anything changes about the
     fractional spatial position, or (if no fractional spatial change),
     movement along scale AND using normalization based on scale */
  if ( ctx->point.frac[0] != frac[0]
       || ctx->point.frac[1] != frac[1]
       || ctx->point.frac[2] != frac[2]
       || (ctx->parm.stackUse && sdiff && ctx->parm.stackNormalizeDeriv)) {
    /* We don't yet record the scale position in ctx->point because
       that's done below while setting stackFsl and stackFw. So, have
       to pass stack pos info to _gageFwSet() */
    ELL_3V_COPY(ctx->point.frac, frac);
    /* these may take some time (especially if using renormalization),
       hence the conditional above */
    _gageFslSet(ctx);
    _gageFwSet(ctx, idx[3], frac[3]);
  }

  /* **** compute *stack* fsl and fw ****  */
  if (ctx->verbose > 2 && ctx->parm.stackUse) {
    fprintf(stderr, "%s: point.frac[3] %f + idx[3] %u = %f %s sif %f\n", me,
            ctx->point.frac[3], ctx->point.idx[3],
            ctx->point.frac[3] + ctx->point.idx[3],
            (sdiff ? "*NOT ==*" : "=="), sif);
  }
  if (!ctx->parm.stackUse) {
    ctx->point.idx[3] = idx[3];
    ctx->point.frac[3] = frac[3];
    ctx->point.stackFwNonZeroNum = 0;
  } else if (sdiff) {
    double sum;
    unsigned int ii, nnz;
    NrrdKernelSpec *sksp;

    /* node-centered sampling of stack indices from 0 to ctx->pvlNum-2 */
    /* HEY: we really are quite far from implementing arbitrary
       nrrdBoundary behaviors here, and centering is stuck on node! */
    /* HEY: honestly, the whole idea that it still makes sense to do
       low-level operations in index space, when the world-space locations
       of the samples can be non-uniform, is a little suspect.  This is
       all legit for nrrdKernelTent and nrrdKernelHermiteScaleSpaceFlag,
       but is pretty fishy otherwise */
    for (ii=0; ii<ctx->pvlNum-1; ii++) {
      ctx->stackFsl[ii] = sif - ii;
      if (ctx->verbose > 2) {
        fprintf(stderr, "%s: ctx->stackFsl[%u] = %g\n",
                me, ii, ctx->stackFsl[ii]);
      }
    }
    sksp = ctx->ksp[gageKernelStack];
    sksp->kernel->evalN_d(ctx->stackFw, ctx->stackFsl,
                          ctx->pvlNum-1, sksp->parm);
    if (ctx->verbose > 2) {
      for (ii=0; ii<ctx->pvlNum-1; ii++) {
        fprintf(stderr, "%s: ctx->stackFw[%u] = %g\n",
                me, ii, ctx->stackFw[ii]);
      }
    }
    /* compute stackFwNonZeroNum whether or not parm.stackNormalizeRecon! */
    nnz = 0;
    if (ctx->parm.stackNormalizeRecon) {
      sum = 0;
      for (ii=0; ii<ctx->pvlNum-1; ii++) {
        nnz += !!ctx->stackFw[ii];
        sum += ctx->stackFw[ii];
      }
      if (!sum) {
        if (ctx->parm.generateErrStr) {
          sprintf(ctx->errStr, "%s: integral of stackFw[] is zero; "
                  "can't do stack reconstruction", me);
        } else {
          strcpy(ctx->errStr, _GAGE_NON_ERR_STR);
        }
        ctx->errNum = gageErrStackIntegral;
        return 1;
      }
      for (ii=0; ii<ctx->pvlNum-1; ii++) {
        ctx->stackFw[ii] /= sum;
      }
      if (ctx->verbose > 2) {
        for (ii=0; ii<ctx->pvlNum-1; ii++) {
          fprintf(stderr, "%s: ctx->stackFw[%u] = %g\n", me,
                  ii, ctx->stackFw[ii]);
        }
      }
    } else {
      for (ii=0; ii<ctx->pvlNum-1; ii++) {
        nnz += !!ctx->stackFw[ii];
      }
      if (!nnz) {
        if (ctx->parm.generateErrStr) {
          sprintf(ctx->errStr, "%s: all stackFw[] weights are zero; "
                  "can't do stack reconstruction", me);
        } else {
          strcpy(ctx->errStr, _GAGE_NON_ERR_STR);
        }
        ctx->errNum = gageErrStackIntegral;
        return 1;
      }
    }

    ctx->point.idx[3] = idx[3];
    ctx->point.frac[3] = frac[3];
    ctx->point.stackFwNonZeroNum = nnz;
  }

  return 0;
}
