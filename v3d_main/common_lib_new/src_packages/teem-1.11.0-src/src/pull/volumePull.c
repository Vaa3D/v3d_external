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


#include "pull.h"
#include "privatePull.h"

pullVolume *
pullVolumeNew() {
  pullVolume *vol;

  vol = AIR_CAST(pullVolume *, calloc(1, sizeof(pullVolume)));
  if (vol) {
    vol->verbose = 0;
    vol->name = NULL;
    vol->kind = NULL;
    vol->ninSingle = NULL;
    vol->ninScale = NULL;
    vol->scaleNum = 0;
    vol->scalePos = NULL;
    vol->scaleDerivNorm = AIR_FALSE;
    vol->scaleDerivNormBias = 0.0;
    vol->ksp00 = nrrdKernelSpecNew();
    vol->ksp11 = nrrdKernelSpecNew();
    vol->ksp22 = nrrdKernelSpecNew();
    vol->kspSS = nrrdKernelSpecNew();
    GAGE_QUERY_RESET(vol->pullValQuery);
    vol->gctx = NULL;
    vol->gpvl = NULL;
    vol->gpvlSS = NULL;
    /* this is turned OFF in volumes that have infos that aren't seedthresh,
       see pullInfoSpecAdd() */
    vol->seedOnly = AIR_TRUE;
    vol->forSeedPreThresh = AIR_FALSE;
  }
  return vol;
}

pullVolume *
pullVolumeNix(pullVolume *vol) {

  if (vol) {
    airFree(vol->name);
    airFree(vol->scalePos);
    vol->ksp00 = nrrdKernelSpecNix(vol->ksp00);
    vol->ksp11 = nrrdKernelSpecNix(vol->ksp11);
    vol->ksp22 = nrrdKernelSpecNix(vol->ksp22);
    vol->kspSS = nrrdKernelSpecNix(vol->kspSS);
    if (vol->gctx) {
      vol->gctx = gageContextNix(vol->gctx);
    }
    airFree(vol->gpvlSS);
    airFree(vol);
  }
  return NULL;
}

