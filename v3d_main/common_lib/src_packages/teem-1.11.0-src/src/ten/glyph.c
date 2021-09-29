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

#include "ten.h"
#include "privateTen.h"

tenGlyphParm *
tenGlyphParmNew() {
  tenGlyphParm *parm;

  parm = (tenGlyphParm *)calloc(1, sizeof(tenGlyphParm));
  if (parm) {
    parm->verbose = 0;
    parm->nmask = NULL;
    parm->anisoType = tenAnisoUnknown;
    parm->onlyPositive = AIR_TRUE;
    parm->confThresh = AIR_NAN;
    parm->anisoThresh = AIR_NAN;
    parm->maskThresh = AIR_NAN;

    parm->glyphType = tenGlyphTypeUnknown;
    parm->facetRes = 10;
    parm->glyphScale = 1.0;
    parm->sqdSharp = 3.0;
    ELL_5V_SET(parm->edgeWidth, 0.0f, 0.0f, 0.4f, 0.2f, 0.1f);

    parm->colEvec = 0;  /* first */
    parm->colMaxSat = 1;
    parm->colGamma = 1;
    parm->colIsoGray = 1;
    parm->colAnisoType = tenAnisoUnknown;
    parm->colAnisoModulate = 0;
    ELL_4V_SET(parm->ADSP, 0, 1, 0, 30);

    parm->doSlice = AIR_FALSE;
    parm->sliceAxis = 0;
    parm->slicePos = 0;
    parm->sliceAnisoType = tenAnisoUnknown;
    parm->sliceOffset = 0.0;
    parm->sliceBias = 0.05f;
    parm->sliceGamma = 1.0;
  }
  return parm;
}

tenGlyphParm *
tenGlyphParmNix(tenGlyphParm *parm) {

  airFree(parm);
  return NULL;
}

int
tenGlyphParmCheck(tenGlyphParm *parm,
                  const Nrrd *nten, const Nrrd *npos, const Nrrd *nslc) {
  static const char me[]="tenGlyphParmCheck";
  int duh;
  size_t tenSize[3];
  char stmp[5][AIR_STRLEN_SMALL];

  if (!(parm && nten)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(tenAniso, parm->anisoType)) {
    biffAddf(TEN, "%s: unset (or invalid) anisoType (%d)",
             me, parm->anisoType);
    return 1;
  }
  if (airEnumValCheck(tenAniso, parm->colAnisoType)) {
    biffAddf(TEN, "%s: unset (or invalid) colAnisoType (%d)",
             me, parm->colAnisoType);
    return 1;
  }
  if (!( parm->facetRes >= 3 )) {
    biffAddf(TEN, "%s: facet resolution %d not >= 3", me, parm->facetRes);
    return 1;
  }
  if (!( AIR_IN_OP(tenGlyphTypeUnknown, parm->glyphType,
                   tenGlyphTypeLast) )) {
    biffAddf(TEN, "%s: unset (or invalid) glyphType (%d)",
             me, parm->glyphType);
    return 1;
  }
  if (!( parm->glyphScale > 0)) {
    biffAddf(TEN, "%s: glyphScale must be > 0 (not %g)", me, parm->glyphScale);
    return 1;
  }
  if (parm->nmask) {
    if (npos) {
      biffAddf(TEN, "%s: can't do masking with explicit coordinate list", me);
      return 1;
    }
    if (!( 3 == parm->nmask->dim
           && parm->nmask->axis[0].size == nten->axis[1].size
           && parm->nmask->axis[1].size == nten->axis[2].size
           && parm->nmask->axis[2].size == nten->axis[3].size )) {
      biffAddf(TEN, "%s: mask isn't 3-D or doesn't have sizes (%s,%s,%s)", me,
               airSprintSize_t(stmp[0], nten->axis[1].size),
               airSprintSize_t(stmp[1], nten->axis[2].size),
               airSprintSize_t(stmp[2], nten->axis[3].size));
      return 1;
    }
    if (!(AIR_EXISTS(parm->maskThresh))) {
      biffAddf(TEN, "%s: maskThresh hasn't been set", me);
      return 1;
    }
  }
  if (!( AIR_EXISTS(parm->anisoThresh)
         && AIR_EXISTS(parm->confThresh) )) {
    biffAddf(TEN, "%s: anisoThresh and confThresh haven't both been set", me);
    return 1;
  }
  if (parm->doSlice) {
    if (npos) {
      biffAddf(TEN, "%s: can't do slice with explicit coordinate list", me);
      return 1;
    }
    if (!( parm->sliceAxis <=2 )) {
      biffAddf(TEN, "%s: slice axis %d invalid", me, parm->sliceAxis);
      return 1;
    }
    if (!( parm->slicePos < nten->axis[1+parm->sliceAxis].size )) {
      biffAddf(TEN, "%s: slice pos %s not in valid range [0..%s]", me,
               airSprintSize_t(stmp[0], parm->slicePos),
               airSprintSize_t(stmp[1], nten->axis[1+parm->sliceAxis].size-1));
      return 1;
    }
    if (nslc) {
      if (2 != nslc->dim) {
        biffAddf(TEN, "%s: explicit slice must be 2-D (not %d)",
                 me, nslc->dim);
        return 1;
      }
      tenSize[0] = nten->axis[1].size;
      tenSize[1] = nten->axis[2].size;
      tenSize[2] = nten->axis[3].size;
      for (duh=parm->sliceAxis; duh<2; duh++) {
        tenSize[duh] = tenSize[duh+1];
      }
      if (!( tenSize[0] == nslc->axis[0].size
             && tenSize[1] == nslc->axis[1].size )) {
        biffAddf(TEN, "%s: axis %u slice of %sx%sx%s volume != %sx%s", me,
                 parm->sliceAxis,
                 airSprintSize_t(stmp[0], nten->axis[1].size),
                 airSprintSize_t(stmp[1], nten->axis[2].size),
                 airSprintSize_t(stmp[2], nten->axis[3].size),
                 airSprintSize_t(stmp[3], nslc->axis[0].size),
                 airSprintSize_t(stmp[4], nslc->axis[1].size));
        return 1;
      }
    } else {
      if (airEnumValCheck(tenAniso, parm->sliceAnisoType)) {
        biffAddf(TEN, "%s: unset (or invalid) sliceAnisoType (%d)",
                 me, parm->sliceAnisoType);
        return 1;
      }
    }
  }
  return 0;
}

