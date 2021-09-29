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

void
gageShapeReset(gageShape *shape) {

  /* NOTE this guards against NULL */
  if (shape) {
    /* input */
    shape->defaultCenter = gageDefDefaultCenter;
    shape->orientationFromSpacing = gageDefOrientationFromSpacing;
    /* output; setting this in ways that can be used to test
       whether the shape has been set */
    shape->center = nrrdCenterUnknown;
    shape->fromOrientation = AIR_FALSE;
    ELL_3V_SET(shape->size, 0, 0, 0);
    ELL_3V_SET(shape->spacing, AIR_NAN, AIR_NAN, AIR_NAN);
    ELL_4M_NAN_SET(shape->ItoW);
    ELL_4M_NAN_SET(shape->WtoI);
    ELL_3M_NAN_SET(shape->ItoWSubInvTransp);
    ELL_3M_NAN_SET(shape->ItoWSubInv);
  }
  return;
}

static void *
_mopShapeReset(void *_shape) {
  gageShape *shape;

  shape = AIR_CAST(gageShape *, _shape);
  gageShapeReset(shape);
  return NULL;
}

gageShape *
gageShapeNew() {
  gageShape *shape;

  shape = (gageShape *)calloc(1, sizeof(gageShape));
  if (shape) {
    gageShapeReset(shape);
  }
  return shape;
}

gageShape *
gageShapeCopy(const gageShape *shp) {
  gageShape *nhp;

  if ((nhp = gageShapeNew())) {
    /* no pointers, so this is easy */
    memcpy(nhp, shp, sizeof(gageShape));
  }
  return nhp;
}

gageShape *
gageShapeNix(gageShape *shape) {

  airFree(shape);
  return NULL;
}

static void
shapeUnitItoW(const gageShape *shape, double world[3],
              const double indx[3], const double volHalfLen[3]) {
  unsigned int i;

  if (nrrdCenterNode == shape->center) {
    for (i=0; i<=2; i++) {
      world[i] = NRRD_NODE_POS(-volHalfLen[i], volHalfLen[i],
                               shape->size[i], indx[i]);
    }
  } else {
    for (i=0; i<=2; i++) {
      world[i] = NRRD_CELL_POS(-volHalfLen[i], volHalfLen[i],
                               shape->size[i], indx[i]);
    }
  }
}