/*
** used to set all the fields of pullVolume at once, including the
** gageContext inside the pullVolume
**
** used both for top-level volumes in the pullContext (pctx->vol[i])
** in which case pctx is non-NULL,
** and for the per-task volumes (task->vol[i]),
** in which case pctx is NULL
*/
int
_pullVolumeSet(pullContext *pctx, pullVolume *vol,
               const gageKind *kind,
               int verbose, const char *name,
               const Nrrd *ninSingle,
               const Nrrd *const *ninScale,
               double *scalePos,
               unsigned int ninNum,
               int scaleDerivNorm,
               double scaleDerivNormBias,
               const NrrdKernelSpec *ksp00,
               const NrrdKernelSpec *ksp11,
               const NrrdKernelSpec *ksp22,
               const NrrdKernelSpec *kspSS) {
  static const char me[]="_pullVolumeSet";
  int E;
  unsigned int vi;

  if (!( vol && kind && airStrlen(name) && ksp00 && ksp11 && ksp22 )) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!ninSingle) {
    biffAddf(PULL, "%s: needed non-NULL ninSingle", me);
    return 1;
  }
  if (pctx) {
    for (vi=0; vi<pctx->volNum; vi++) {
      if (pctx->vol[vi] == vol) {
        biffAddf(PULL, "%s: already got vol %p as vol[%u]", me,
                 AIR_VOIDP(vol), vi);
        return 1;
      }
    }
  }
  if (ninNum) {
    if (!( ninNum >= 2 )) {
      biffAddf(PULL, "%s: need at least 2 volumes (not %u)", me, ninNum);
      return 1;
    }
    if (!scalePos) {
      biffAddf(PULL, "%s: need non-NULL scalePos with ninNum %u", me, ninNum);
      return 1;
    }
    if (!ninScale) {
      biffAddf(PULL, "%s: need non-NULL ninScale with ninNum %u", me, ninNum);
      return 1;
    }
  }

  vol->verbose = verbose;
  vol->kind = kind;
  vol->gctx = gageContextNew();
  gageParmSet(vol->gctx, gageParmVerbose,
              vol->verbose > 0 ? vol->verbose - 1 : 0);
  gageParmSet(vol->gctx, gageParmRenormalize, AIR_FALSE);
  /* because we're likely only using accurate kernels */
  gageParmSet(vol->gctx, gageParmStackNormalizeRecon, AIR_FALSE);
  vol->scaleDerivNorm = scaleDerivNorm;
  gageParmSet(vol->gctx, gageParmStackNormalizeDeriv, scaleDerivNorm);
  vol->scaleDerivNormBias = scaleDerivNormBias;
  gageParmSet(vol->gctx, gageParmStackNormalizeDerivBias,
              scaleDerivNormBias);
  gageParmSet(vol->gctx, gageParmCheckIntegrals, AIR_TRUE);
  E = 0;
  if (!E) E |= gageKernelSet(vol->gctx, gageKernel00,
                             ksp00->kernel, ksp00->parm);
  if (!E) E |= gageKernelSet(vol->gctx, gageKernel11,
                             ksp11->kernel, ksp11->parm);
  if (!E) E |= gageKernelSet(vol->gctx, gageKernel22,
                             ksp22->kernel, ksp22->parm);
  if (ninScale) {
    if (!kspSS) {
      biffAddf(PULL, "%s: got NULL kspSS", me);
      return 1;
    }
    gageParmSet(vol->gctx, gageParmStackUse, AIR_TRUE);
    if (!E) E |= !(vol->gpvl = gagePerVolumeNew(vol->gctx, ninSingle, kind));
    vol->gpvlSS = AIR_CAST(gagePerVolume **,
                           calloc(ninNum, sizeof(gagePerVolume *)));
    if (!E) E |= gageStackPerVolumeNew(vol->gctx, vol->gpvlSS,
                                       ninScale, ninNum, kind);
    if (!E) E |= gageStackPerVolumeAttach(vol->gctx, vol->gpvl, vol->gpvlSS,
                                          scalePos, ninNum);
    if (!E) E |= gageKernelSet(vol->gctx, gageKernelStack,
                               kspSS->kernel, kspSS->parm);
  } else {
    vol->gpvlSS = NULL;
    if (!E) E |= !(vol->gpvl = gagePerVolumeNew(vol->gctx, ninSingle, kind));
    if (!E) E |= gagePerVolumeAttach(vol->gctx, vol->gpvl);
  }
  if (E) {
    biffMovef(PULL, GAGE, "%s: trouble (%s %s)", me,
              ninSingle ? "ninSingle" : "",
              ninScale ? "ninScale" : "");
    return 1;
  }
  gageQueryReset(vol->gctx, vol->gpvl);
  /* the query is the single thing remaining unset in the gageContext */

  vol->name = airStrdup(name);
  if (!vol->name) {
    biffAddf(PULL, "%s: couldn't strdup name (len %u)", me,
             AIR_CAST(unsigned int, airStrlen(name)));
    return 1;
  }
  if (vol->verbose) {
    printf("%s: ---- vol=%p, name = %p = |%s|\n", me, AIR_VOIDP(vol),
           AIR_VOIDP(vol->name), vol->name);
    if (0 != vol->scaleDerivNormBias) {
      printf("%s: ---- scale deriv norm bias = %g\n", me,
             vol->scaleDerivNormBias);
    }
  }
  nrrdKernelSpecSet(vol->ksp00, ksp00->kernel, ksp00->parm);
  nrrdKernelSpecSet(vol->ksp11, ksp11->kernel, ksp11->parm);
  nrrdKernelSpecSet(vol->ksp22, ksp22->kernel, ksp22->parm);
  if (ninScale) {
    vol->ninSingle = ninSingle;
    vol->ninScale = ninScale;
    vol->scaleNum = ninNum;
    vol->scalePos = AIR_CAST(double *, calloc(ninNum, sizeof(double)));
    if (!vol->scalePos) {
      biffAddf(PULL, "%s: couldn't calloc scalePos", me);
      return 1;
    }
    for (vi=0; vi<ninNum; vi++) {
      vol->scalePos[vi] = scalePos[vi];
    }
    nrrdKernelSpecSet(vol->kspSS, kspSS->kernel, kspSS->parm);
  } else {
    vol->ninSingle = ninSingle;
    vol->ninScale = NULL;
    vol->scaleNum = 0;
    /* leave kspSS as is (unset) */
  }

  return 0;
}

