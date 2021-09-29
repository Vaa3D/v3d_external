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

typedef union {
  void **vd;
  gagePerVolume ***pvl;
} perVolumeUnion;


/*
******** gageContextNew()
**
** doesn't use biff
*/
gageContext *
gageContextNew() {
  gageContext *ctx;
  perVolumeUnion pvu;
  int ii;

  ctx = AIR_CALLOC(1, gageContext);
  if (ctx) {
    ctx->verbose = gageDefVerbose;
    gageParmReset(&ctx->parm);
    for (ii=gageKernelUnknown+1; ii<gageKernelLast; ii++) {
      ctx->ksp[ii] = NULL;
    }
    ctx->pvl = NULL;
    ctx->pvlNum = 0;
    pvu.pvl = &(ctx->pvl);
    ctx->pvlArr = airArrayNew(pvu.vd, &(ctx->pvlNum),
                              sizeof(gagePerVolume*),
                              GAGE_PERVOLUME_ARR_INCR);
    gageKernelReset(ctx); /* placed here for logic of kernel flag */
    ctx->shape = gageShapeNew();
    for (ii=gageCtxFlagUnknown+1; ii<gageCtxFlagLast; ii++) {
      ctx->flag[ii] = AIR_FALSE;
    }
    ctx->stackPos = NULL;
    ctx->stackFsl = NULL;
    ctx->stackFw = NULL;
    for (ii=0; ii<=GAGE_DERIV_MAX; ii++) {
      ctx->needD[ii] = AIR_FALSE;
    }
    for (ii=gageKernelUnknown+1; ii<gageKernelLast; ii++) {
      ctx->needK[ii] = AIR_FALSE;
    }
    ctx->radius = 0;
    ctx->fsl = ctx->fw = NULL;
    ctx->off = NULL;
    gagePointReset(&ctx->point);
    strcpy(ctx->errStr, "");
    ctx->errNum = gageErrNone;
    ctx->edgeFrac = 0;
  }
  return ctx;
}

/*
******** gageContextCopy()
**
** gives you a new context, which behaves the same as the given context,
** with newly allocated pervolumes attached.  With the avoidance of
** padding to create a private copy of the volume, the gageContext is
** light-weight enough that there is no reason that this function can't
** return an independent and fully functioning copy of the context (whereas
** before you weren't allowed to do anything but gageProbe() on the
** copied context).
*/
gageContext * /*Teem: biff if (!ret) */
gageContextCopy(gageContext *ctx) {
  static const char me[]="gageContextCopy";
  gageContext *ntx;
  unsigned int fd, pvlIdx;
  perVolumeUnion pvu;
  int ki;

  ntx = AIR_CALLOC(1, gageContext);
  if (!ntx) {
    biffAddf(GAGE, "%s: couldn't make a gageContext", me);
    return NULL;
  }
  /* we should probably restrict ourselves to gage API calls, but given the
     constant state of gage construction, this seems much simpler.
     Pointers are fixed below */
  memcpy(ntx, ctx, sizeof(gageContext));
  for (ki=gageKernelUnknown+1; ki<gageKernelLast; ki++) {
    ntx->ksp[ki] = nrrdKernelSpecCopy(ctx->ksp[ki]);
  }
  pvu.pvl = &(ntx->pvl);
  ntx->pvlArr = airArrayNew(pvu.vd, &(ntx->pvlNum),
                            sizeof(gagePerVolume*),
                            GAGE_PERVOLUME_ARR_INCR);
  airArrayLenSet(ntx->pvlArr, ctx->pvlNum);
  if (!ntx->pvl) {
    biffAddf(GAGE, "%s: couldn't allocate new pvl array", me);
    return NULL;
  }
  for (pvlIdx=0; pvlIdx<ntx->pvlNum; pvlIdx++) {
    ntx->pvl[pvlIdx] = _gagePerVolumeCopy(ctx->pvl[pvlIdx], 2*ctx->radius);
    if (!ntx->pvl[pvlIdx]) {
      biffAddf(GAGE, "%s: trouble copying pervolume %u", me, pvlIdx);
      return NULL;
    }
  }
  if (ctx->stackPos && ctx->stackFsl && ctx->stackFw) {
    ntx->stackPos = AIR_CALLOC(ctx->pvlNum-1, double);
    ntx->stackFsl = AIR_CALLOC(ctx->pvlNum-1, double);
    ntx->stackFw = AIR_CALLOC(ctx->pvlNum-1, double);
    if (!( ntx->stackPos && ntx->stackFsl && ntx->stackFw )) {
      biffAddf(GAGE, "%s: couldn't allocate stack Pos, Fsl, Fw", me);
      return NULL;
    }
    for (pvlIdx=0; pvlIdx<ntx->pvlNum-1; pvlIdx++) {
      ntx->stackPos[pvlIdx] = ctx->stackPos[pvlIdx];
      ntx->stackFsl[pvlIdx] = ctx->stackFsl[pvlIdx];
      ntx->stackFw[pvlIdx] = ctx->stackFw[pvlIdx];
    }
  } else {
    ntx->stackPos = NULL;
    ntx->stackFsl = NULL;
    ntx->stackFw = NULL;
  }
  ntx->shape = gageShapeCopy(ctx->shape);
  fd = 2*ntx->radius;
  ntx->fsl = AIR_CALLOC(fd*3, double);
  ntx->fw = AIR_CALLOC(fd*3*(GAGE_KERNEL_MAX+1), double);
  ntx->off = AIR_CALLOC(fd*fd*fd, unsigned int);
  if (!( ntx->fsl && ntx->fw && ntx->off )) {
    biffAddf(GAGE, "%s: couldn't allocate new filter caches for fd=%d",
             me, fd);
    return NULL;
  }
  /* the content of the offset array needs to be copied because
     it won't be refilled simply by calls to gageProbe() */
  memcpy(ntx->off, ctx->off, fd*fd*fd*sizeof(unsigned int));

  /* make sure gageProbe() has to refill caches */
  gagePointReset(&ntx->point);

  return ntx;
}