/*
** _gageShapeSet
**
** we are serving two masters here.  If ctx is non-NULL, we are being called
** from within gage, and we are to be lax or strict according to the settings
** of ctx->parm.requireAllSpacings and ctx->parm.requireEqualCenters.  If
** ctx is NULL, gageShapeSet was called, in which case we go with lax
** behavior (nothing "required")
**
** This function has subsumed the contents of the old gageVolumeCheck,
** and hence has become this weird beast- part error checker and part
** (gageShape) initializer.  Oh well...
*/
int
_gageShapeSet(const gageContext *ctx, gageShape *shape,
              const Nrrd *nin, unsigned int baseDim) {
  static const char me[]="_gageShapeSet";
  int ai, cx, cy, cz, statCalc[3], status, ofspc;
  unsigned int minsize;
  const NrrdAxisInfo *ax[3];
  double vecA[4], vecB[3], vecC[3], vecD[4], matA[9],
    spcCalc[3], vecCalc[3][NRRD_SPACE_DIM_MAX], orig[NRRD_SPACE_DIM_MAX];
  airArray *mop;

  /*
  fprintf(stderr, "!%s: ctx = %p (%s, %s)\n", me, ctx,
          (ctx
           ? (ctx->shape->fromOrientation
              ? "YES from orient"
              : "not from orient")
           : "???"),
          (ctx
           ? (ctx->parm.orientationFromSpacing
              ? "YES ofs"
              : "not ofs")
           : "???"));
  */
  /* ------ basic error checking */
  mop = airMopNew();
  airMopAdd(mop, shape, _mopShapeReset, airMopOnError);
  if (!( shape && nin )) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    airMopError(mop); return 1;
  }
  if (nrrdCheck(nin)) {
    biffMovef(GAGE, NRRD, "%s: basic nrrd validity check failed", me);
    airMopError(mop); return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    biffAddf(GAGE, "%s: need a non-block type nrrd", me);
    airMopError(mop); return 1;
  }
  if (!(nin->dim == 3 + baseDim)) {
    biffAddf(GAGE, "%s: nrrd should be %u-D, not %u-D",
             me, 3 + baseDim, nin->dim);
    airMopError(mop); return 1;
  }
  ax[0] = &(nin->axis[baseDim+0]);
  ax[1] = &(nin->axis[baseDim+1]);
  ax[2] = &(nin->axis[baseDim+2]);

  statCalc[0] = nrrdSpacingCalculate(nin, baseDim + 0,
                                     spcCalc + 0, vecCalc[0]);
  statCalc[1] = nrrdSpacingCalculate(nin, baseDim + 1,
                                     spcCalc + 1, vecCalc[1]);
  statCalc[2] = nrrdSpacingCalculate(nin, baseDim + 2,
                                     spcCalc + 2, vecCalc[2]);
  /* see if nrrdSpacingCalculate ever *failed* */
  if (nrrdSpacingStatusUnknown == statCalc[0]
      || nrrdSpacingStatusUnknown == statCalc[1]
      || nrrdSpacingStatusUnknown == statCalc[2]) {
    biffAddf(GAGE, "%s: nrrdSpacingCalculate trouble on axis %d, %d, or %d",
             me, baseDim + 0, baseDim + 1, baseDim + 2);
    airMopError(mop); return 1;
  }
  if (!( statCalc[0] == statCalc[1] && statCalc[1] == statCalc[2] )) {
    biffAddf(GAGE, "%s: inconsistent spacing information on axes "
             "%u (%s), %u (%s), and %u (%s)", me,
             baseDim + 0, airEnumDesc(nrrdSpacingStatus, statCalc[0]),
             baseDim + 1, airEnumDesc(nrrdSpacingStatus, statCalc[1]),
             baseDim + 2, airEnumDesc(nrrdSpacingStatus, statCalc[2]));
    airMopError(mop); return 1;
  }
  /* this simplifies reasoning in the code that follows */
  status = statCalc[0];
  /* zero spacing would be problematic */
  if (0 == spcCalc[0] && 0 == spcCalc[1] && 0 == spcCalc[2]) {
    biffAddf(GAGE, "%s: spacings (%g,%g,%g) for axes %d,%d,%d not all "
             "non-zero", me, spcCalc[1], spcCalc[1], spcCalc[2],
             baseDim+0, baseDim+1, baseDim+2);
    airMopError(mop); return 1;
  }

  /* error checking based on status */
  if (nrrdSpacingStatusScalarWithSpace == status) {
    biffAddf(GAGE, "%s: sorry, can't handle per-axis spacing that isn't part "
             "of a surrounding world space (%s)",
             me, airEnumStr(nrrdSpacingStatus, status));
    airMopError(mop); return 1;
  }
  /* we no longer allow a nrrd to come in with no spacing info at all */
  if (nrrdSpacingStatusNone == status) {
    biffAddf(GAGE, "%s: sorry, need some spacing info for spatial axes "
             "%u, %u, %u", me,
             baseDim+0, baseDim+1, baseDim+2);
    airMopError(mop); return 1;
  }
  /* actually, there shouldn't be any other options for spacing status
     besides these too; this is just being careful */
  if (!( nrrdSpacingStatusDirection == status
         || nrrdSpacingStatusScalarNoSpace == status )) {
    biffAddf(GAGE, "%s: sorry, can only handle spacing status %d (%s) "
             "or %d (%s), not %d (%s)", me,
             nrrdSpacingStatusDirection,
             airEnumStr(nrrdSpacingStatus, nrrdSpacingStatusDirection),
             nrrdSpacingStatusScalarNoSpace,
             airEnumStr(nrrdSpacingStatus, nrrdSpacingStatusScalarNoSpace),
             status, airEnumStr(nrrdSpacingStatus, status));
    airMopError(mop); return 1;
  }

  if (nrrdSpacingStatusDirection == status) {
    shape->fromOrientation = AIR_TRUE;
    if (3 != nin->spaceDim) {
      biffAddf(GAGE, "%s: orientation space dimension %d != 3",
               me, nin->spaceDim);
      airMopError(mop); return 1;
    }
  } else {
    shape->fromOrientation = AIR_FALSE;
  }

  /* ------ find centering (set shape->center) */
  /* NOTE: when the volume is being crammed in a bi-unit cube, the centering
     will actually affect the positions of the samples.  Otherwise,
     (having full orientation, or using orientationFromSpacing), the
     centering will only affect the probe-able bounds of the volume, but
     the sample positions in space don't depend on centering */
  cx = ax[0]->center;
  cy = ax[1]->center;
  cz = ax[2]->center;
  if (!( cx == cy && cy == cz )) {
    biffAddf(GAGE,
             "%s: axes %d,%d,%d centerings (%s,%s,%s) not all equal",
             me, baseDim+0, baseDim+1, baseDim+2,
             airEnumStr(nrrdCenter, cx),
             airEnumStr(nrrdCenter, cy),
             airEnumStr(nrrdCenter, cz));
    airMopError(mop); return 1;
  }
  /* Hopefully, ctx->parm.defaultCenter == shape->defaultCenter; and this
     worry will be moot if ctx->parm.defaultCenter goes away */
  shape->center = (nrrdCenterUnknown != cx
                   ? cx /* cx == cy == cz, by above */
                   : (ctx
                      ? ctx->parm.defaultCenter
                      : shape->defaultCenter));

  /* ------ find sizes (set shape->size[0,1,2]) */
  shape->size[0] = ax[0]->size;
  shape->size[1] = ax[1]->size;
  shape->size[2] = ax[2]->size;
  minsize = (nrrdCenterCell == shape->center ? 1 : 2);
  /* this can't be relaxed in the face of having full orientation info,
     because even then, you can't have a non-zero probe-able volume if
     there's only one sample along a node-centered axis */
  if (!(shape->size[0] >= minsize
        && shape->size[1] >= minsize
        && shape->size[2] >= minsize )) {
    biffAddf(GAGE, "%s: sizes (%u,%u,%u) must all be >= %u "
             "(min number of %s-centered samples)", me,
             shape->size[0], shape->size[1], shape->size[2],
             minsize, airEnumStr(nrrdCenter, shape->center));
    airMopError(mop); return 1;
  }

  /* ------ find spacings[0,1,2] and ItoW matrix */
  /* Hopefully, ctx->parm.orientationFromSpacing and
     shape->orientationFromSpacing don't represent competing interests;
     this worry will be moot if ctx->parm.orientationFromSpacing goes away */
  ofspc = ((ctx && ctx->parm.orientationFromSpacing)
           || shape->orientationFromSpacing);
  if (shape->fromOrientation || ofspc) {
    if (ofspc) {
      /* need abs() in case an axis had negative spacing */
      ELL_3V_ABS(shape->spacing, spcCalc);
      ELL_3V_SET(vecCalc[0], airSgn(spcCalc[0]), 0.0, 0.0);
      ELL_3V_SET(vecCalc[1], 0.0, airSgn(spcCalc[1]), 0.0);
      ELL_3V_SET(vecCalc[2], 0.0, 0.0, airSgn(spcCalc[2]));
    } else {
      ELL_3V_COPY(shape->spacing, spcCalc);
      /* vecCalc set by nrrdSpacingCalculate */
    }
    if (shape->fromOrientation) {
      /* if the spaceOrigin isn't set, this will be all NaNs */
      nrrdSpaceOriginGet(nin, orig);
    } else {
      /* sorry, if you want to specify an image origin that over-rides the
         behavior of centering the volume at (0,0,0), then it has to be
         done through the full orientation info.  That is, we don't want
         to use nrrdOriginCalculate() because otherwise the logic gets
         too complicated */
      ELL_3V_SET(orig, AIR_NAN, AIR_NAN, AIR_NAN);
    }
    if (!ELL_3V_EXISTS(orig)) {
      /* don't have origin, for whatever reason; center volume on (0,0,0) */
      ELL_3V_SET(orig, 0.0, 0.0, 0.0);
      ELL_3V_SCALE_INCR(orig, -(shape->size[0] - 1.0)*shape->spacing[0]/2.0,
                        vecCalc[0]);
      ELL_3V_SCALE_INCR(orig, -(shape->size[1] - 1.0)*shape->spacing[1]/2.0,
                        vecCalc[1]);
      ELL_3V_SCALE_INCR(orig, -(shape->size[2] - 1.0)*shape->spacing[2]/2.0,
                        vecCalc[2]);
    }
    vecD[3] = 0;
    ELL_3V_SCALE(vecD, spcCalc[0], vecCalc[0]);
    ELL_4MV_COL0_SET(shape->ItoW, vecD);
    ELL_3V_SCALE(vecD, spcCalc[1], vecCalc[1]);
    ELL_4MV_COL1_SET(shape->ItoW, vecD);
    ELL_3V_SCALE(vecD, spcCalc[2], vecCalc[2]);
    ELL_4MV_COL2_SET(shape->ItoW, vecD);
    vecD[3] = 1;
    ELL_3V_COPY(vecD, orig);
    ELL_4MV_COL3_SET(shape->ItoW, vecD);
    /*
    fprintf(stderr, "%s: %g (%g,%g,%g)\n", me,
            spcCalc[0], vecCalc[0][0], vecCalc[0][1], vecCalc[0][2]);
    fprintf(stderr, "%s: %g (%g,%g,%g)\n", me,
            spcCalc[1], vecCalc[1][0], vecCalc[1][1], vecCalc[1][2]);
    fprintf(stderr, "%s: %g (%g,%g,%g)\n", me,
            spcCalc[2], vecCalc[2][0], vecCalc[2][1], vecCalc[2][2]);
    */
    /*
    fprintf(stderr, "%s: ItoW = %g %g %g %g\n", me,
           shape->ItoW[ 0], shape->ItoW[ 1], shape->ItoW[ 2], shape->ItoW[ 3]);
    fprintf(stderr, "%s:        %g %g %g %g\n", me,
           shape->ItoW[ 4], shape->ItoW[ 5], shape->ItoW[ 6], shape->ItoW[ 7]);
    fprintf(stderr, "%s:        %g %g %g %g\n", me,
           shape->ItoW[ 8], shape->ItoW[ 9], shape->ItoW[10], shape->ItoW[11]);
    fprintf(stderr, "%s:        %g %g %g %g\n", me,
           shape->ItoW[12], shape->ItoW[13], shape->ItoW[14], shape->ItoW[15]);
    */
  } else { /* not (shape->fromOrientation || ofspc) */
    double maxLen, volHalfLen[3];
    size_t num[3];
    /* ------ learn lengths for bounding nrrd in bi-unit cube */
    ELL_3V_ABS(shape->spacing, spcCalc);
    maxLen = 0.0;
    for (ai=0; ai<=2; ai++) {
      num[ai] = (nrrdCenterNode == shape->center
                 ? shape->size[ai]-1
                 : shape->size[ai]);
      volHalfLen[ai] = num[ai]*shape->spacing[ai];
      maxLen = AIR_MAX(maxLen, volHalfLen[ai]);
    }
    /* Thu Dec 13 02:45:01 EST 2007
       fixed long-standing bug in handling vols without full orientation info:
       spacing[ai] was never scaled to account for being crammed into
       the bi-unit cube!! */
    for (ai=0; ai<=2; ai++) {
      volHalfLen[ai] /= maxLen;
      shape->spacing[ai] = 2*volHalfLen[ai]/num[ai];
    }
    ELL_3V_SET(vecC, 0, 0, 0);
    shapeUnitItoW(shape, vecA, vecC, volHalfLen);
    ELL_3V_SET(vecC, 1, 0, 0);
    shapeUnitItoW(shape, vecB, vecC, volHalfLen);
    ELL_3V_SUB(vecD, vecB, vecA);
    vecD[3] = 0;
    ELL_4MV_COL0_SET(shape->ItoW, vecD);

    ELL_3V_SET(vecC, 0, 1, 0);
    shapeUnitItoW(shape, vecB, vecC, volHalfLen);
    ELL_3V_SUB(vecD, vecB, vecA);
    vecD[3] = 0;
    ELL_4MV_COL1_SET(shape->ItoW, vecD);

    ELL_3V_SET(vecC, 0, 0, 1);
    shapeUnitItoW(shape, vecB, vecC, volHalfLen);
    ELL_3V_SUB(vecD, vecB, vecA);
    vecD[3] = 0;
    ELL_4MV_COL2_SET(shape->ItoW, vecD);

    vecA[3] = 1;
    ELL_4MV_COL3_SET(shape->ItoW, vecA);
  }

  /* ------ set the rest of the matrices */
  ell_4m_inv_d(shape->WtoI, shape->ItoW);
  ELL_34M_EXTRACT(matA, shape->ItoW);
  ell_3m_inv_d(shape->ItoWSubInv, matA);
  ELL_3M_TRANSPOSE(shape->ItoWSubInvTransp, shape->ItoWSubInv);

  airMopOkay(mop);
  return 0;
}