/*
** the effect is to give pctx ownership of the vol
*/
int
pullVolumeSingleAdd(pullContext *pctx,
                    const gageKind *kind,
                    char *name, const Nrrd *nin,
                    const NrrdKernelSpec *ksp00,
                    const NrrdKernelSpec *ksp11,
                    const NrrdKernelSpec *ksp22) {
  static const char me[]="pullVolumeSingleSet";
  pullVolume *vol;

  vol = pullVolumeNew();
  if (_pullVolumeSet(pctx, vol, kind,
                     pctx->verbose, name,
                     nin,
                     NULL, NULL, 0, AIR_FALSE, 0.0,
                     ksp00, ksp11, ksp22, NULL)) {
    biffAddf(PULL, "%s: trouble", me);
    return 1;
  }

  /* add this volume to context */
  if (pctx->verbose) {
    printf("%s: adding pctx->vol[%u] = %p\n", me, pctx->volNum,
           AIR_VOIDP(vol));
  }
  pctx->vol[pctx->volNum] = vol;
  pctx->volNum++;
  return 0;
}

/*
** the effect is to give pctx ownership of the vol
*/
int
pullVolumeStackAdd(pullContext *pctx,
                   const gageKind *kind,
                   char *name,
                   const Nrrd *nin,
                   const Nrrd *const *ninSS,
                   double *scalePos,
                   unsigned int ninNum,
                   int scaleDerivNorm,
                   double scaleDerivNormBias,
                   const NrrdKernelSpec *ksp00,
                   const NrrdKernelSpec *ksp11,
                   const NrrdKernelSpec *ksp22,
                   const NrrdKernelSpec *kspSS) {
  static const char me[]="pullVolumeStackAdd";
  pullVolume *vol;

  vol = pullVolumeNew();
  if (_pullVolumeSet(pctx, vol, kind, pctx->verbose, name,
                     nin,
                     ninSS, scalePos, ninNum,
                     scaleDerivNorm, scaleDerivNormBias,
                     ksp00, ksp11, ksp22, kspSS)) {
    biffAddf(PULL, "%s: trouble", me);
    return 1;
  }

  /* add this volume to context */
  pctx->vol[pctx->volNum++] = vol;
  return 0;
}

/*
** this is only used to create pullVolumes for the pullTasks
**
** DOES use biff
*/
pullVolume *
_pullVolumeCopy(const pullVolume *volOrig) {
  static const char me[]="pullVolumeCopy";
  pullVolume *volNew;

  volNew = pullVolumeNew();
  if (_pullVolumeSet(NULL, volNew, volOrig->kind,
                     volOrig->verbose, volOrig->name,
                     volOrig->ninSingle,
                     volOrig->ninScale,
                     volOrig->scalePos,
                     volOrig->scaleNum,
                     volOrig->scaleDerivNorm,
                     volOrig->scaleDerivNormBias,
                     volOrig->ksp00, volOrig->ksp11,
                     volOrig->ksp22, volOrig->kspSS)) {
    biffAddf(PULL, "%s: trouble creating new volume", me);
    return NULL;
  }
  volNew->seedOnly = volOrig->seedOnly;
  volNew->forSeedPreThresh = volOrig->forSeedPreThresh;
  /* _pullVolumeSet just created a new (per-task) gageContext, and
     it will not learn the items from the info specs, so we have to
     add query here */
  if (gageQuerySet(volNew->gctx, volNew->gpvl, volOrig->gpvl->query)
      || gageUpdate(volNew->gctx)) {
    biffMovef(PULL, GAGE, "%s: trouble with new volume gctx", me);
    return NULL;
  }
  return volNew;
}

int
_pullInsideBBox(pullContext *pctx, double pos[4]) {

  return (AIR_IN_CL(pctx->bboxMin[0], pos[0], pctx->bboxMax[0]) &&
          AIR_IN_CL(pctx->bboxMin[1], pos[1], pctx->bboxMax[1]) &&
          AIR_IN_CL(pctx->bboxMin[2], pos[2], pctx->bboxMax[2]) &&
          AIR_IN_CL(pctx->bboxMin[3], pos[3], pctx->bboxMax[3]));
}