int
tenGlyphGen(limnObject *glyphsLimn, echoScene *glyphsEcho,
            tenGlyphParm *parm,
            const Nrrd *nten, const Nrrd *npos, const Nrrd *nslc) {
  static const char me[]="tenGlyphGen";
  gageShape *shape;
  airArray *mop;
  float *tdata, eval[3], evec[9], *cvec, rotEvec[9], mA_f[16],
    absEval[3], glyphScl[3];
  double pI[3], pW[3], cl, cp, sRot[16], mA[16], mB[16], msFr[9], tmpvec[3],
    R, G, B, qA, qB, qC, glyphAniso, sliceGray;
  unsigned int duh;
  int slcCoord[3], idx, glyphIdx, axis, numGlyphs,
    svRGBAfl=AIR_FALSE;
  limnLook *look; int lookIdx;
  echoObject *eglyph, *inst, *list=NULL, *split, *esquare;
  echoPos_t eM[16], originOffset[3], edge0[3], edge1[3];
  char stmp[AIR_STRLEN_SMALL];
  /*
  int eret;
  double tmp1[3], tmp2[3];
  */

  if (!( (glyphsLimn || glyphsEcho) && nten && parm)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  mop = airMopNew();
  shape = gageShapeNew();
  shape->defaultCenter = nrrdCenterCell;
  airMopAdd(mop, shape, (airMopper)gageShapeNix, airMopAlways);
  if (npos) {
    if (!( 2 == nten->dim && 7 == nten->axis[0].size )) {
      biffAddf(TEN, "%s: nten isn't 2-D 7-by-N array", me);
      airMopError(mop); return 1;
    }
    if (!( 2 == npos->dim && 3 == npos->axis[0].size
           && nten->axis[1].size == npos->axis[1].size )) {
      biffAddf(TEN, "%s: npos isn't 2-D 3-by-%s array", me,
               airSprintSize_t(stmp, nten->axis[1].size));
      airMopError(mop); return 1;
    }
    if (!( nrrdTypeFloat == nten->type && nrrdTypeFloat == npos->type )) {
      biffAddf(TEN, "%s: nten and npos must be %s, not %s and %s", me,
               airEnumStr(nrrdType, nrrdTypeFloat),
               airEnumStr(nrrdType, nten->type),
               airEnumStr(nrrdType, npos->type));
      airMopError(mop); return 1;
    }
  } else {
    if (tenTensorCheck(nten, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
      biffAddf(TEN, "%s: didn't get a valid DT volume", me);
      airMopError(mop); return 1;
    }
  }
  if (tenGlyphParmCheck(parm, nten, npos, nslc)) {
    biffAddf(TEN, "%s: trouble", me);
    airMopError(mop); return 1;
  }
  if (!npos) {
    if (gageShapeSet(shape, nten, tenGageKind->baseDim)) {
      biffMovef(TEN, GAGE, "%s: trouble", me);
      airMopError(mop); return 1;
    }
  }
  if (parm->doSlice) {
    ELL_3V_COPY(edge0, shape->spacing);
    ELL_3V_COPY(edge1, shape->spacing);
    edge0[parm->sliceAxis] = edge1[parm->sliceAxis] = 0.0;
    switch(parm->sliceAxis) {
    case 0:
      edge0[1] = edge1[2] = 0;
      ELL_4M_ROTATE_Y_SET(sRot, AIR_PI/2);
      break;
    case 1:
      edge0[0] = edge1[2] = 0;
      ELL_4M_ROTATE_X_SET(sRot, AIR_PI/2);
      break;
    case 2: default:
      edge0[0] = edge1[1] = 0;
      ELL_4M_IDENTITY_SET(sRot);
      break;
    }
    ELL_3V_COPY(originOffset, shape->spacing);
    ELL_3V_SCALE(originOffset, -0.5, originOffset);
    originOffset[parm->sliceAxis] *= -2*parm->sliceOffset;
  }
  if (glyphsLimn) {
    /* create limnLooks for diffuse and ambient-only shading */
    /* ??? */
    /* hack: save old value of setVertexRGBAFromLook, and set to true */
    svRGBAfl = glyphsLimn->setVertexRGBAFromLook;
    glyphsLimn->setVertexRGBAFromLook = AIR_TRUE;
  }
  if (glyphsEcho) {
    list = echoObjectNew(glyphsEcho, echoTypeList);
  }
  if (npos) {
    numGlyphs = AIR_UINT(nten->axis[1].size);
  } else {
    numGlyphs = shape->size[0] * shape->size[1] * shape->size[2];
  }
  /* find measurement frame transform */
  if (3 == nten->spaceDim
      && AIR_EXISTS(nten->measurementFrame[0][0])) {
    /*     msFr        nten->measurementFrame
    **   0  1  2      [0][0]   [1][0]   [2][0]
    **   3  4  5      [0][1]   [1][1]   [2][1]
    **   6  7  8      [0][2]   [1][2]   [2][2]
    */
    msFr[0] = nten->measurementFrame[0][0];
    msFr[3] = nten->measurementFrame[0][1];
    msFr[6] = nten->measurementFrame[0][2];
    msFr[1] = nten->measurementFrame[1][0];
    msFr[4] = nten->measurementFrame[1][1];
    msFr[7] = nten->measurementFrame[1][2];
    msFr[2] = nten->measurementFrame[2][0];
    msFr[5] = nten->measurementFrame[2][1];
    msFr[8] = nten->measurementFrame[2][2];
  } else {
    ELL_3M_IDENTITY_SET(msFr);
  }
  for (idx=0; idx<numGlyphs; idx++) {
    tdata = (float*)(nten->data) + 7*idx;
    if (parm->verbose >= 2) {
      fprintf(stderr, "%s: glyph %d/%d: hello %g    %g %g %g %g %g %g\n",
              me, idx, numGlyphs, tdata[0],
              tdata[1], tdata[2], tdata[3],
              tdata[4], tdata[5], tdata[6]);
    }
    if (!( TEN_T_EXISTS(tdata) )) {
      /* there's nothing we can do here */
      if (parm->verbose >= 2) {
        fprintf(stderr, "%s: glyph %d/%d: non-existent data\n",
                me, idx, numGlyphs);
      }
      continue;
    }
    if (npos) {
      ELL_3V_COPY(pW, (float*)(npos->data) + 3*idx);
      if (!( AIR_EXISTS(pW[0]) && AIR_EXISTS(pW[1]) && AIR_EXISTS(pW[2]) )) {
        /* position doesn't exist- perhaps because its from the push
           library, which might kill points by setting coords to nan */
        continue;
      }
    } else {
      NRRD_COORD_GEN(pI, shape->size, 3, idx);
      /* this does take into account full orientation */
      gageShapeItoW(shape, pW, pI);
      if (parm->nmask) {
        if (!( nrrdFLookup[parm->nmask->type](parm->nmask->data, idx)
               >= parm->maskThresh )) {
          if (parm->verbose >= 2) {
            fprintf(stderr, "%s: glyph %d/%d: doesn't meet mask thresh\n",
                    me, idx, numGlyphs);
          }
          continue;
        }
      }
    }
    tenEigensolve_f(eval, evec, tdata);
    /* transform eigenvectors by measurement frame */
    ELL_3MV_MUL(tmpvec, msFr, evec + 0);
    ELL_3V_COPY_TT(evec + 0, float, tmpvec);
    ELL_3MV_MUL(tmpvec, msFr, evec + 3);
    ELL_3V_COPY_TT(evec + 3, float, tmpvec);
    ELL_3MV_MUL(tmpvec, msFr, evec + 6);
    ELL_3V_COPY_TT(evec + 6, float, tmpvec);
    ELL_3V_CROSS(tmpvec, evec + 0, evec + 3);
    if (0 > ELL_3V_DOT(tmpvec, evec + 6)) {
      ELL_3V_SCALE(evec + 6, -1, evec + 6);
    }
    ELL_3M_TRANSPOSE(rotEvec, evec);
    if (parm->doSlice
        && pI[parm->sliceAxis] == parm->slicePos) {
      /* set sliceGray */
      if (nslc) {
        /* we aren't masked by confidence, as anisotropy slice is */
        for (duh=0; duh<parm->sliceAxis; duh++) {
          slcCoord[duh] = (int)(pI[duh]);
        }
        for (duh=duh<parm->sliceAxis; duh<2; duh++) {
          slcCoord[duh] = (int)(pI[duh+1]);
        }
        /* HEY: GLK has no idea what's going here */
        slcCoord[0] = (int)(pI[0]);
        slcCoord[1] = (int)(pI[1]);
        slcCoord[2] = (int)(pI[2]);
        sliceGray =
          nrrdFLookup[nslc->type](nslc->data, slcCoord[0]
                                  + nslc->axis[0].size*slcCoord[1]);
      } else {
        if (!( tdata[0] >= parm->confThresh )) {
          if (parm->verbose >= 2) {
            fprintf(stderr, "%s: glyph %d/%d (slice): conf %g < thresh %g\n",
                    me, idx, numGlyphs, tdata[0], parm->confThresh);
          }
          continue;
        }
        sliceGray = tenAnisoEval_f(eval, parm->sliceAnisoType);
      }
      if (parm->sliceGamma > 0) {
        sliceGray = AIR_AFFINE(0, sliceGray, 1, parm->sliceBias, 1);
        sliceGray = pow(sliceGray, 1.0/parm->sliceGamma);
      } else {
        sliceGray = AIR_AFFINE(0, sliceGray, 1, 0, 1-parm->sliceBias);
        sliceGray = 1.0 - pow(sliceGray, -1.0/parm->sliceGamma);
      }
      /* make slice contribution */
      /* HEY: this is *NOT* aware of shape->fromOrientation */
      if (glyphsLimn) {
        lookIdx = limnObjectLookAdd(glyphsLimn);
        look = glyphsLimn->look + lookIdx;
        ELL_4V_SET_TT(look->rgba, float, sliceGray, sliceGray, sliceGray, 1);
        ELL_3V_SET(look->kads, 1, 0, 0);
        look->spow = 0;
        glyphIdx = limnObjectSquareAdd(glyphsLimn, lookIdx);
        ELL_4M_IDENTITY_SET(mA);
        ell_4m_post_mul_d(mA, sRot);
        if (!npos) {
          ELL_4M_SCALE_SET(mB,
                           shape->spacing[0],
                           shape->spacing[1],
                           shape->spacing[2]);
        }
        ell_4m_post_mul_d(mA, mB);
        ELL_4M_TRANSLATE_SET(mB, pW[0], pW[1], pW[2]);
        ell_4m_post_mul_d(mA, mB);
        ELL_4M_TRANSLATE_SET(mB,
                             originOffset[0],
                             originOffset[1],
                             originOffset[2]);
        ell_4m_post_mul_d(mA, mB);
        ELL_4M_COPY_TT(mA_f, float, mA);
        limnObjectPartTransform(glyphsLimn, glyphIdx, mA_f);
      }
      if (glyphsEcho) {
        esquare = echoObjectNew(glyphsEcho,echoTypeRectangle);
        ELL_3V_ADD2(((echoRectangle*)esquare)->origin, pW, originOffset);
        ELL_3V_COPY(((echoRectangle*)esquare)->edge0, edge0);
        ELL_3V_COPY(((echoRectangle*)esquare)->edge1, edge1);
        echoColorSet(esquare,
                     AIR_CAST(echoCol_t, sliceGray),
                     AIR_CAST(echoCol_t, sliceGray),
                     AIR_CAST(echoCol_t, sliceGray), 1);
        /* this is pretty arbitrary- but I want shadows to have some effect.
           Previously, the material was all ambient: (A,D,S) = (1,0,0),
           which avoided all shadow effects. */
        echoMatterPhongSet(glyphsEcho, esquare, 0.4f, 0.6f, 0, 40);
        echoListAdd(list, esquare);
      }
    }
    if (parm->onlyPositive) {
      if (eval[2] < 0) {
        /* didn't have all positive eigenvalues, its outta here */
        if (parm->verbose >= 2) {
          fprintf(stderr, "%s: glyph %d/%d: not all evals %g %g %g > 0\n",
                  me, idx, numGlyphs, eval[0], eval[1], eval[2]);
        }
        continue;
      }
    }
    if (!( tdata[0] >= parm->confThresh )) {
      if (parm->verbose >= 2) {
        fprintf(stderr, "%s: glyph %d/%d: conf %g < thresh %g\n",
                me, idx, numGlyphs, tdata[0], parm->confThresh);
      }
      continue;
    }
    if (!( tenAnisoEval_f(eval, parm->anisoType) >= parm->anisoThresh )) {
      if (parm->verbose >= 2) {
        fprintf(stderr, "%s: glyph %d/%d: aniso[%d] %g < thresh %g\n",
                me, idx, numGlyphs, parm->anisoType,
                tenAnisoEval_f(eval, parm->anisoType), parm->anisoThresh);
      }
      continue;
    }
    glyphAniso = tenAnisoEval_f(eval, parm->colAnisoType);
    /*
      fprintf(stderr, "%s: eret = %d; evals = %g %g %g\n", me,
      eret, eval[0], eval[1], eval[2]);
      ELL_3V_CROSS(tmp1, evec+0, evec+3); tmp2[0] = ELL_3V_LEN(tmp1);
      ELL_3V_CROSS(tmp1, evec+0, evec+6); tmp2[1] = ELL_3V_LEN(tmp1);
      ELL_3V_CROSS(tmp1, evec+3, evec+6); tmp2[2] = ELL_3V_LEN(tmp1);
      fprintf(stderr, "%s: crosses = %g %g %g\n", me,
      tmp2[0], tmp2[1], tmp2[2]);
    */

    /* set transform (in mA) */
    ELL_3V_ABS(absEval, eval);
    ELL_4M_IDENTITY_SET(mA);                        /* reset */
    ELL_3V_SCALE(glyphScl, parm->glyphScale, absEval); /* scale by evals */
    ELL_4M_SCALE_SET(mB, glyphScl[0], glyphScl[1], glyphScl[2]);

    ell_4m_post_mul_d(mA, mB);
    ELL_43M_INSET(mB, rotEvec);                     /* rotate by evecs */
    ell_4m_post_mul_d(mA, mB);
    ELL_4M_TRANSLATE_SET(mB, pW[0], pW[1], pW[2]);  /* translate */
    ell_4m_post_mul_d(mA, mB);

    /* set color (in R,G,B) */
    cvec = evec + 3*(AIR_CLAMP(0, parm->colEvec, 2));
    R = AIR_ABS(cvec[0]);                           /* standard mapping */
    G = AIR_ABS(cvec[1]);
    B = AIR_ABS(cvec[2]);
    /* desaturate by colMaxSat */
    R = AIR_AFFINE(0.0, parm->colMaxSat, 1.0, parm->colIsoGray, R);
    G = AIR_AFFINE(0.0, parm->colMaxSat, 1.0, parm->colIsoGray, G);
    B = AIR_AFFINE(0.0, parm->colMaxSat, 1.0, parm->colIsoGray, B);
    /* desaturate some by anisotropy */
    R = AIR_AFFINE(0.0, parm->colAnisoModulate, 1.0,
                   R, AIR_AFFINE(0.0, glyphAniso, 1.0, parm->colIsoGray, R));
    G = AIR_AFFINE(0.0, parm->colAnisoModulate, 1.0,
                   G, AIR_AFFINE(0.0, glyphAniso, 1.0, parm->colIsoGray, G));
    B = AIR_AFFINE(0.0, parm->colAnisoModulate, 1.0,
                   B, AIR_AFFINE(0.0, glyphAniso, 1.0, parm->colIsoGray, B));
    /* clamp and do gamma */
    R = AIR_CLAMP(0.0, R, 1.0);
    G = AIR_CLAMP(0.0, G, 1.0);
    B = AIR_CLAMP(0.0, B, 1.0);
    R = pow(R, parm->colGamma);
    G = pow(G, parm->colGamma);
    B = pow(B, parm->colGamma);

    /* find axis, and superquad exponents qA and qB */
    if (eval[2] > 0) {
      /* all evals positive */
      cl = AIR_MIN(0.99, tenAnisoEval_f(eval, tenAniso_Cl1));
      cp = AIR_MIN(0.99, tenAnisoEval_f(eval, tenAniso_Cp1));
      if (cl > cp) {
        axis = 0;
        qA = pow(1-cp, parm->sqdSharp);
        qB = pow(1-cl, parm->sqdSharp);
      } else {
        axis = 2;
        qA = pow(1-cl, parm->sqdSharp);
        qB = pow(1-cp, parm->sqdSharp);
      }
      qC = qB;
    } else if (eval[0] < 0) {
      /* all evals negative */
      float aef[3];
      aef[0] = absEval[2];
      aef[1] = absEval[1];
      aef[2] = absEval[0];
      cl = AIR_MIN(0.99, tenAnisoEval_f(aef, tenAniso_Cl1));
      cp = AIR_MIN(0.99, tenAnisoEval_f(aef, tenAniso_Cp1));
      if (cl > cp) {
        axis = 2;
        qA = pow(1-cp, parm->sqdSharp);
        qB = pow(1-cl, parm->sqdSharp);
      } else {
        axis = 0;
        qA = pow(1-cl, parm->sqdSharp);
        qB = pow(1-cp, parm->sqdSharp);
      }
      qC = qB;
    } else {
#define OOSQRT2 0.70710678118654752440
#define OOSQRT3 0.57735026918962576451
      /* double poleA[3]={OOSQRT3, OOSQRT3, OOSQRT3}; */
      double poleB[3]={1, 0, 0};
      double poleC[3]={OOSQRT2, OOSQRT2, 0};
      double poleD[3]={OOSQRT3, -OOSQRT3, -OOSQRT3};
      double poleE[3]={OOSQRT2, 0, -OOSQRT2};
      double poleF[3]={OOSQRT3, OOSQRT3, -OOSQRT3};
      double poleG[3]={0, -OOSQRT2, -OOSQRT2};
      double poleH[3]={0, 0, -1};
      /* double poleI[3]={-OOSQRT3, -OOSQRT3, -OOSQRT3}; */
      double funk[3]={0,4,2}, thrn[3]={1,4,4};
      double octa[3]={0,2,2}, cone[3]={1,2,2};
      double evalN[3], tmp, bary[3];
      double qq[3];

      ELL_3V_NORM(evalN, eval, tmp);
      if (eval[1] >= -eval[2]) {
        /* inside B-F-C */
        ell_3v_barycentric_spherical_d(bary, poleB, poleF, poleC, evalN);
        ELL_3V_SCALE_ADD3(qq, bary[0], octa, bary[1], thrn, bary[2], cone);
        axis = 2;
      } else if (eval[0] >= -eval[2]) {
        /* inside B-D-F */
        if (eval[1] >= 0) {
          /* inside B-E-F */
          ell_3v_barycentric_spherical_d(bary, poleB, poleE, poleF, evalN);
          ELL_3V_SCALE_ADD3(qq, bary[0], octa, bary[1], funk, bary[2], thrn);
          axis = 2;
        } else {
          /* inside B-D-E */
          ell_3v_barycentric_spherical_d(bary, poleB, poleD, poleE, evalN);
          ELL_3V_SCALE_ADD3(qq, bary[0], cone, bary[1], thrn, bary[2], funk);
          axis = 0;
        }
      } else if (eval[0] < -eval[1]) {
        /* inside D-G-H */
        ell_3v_barycentric_spherical_d(bary, poleD, poleG, poleH, evalN);
        ELL_3V_SCALE_ADD3(qq, bary[0], thrn, bary[1], cone, bary[2], octa);
        axis = 0;
      } else if (eval[1] < 0) {
        /* inside E-D-H */
        ell_3v_barycentric_spherical_d(bary, poleE, poleD, poleH, evalN);
        ELL_3V_SCALE_ADD3(qq, bary[0], funk, bary[1], thrn, bary[2], octa);
        axis = 0;
      } else {
        /* inside F-E-H */
        ell_3v_barycentric_spherical_d(bary, poleF, poleE, poleH, evalN);
        ELL_3V_SCALE_ADD3(qq, bary[0], thrn, bary[1], funk, bary[2], cone);
        axis = 2;
      }
      qA = qq[0];
      qB = qq[1];
      qC = qq[2];
#undef OOSQRT2
#undef OOSQRT3
    }

    /* add the glyph */
    if (parm->verbose >= 2) {
      fprintf(stderr, "%s: glyph %d/%d: the glyph stays!\n",
              me, idx, numGlyphs);
    }
    if (glyphsLimn) {
      lookIdx = limnObjectLookAdd(glyphsLimn);
      look = glyphsLimn->look + lookIdx;
      ELL_4V_SET_TT(look->rgba, float, R, G, B, 1);
      ELL_3V_SET(look->kads, parm->ADSP[0], parm->ADSP[1], parm->ADSP[2]);
      look->spow = 0;
      switch(parm->glyphType) {
      case tenGlyphTypeBox:
        glyphIdx = limnObjectCubeAdd(glyphsLimn, lookIdx);
        break;
      case tenGlyphTypeSphere:
        glyphIdx = limnObjectPolarSphereAdd(glyphsLimn, lookIdx, axis,
                                            2*parm->facetRes, parm->facetRes);
        break;
      case tenGlyphTypeCylinder:
        glyphIdx = limnObjectCylinderAdd(glyphsLimn, lookIdx, axis,
                                         parm->facetRes);
        break;
      case tenGlyphTypeSuperquad:
      default:
        glyphIdx =
          limnObjectPolarSuperquadFancyAdd(glyphsLimn, lookIdx, axis,
                                           AIR_CAST(float, qA),
                                           AIR_CAST(float, qB),
                                           AIR_CAST(float, qC), 0,
                                           2*parm->facetRes,
                                           parm->facetRes);
        break;
      }
      ELL_4M_COPY_TT(mA_f, float, mA);
      limnObjectPartTransform(glyphsLimn, glyphIdx, mA_f);
    }
    if (glyphsEcho) {
      switch(parm->glyphType) {
      case tenGlyphTypeBox:
        eglyph = echoObjectNew(glyphsEcho, echoTypeCube);
        /* nothing else to set */
        break;
      case tenGlyphTypeSphere:
        eglyph = echoObjectNew(glyphsEcho, echoTypeSphere);
        echoSphereSet(eglyph, 0, 0, 0, 1);
        break;
      case tenGlyphTypeCylinder:
        eglyph = echoObjectNew(glyphsEcho, echoTypeCylinder);
        echoCylinderSet(eglyph, axis);
        break;
      case tenGlyphTypeSuperquad:
      default:
        eglyph = echoObjectNew(glyphsEcho, echoTypeSuperquad);
        echoSuperquadSet(eglyph, axis, qA, qB);
        break;
      }
      echoColorSet(eglyph,
                   AIR_CAST(echoCol_t, R),
                   AIR_CAST(echoCol_t, G),
                   AIR_CAST(echoCol_t, B), 1);
      echoMatterPhongSet(glyphsEcho, eglyph,
                         parm->ADSP[0], parm->ADSP[1],
                         parm->ADSP[2], parm->ADSP[3]);
      inst = echoObjectNew(glyphsEcho, echoTypeInstance);
      ELL_4M_COPY(eM, mA);
      echoInstanceSet(inst, eM, eglyph);
      echoListAdd(list, inst);
    }
  }
  if (glyphsLimn) {
    glyphsLimn->setVertexRGBAFromLook = svRGBAfl;
  }
  if (glyphsEcho) {
    split = echoListSplit3(glyphsEcho, list, 10);
    echoObjectAdd(glyphsEcho, split);
  }

  airMopOkay(mop);
  return 0;
}

/*
** Zone from Eval
*/
unsigned int
tenGlyphBqdZoneEval(const double eval[3]) {
  double x, y, z;
  unsigned int zone;

  x = eval[0];
  y = eval[1];
  z = eval[2];
  if (y > 0) {   /* 0 1 2 3 4 */
    if (z > 0) { /* 0 1 */
      if (x - y > y - z) {
        zone = 0;
      } else {
        zone = 1;
      }
    } else {     /* 2 3 4 */
      if (y > -z) {
        zone = 2;
      } else if (x > -z) {
        zone = 3;
      } else {
        zone = 4;
      }
    }
  } else {       /* 5 6 7 8 9 */
    if (x > 0) { /* 5 6 7 */
      if (x > -z) {
        zone = 5;
      } else if (x > -y) {
        zone = 6;
      } else {
        zone = 7;
      }
    } else {     /* 8 9 */
      if (x - y > y - z) {
        zone = 8;
      } else {
        zone = 9;
      }
    }
  }
  return zone;
}

/*
** UV from Eval
*/
void
tenGlyphBqdUvEval(double uv[2], const double eval[3]) {
  double xx, yy, zz, ax, ay, az, mm;

  ax = AIR_ABS(eval[0]);
  ay = AIR_ABS(eval[1]);
  az = AIR_ABS(eval[2]);
  mm = AIR_MAX(ax, AIR_MAX(ay, az));
  if (mm==0) { /* do not divide */
    uv[0]=uv[1]=0;
    return;
  }
  xx = eval[0]/mm;
  yy = eval[1]/mm;
  zz = eval[2]/mm;
  uv[0] = AIR_AFFINE(-1, yy, 1, 0, 1);
  if (xx > -zz) {
    uv[1] = AIR_AFFINE(-1, zz, 1, 0, 1) - uv[0] + 1;
  } else {
    uv[1] = AIR_AFFINE(-1, xx, 1, -1, 0) - uv[0] + 1;
  }
  return;
}

/*
** Eval from UV
*/
void
tenGlyphBqdEvalUv(double eval[3], const double uv[2]) {
  double xx, yy, zz, ll;

  yy = AIR_AFFINE(0, uv[0], 1, -1, 1);
  if (uv[0] + uv[1] > 1) {
    zz = AIR_AFFINE(0, uv[1], 1, -1, 1) - 1 + yy;
    xx = 1;
  } else {
    xx = AIR_AFFINE(0, uv[1], 1, -1, 1) + yy + 1;
    zz = -1;
  }
  ELL_3V_SET(eval, xx, yy, zz);
  ELL_3V_NORM(eval, eval, ll);
  return;
}

/*
** Zone from UV
*/
unsigned int
tenGlyphBqdZoneUv(const double uv[2]) {
  /* the use of "volatile" here, as well as additional variables for
     expressions involving u and v, is based on browsing this summary of the
     subtleties of IEEE 754: http://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
     In this function, "if (u + v > 0.5)" returned one thing for cygwin, and
     something else for other platforms.  Adding volatile and more variables
     for expressions brings cygwin back into line with the other platforms */
  volatile double u, v, upv, tupv;
  unsigned int zone;

  u = uv[0];
  v = uv[1];
  upv = u + v;
  tupv = 2*u + v;
  if (u > 0.5) {       /* 0 1 2 3 4 */
    if (upv > 1.5) { /* 0 1 */
      if (u < v) {
        zone = 0;
      } else {
        zone = 1;
      }
    } else {           /* 2 3 4 */
      if (tupv > 2) {
        zone = 2;
      } else if (upv > 1) {
        zone = 3;
      } else {
        zone = 4;
      }
    }
  } else {             /* 5 6 7 8 9 */
    if (upv > 0.5) { /* 5 6 7 */
      if (upv > 1) {
        zone = 5;
      } else if (tupv > 1) {
        zone = 6;
      } else {
        zone = 7;
      }
    } else {           /* 8 9 */
      if (u < v) {
        zone = 8;
      } else {
        zone = 9;
      }
    }
  }
  return zone;
}

static void
baryFind(double bcoord[3], const double uvp[2],
         const double uv0[2],
         const double uv1[2],
         const double uv2[2]) {
  double mat[9], a, a01, a02, a12;

  ELL_3M_SET(mat,
             uv0[0], uv0[1], 1,
             uv1[0], uv1[1], 1,
             uvp[0], uvp[1], 1);
  a01 = ELL_3M_DET(mat); a01 = AIR_ABS(a01);

  ELL_3M_SET(mat,
             uv0[0], uv0[1], 1,
             uv2[0], uv2[1], 1,
             uvp[0], uvp[1], 1);
  a02 = ELL_3M_DET(mat); a02 = AIR_ABS(a02);

  ELL_3M_SET(mat,
             uv1[0], uv1[1], 1,
             uv2[0], uv2[1], 1,
             uvp[0], uvp[1], 1);
  a12 = ELL_3M_DET(mat); a12 = AIR_ABS(a12);

  a = a01 + a02 + a12;
  ELL_3V_SET(bcoord, a12/a, a02/a, a01/a);
  return;
}

static void
baryBlend(double abc[3], const double co[3],
          const double abc0[3],
          const double abc1[3],
          const double abc2[3]) {
  unsigned int ii;

  for (ii=0; ii<3; ii++) {
    abc[ii] = co[0]*abc0[ii] + co[1]*abc1[ii] + co[2]*abc2[ii];
  }
  return;
}

void
tenGlyphBqdAbcUv(double abc[3], const double uv[2], double betaMax) {
  static const unsigned int vertsZone[10][3] = {{0, 1, 2},   /* 0 */
                                                {0, 2, 3},   /* 1 */
                                                {1, 3, 4},   /* 2 */
                                                {1, 4, 5},   /* 3 */
                                                {4, 5, 9},   /* 4 */
                                                {1, 5, 6},   /* 5 */
                                                {5, 6, 9},   /* 6 */
                                                {6, 7, 9},   /* 7 */
                                                {7, 8, 10},  /* 8 */
                                                {8, 9, 10}}; /* 9 */
  static const double uvVert[11][2] = {{1.00, 1.00},   /* 0 */
                                       {0.50, 1.00},   /* 1 */
                                       {0.75, 0.75},   /* 2 */
                                       {1.00, 0.50},   /* 3 */
                                       {1.00, 0.00},   /* 4 */
                                       {0.50, 0.50},   /* 5 */
                                       {0.00, 1.00},   /* 6 */
                                       {0.00, 0.50},   /* 7 */
                                       {0.25, 0.25},   /* 8 */
                                       {0.50, 0.00},   /* 9 */
                                       {0.00, 0.00}};  /* 10 */
  double abcBall[3], abcCyli[3], abcFunk[3], abcThrn[3],
    abcOcta[3], abcCone[3], abcHalf[3];
  /* old compile-time setting
  const double *abcAll[10][11] = {
     zone \ vert 0      1        2        3        4        5        6        7        8        9       10
      0    {abcBall, abcCyli, abcHalf,  NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL   },
      1    {abcBall,  NULL,   abcHalf, abcCyli,  NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL   },
      2    { NULL,   abcOcta,  NULL,   abcCone, abcThrn,  NULL,    NULL,    NULL,    NULL,    NULL,    NULL   },
      3    { NULL,   abcOcta,  NULL,    NULL,   abcThrn, abcFunk,  NULL,    NULL,    NULL,    NULL,    NULL   },
      4    { NULL,    NULL,    NULL,    NULL,   abcThrn, abcFunk,  NULL,    NULL,    NULL,   abcCone,  NULL   },
      5    { NULL,   abcCone,  NULL,    NULL,    NULL,   abcFunk, abcThrn,  NULL,    NULL,    NULL,    NULL   },
      6    { NULL,    NULL,    NULL,    NULL,    NULL,   abcFunk, abcThrn,  NULL,    NULL,   abcOcta,  NULL   },
      7    { NULL,    NULL,    NULL,    NULL,    NULL,    NULL,   abcThrn, abcCone,  NULL,   abcOcta,  NULL   },
      8    { NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,   abcCyli, abcHalf,  NULL,   abcBall },
      9    { NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,   abcHalf, abcCyli, abcBall }};
  */
  const double *abcAll[10][11];
  unsigned int pvi[3], zone, vert;
  double bcoord[3];

  ELL_3V_SET(abcBall, 1, 1, 1);
  ELL_3V_SET(abcCyli, 1, 0, 0);
  ELL_3V_SET(abcFunk, 0, betaMax, 2);  /* only one with c != b  */
  ELL_3V_SET(abcThrn, 1, betaMax, 3);
  ELL_3V_SET(abcOcta, 0, 2, 2);
  ELL_3V_SET(abcCone, 1, 2, 2);
  ELL_3V_SET(abcHalf, 0.5, 0.5, 0.5); /* alpha is half-way between alpha of
                                         octa and cone and beta has to be
                                         the same as alpha at for the
                                         seam to be shape-continuous */
  /* run-time setting of abcAll[][]; compile-time setting (comments above)
     gives "initializer element is not computable at load time" warnings */
  for (zone=0; zone<10; zone++) {
    for (vert=0; vert<11; vert++) {
      abcAll[zone][vert]=NULL;
    }
  }
#define SET(zi, vi0, vi1, vi2, sh0, sh1, sh2) \
  abcAll[zi][vi0] = abc##sh0; \
  abcAll[zi][vi1] = abc##sh1; \
  abcAll[zi][vi2] = abc##sh2

  SET(0, 0, 1, 2, Ball, Cyli, Half);
  SET(1, 0, 2, 3, Ball, Half, Cyli);
  SET(2, 1, 3, 4, Octa, Cone, Thrn);
  SET(3, 1, 4, 5, Octa, Thrn, Funk);
  SET(4, 4, 5, 9, Thrn, Funk, Cone);
  SET(5, 1, 5, 6, Cone, Funk, Thrn);
  SET(6, 5, 6, 9, Funk, Thrn, Octa);
  SET(7, 6, 7, 9, Thrn, Cone, Octa);
  SET(8, 7, 8,10, Cyli, Half, Ball);
  SET(9, 8, 9,10, Half, Cyli, Ball);

#undef SET

  zone = tenGlyphBqdZoneUv(uv);
  ELL_3V_COPY(pvi, vertsZone[zone]);
  baryFind(bcoord, uv, uvVert[pvi[0]], uvVert[pvi[1]], uvVert[pvi[2]]);
  baryBlend(abc, bcoord,
            abcAll[zone][pvi[0]],
            abcAll[zone][pvi[1]],
            abcAll[zone][pvi[2]]);
  return;
}

