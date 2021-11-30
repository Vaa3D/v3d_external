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

pullContext *
pullContextNew(void) {
  pullContext *pctx;
  unsigned int ii;

  pctx = (pullContext *)calloc(1, sizeof(pullContext));
  if (!pctx) {
    return NULL;
  }

  _pullInitParmInit(&(pctx->initParm));
  _pullIterParmInit(&(pctx->iterParm));
  _pullSysParmInit(&(pctx->sysParm));
  _pullFlagInit(&(pctx->flag));
  pctx->verbose = 0;
  pctx->threadNum = 1;
  pctx->rngSeed = 42;
  pctx->progressBinMod = 50;
  pctx->iter_cb = NULL;
  pctx->data_cb = NULL;

  for (ii=0; ii<PULL_VOLUME_MAXNUM; ii++) {
    pctx->vol[ii] = NULL;
  }
  pctx->volNum = 0;
  for (ii=0; ii<=PULL_INFO_MAX; ii++) {
    pctx->ispec[ii] = NULL;
    pctx->infoIdx[ii] = UINT_MAX;
  }
  pctx->interType = pullInterTypeUnknown;
  pctx->energySpecR = pullEnergySpecNew();
  pctx->energySpecS = pullEnergySpecNew();
  pctx->energySpecWin = pullEnergySpecNew();

  pctx->haltonOffset = 0;
  ELL_4V_SET(pctx->bboxMin, AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_4V_SET(pctx->bboxMax, AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN);
  pctx->infoTotalLen = 0; /* will be set later */
  pctx->idtagNext = 0;
  pctx->haveScale = AIR_FALSE;
  pctx->constraint = 0;
  pctx->constraintDim = -1;
  pctx->targetDim = -1;
  pctx->finished = AIR_FALSE;
  pctx->maxDistSpace = AIR_NAN;
  pctx->maxDistScale = AIR_NAN;
  pctx->voxelSizeSpace = AIR_NAN;
  pctx->voxelSizeScale = AIR_NAN;
  pctx->eipScale = 1.0;

  pctx->bin = NULL;
  ELL_4V_SET(pctx->binsEdge, 0, 0, 0, 0);
  pctx->binNum = 0;
  pctx->binNextIdx = 0;

  pctx->tmpPointPerm = NULL;
  pctx->tmpPointPtr = NULL;
  pctx->tmpPointNum = 0;

  /* pctx->binMutex setup my pullStart */
  pctx->task = NULL;
  pctx->iterBarrierA = NULL;
  pctx->iterBarrierB = NULL;
#if PULL_HINTER
  pctx->nhinter  = nrrdNew();
#endif
  pctx->logAdd = NULL;

  pctx->timeIteration = 0;
  pctx->timeRun = 0;
  pctx->energy = AIR_NAN;
  pctx->addNum = 0;
  pctx->nixNum = 0;
  pctx->stuckNum = 0;
  pctx->pointNum = 0;
  pctx->iter = 0;
  for (ii=pullCountUnknown; ii<pullCountLast; ii++) {
    pctx->count[ii] = 0;
  }
  return pctx;
}

/*
** this should only nix things created by pullContextNew, or the things
** (vols and ispecs) that were explicitly added to this context
*/
pullContext *
pullContextNix(pullContext *pctx) {
  unsigned int ii;

  if (pctx) {
    for (ii=0; ii<pctx->volNum; ii++) {
      pctx->vol[ii] = pullVolumeNix(pctx->vol[ii]);
    }
    pctx->volNum = 0;
    for (ii=0; ii<=PULL_INFO_MAX; ii++) {
      if (pctx->ispec[ii]) {
        pctx->ispec[ii] = pullInfoSpecNix(pctx->ispec[ii]);
      }
    }
    pctx->energySpecR = pullEnergySpecNix(pctx->energySpecR);
    pctx->energySpecS = pullEnergySpecNix(pctx->energySpecS);
    pctx->energySpecWin = pullEnergySpecNix(pctx->energySpecWin);
#if PULL_HINTER
    nrrdNuke(pctx->nhinter);
#endif
    /* handled elsewhere: bin, task, iterBarrierA, iterBarrierB */
    airFree(pctx);
  }
  return NULL;
}

int
_pullMiscParmCheck(pullContext *pctx) {
  static const char me[]="_pullMiscParmCheck";
  double denr;

  if (!( AIR_IN_CL(1, pctx->threadNum, PULL_THREAD_MAXNUM) )) {
    biffAddf(PULL, "%s: pctx->threadNum (%d) outside valid range [1,%d]", me,
             pctx->threadNum, PULL_THREAD_MAXNUM);
    return 1;
  }
  if (airEnumValCheck(pullInterType, pctx->interType)) {
    biffAddf(PULL, "%s: pctx->interType %d not a valid %s", me,
             pctx->interType, pullInterType->name);
    return 1;
  }
  /* HEY: error checking on energySpec's seems rather spotty . . . */
  if (pullEnergyUnknown == pctx->energySpecR->energy) {
    biffAddf(PULL, "%s: need to set space energy", me);
    return 1;
  }
  if (pullInterTypeJustR == pctx->interType
      || pullInterTypeUnivariate == pctx->interType) {
    if (pullEnergyZero != pctx->energySpecS->energy) {
      biffAddf(PULL, "%s: can't use scale energy %s with inter type %s", me,
               pctx->energySpecS->energy->name,
               airEnumStr(pullInterType, pctx->interType));
      return 1;
    }
  } else {
    if (pullEnergyZero == pctx->energySpecS->energy) {
      biffAddf(PULL, "%s: need a non-zero scale energy for inter type %s", me,
               airEnumStr(pullInterType, pctx->interType));
      return 1;
    }
  }
  /* make sure that spatial repulsion is really repulsive at r=0 */
  pctx->energySpecR->energy->eval(&denr, 0.0000001, pctx->energySpecR->parm);
  if (!( denr < 0 )) {
    biffAddf(PULL, "%s: spatial energy doesn't have negative slope near r=0",
             me);
    return 1;
  }

  return 0;
}

int
_pullContextCheck(pullContext *pctx) {
  static const char me[]="_pullContextCheck";
  unsigned int ii, ccount;
  int gotIspec, gotConstr;
  const pullInfoSpec *lthr, *strn;

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (_pullInitParmCheck(&(pctx->initParm))
      || _pullIterParmCheck(&(pctx->iterParm))
      || _pullSysParmCheck(&(pctx->sysParm))
      || _pullMiscParmCheck(pctx)) {
    biffAddf(PULL, "%s: problem with parameters", me);
    return 1;
  }

  if (!pctx->volNum) {
    biffAddf(PULL, "%s: have no volumes set", me);
    return 1;
  }
  gotConstr = 0;
  gotIspec = AIR_FALSE;
  for (ii=0; ii<=PULL_INFO_MAX; ii++) {
    if (pctx->ispec[ii]) {
      if (pctx->ispec[ii]->constraint) {
        if (1 != pullInfoLen(ii)) {
          biffAddf(PULL, "%s: can't use non-scalar (len %u) %s as constraint",
                   me, pullInfoLen(ii), airEnumStr(pullInfo, ii));
          return 1;
        }
        if (pullSourceGage != pctx->ispec[ii]->source) {
          biffAddf(PULL, "%s: sorry, constraints can currently only "
                   "come from %s", me,
                   airEnumStr(pullSource, pullSourceGage));
          return 1;
        }
        if (gotConstr) {
          biffAddf(PULL, "%s: can't also have %s constraint, already have "
                   "constraint on %s ", me, airEnumStr(pullInfo, ii),
                   airEnumStr(pullInfo, gotConstr));
          return 1;
        }
        /* elso no problems having constraint on ii */
        gotConstr = ii;
      }
      /* make sure we have extra info as necessary */
      switch (ii) {
      case pullInfoInside:
      case pullInfoHeight:
      case pullInfoHeightLaplacian:
      case pullInfoSeedThresh:
      case pullInfoLiveThresh:
      case pullInfoLiveThresh2:
      case pullInfoLiveThresh3:
      case pullInfoIsovalue:
      case pullInfoStrength:
        if (!( AIR_EXISTS(pctx->ispec[ii]->scale)
               && AIR_EXISTS(pctx->ispec[ii]->zero) )) {
          biffAddf(PULL, "%s: %s info needs scale (%g) and zero (%g)", me,
                   airEnumStr(pullInfo, ii),
                   pctx->ispec[ii]->scale, pctx->ispec[ii]->zero);
          return 1;
        }
        break;
      }
      gotIspec = AIR_TRUE;
    }
  }

  if (!gotIspec) {
    biffAddf(PULL, "%s: have no infos set", me);
    return 1;
  }
  if (pctx->ispec[pullInfoStrength]) {
    if (pullSourceGage != pctx->ispec[pullInfoStrength]->source) {
      biffAddf(PULL, "%s: %s info must come from %s (not %s)", me,
               airEnumStr(pullInfo, pullInfoStrength),
               airEnumStr(pullSource, pullSourceGage),
               airEnumStr(pullSource, pctx->ispec[pullInfoStrength]->source));
      return 1;
    }
  }
  if (pctx->ispec[pullInfoInside]) {
    if (!pctx->ispec[pullInfoInsideGradient]) {
      biffAddf(PULL, "%s: want %s but don't have %s set", me,
               airEnumStr(pullInfo, pullInfoInside),
               airEnumStr(pullInfo, pullInfoInsideGradient));
      return 1;
    }
  }
  if (pctx->ispec[pullInfoTangent2]) {
    if (!pctx->ispec[pullInfoTangent1]) {
      biffAddf(PULL, "%s: want %s but don't have %s set", me,
               airEnumStr(pullInfo, pullInfoTangent2),
               airEnumStr(pullInfo, pullInfoTangent1));
      return 1;
    }
  }
  if (pctx->ispec[pullInfoNegativeTangent2]) {
    if (!pctx->ispec[pullInfoNegativeTangent1]) {
      biffAddf(PULL, "%s: want %s but don't have %s set", me,
               airEnumStr(pullInfo, pullInfoNegativeTangent2),
               airEnumStr(pullInfo, pullInfoNegativeTangent1));
      return 1;
    }
  }
  ccount = 0;
  ccount += !!(pctx->ispec[pullInfoTangent1]);
  ccount += !!(pctx->ispec[pullInfoTangent2]);
  ccount += !!(pctx->ispec[pullInfoNegativeTangent1]);
  ccount += !!(pctx->ispec[pullInfoNegativeTangent2]);
  if (4 == ccount) {
    biffAddf(PULL, "%s: can't specify all 4 tangents together", me);
    return 1;
  }
  if (3 == ccount && !pctx->flag.allowCodimension3Constraints) {
    biffAddf(PULL, "%s: must turn on allowCodimension3Constraints "
             "with 3 tangents", me);
    return 1;
  }
  if (pctx->ispec[pullInfoHeight]) {
    if (!( pctx->ispec[pullInfoHeightGradient] )) {
      biffAddf(PULL, "%s: want %s but don't have %s set", me,
               airEnumStr(pullInfo, pullInfoHeight),
               airEnumStr(pullInfo, pullInfoHeightGradient));
      return 1;
    }
    if (pctx->ispec[pullInfoHeight]->constraint) {
      if (!pctx->ispec[pullInfoHeightHessian]) {
        biffAddf(PULL, "%s: want constrained %s but don't have %s set", me,
                 airEnumStr(pullInfo, pullInfoHeight),
                 airEnumStr(pullInfo, pullInfoHeightHessian));
        return 1;
      }
      if (!( pctx->ispec[pullInfoTangent1]
             || pctx->ispec[pullInfoNegativeTangent1] )) {
        if (!pctx->flag.allowCodimension3Constraints) {
          biffAddf(PULL, "%s: want constrained %s but need (at least) "
                   "%s or %s set (maybe enable "
                   "pullFlagAllowCodimension3Constraints?)",
                   me,
                   airEnumStr(pullInfo, pullInfoHeight),
                   airEnumStr(pullInfo, pullInfoTangent1),
                   airEnumStr(pullInfo, pullInfoNegativeTangent1));
          return 1;
        }
      }
    }
  }
  if (pctx->ispec[pullInfoHeightLaplacian]) {
    if (!( pctx->ispec[pullInfoHeight] )) {
      biffAddf(PULL, "%s: want %s but don't have %s set", me,
               airEnumStr(pullInfo, pullInfoHeightLaplacian),
               airEnumStr(pullInfo, pullInfoHeight));
      return 1;
    }
  }
  if (pctx->ispec[pullInfoIsovalue]) {
    if (!( pctx->ispec[pullInfoIsovalueGradient]
           && pctx->ispec[pullInfoIsovalueHessian] )) {
      biffAddf(PULL, "%s: want %s but don't have %s and %s set", me,
               airEnumStr(pullInfo, pullInfoIsovalue),
               airEnumStr(pullInfo, pullInfoIsovalueGradient),
               airEnumStr(pullInfo, pullInfoIsovalueHessian));
      return 1;
    }
  }
  if ((lthr = pctx->ispec[pullInfoLiveThresh])
      && (strn = pctx->ispec[pullInfoStrength])
      && lthr->volIdx == strn->volIdx
      && lthr->item == strn->item
      && lthr->scale*strn->scale < 0) {
    biffAddf(PULL, "%s: %s and %s refer to same item (%s in %s), but have "
             "scaling factors with different signs (%g and %g); really?", me,
             airEnumStr(pullInfo, pullInfoLiveThresh),
             airEnumStr(pullInfo, pullInfoStrength),
             airEnumStr(pctx->vol[lthr->volIdx]->kind->enm, lthr->item),
             lthr->volName, lthr->scale, strn->scale);
    return 1;
  }
  if (pullInitMethodPointPerVoxel == pctx->initParm.method) {
    if (!( pctx->ispec[pullInfoSeedThresh] )) {
      biffAddf(PULL, "%s: sorry, need to have %s info set with %s init",
               me, airEnumStr(pullInfo, pullInfoSeedThresh),
               "point-per-voxel" /* HEY no airEnum for this */);
      return 1;
    }
  }

  return 0;
}

/*
** the API for this is most certainly going to change; the
** tensor output at this point is a hack created for vis purposes
*/
int
pullOutputGetFilter(Nrrd *nPosOut, Nrrd *nTenOut, Nrrd *nStrengthOut,
                    const double _scaleVec[3], double scaleRad,
                    pullContext *pctx,
                    unsigned int idtagMin, unsigned int idtagMax) {
  static const char me[]="pullOutputGetFilter";
  unsigned int binIdx, pointNum, pointIdx, outIdx;
  int E;
  double *posOut, *tenOut, *strnOut, scaleVec[3], scaleDir[3], scaleMag;
  pullBin *bin;
  pullPoint *point;

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (nStrengthOut && !pctx->ispec[pullInfoStrength]) {
    biffAddf(PULL, "%s: can't save out %s info that hasn't been set",
             me, airEnumStr(pullInfo, pullInfoStrength));
    return 1;
  }
  if (!AIR_EXISTS(scaleRad)) {
    biffAddf(PULL, "%s: got non-existent scaleRad %g", me, scaleRad);
    return 1;
  }
  if (!_scaleVec) {
    ELL_3V_SET(scaleVec, 0, 0, 0);
    ELL_3V_SET(scaleDir, 0, 0, 0);
    scaleMag = 0;
  } else {
    ELL_3V_COPY(scaleVec, _scaleVec);
    if (ELL_3V_LEN(scaleVec)) {
      ELL_3V_NORM(scaleDir, scaleVec, scaleMag);
    } else {
      ELL_3V_SET(scaleDir, 0, 0, 0);
      scaleMag = 0;
    }
  }
  pointNum = pullPointNumberFilter(pctx, idtagMin, idtagMax);
  E = AIR_FALSE;
  if (nPosOut) {
    E |= nrrdMaybeAlloc_va(nPosOut, nrrdTypeDouble, 2,
                           AIR_CAST(size_t, 4),
                           AIR_CAST(size_t, pointNum));
  }
  if (nTenOut) {
    E |= nrrdMaybeAlloc_va(nTenOut, nrrdTypeDouble, 2,
                           AIR_CAST(size_t, 7),
                           AIR_CAST(size_t, pointNum));
  }
  if (nStrengthOut) {
    E |= nrrdMaybeAlloc_va(nStrengthOut, nrrdTypeDouble, 1,
                           AIR_CAST(size_t, pointNum));
  }
  if (E) {
    biffMovef(PULL, NRRD, "%s: trouble allocating outputs", me);
    return 1;
  }
  posOut = nPosOut ? (double*)(nPosOut->data) : NULL;
  tenOut = nTenOut ? (double*)(nTenOut->data) : NULL;
  strnOut = nStrengthOut ? (double*)(nStrengthOut->data) : NULL;

  outIdx = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      if (!( idtagMin <= point->idtag
             && (0 == idtagMax
                 || point->idtag <= idtagMax) )) {
        continue;
      }
      /** to find idtag of point at particular location **/
      /*
      if (AIR_ABS(514.113  - point->pos[0]) < 0.5 &&
          AIR_ABS(453.519 - point->pos[1]) < 0.5 &&
          AIR_ABS(606.723  - point->pos[ 2 ]) < 0.5) {
        printf("!%s: point %u at (%g,%g,%g,%g) ##############\n",
               me, point->idtag,
               point->pos[0], point->pos[1],
               point->pos[2], point->pos[3]);
      }
      */
      if (nPosOut) {
        ELL_4V_COPY(posOut + 4*outIdx, point->pos);
        if (pctx->haveScale && scaleMag) {
          double *tpos, tvec[3], sc;
          tpos = posOut + 4*outIdx;
          sc = ELL_3V_DOT(tpos, scaleDir);
          ELL_3V_SCALE(tvec, sc, scaleDir);
          ELL_3V_SUB(tpos, tpos, tvec);
          ELL_3V_SCALE(tvec, scaleMag*tpos[3], scaleDir);
          ELL_3V_ADD2(tpos, tpos, tvec);
        }
        /*
        if (4523 == point->idtag) {
          fprintf(stderr, "!%s: point %u at index %u and pos %g %g %g %g\n",
                  me, point->idtag, outIdx,
                  (posOut + 4*outIdx)[0], (posOut + 4*outIdx)[1],
                  (posOut + 4*outIdx)[2], (posOut + 4*outIdx)[3]);
        }
        */
      }
      if (nStrengthOut) {
        strnOut[outIdx] = pullPointScalar(pctx, point, pullInfoStrength,
                                          NULL, NULL);
      }
      if (nTenOut) {
        double scl, tout[7];
        scl = 1;
        if (pctx->ispec[pullInfoTensor]) {
          TEN_T_COPY(tout, point->info + pctx->infoIdx[pullInfoTensor]);
        } else if (pctx->ispec[pullInfoHeightHessian]) {
          double *hess, eval[3], evec[9], eceil, maxeval, elen;
          unsigned int maxi;
          hess = point->info + pctx->infoIdx[pullInfoHeightHessian];
          if (0) {
            /* do this if using general symmetric tensor glyphs */
            TEN_M2T(tout, hess);
            tout[0] = 1.0;
          } if (0) {
            /* for spheres and only spheres */
            TEN_T_SET(tout, 1, 1, 0, 0, 1, 0, 1);
          } else {
            ell_3m_eigensolve_d(eval, evec, hess, 10);
            eval[0] = AIR_ABS(eval[0]);
            eval[1] = AIR_ABS(eval[1]);
            eval[2] = AIR_ABS(eval[2]);
            /* elen = ELL_3V_LEN(eval); */
            elen = (eval[0]+eval[1]+eval[2]);
            eceil = elen ? 10/elen : 10;
            eval[0] = eval[0] ? AIR_MIN(eceil, 1.0/eval[0]) : eceil;
            eval[1] = eval[1] ? AIR_MIN(eceil, 1.0/eval[1]) : eceil;
            eval[2] = eval[2] ? AIR_MIN(eceil, 1.0/eval[2]) : eceil;
            maxi = ELL_MAX3_IDX(eval[0], eval[1], eval[2]);
            maxeval = eval[maxi];
            ELL_3V_SCALE(eval, 1/maxeval, eval);
            tenMakeSingle_d(tout, 1, eval, evec);
            if (scaleRad && pctx->ispec[pullInfoHeight]->constraint) {
              double emin, sig;
              if (pctx->flag.scaleIsTau) {
                sig = gageSigOfTau(point->pos[3]);
              } else {
                sig = point->pos[3];
              }
              tenEigensolve_d(eval, evec, tout);  /* lazy way to sort */
              emin = eval[2];
              if (1 == pctx->constraintDim) {
                eval[1] = scaleRad*sig + emin/2;
                eval[2] = scaleRad*sig + emin/2;
              } if (2 == pctx->constraintDim) {
                double eavg;
                eavg = (2*eval[0] + eval[2])/3;
                eval[0] = eavg;
                eval[1] = eavg;
                eval[2] = scaleRad*sig + emin/2;
              }
              tenMakeSingle_d(tout, 1, eval, evec);
            }
          }
        } else if (0   /* another hack for general symmetric tensor glyphs */
                   && pctx->constraint
                   && (pctx->ispec[pullInfoIsovalueHessian])) {
          double *hess;
          hess = point->info + pctx->infoIdx[pullInfoIsovalueHessian];
          TEN_M2T(tout, hess);
          tout[0] = 1.0;
        } else if (pctx->constraint
                   && (pctx->ispec[pullInfoHeightGradient]
                       || pctx->ispec[pullInfoIsovalueGradient])) {
          double *grad, norm[3], len, mat[9], out[9];
          grad = point->info + (pctx->ispec[pullInfoHeightGradient]
                                ? pctx->infoIdx[pullInfoHeightGradient]
                                : pctx->infoIdx[pullInfoIsovalueGradient]);
          ELL_3V_NORM(norm, grad, len);
          ELL_3MV_OUTER(out, norm, norm);
          ELL_3M_IDENTITY_SET(mat);
          ELL_3M_SCALE_INCR(mat, -0.95, out);
          TEN_M2T(tout, mat);
          tout[0] = 1;
        } else {
          TEN_T_SET(tout, 1, 1, 0, 0, 1, 0, 1);
        }
        TEN_T_SCALE(tout, scl, tout);
        TEN_T_COPY(tenOut + 7*outIdx, tout);
        /*
        if (4523 == point->idtag) {
          fprintf(stderr, "!%s: point %u at index %u and ten (%g) %g %g %g %g %g %g\n",
                  me, point->idtag, outIdx,
                  tout[0], tout[1], tout[2], tout[3],
                  tout[4], tout[5], tout[6]);
        }
        */
      } /* if (nTenOut) */
      ++outIdx;
    }
  }

  return 0;
}