/*
******** gageContextNix()
**
** responsible for freeing and clearing up everything hanging off a
** context so that things can be returned to the way they were prior
** to gageContextNew().
**
** does not use biff
*/
gageContext *
gageContextNix(gageContext *ctx) {
  /* static const char me[]="gageContextNix"; */
  unsigned int pvlIdx;

  if (ctx) {
    gageKernelReset(ctx);
    for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
      gagePerVolumeNix(ctx->pvl[pvlIdx]);
      /* no point in doing a detach, the whole context is going bye-bye */
    }
    airArrayNuke(ctx->pvlArr);
    ctx->shape = gageShapeNix(ctx->shape);
    ctx->stackPos = AIR_CAST(double *, airFree(ctx->stackPos));
    ctx->stackFsl = AIR_CAST(double *, airFree(ctx->stackFsl));
    ctx->stackFw = AIR_CAST(double *, airFree(ctx->stackFw));
    ctx->fw = AIR_CAST(double *, airFree(ctx->fw));
    ctx->fsl = AIR_CAST(double *, airFree(ctx->fsl));
    ctx->off = AIR_CAST(unsigned int *, airFree(ctx->off));
  }
  airFree(ctx);
  return NULL;
}

/*
******** gageKernelSet()
**
** sets one kernel in a gageContext; but the value of this function
** is all the error checking it does.
**
** Refers to ctx->checkIntegrals and acts appropriately.
**
** Does use biff.
*/
int
gageKernelSet(gageContext *ctx,
              int which, const NrrdKernel *k, const double *kparm) {
  static const char me[]="gageKernelSet";
  int numParm;
  double support, integral;

  if (!(ctx && k && kparm)) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(gageKernel, which)) {
    biffAddf(GAGE, "%s: \"which\" (%d) not in range [%d,%d]", me,
             which, gageKernelUnknown+1, gageKernelLast-1);
    return 1;
  }
  if (ctx->verbose) {
    fprintf(stderr, "%s: which = %d -> %s\n", me, which,
            airEnumStr(gageKernel, which));
  }
  numParm = k->numParm;
  if (!(AIR_IN_CL(0, numParm, NRRD_KERNEL_PARMS_NUM))) {
    biffAddf(GAGE, "%s: kernel's numParm (%d) not in range [%d,%d]",
             me, numParm, 0, NRRD_KERNEL_PARMS_NUM);
    return 1;
  }
  support = k->support(kparm);
  if (!( support > 0 )) {
    biffAddf(GAGE, "%s: kernel's support (%g) not > 0", me, support);
    return 1;
  }
  if (ctx->parm.checkIntegrals) {
    integral = k->integral(kparm);
    if (gageKernel00 == which ||
        gageKernel10 == which ||
        gageKernel20 == which ||
        gageKernelStack == which) {
      if (!( integral > 0 )) {
        biffAddf(GAGE, "%s: reconstruction kernel's integral (%g) not > 0.0",
                 me, integral);
        return 1;
      }
    } else {
      /* its a derivative, so integral must be near zero */
      if (!( AIR_ABS(integral) <= ctx->parm.kernelIntegralNearZero )) {
        char str[AIR_STRLEN_LARGE]="";
        nrrdKernelSprint(str, k, kparm);
        biffAddf(GAGE, "%s: derivative %s kernel (%s) integral %g not within "
                 "%g of 0.0", me, airEnumStr(gageKernel, which), str,
                 integral, ctx->parm.kernelIntegralNearZero);
        return 1;
      }
    }
  }

  /* okay, enough enough, go set the kernel */
  if (!ctx->ksp[which]) {
    ctx->ksp[which] = nrrdKernelSpecNew();
  }
  nrrdKernelSpecSet(ctx->ksp[which], k, kparm);
  ctx->flag[gageCtxFlagKernel] = AIR_TRUE;

  return 0;
}