/*
** sets:
** pctx->haveScale
** pctx->voxelSizeSpace, voxelSizeScale
** pctx->bboxMin  ([0] through [3], always)
** pctx->bboxMax  (same)
*/
int
_pullVolumeSetup(pullContext *pctx) {
  static const char me[]="_pullVolumeSetup";
  unsigned int ii, numScale;

  /* first see if there are any gage problems */
  for (ii=0; ii<pctx->volNum; ii++) {
    if (pctx->verbose) {
      printf("%s: gageUpdate(vol[%u])\n", me, ii);
    }
    if (pctx->vol[ii]->gctx) {
      if (gageUpdate(pctx->vol[ii]->gctx)) {
        biffMovef(PULL, GAGE, "%s: trouble setting up gage on vol "
                  "%u/%u (\"%s\")",  me, ii, pctx->volNum,
                  pctx->vol[ii]->name);
        return 1;
      }
    } else {
      biffAddf(PULL, "%s: vol[%u] has NULL gctx", me, ii);
    }
  }

  pctx->voxelSizeSpace = 0.0;
  for (ii=0; ii<pctx->volNum; ii++) {
    double min[3], max[3];
    gageContext *gctx;
    gctx = pctx->vol[ii]->gctx;
    gageShapeBoundingBox(min, max, gctx->shape);
    if (!ii) {
      ELL_3V_COPY(pctx->bboxMin, min);
      ELL_3V_COPY(pctx->bboxMax, max);
    } else {
      ELL_3V_MIN(pctx->bboxMin, pctx->bboxMin, min);
      ELL_3V_MIN(pctx->bboxMax, pctx->bboxMax, max);
    }
    pctx->voxelSizeSpace += ELL_3V_LEN(gctx->shape->spacing)/sqrt(3.0);
    if (ii && !pctx->initParm.unequalShapesAllow) {
      if (!gageShapeEqual(pctx->vol[0]->gctx->shape, pctx->vol[0]->name,
                          pctx->vol[ii]->gctx->shape, pctx->vol[ii]->name)) {
        biffMovef(PULL, GAGE,
                  "%s: need equal shapes, but vol 0 and %u different",
                  me, ii);
        return 1;
      }
    }
  }
  pctx->voxelSizeSpace /= pctx->volNum;
  /* have now computed bbox{Min,Max}[0,1,2]; now do bbox{Min,Max}[3] */
  pctx->bboxMin[3] = pctx->bboxMax[3] = 0.0;
  pctx->haveScale = AIR_FALSE;
  pctx->voxelSizeScale = 0.0;
  numScale = 0;
  for (ii=0; ii<pctx->volNum; ii++) {
    if (pctx->vol[ii]->ninScale) {
      double sclMin, sclMax, sclStep;
      unsigned int si;
      numScale ++;
      sclMin = pctx->vol[ii]->scalePos[0];
      if (pctx->flag.scaleIsTau) {
        sclMin = gageTauOfSig(sclMin);
      }
      sclMax = pctx->vol[ii]->scalePos[pctx->vol[ii]->scaleNum-1];
      if (pctx->flag.scaleIsTau) {
        sclMax = gageTauOfSig(sclMax);
      }
      sclStep = 0;
      for (si=0; si<pctx->vol[ii]->scaleNum-1; si++) {
        double scl0, scl1;
        scl1 = pctx->vol[ii]->scalePos[si+1];
        scl0 = pctx->vol[ii]->scalePos[si];
        if (pctx->flag.scaleIsTau) {
          scl1 = gageTauOfSig(scl1);
          scl0 = gageTauOfSig(scl0);
        }
        sclStep += (scl1 - scl0);
      }
      sclStep /= pctx->vol[ii]->scaleNum-1;
      pctx->voxelSizeScale += sclStep;
      if (!pctx->haveScale) {
        pctx->bboxMin[3] = sclMin;
        pctx->bboxMax[3] = sclMax;
        pctx->haveScale = AIR_TRUE;
      } else {
        /* we already know haveScale; expand existing range */
        pctx->bboxMin[3] = AIR_MIN(sclMin, pctx->bboxMin[3]);
        pctx->bboxMax[3] = AIR_MAX(sclMax, pctx->bboxMax[3]);
      }
    }
  }
  if (numScale) {
    pctx->voxelSizeScale /= numScale;
  }
  if (pctx->verbose) {
    printf("%s: bboxMin (%g,%g,%g,%g) max (%g,%g,%g,%g)\n", me,
           pctx->bboxMin[0], pctx->bboxMin[1],
           pctx->bboxMin[2], pctx->bboxMin[3],
           pctx->bboxMax[0], pctx->bboxMax[1],
           pctx->bboxMax[2], pctx->bboxMax[3]);
    printf("%s: voxelSizeSpace %g Scale %g\n", me,
           pctx->voxelSizeSpace, pctx->voxelSizeScale);
  }

  /* _energyInterParticle() depends on this error checking */
  if (pctx->haveScale) {
    if (pullInterTypeJustR == pctx->interType) {
      biffAddf(PULL, "%s: need scale-aware intertype (not %s) with "
               "a scale-space volume",
               me, airEnumStr(pullInterType, pullInterTypeJustR));
      return 1;
    }
  } else {
    /* don't have scale */
    if (pullInterTypeJustR != pctx->interType) {
      biffAddf(PULL, "%s: can't use scale-aware intertype (%s) without "
               "a scale-space volume",
               me, airEnumStr(pullInterType, pctx->interType));
      return 1;
    }
  }
  if (pctx->flag.energyFromStrength
      && !(pctx->ispec[pullInfoStrength] && pctx->haveScale)) {
    biffAddf(PULL, "%s: sorry, can use energyFromStrength only with both "
             "a scale-space volume, and a strength info", me);
    return 1;
  }

  return 0;
}