int
pullOutputGet(Nrrd *nPosOut, Nrrd *nTenOut, Nrrd *nStrengthOut,
              const double scaleVec[3], double scaleRad,
              pullContext *pctx) {
  static const char me[]="pullOutputGet";

  if (pullOutputGetFilter(nPosOut, nTenOut, nStrengthOut, scaleVec, scaleRad, pctx, 0, 0)) {
    biffAddf(PULL, "%s: trouble", me);
    return 1;
  }
  return 0;
}


int
pullPropGet(Nrrd *nprop, int prop, pullContext *pctx) {
  static const char me[]="pullPropGet";
  int typeOut;
  size_t size[2];
  unsigned int dim, pointNum, pointIdx, binIdx, *out_ui, outIdx;
  double *out_d, covar[16];
  float *out_f, *pnc;
  unsigned char *out_uc;
  pullBin *bin;
  pullPoint *point;

  pointNum = pullPointNumber(pctx);
  switch(prop) {
  case pullPropEnergy:
  case pullPropStepEnergy:
  case pullPropStepConstr:
  case pullPropScale:
  case pullPropNeighCovarTrace:
  case pullPropNeighCovarDet:
  case pullPropStability:
    dim = 1;
    size[0] = pointNum;
    typeOut = nrrdTypeDouble;
    break;
  case pullPropIdtag:
  case pullPropIdCC:
  case pullPropNeighInterNum:
    dim = 1;
    size[0] = pointNum;
    typeOut = nrrdTypeUInt;
    break;
  case pullPropStuck:
    dim = 1;
    size[0] = pointNum;
    /* HEY should be nrrdTypeUInt, no? */
    typeOut = nrrdTypeUChar;
    break;
  case pullPropPosition:
  case pullPropForce:
    dim = 2;
    size[0] = 4;
    size[1] = pointNum;
    typeOut = nrrdTypeDouble;
    break;
  case pullPropNeighDistMean:
    dim = 1;
    size[0] = pointNum;
    typeOut = nrrdTypeDouble;
    break;
  case pullPropNeighCovar:
    dim = 2;
    size[0] = 10;
    size[1] = pointNum;
    typeOut = nrrdTypeFloat;
    break;
  case pullPropNeighCovar7Ten:
  case pullPropNeighTanCovar:
    dim = 2;
    size[0] = 7;
    size[1] = pointNum;
    typeOut = nrrdTypeFloat;
    break;
  default:
    biffAddf(PULL, "%s: prop %d unrecognized", me, prop);
    return 1;
    break;
  }
  if (nrrdMaybeAlloc_nva(nprop, typeOut, dim, size)) {
    biffMovef(PULL, NRRD, "%s: trouble allocating output", me);
    return 1;
  }
  out_d = AIR_CAST(double *, nprop->data);
  out_f = AIR_CAST(float *, nprop->data);
  out_ui = AIR_CAST(unsigned int *, nprop->data);
  out_uc = AIR_CAST(unsigned char *, nprop->data);

  outIdx = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      pnc = point->neighCovar;
      switch(prop) {
      case pullPropEnergy:
        out_d[outIdx] = point->energy;
        break;
      case pullPropStepEnergy:
        out_d[outIdx] = point->stepEnergy;
        break;
      case pullPropStepConstr:
        out_d[outIdx] = point->stepConstr;
        break;
      case pullPropIdtag:
        out_ui[outIdx] = point->idtag;
        break;
      case pullPropIdCC:
        out_ui[outIdx] = point->idCC;
        break;
      case pullPropNeighInterNum:
        out_ui[outIdx] = point->neighInterNum;
        break;
      case pullPropStuck:
        out_uc[outIdx] = ((point->status & PULL_STATUS_STUCK_BIT)
                          ? point->stuckIterNum
                          : 0);
        break;
      case pullPropPosition:
        ELL_4V_COPY(out_d + 4*outIdx, point->pos);
        break;
      case pullPropForce:
        ELL_4V_COPY(out_d + 4*outIdx, point->force);
        break;
      case pullPropNeighDistMean:
        out_d[outIdx] = point->neighDistMean;
        break;
      case pullPropScale:
        out_d[outIdx] = (pctx->flag.scaleIsTau
                         ? gageSigOfTau(point->pos[3])
                         : point->pos[3]);
        break;
        /*
          0:xx 1:xy 2:xz 3:xs
          1    4:yy 5:yz 6:ys
          2    5    7:zz 8:zs
          3    6    8    9:ss
        */
      case pullPropNeighCovar:
        ELL_10V_COPY(out_f + 10*outIdx, point->neighCovar);
        break;
      case pullPropNeighCovar7Ten:
        TEN_T_SET(out_f + 7*outIdx, 1.0f,
                  pnc[0],
                  pnc[1],
                  pnc[2],
                  pnc[4],
                  pnc[5],
                  pnc[7]);
        break;
      case pullPropNeighTanCovar:
#if PULL_TANCOVAR
        TEN_T_SET(out_f + 7*outIdx, 1.0f,
                  point->neighTanCovar[0],
                  point->neighTanCovar[1],
                  point->neighTanCovar[2],
                  point->neighTanCovar[3],
                  point->neighTanCovar[4],
                  point->neighTanCovar[5]);
#else
        TEN_T_SET(out_f + 7*outIdx, 0.0f,
                  0.0f, 0.0f, 0.0f,
                  0.0f, 0.0f,
                  0.0f);
#endif
        break;
      case pullPropNeighCovarTrace:
        out_d[outIdx] = pnc[0] + pnc[4] + pnc[7] + pnc[9];
        break;
      case pullPropNeighCovarDet:
        if (pctx->haveScale) {
          ELL_4V_SET(covar +  0, pnc[0], pnc[1], pnc[2], pnc[3]);
          ELL_4V_SET(covar +  4, pnc[1], pnc[4], pnc[5], pnc[6]);
          ELL_4V_SET(covar +  8, pnc[2], pnc[5], pnc[7], pnc[8]);
          ELL_4V_SET(covar + 12, pnc[3], pnc[6], pnc[8], pnc[9]);
          out_d[outIdx] = ELL_4M_DET(covar);
        } else {
          ELL_3V_SET(covar +  0, pnc[0], pnc[1], pnc[2]);
          ELL_3V_SET(covar +  3, pnc[1], pnc[4], pnc[5]);
          ELL_3V_SET(covar +  6, pnc[2], pnc[5], pnc[7]);
          out_d[outIdx] = ELL_3M_DET(covar);
        }
        break;
      case pullPropStability:
        out_d[outIdx] = point->stability;
        break;
      default:
        biffAddf(PULL, "%s: prop %d unrecognized", me, prop);
        return 1;
        break;
      }
      ++outIdx;
    } /* for (pointIdx) */
  }

  return 0;
}