/*
******** gageKernelReset()
**
** reset kernels (including the stack ksp) and parameters.
*/
void
gageKernelReset(gageContext *ctx) {
  /* static const char me[]="gageKernelReset"; */
  int i;

  if (ctx) {
    for(i=gageKernelUnknown+1; i<gageKernelLast; i++) {
      ctx->ksp[i] = nrrdKernelSpecNix(ctx->ksp[i]);
    }
    ctx->flag[gageCtxFlagKernel] = AIR_TRUE;
  }
  return;
}

/*
******** gageParmSet()
**
** for setting the boolean-ish flags in the context in a safe and
** intelligent manner, since changing some of them can have many
** consequences
*/
void
gageParmSet(gageContext *ctx, int which, double val) {
  static const char me[]="gageParmSet";
  unsigned int pvlIdx;

  switch (which) {
  case gageParmVerbose:
    ctx->verbose = AIR_CAST(int, val);
    if (ctx->verbose > 3) {
      fprintf(stderr, "%s(%p): ctx->verbose now %d\n", me,
              AIR_CAST(void *, ctx), ctx->verbose);
    }
    for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
      ctx->pvl[pvlIdx]->verbose = AIR_CAST(int, val);
      if (ctx->pvl[pvlIdx]->verbose > 3) {
        fprintf(stderr, "%s: ctx->pvl[%u]->verbose now %d\n", me, pvlIdx,
                ctx->pvl[pvlIdx]->verbose);
      }
    }
    break;
  case gageParmRenormalize:
    ctx->parm.renormalize = val ? AIR_TRUE : AIR_FALSE;
    /* we have to make sure that any existing filter weights
       are not re-used; because gageUpdage() is not called mid-probing,
       we don't use the flag machinery.  Instead we just invalidate
       the last known fractional probe locations */
    gagePointReset(&ctx->point);
    break;
  case gageParmCheckIntegrals:
    ctx->parm.checkIntegrals = val ? AIR_TRUE : AIR_FALSE;
    /* no flags to set, simply affects future calls to gageKernelSet() */
    break;
  case gageParmK3Pack:
    ctx->parm.k3pack = val ? AIR_TRUE : AIR_FALSE;
    ctx->flag[gageCtxFlagK3Pack] = AIR_TRUE;
    break;
  case gageParmGradMagCurvMin:
    ctx->parm.gradMagCurvMin = val;
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  case gageParmCurvNormalSide:
    ctx->parm.curvNormalSide = AIR_CAST(int, val);
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  case gageParmKernelIntegralNearZero:
    ctx->parm.kernelIntegralNearZero = val;
    /* no flag to set, simply affects future calls to gageKernelSet() */
    break;
  case gageParmDefaultCenter:
    ctx->parm.defaultCenter = AIR_CAST(int, val);
    /* no flag to set, I guess, although the value here effects the
       action of _gageShapeSet when called by gagePerVolumeAttach . . . */
    break;
  case gageParmStackUse:
    ctx->parm.stackUse = AIR_CAST(int, val);
    /* no flag to set, right? simply affects future calls to gageProbe()? */
    /* HEY: no? because if you're turning on the stack behavior, you now
       should be doing the error checking to make sure that all the pvls
       have the same kind!  This should be caught by gageUpdate(), which is
       supposed to be called after changing anything, prior to gageProbe() */
    break;
  case gageParmStackNormalizeRecon:
    ctx->parm.stackNormalizeRecon = AIR_CAST(int, val);
    break;
  case gageParmStackNormalizeDeriv:
    ctx->parm.stackNormalizeDeriv = AIR_CAST(int, val);
    break;
  case gageParmStackNormalizeDerivBias:
    ctx->parm.stackNormalizeDerivBias = val;
    break;
  case gageParmOrientationFromSpacing:
    ctx->parm.orientationFromSpacing = AIR_CAST(int, val);
    /* affects future calls to _gageShapeSet */
    break;
  case gageParmGenerateErrStr:
    ctx->parm.generateErrStr = AIR_CAST(int, val);
    break;
  default:
    fprintf(stderr, "\n%s: sorry, which = %d not valid\n\n", me, which);
    break;
  }
  return;
}

