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

#include "hoover.h"

hooverContext *
hooverContextNew() {
  hooverContext *ctx;

  ctx = (hooverContext *)calloc(1, sizeof(hooverContext));
  if (ctx) {
    ctx->cam = limnCameraNew();
    ELL_3V_SET(ctx->volSize, 0, 0, 0);
    ELL_3V_SET(ctx->volSpacing, AIR_NAN, AIR_NAN, AIR_NAN);
    ctx->volCentering = hooverDefVolCentering;
    ctx->shape = NULL;
    ctx->imgSize[0] = ctx->imgSize[1] = 0;
    ctx->imgCentering = hooverDefImgCentering;
    ctx->user = NULL;
    ctx->numThreads = 1;
    ctx->workIdx = 0;
    ctx->workMutex = NULL;
    ctx->renderBegin = hooverStubRenderBegin;
    ctx->threadBegin = hooverStubThreadBegin;
    ctx->rayBegin = hooverStubRayBegin;
    ctx->sample = hooverStubSample;
    ctx->rayEnd = hooverStubRayEnd;
    ctx->threadEnd = hooverStubThreadEnd;
    ctx->renderEnd = hooverStubRenderEnd;
  }
  return(ctx);
}

int
hooverContextCheck(hooverContext *ctx) {
  static const char me[]="hooverContextCheck";
  int sxe, sye, sze, minSize, centr;

  if (!ctx) {
    biffAddf(HOOVER, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(nrrdCenter, ctx->imgCentering)) {
    biffAddf(HOOVER, "%s: pixel centering (%d) invalid",
             me, ctx->imgCentering);
    return 1;
  }
  centr = (ctx->shape ? ctx->shape->center : ctx->volCentering);
  if (airEnumValCheck(nrrdCenter, centr)) {
    biffAddf(HOOVER, "%s: voxel centering (%d) invalid", me, centr);
    return 1;
  }
  if (limnCameraAspectSet(ctx->cam,
                          ctx->imgSize[0], ctx->imgSize[1], ctx->imgCentering)
      || limnCameraUpdate(ctx->cam)) {
    biffMovef(HOOVER, LIMN, "%s: trouble setting up camera", me);
    return 1;
  }
  if (ctx->shape) {
    if (!ELL_4M_EXISTS(ctx->shape->ItoW)) {
      biffAddf(HOOVER, "%s: given shape doesn't seem to be set", me);
      return 1;
    }
  } else {
    minSize = (nrrdCenterCell == centr ? 1 : 2);
    if (!(ctx->volSize[0] >= minSize
          && ctx->volSize[1] >= minSize
          && ctx->volSize[2] >= minSize)) {
      biffAddf(HOOVER, "%s: volume dimensions (%dx%dx%d) too small", me,
               ctx->volSize[0], ctx->volSize[1], ctx->volSize[2]);
      return 1;
    }
    sxe = AIR_EXISTS(ctx->volSpacing[0]);
    sye = AIR_EXISTS(ctx->volSpacing[1]);
    sze = AIR_EXISTS(ctx->volSpacing[2]);
    if (!sxe && !sye && !sze) {
      /* none of the incoming spacings existed, we'll go out on a limb
         and assume unit spacing */
      ctx->volSpacing[0] = nrrdDefaultSpacing;
      ctx->volSpacing[1] = ctx->volSpacing[2] = ctx->volSpacing[0];
      fprintf(stderr, "%s: WARNING: assuming spacing %g for all axes\n",
              me, ctx->volSpacing[0]);
      /* HEY : nrrdDefaultSpacing need not be the same as gageParm's
         defaultSpacing, but we don't know anything about gage here,
         so what else can we do? */
    } else if (sxe && sye && sze) {
      /* all existed */
      if (!(ctx->volSpacing[0] > 0.0
            && ctx->volSpacing[1] > 0.0
            && ctx->volSpacing[2] > 0.0)) {
        biffAddf(HOOVER, "%s: volume spacing (%gx%gx%g) invalid", me,
                 ctx->volSpacing[0], ctx->volSpacing[1], ctx->volSpacing[2]);
        return 1;
      }
    } else {
      /* some existed, some didn't */
      biffAddf(HOOVER, "%s: spacings %g, %g, %g don't all exist or not", me,
               ctx->volSpacing[0], ctx->volSpacing[1], ctx->volSpacing[2]);
      return 1;
    }
  }
  if (!(ctx->imgSize[0] > 0 && ctx->imgSize[1] > 0)) {
    biffAddf(HOOVER, "%s: image dimensions (%dx%d) invalid", me,
             ctx->imgSize[0], ctx->imgSize[1]);
    return 1;
  }
  if (!(ctx->numThreads >= 1)) {
    biffAddf(HOOVER, "%s: number threads (%d) invalid", me, ctx->numThreads);
    return 1;
  }
  if (!(ctx->numThreads <= HOOVER_THREAD_MAX)) {
    biffAddf(HOOVER, "%s: sorry, number threads (%d) > max (%d)", me,
             ctx->numThreads, HOOVER_THREAD_MAX);
    return 1;
  }
  if (!ctx->renderBegin) {
    biffAddf(HOOVER, "%s: need a non-NULL begin rendering callback", me);
    return 1;
  }
  if (!ctx->rayBegin) {
    biffAddf(HOOVER, "%s: need a non-NULL begin ray callback", me);
    return 1;
  }
  if (!ctx->threadBegin) {
    biffAddf(HOOVER, "%s: need a non-NULL begin thread callback", me);
    return 1;
  }
  if (!ctx->sample) {
    biffAddf(HOOVER, "%s: need a non-NULL sampler callback function", me);
    return 1;
  }
  if (!ctx->rayEnd) {
    biffAddf(HOOVER, "%s: need a non-NULL end ray callback", me);
    return 1;
  }
  if (!ctx->threadEnd) {
    biffAddf(HOOVER, "%s: need a non-NULL end thread callback", me);
    return 1;
  }
  if (!ctx->renderEnd) {
    biffAddf(HOOVER, "%s: need a non-NULL end render callback", me);
    return 1;
  }

  return 0;
}

void
hooverContextNix(hooverContext *ctx) {

  if (ctx) {
    limnCameraNix(ctx->cam);
    /* workMutex is cleaned up at end of render */
    free(ctx);
  }
}