int
gageShapeSet(gageShape *shape, const Nrrd *nin, int baseDim) {
  static const char me[]="gageShapeSet";

  if (_gageShapeSet(NULL, shape, nin, baseDim)) {
    biffAddf(GAGE, "%s: trouble", me);
    return 1;
  }
  return 0;
}

/*
** this wasn't being used at all
void
gageShapeUnitWtoI(gageShape *shape, double indx[3], double world[3]) {
  int i;

  if (nrrdCenterNode == shape->center) {
    for (i=0; i<=2; i++) {
      indx[i] = NRRD_NODE_IDX(-shape->volHalfLen[i], shape->volHalfLen[i],
                               shape->size[i], world[i]);
    }
  } else {
    for (i=0; i<=2; i++) {
      indx[i] = NRRD_CELL_IDX(-shape->volHalfLen[i], shape->volHalfLen[i],
                               shape->size[i], world[i]);
    }
  }
}
*/

void
gageShapeWtoI(const gageShape *shape,
              double _indx[3], const double _world[3]) {
  /* static const char me[]="gageShapeWtoI"; */
  double indx[4], world[4];

  /*
  fprintf(stderr, "!%s: hello %p %p %p; %p\n", me,
          shape, _indx, _world, shape->WtoI);
  */
  ELL_3V_COPY(world, _world);
  world[3] = 1.0;
  ELL_4MV_MUL(indx, shape->WtoI, world);
  ELL_3V_SCALE(_indx, 1.0/indx[3], indx);
}