/*
******** gagePerVolumeIsAttached()
**
*/
int
gagePerVolumeIsAttached(const gageContext *ctx, const gagePerVolume *pvl) {
  int ret;
  unsigned int pvlIdx;

  ret = AIR_FALSE;
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    if (pvl == ctx->pvl[pvlIdx]) {
      ret = AIR_TRUE;
    }
  }
  return ret;
}

/*
******** gagePerVolumeAttach()
**
** attaches a pervolume to a context, which actually involves
** very little work
*/
int
gagePerVolumeAttach(gageContext *ctx, gagePerVolume *pvl) {
  static const char me[]="gagePerVolumeAttach";
  gageShape *shape;
  unsigned int newidx;

  if (!( ctx && pvl )) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (gagePerVolumeIsAttached(ctx, pvl)) {
    biffAddf(GAGE, "%s: given pervolume already attached", me);
    return 1;
  }

  if (0 == ctx->pvlNum) {
    /* the volume "shape" is context state that we set now, because unlike
       everything else (handled by gageUpdate()), it does not effect
       the kind or amount of padding done */
    if (_gageShapeSet(ctx, ctx->shape, pvl->nin, pvl->kind->baseDim)) {
      biffAddf(GAGE, "%s: trouble", me);
      return 1;
    }
    ctx->flag[gageCtxFlagShape] = AIR_TRUE;
  } else {
    /* have to check to that new pvl matches first one.  Since all
       attached pvls were at some point the "new" one, they all
       should match each other */
    shape = gageShapeNew();
    if (_gageShapeSet(ctx, shape, pvl->nin, pvl->kind->baseDim)) {
      biffAddf(GAGE, "%s: trouble", me);
      return 1;
    }
    if (!gageShapeEqual(ctx->shape, "existing context", shape, "new volume")) {
      biffAddf(GAGE, "%s: trouble", me);
      gageShapeNix(shape); return 1;
    }
    gageShapeNix(shape);
  }
  /* here we go */
  newidx = airArrayLenIncr(ctx->pvlArr, 1);
  if (!ctx->pvl) {
    biffAddf(GAGE, "%s: couldn't increase length of pvl", me);
    return 1;
  }
  ctx->pvl[newidx] = pvl;
  pvl->verbose = ctx->verbose;

  return 0;
}

/*
******** gagePerVolumeDetach()
**
** detaches a pervolume from a context, but does nothing else
** with the pervolume; caller may want to call gagePerVolumeNix
** if this pervolume will no longer be used
*/
int
gagePerVolumeDetach(gageContext *ctx, gagePerVolume *pvl) {
  static const char me[]="gagePerVolumeDetach";
  unsigned int pvlIdx, foundIdx=0;

  if (!( ctx && pvl )) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (!gagePerVolumeIsAttached(ctx, pvl)) {
    biffAddf(GAGE, "%s: given pervolume not currently attached", me);
    return 1;
  }
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    if (pvl == ctx->pvl[pvlIdx]) {
      foundIdx = pvlIdx;
    }
  }
  for (pvlIdx=foundIdx+1; pvlIdx<ctx->pvlNum; pvlIdx++) {
    ctx->pvl[pvlIdx-1] = ctx->pvl[pvlIdx];
  }
  ctx->pvl[ctx->pvlNum-1] = NULL;
  airArrayLenIncr(ctx->pvlArr, -1);
  if (0 == ctx->pvlNum) {
    /* leave things the way that they started */
    gageShapeReset(ctx->shape);
    ctx->flag[gageCtxFlagShape] = AIR_TRUE;
  }
  return 0;
}