int
pullPositionHistoryGet(limnPolyData *pld, pullContext *pctx) {
  static const char me[]="pullPositionHistoryGet";
#if PULL_PHIST
  pullBin *bin;
  pullPoint *point;
  unsigned int binIdx, pointIdx, pointNum, vertNum, vertIdx,
    primIdx, phistIdx, phistNum;

  if (!(pld && pctx)) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }

  pointNum = 0;
  vertNum = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      vertNum += point->phistArr->len;
      pointNum++;
    }
  }
  if (limnPolyDataAlloc(pld, 1 << limnPolyDataInfoRGBA,
                        vertNum, vertNum, pointNum)) {
    biffMovef(PULL, LIMN, "%s: couldn't allocate output", me);
    return 1;
  }
  primIdx = 0;
  vertIdx = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      phistNum = point->phistArr->len;
      for (phistIdx=0; phistIdx<phistNum; phistIdx++) {
        int cond;
        unsigned char rgb[3];
        ELL_3V_SET(rgb, 0, 0, 0);
        ELL_3V_COPY(pld->xyzw + 4*vertIdx, point->phist + 5*phistIdx);
        (pld->xyzw + 4*vertIdx)[3] = 1;
        cond = AIR_CAST(int, (point->phist + 5*phistIdx)[4]);
        switch (cond) {
        case pullCondOld:
          ELL_3V_SET(rgb, 128, 128, 128);
          break;
        case pullCondConstraintSatA:
          ELL_3V_SET(rgb, 0, 255, 0);
          break;
        case pullCondConstraintSatB:
          ELL_3V_SET(rgb, 0, 0, 255);
          break;
        case pullCondEnergyTry:
          ELL_3V_SET(rgb, 255, 255, 255);
          break;
        case pullCondEnergyBad:
          ELL_3V_SET(rgb, 255, 0, 0);
          break;
        case pullCondConstraintFail:
          ELL_3V_SET(rgb, 255, 0, 255);
          break;
        case pullCondNew:
          ELL_3V_SET(rgb, 128, 255, 128);
          break;
        }
        ELL_4V_SET(pld->rgba + 4*vertIdx, rgb[0], rgb[1], rgb[2], 255);
        pld->indx[vertIdx] = vertIdx;
        vertIdx++;
      }
      pld->type[primIdx] = limnPrimitiveLineStrip;
      pld->icnt[primIdx] = phistNum;
      primIdx++;
    }
  }


  return 0;
#else
  AIR_UNUSED(pld);
  AIR_UNUSED(pctx);
  biffAddf(PULL, "%s: sorry, not compiled with PULL_PHIST", me);
  return 1;
#endif
}