void
gageShapeItoW(const gageShape *shape,
              double _world[3], const double _indx[3]) {
  double world[4], indx[4];

  ELL_3V_COPY(indx, _indx);
  indx[3] = 1.0;
  ELL_4MV_MUL(world, shape->ItoW, indx);
  ELL_3V_SCALE(_world, 1.0/world[3], world);
}

/*
******** gageShapeEqual
**
** shapes not being equal is a biffable error,
** returning 0 signifies this "error"
** returning 1 means no error, they ARE equal
*/
int
gageShapeEqual(const gageShape *shape1, const char *_name1,
               const gageShape *shape2, const char *_name2) {
  static const char me[]="gageShapeEqual";
  const char *name1, *name2, what[] = "???";

  if (!( shape1 && shape2 )) {
    biffAddf(GAGE, "%s: can't judge equality w/ NULL pointer", me);
    return 0;
  }
  name1 = _name1 ? _name1 : what;
  name2 = _name2 ? _name2 : what;
  if (!( shape1->fromOrientation == shape2->fromOrientation )) {
    biffAddf(GAGE,
             "%s: fromOrientation of %s (%s) != %s's (%s)", me,
             name1, shape1->fromOrientation ? "true" : "false",
             name2, shape2->fromOrientation ? "true" : "false");
    return 0;
  }
  if (!( shape1->size[0] == shape2->size[0] &&
         shape1->size[1] == shape2->size[1] &&
         shape1->size[2] == shape2->size[2] )) {
    biffAddf(GAGE,
             "%s: dimensions of %s (%u,%u,%u) != %s's (%u,%u,%u)",
             me, name1,
             shape1->size[0], shape1->size[1], shape1->size[2],
             name2,
             shape2->size[0], shape2->size[1], shape2->size[2]);
    return 0;
  }
  if (shape1->fromOrientation) {
    if (!( ELL_4M_EQUAL(shape1->ItoW, shape2->ItoW) )) {
      biffAddf(GAGE, "%s: ItoW matrices of %s and %s not the same", me,
               name1, name2);
      return 0;
    }
  } else {
    if (!( shape1->spacing[0] == shape2->spacing[0] &&
           shape1->spacing[1] == shape2->spacing[1] &&
           shape1->spacing[2] == shape2->spacing[2] )) {
      biffAddf(GAGE, "%s: spacings of %s (%g,%g,%g) != %s's (%g,%g,%g)",
               me, name1,
               shape1->spacing[0], shape1->spacing[1], shape1->spacing[2],
               name2,
               shape2->spacing[0], shape2->spacing[1], shape2->spacing[2]);
      return 0;
    }
    if (!( shape1->center == shape2->center )) {
      biffAddf(GAGE,
               "%s: centering of %s (%s) != %s's (%s)", me,
               name1, airEnumStr(nrrdCenter, shape1->center),
               name2, airEnumStr(nrrdCenter, shape2->center));
      return 0;
    }
  }

  return 1;
}