/*
** gageIv3Fill()
**
** based on ctx's shape and radius, and the (xi,yi,zi) determined from
** the probe location, fills the iv3 cache in the given pervolume
**
** This function is a bottleneck for so many things, so forcing off
** the verbose comments seems like one way of trying to speed it up.
** However, temporarily if-def'ing out unused branches of the
** "switch(pvl->kind->valLen)", and #define'ing fddd to 64 (for 4x4x4
** neighborhoods) did not noticeably speed anything up.
**
*/
void
gageIv3Fill(gageContext *ctx, gagePerVolume *pvl) {
  static const char me[]="gageIv3Fill";
  int lx, ly, lz, hx, hy, hz, _xx, _yy, _zz;
  unsigned int xx, yy, zz,
    fr, cacheIdx, dataIdx, fddd;
  unsigned int sx, sy, sz;
  char *data, *here;
  unsigned int tup;

  sx = ctx->shape->size[0];
  sy = ctx->shape->size[1];
  sz = ctx->shape->size[2];
  fr = ctx->radius;
  /* idx[0]-1: see Thu Jan 14 comment in filter.c */
  lx = ctx->point.idx[0]-1 - (fr - 1);
  ly = ctx->point.idx[1]-1 - (fr - 1);
  lz = ctx->point.idx[2]-1 - (fr - 1);
  hx = lx + 2*fr - 1;
  hy = ly + 2*fr - 1;
  hz = lz + 2*fr - 1;
  fddd = 2*fr*2*fr*2*fr;
  if (ctx->verbose > 1) {
    fprintf(stderr, "%s: ___ hello; s %u %u %u; fr %u\n", me,
            sx, sy, sz, fr);
    fprintf(stderr, "%s:     point.idx %u %u %u\n", me,
            ctx->point.idx[0], ctx->point.idx[1], ctx->point.idx[2]);
    fprintf(stderr, "%s:     l %d %d %d; h %d %d %d; fddd %u\n", me,
            lx, ly, lz, hx, hy, hz, fddd);
  }
  data = (char*)pvl->nin->data;
  if (lx >= 0 && ly >= 0 && lz >= 0
      && hx < AIR_CAST(int, sx)
      && hy < AIR_CAST(int, sy)
      && hz < AIR_CAST(int, sz)) {
    /* all the samples we need are inside the existing volume */
    dataIdx = lx + sx*(ly + sy*(lz));
    if (ctx->verbose > 1) {
      fprintf(stderr, "%s:     hello, valLen = %d, pvl->nin = %p, data = %p\n",
              me, pvl->kind->valLen,
              AIR_CVOIDP(pvl->nin), pvl->nin->data);
    }
    here = data + dataIdx*pvl->kind->valLen*nrrdTypeSize[pvl->nin->type];
    if (ctx->verbose > 1) {
      fprintf(stderr, "%s:     size = (%u,%u,%u);\n"
              "%s:     fd = %d; coord = (%u,%u,%u) --> dataIdx = %d\n",
              me, sx, sy, sz, me, 2*fr,
              ctx->point.idx[0], ctx->point.idx[1], ctx->point.idx[2],
              dataIdx);
      fprintf(stderr, "%s:     here = %p; iv3 = %p; off[0,1,2,3,4,5,6,7] = "
              "%d,%d,%d,%d,%d,%d,%d,%d\n",
              me, here, AIR_CAST(void*, pvl->iv3),
              ctx->off[0], ctx->off[1], ctx->off[2], ctx->off[3],
              ctx->off[4], ctx->off[5], ctx->off[6], ctx->off[7]);
    }
    switch(pvl->kind->valLen) {
    case 1:
      for (cacheIdx=0; cacheIdx<fddd; cacheIdx++) {
        pvl->iv3[cacheIdx] = pvl->lup(here, ctx->off[cacheIdx]);
      }
      break;
      /* NOTE: the tuple axis is being shifted from the fastest to
         the slowest axis, to anticipate component-wise filtering
         operations */
    case 3:
      for (cacheIdx=0; cacheIdx<fddd; cacheIdx++) {
        pvl->iv3[cacheIdx + fddd*0] = pvl->lup(here, 0 + 3*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*1] = pvl->lup(here, 1 + 3*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*2] = pvl->lup(here, 2 + 3*ctx->off[cacheIdx]);
      }
      break;
    case 7:
      /* this might come in handy for tenGage . . . */
      for (cacheIdx=0; cacheIdx<fddd; cacheIdx++) {
        pvl->iv3[cacheIdx + fddd*0] = pvl->lup(here, 0 + 7*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*1] = pvl->lup(here, 1 + 7*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*2] = pvl->lup(here, 2 + 7*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*3] = pvl->lup(here, 3 + 7*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*4] = pvl->lup(here, 4 + 7*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*5] = pvl->lup(here, 5 + 7*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*6] = pvl->lup(here, 6 + 7*ctx->off[cacheIdx]);
      }
      break;
    default:
      for (cacheIdx=0; cacheIdx<fddd; cacheIdx++) {
        for (tup=0; tup<pvl->kind->valLen; tup++) {
          pvl->iv3[cacheIdx + fddd*tup] =
            pvl->lup(here, tup + pvl->kind->valLen*ctx->off[cacheIdx]);
        }
      }
      break;
    }
    ctx->edgeFrac = 0;
  } else {
    unsigned int edgeNum;
    /* the query requires samples which don't actually lie
       within the volume- more care has to be taken */
    cacheIdx = 0;
    edgeNum = 0;
    for (_zz=lz; _zz<=hz; _zz++) {
      zz = AIR_CLAMP(0, _zz, AIR_CAST(int, sz-1));
      for (_yy=ly; _yy<=hy; _yy++) {
        yy = AIR_CLAMP(0, _yy, AIR_CAST(int, sy-1));
        for (_xx=lx; _xx<=hx; _xx++) {
          xx = AIR_CLAMP(0, _xx, AIR_CAST(int, sx-1));
          edgeNum += ((AIR_CAST(int, zz) != _zz)
                      || (AIR_CAST(int, yy) != _yy)
                      || (AIR_CAST(int, xx) != _xx));
          dataIdx = xx + sx*(yy + sy*zz);
          here = data + dataIdx*pvl->kind->valLen*nrrdTypeSize[pvl->nin->type];
          if (ctx->verbose > 2) {
            fprintf(stderr, "%s:    (%d,%d,%d) --clamp--> (%u,%u,%u)\n", me,
                    _xx, _yy, _zz, xx, yy, zz);
            fprintf(stderr, "       --> dataIdx = %d; data = %p -> here = %p\n",
                    dataIdx, data, here);
          }
          for (tup=0; tup<pvl->kind->valLen; tup++) {
            pvl->iv3[cacheIdx + fddd*tup] = pvl->lup(here, tup);
            if (ctx->verbose > 3) {
              fprintf(stderr, "%s:    iv3[%u + %u*%u=%u] = %g\n", me,
                      cacheIdx, fddd, tup, cacheIdx + fddd*tup,
                      pvl->iv3[cacheIdx + fddd*tup]);
            }
          }
          cacheIdx++;
        }
      }
    }
    ctx->edgeFrac = AIR_CAST(double, edgeNum)/fddd;
  }
  if (ctx->verbose > 1) {
    fprintf(stderr, "%s: ^^^ bye\n", me);
  }
  return;
}