/*
** basis of pullVolumeLookup
**
** uses biff, returns UINT_MAX in case of error
*/
unsigned int
_pullVolumeIndex(const pullContext *pctx,
                 const char *volName) {
  static const char me[]="_pullVolumeIndex";
  unsigned int vi;

  if (!( pctx && volName )) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return UINT_MAX;
  }
  if (0 == pctx->volNum) {
    biffAddf(PULL, "%s: given context has no volumes", me);
    return UINT_MAX;
  }
  for (vi=0; vi<pctx->volNum; vi++) {
    if (!strcmp(pctx->vol[vi]->name, volName)) {
      break;
    }
  }
  if (vi == pctx->volNum) {
    biffAddf(PULL, "%s: no volume has name \"%s\"", me, volName);
    return UINT_MAX;
  }
  return vi;
}

const pullVolume *
pullVolumeLookup(const pullContext *pctx,
                 const char *volName) {
  static const char me[]="pullVolumeLookup";
  unsigned int vi;

  vi = _pullVolumeIndex(pctx, volName);
  if (UINT_MAX == vi) {
    biffAddf(PULL, "%s: trouble looking up \"%s\"", me, volName);
    return NULL;
  }
  return pctx->vol[vi];
}

int
pullConstraintScaleRange(pullContext *pctx, double ssrange[2]) {
  static const char me[]="pullConstraintScaleRange";
  pullVolume *cvol;

  if (!(pctx && ssrange)) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(pctx->constraint)) {
    biffAddf(PULL, "%s: given context doesn't have constraint set", me);
    return 1;
  }
  if (!(pctx->ispec[pctx->constraint])) {
    biffAddf(PULL, "%s: info %s not set for constriant", me,
             airEnumStr(pullInfo, pctx->constraint));
    return 1;
  }
  cvol = pctx->vol[pctx->ispec[pctx->constraint]->volIdx];
  if (!cvol->ninScale) {
    biffAddf(PULL, "%s: volume \"%s\" has constraint but no scale-space",
             me, cvol->name);
    return 1;
  }
  ssrange[0] = cvol->scalePos[0];
  ssrange[1] = cvol->scalePos[cvol->scaleNum-1];
  if (pctx->flag.scaleIsTau) {
    ssrange[0] = gageTauOfSig(ssrange[0]);
    ssrange[1] = gageTauOfSig(ssrange[1]);
  }

  return 0;
}