void
gageShapeBoundingBox(double min[3], double max[3],
                     const gageShape *shape) {
  /* static const char me[]="gageShapeBoundingBox"; */
  double minIdx[3], maxIdx[3], cornerIdx[8][3], tmp[3];
  unsigned int ii;

  if (!( min && max && shape )) {
    return;
  }
  if (nrrdCenterNode == shape->center) {
    ELL_3V_SET(minIdx, 0, 0, 0);
    ELL_3V_SET(maxIdx,
               shape->size[0]-1,
               shape->size[1]-1,
               shape->size[2]-1);
  } else {
    ELL_3V_SET(minIdx, -0.5, -0.5, -0.5);
    ELL_3V_SET(maxIdx,
               shape->size[0]-0.5,
               shape->size[1]-0.5,
               shape->size[2]-0.5);
  }
  ELL_3V_SET(cornerIdx[0], minIdx[0], minIdx[1], minIdx[2]);
  ELL_3V_SET(cornerIdx[1], maxIdx[0], minIdx[1], minIdx[2]);
  ELL_3V_SET(cornerIdx[2], minIdx[0], maxIdx[1], minIdx[2]);
  ELL_3V_SET(cornerIdx[3], maxIdx[0], maxIdx[1], minIdx[2]);
  ELL_3V_SET(cornerIdx[4], minIdx[0], minIdx[1], maxIdx[2]);
  ELL_3V_SET(cornerIdx[5], maxIdx[0], minIdx[1], maxIdx[2]);
  ELL_3V_SET(cornerIdx[6], minIdx[0], maxIdx[1], maxIdx[2]);
  ELL_3V_SET(cornerIdx[7], maxIdx[0], maxIdx[1], maxIdx[2]);
  gageShapeItoW(shape, tmp, cornerIdx[0]);
  ELL_3V_COPY(min, tmp);
  ELL_3V_COPY(max, tmp);
  for (ii=1; ii<8; ii++) {
    gageShapeItoW(shape, tmp, cornerIdx[ii]);
    ELL_3V_MIN(min, min, tmp);
    ELL_3V_MAX(max, max, tmp);
  }
  return;
}