/*
** _gageProbe
**
** how to do probing.  (x,y,z) position is *index space* position.
** Note, however, that derivatives (gradients and hessians) will
** effectively be computed in *world space*.
**
** doesn't actually do much more than call callbacks in the gageKind
** structs of the attached pervolumes
**
** NOTE: the stack filter weights are (like the spatial filter weights)
** computed inside _gageLocationSet()
*/
int
_gageProbe(gageContext *ctx, double _xi, double _yi, double _zi, double _si) {
  static const char me[]="_gageProbe";
  unsigned int oldIdx[4], oldNnz=0, pvlIdx;
  int idxChanged;

  if (!ctx) {
    return 1;
  }
  if (ctx->verbose > 3) {
    fprintf(stderr, "%s: hello(%g,%g,%g,%g) _____________ \n", me,
            _xi, _yi, _zi, _si);
  }
  ELL_4V_COPY(oldIdx, ctx->point.idx);
  oldNnz = ctx->point.stackFwNonZeroNum;
  if (_gageLocationSet(ctx, _xi, _yi, _zi, _si)) {
    /* we're outside the volume; leave ctx->errNum and ctx->errStr set;
       as they have just been set by _gageLocationSet() */
    return 1;
  }

  /* if necessary, refill the iv3 cache */
  idxChanged = (oldIdx[0] != ctx->point.idx[0]
                || oldIdx[1] != ctx->point.idx[1]
                || oldIdx[2] != ctx->point.idx[2]);
  if (ctx->parm.stackUse) {
    idxChanged |= oldIdx[3] != ctx->point.idx[3];
    /* this is subtle (and the source of a difficult bug): even if
       point.idx[3] has not changed, you can still have a change in
       which of the stackFw[] are non-zero, which in turn determines
       which iv3s have to be refilled.  For example, changing _si
       from 0.0 to 0.1, using tent or hermite reconstruction, will
       newly require pvl[1]'s iv3 to be refilled.  To catch this kind
       of situation, we could keep a list of which iv3s are active and
       look for changes in that, or, we could just look for changes in
       point.idx[3] AND changes in stackFwNonZeroNum */
    idxChanged |= oldNnz != ctx->point.stackFwNonZeroNum;
  }
  if (ctx->verbose > 3) {
    fprintf(stderr, "%s: oldIdx %u %u %u %u, point.idx %u %u %u %u --> %d\n",
            me, oldIdx[0], oldIdx[1], oldIdx[2], oldIdx[3],
            ctx->point.idx[0], ctx->point.idx[1],
            ctx->point.idx[2], ctx->point.idx[3], idxChanged);
  }
  if (idxChanged) {
    if (!ctx->parm.stackUse) {
      for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
        if (ctx->verbose > 3) {
          fprintf(stderr, "%s: gageIv3Fill(pvl[%u/%u] %s): .......\n", me,
                  pvlIdx, ctx->pvlNum, ctx->pvl[pvlIdx]->kind->name);
        }
        gageIv3Fill(ctx, ctx->pvl[pvlIdx]);
      }
    } else {
      for (pvlIdx=0; pvlIdx<ctx->pvlNum-1; pvlIdx++) {
        /* note that we only fill the cache for the stack samples that
           have a non-zero weight. HEY, however, it would be nice to
           only refill the iv3 that we have to, based on the change in
           scale, instead of refilling all of them in the support of
           the stack recon */
        if (ctx->stackFw[pvlIdx]) {
          if (ctx->verbose > 3) {
            fprintf(stderr, "%s: stackFw[%u] == %g -> iv3fill needed\n", me,
                    pvlIdx, ctx->stackFw[pvlIdx]);
          }
          gageIv3Fill(ctx, ctx->pvl[pvlIdx]);
        } else {
          if (ctx->verbose > 3) {
            fprintf(stderr, "%s: stackFw[%u] == %g -> NO iv3fill\n", me,
                    pvlIdx, ctx->stackFw[pvlIdx]);
          }
        }
      }
    }
  }
  if (ctx->parm.stackUse) {
    unsigned int baseIdx, vi;
    baseIdx = ctx->pvlNum - 1;
    if (ctx->verbose > 3) {
      for (vi=0; vi<baseIdx; vi++) {
        fprintf(stderr, "%s: (stack) pvl[%u]'s value cache at "
                "coords = %u,%u,%u:\n", me, vi,
                ctx->point.idx[0], ctx->point.idx[1], ctx->point.idx[2]);
        ctx->pvl[vi]->kind->iv3Print(stderr, ctx, ctx->pvl[vi]);
      }
    }
    _gageStackBaseIv3Fill(ctx);
    if (ctx->verbose > 3) {
      fprintf(stderr, "%s: (stack) base pvl's value cache at "
              "coords = %u,%u,%u:\n", me,
              ctx->point.idx[0], ctx->point.idx[1], ctx->point.idx[2]);
      ctx->pvl[baseIdx]->kind->iv3Print(stderr, ctx, ctx->pvl[baseIdx]);
    }
    ctx->pvl[baseIdx]->kind->filter(ctx, ctx->pvl[baseIdx]);
    ctx->pvl[baseIdx]->kind->answer(ctx, ctx->pvl[baseIdx]);
  } else {
    for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
      if (ctx->verbose > 3) {
        fprintf(stderr, "%s: pvl[%u/%u %s]'s value cache at "
                "coords = %u,%u,%u:\n", me, pvlIdx, ctx->pvlNum,
                ctx->pvl[pvlIdx]->kind->name,
                ctx->point.idx[0], ctx->point.idx[1], ctx->point.idx[2]);
        ctx->pvl[pvlIdx]->kind->iv3Print(stderr, ctx, ctx->pvl[pvlIdx]);
      }
      ctx->pvl[pvlIdx]->kind->filter(ctx, ctx->pvl[pvlIdx]);
      ctx->pvl[pvlIdx]->kind->answer(ctx, ctx->pvl[pvlIdx]);
    }
  }

  if (ctx->verbose > 3) {
    fprintf(stderr, "%s: bye ^^^^^^^^^^^^^ \n\n", me);
  }
  return 0;
}

/*
******** gageProbe()
**
** bummer: the non-stack gageProbe function is now just a wrapper
** around the stack-based gageProbe
*/
int
gageProbe(gageContext *ctx, double xi, double yi, double zi) {

  return _gageProbe(ctx, xi, yi, zi, 0.0);
}

int
_gageProbeSpace(gageContext *ctx, double xx, double yy, double zz, double ss,
               int indexSpace, int clamp) {
  static const char me[]="_gageProbeSpace";
  unsigned int *size;
  double xi, yi, zi, si;

  if (ctx->verbose > 3) {
    fprintf(stderr, "%s: hi; pos=(%g,%g,%g,%g) in %s space %s clamping\n", me,
            xx, yy, zz, ss, indexSpace ? "index" : "world",
            clamp ? "WITH" : "w/out");
  }
  size = ctx->shape->size;
  if (indexSpace) {
    xi = xx;
    yi = yy;
    zi = zz;
    if (ctx->parm.stackUse) {
      si = ss;
    } else {
      si = 0;
    }
    if (ctx->verbose > 3) {
      fprintf(stderr, "%s: staying at ipos (%g,%g,%g)\n", me,
              xi, yi, zi);
    }
  } else {
    /* have to convert from world to index.  NOTE: the [4]s here are
       for homogeneous coordinates, not for anything stack-related */
    double icoord[4]; double wcoord[4];
    ELL_4V_SET(wcoord, xx, yy, zz, 1);
    ELL_4MV_MUL(icoord, ctx->shape->WtoI, wcoord);
    ELL_4V_HOMOG(icoord, icoord);
    xi = icoord[0];
    yi = icoord[1];
    zi = icoord[2];
    if (ctx->parm.stackUse) {
      unsigned int sidx;
      /* HEY: this is a stupid linear search! */
      if (ss < ctx->stackPos[0]) {
        /* we'll extrapolate from stackPos[0] and [1]. um, actually the
         extrapolation is overkill because we're either going to clamp
         out-of-range scale index positions, or they'll cause a
         gageErrStackBounds error downstream */
        sidx = 0;
      } else if (ss > ctx->stackPos[ctx->pvlNum-2]) {
        /* extrapolate from stackPos[ctx->pvlNum-3] and [ctx->pvlNum-2];
           gageStackPerVolumeAttach ensures that we there are at least
           two blurrings pvls and one base pvl */
        sidx = ctx->pvlNum-3;
      } else {
        for (sidx=0; sidx<ctx->pvlNum-2; sidx++) {
          if (AIR_IN_CL(ctx->stackPos[sidx], ss, ctx->stackPos[sidx+1])) {
            break;
          }
        }
        if (sidx == ctx->pvlNum-2) {
          if (ctx->parm.generateErrStr) {
            sprintf(ctx->errStr, "%s: search failure for ss = %g", me, ss);
          } else {
            strcpy(ctx->errStr, _GAGE_NON_ERR_STR);
          }
          ctx->errNum = gageErrStackSearch;
          return 1;
        }
      }
      si = AIR_AFFINE(ctx->stackPos[sidx], ss, ctx->stackPos[sidx+1],
                      sidx, sidx+1);
      if (ctx->verbose > 3) {
        fprintf(stderr, "%s: si = affine(%g, %g, %g  -> %u %u) = %g\n", me,
                ctx->stackPos[sidx], ss, ctx->stackPos[sidx+1],
                sidx, sidx+1, si);
      }
    } else {
      si = 0;
    }
    if (ctx->verbose > 3) {
      fprintf(stderr, "%s: wpos (%g,%g,%g) --> ipos (%g,%g,%g)\n", me,
              xx, yy, zz, xi, yi, zi);
    }
  }
  if (clamp) {
    if (nrrdCenterNode == ctx->shape->center) {
      xi = AIR_CLAMP(0, xi, size[0]-1);
      yi = AIR_CLAMP(0, yi, size[1]-1);
      zi = AIR_CLAMP(0, zi, size[2]-1);
    } else {
      xi = AIR_CLAMP(-0.5, xi, size[0]-0.5);
      yi = AIR_CLAMP(-0.5, yi, size[1]-0.5);
      zi = AIR_CLAMP(-0.5, zi, size[2]-0.5);
    }
    if (ctx->parm.stackUse) {
      si = AIR_CLAMP(0, si, ctx->pvlNum-2);
    }
    if (ctx->verbose > 3) {
      fprintf(stderr, "%s: with clamping --> ipos (%g,%g,%g)\n", me,
              xi, yi, zi);
    }
  }
  return _gageProbe(ctx, xi, yi, zi, si);
}

int
gageProbeSpace(gageContext *ctx, double xx, double yy, double zz,
               int indexSpace, int clamp) {

  return _gageProbeSpace(ctx, xx, yy, zz, AIR_NAN, indexSpace, clamp);
}
