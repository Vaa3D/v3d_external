/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah
  Copyright (C) 2011  Thomas Schultz

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


#include "limn.h"

limnPolyData *
limnPolyDataNew(void) {
  limnPolyData *pld;

  pld = AIR_CALLOC(1, limnPolyData);
  if (pld) {
    pld->xyzw = NULL;
    pld->xyzwNum = 0;
    pld->rgba = NULL;
    pld->rgbaNum = 0;
    pld->norm = NULL;
    pld->normNum = 0;
    pld->tex2 = NULL;
    pld->tex2Num = 0;
    pld->tang = NULL;
    pld->tangNum = 0;
    pld->indx = NULL;
    pld->indxNum = 0;
    pld->primNum = 0;
    pld->type = NULL;
    pld->icnt = NULL;
  }
  return pld;
}

limnPolyData *
limnPolyDataNix(limnPolyData *pld) {

  if (pld) {
    airFree(pld->xyzw);
    airFree(pld->rgba);
    airFree(pld->norm);
    airFree(pld->tex2);
    airFree(pld->tang);
    airFree(pld->indx);
    airFree(pld->type);
    airFree(pld->icnt);
  }
  airFree(pld);
  return NULL;
}

/*
** doesn't set pld->xyzwNum, only the per-attribute xxxNum variables
*/
int
_limnPolyDataInfoAlloc(limnPolyData *pld, unsigned int infoBitFlag,
                       unsigned int vertNum) {
  static const char me[]="_limnPolyDataInfoAlloc";

  if (vertNum != pld->rgbaNum
      && ((1 << limnPolyDataInfoRGBA) & infoBitFlag)) {
    pld->rgba = (unsigned char *)airFree(pld->rgba);
    if (vertNum) {
      pld->rgba = AIR_CALLOC(vertNum*4, unsigned char);
      if (!pld->rgba) {
        biffAddf(LIMN, "%s: couldn't allocate %u rgba", me, vertNum);
        return 1;
      }
    }
    pld->rgbaNum = vertNum;
  }

  if (vertNum != pld->normNum
      && ((1 << limnPolyDataInfoNorm) & infoBitFlag)) {
    pld->norm = (float *)airFree(pld->norm);
    if (vertNum) {
      pld->norm = AIR_CALLOC(vertNum*4, float);
      if (!pld->norm) {
        biffAddf(LIMN, "%s: couldn't allocate %u norm", me, vertNum);
        return 1;
      }
    }
    pld->normNum = vertNum;
  }

  if (vertNum != pld->tex2Num
      && ((1 << limnPolyDataInfoTex2) & infoBitFlag)) {
    pld->tex2 = (float *)airFree(pld->tex2);
    if (vertNum) {
      pld->tex2 = AIR_CALLOC(vertNum*2, float);
      if (!pld->tex2) {
        biffAddf(LIMN, "%s: couldn't allocate %u tex2", me, vertNum);
        return 1;
      }
    }
    pld->tex2Num = vertNum;
  }

  if (vertNum != pld->tangNum
      && ((1 << limnPolyDataInfoTang) & infoBitFlag)) {
    pld->tang = (float *)airFree(pld->tang);
    if (vertNum) {
      pld->tang = AIR_CALLOC(vertNum*3, float);
      if (!pld->tang) {
        biffAddf(LIMN, "%s: couldn't allocate %u tang", me, vertNum);
        return 1;
      }
    }
    pld->tangNum = vertNum;
  }

  return 0;
}

unsigned int
limnPolyDataInfoBitFlag(const limnPolyData *pld) {
  unsigned int ret;

  ret = 0;
  if (pld) {
    if (pld->rgba && pld->rgbaNum == pld->xyzwNum) {
      ret |= (1 << limnPolyDataInfoRGBA);
    }
    if (pld->norm && pld->normNum == pld->xyzwNum) {
      ret |= (1 << limnPolyDataInfoNorm);
    }
    if (pld->tex2 && pld->tex2Num == pld->xyzwNum) {
      ret |= (1 << limnPolyDataInfoTex2);
    }
    if (pld->tang && pld->tangNum == pld->xyzwNum) {
      ret |= (1 << limnPolyDataInfoTang);
    }
  }
  return ret;
}

int
limnPolyDataAlloc(limnPolyData *pld,
                  unsigned int infoBitFlag,
                  unsigned int vertNum,
                  unsigned int indxNum,
                  unsigned int primNum) {
  static const char me[]="limnPolyDataAlloc";

  if (!pld) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if (vertNum != pld->xyzwNum) {
    pld->xyzw = (float *)airFree(pld->xyzw);
    if (vertNum) {
      pld->xyzw = AIR_CALLOC(vertNum*4, float);
      if (!pld->xyzw) {
        biffAddf(LIMN, "%s: couldn't allocate %u xyzw", me, vertNum);
        return 1;
      }
    }
    pld->xyzwNum = vertNum;
  }
  if (_limnPolyDataInfoAlloc(pld, infoBitFlag, vertNum)) {
    biffAddf(LIMN, "%s: couldn't allocate info", me);
    return 1;
  }
  if (indxNum != pld->indxNum) {
    pld->indx = (unsigned int *)airFree(pld->indx);
    if (indxNum) {
      pld->indx = AIR_CALLOC(indxNum, unsigned int);
      if (!pld->indx) {
        biffAddf(LIMN, "%s: couldn't allocate %u indices", me, indxNum);
        return 1;
      }
    }
    pld->indxNum = indxNum;
  }
  if (primNum != pld->primNum) {
    pld->type = (unsigned char *)airFree(pld->type);
    pld->icnt = (unsigned int *)airFree(pld->icnt);
    if (primNum) {
      pld->type = AIR_CALLOC(primNum, unsigned char);
      pld->icnt = AIR_CALLOC(primNum, unsigned int);
      if (!(pld->type && pld->icnt)) {
        biffAddf(LIMN, "%s: couldn't allocate %u primitives", me, primNum);
        return 1;
      }
    }
    pld->primNum = primNum;
  }
  return 0;
}

size_t
limnPolyDataSize(const limnPolyData *pld) {
  size_t ret = 0;

  if (pld) {
    ret += pld->xyzwNum*sizeof(float)*4;
    if (pld->rgba) {
      ret += pld->rgbaNum*sizeof(unsigned char)*4;
    }
    if (pld->norm) {
      ret += pld->normNum*sizeof(float)*3;
    }
    if (pld->tex2) {
      ret += pld->tex2Num*sizeof(float)*2;
    }
    if (pld->tang) {
      ret += pld->tangNum*sizeof(float)*3;
    }
    ret += pld->indxNum*sizeof(unsigned int);
    ret += pld->primNum*sizeof(signed char);
    ret += pld->primNum*sizeof(unsigned int);
  }
  return ret;
}

int
limnPolyDataCopy(limnPolyData *pldB, const limnPolyData *pldA) {
  static const char me[]="limnPolyDataCopy";

  if (!( pldB && pldA )) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if (limnPolyDataAlloc(pldB, limnPolyDataInfoBitFlag(pldA),
                        pldA->xyzwNum, pldA->indxNum, pldA->primNum)) {
    biffAddf(LIMN, "%s: couldn't allocate output", me);
    return 1;
  }
  memcpy(pldB->xyzw, pldA->xyzw, pldA->xyzwNum*sizeof(float)*4);
  if (pldA->rgba) {
    memcpy(pldB->rgba, pldA->rgba, pldA->rgbaNum*sizeof(unsigned char)*4);
  }
  if (pldA->norm) {
    memcpy(pldB->norm, pldA->norm, pldA->normNum*sizeof(float)*3);
  }
  if (pldA->tex2) {
    memcpy(pldB->tex2, pldA->tex2, pldA->tex2Num*sizeof(float)*2);
  }
  if (pldA->tang) {
    memcpy(pldB->tang, pldA->tang, pldA->tangNum*sizeof(float)*3);
  }
  memcpy(pldB->indx, pldA->indx, pldA->indxNum*sizeof(unsigned int));
  memcpy(pldB->type, pldA->type, pldA->primNum*sizeof(signed char));
  memcpy(pldB->icnt, pldA->icnt, pldA->primNum*sizeof(unsigned int));
  return 0;
}

int
limnPolyDataCopyN(limnPolyData *pldB, const limnPolyData *pldA,
                  unsigned int num) {
  static const char me[]="limnPolyDataCopyN";
  unsigned int ii, jj, size;

  if (!( pldB && pldA )) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if (limnPolyDataAlloc(pldB, limnPolyDataInfoBitFlag(pldA),
                        num*pldA->xyzwNum,
                        num*pldA->indxNum,
                        num*pldA->primNum)) {
    biffAddf(LIMN, "%s: couldn't allocate output", me);
    return 1;
  }
  for (ii=0; ii<num; ii++) {
    /* fprintf(stderr, "!%s: ii = %u/%u\n", me, ii, num); */
    size = pldA->xyzwNum*4;
    /*
    char *_beg = (char *)(pldB->xyzw + ii*size);
    char *_end = _beg + size - 1;
    fprintf(stderr, "!%s: memcpy(%p+%u=%p,%u) --> [%p,%p] inside: %d %d\n", me,
            pldB->xyzw, ii*size, pldB->xyzw + ii*size, size,
            _beg, _end, AIR_IN_CL(_xyzwBeg, _beg, _xyzwEnd),
            AIR_IN_CL(_xyzwBeg, _end, _xyzwEnd));
    */
    memcpy(pldB->xyzw + ii*size, pldA->xyzw, size*sizeof(float));
    for (jj=0; jj<pldA->indxNum; jj++) {
      (pldB->indx + ii*pldA->indxNum)[jj] = pldA->indx[jj] + ii*pldA->xyzwNum;
    }
    size = pldA->primNum;
    memcpy(pldB->type + ii*size, pldA->type, size*sizeof(unsigned char));
    memcpy(pldB->icnt + ii*size, pldA->icnt, size*sizeof(unsigned int));
    if (pldA->rgba) {
      size = pldA->rgbaNum*4;
      memcpy(pldB->rgba + ii*size, pldA->rgba, size*sizeof(unsigned char));
    }
    if (pldA->norm) {
      size = pldA->normNum*3;
      memcpy(pldB->norm + ii*size, pldA->norm, size*sizeof(float));
    }
    if (pldA->tex2) {
      size = pldA->tex2Num*2;
      memcpy(pldB->tex2 + ii*size, pldA->tex2, size*sizeof(float));
    }
    if (pldA->tang) {
      size = pldA->tangNum*3;
      memcpy(pldB->tang + ii*size, pldA->tang, size*sizeof(float));
    }
  }
  return 0;
}

/*
******** limnPolyDataTransform_f, limnPolyDataTransform_d
**
** transforms a surface (both positions, and normals (if set))
** by given homogenous transform
*/
void
limnPolyDataTransform_f(limnPolyData *pld,
                        const float homat[16]) {
  double hovec[4], mat[9], inv[9], norm[3], tang[3], nmat[9], tlen;
  unsigned int vertIdx;

  if (pld && homat) {
    ELL_34M_EXTRACT(mat, homat);
    if (pld->norm) {
      ell_3m_inv_d(inv, mat);
      ELL_3M_TRANSPOSE(nmat, inv);
    } else {
      ELL_3M_IDENTITY_SET(nmat);  /* hush unused value compiler warnings */
    }
    for (vertIdx=0; vertIdx<pld->xyzwNum; vertIdx++) {
      ELL_4MV_MUL(hovec, homat, pld->xyzw + 4*vertIdx);
      ELL_4V_COPY_TT(pld->xyzw + 4*vertIdx, float, hovec);
      if (pld->norm) {
        ELL_3MV_MUL(norm, nmat, pld->norm + 3*vertIdx);
        ELL_3V_NORM(norm, norm, tlen);
        ELL_3V_COPY_TT(pld->norm + 3*vertIdx, float, norm);
      }
      if (pld->tang) { /* HEY: not tested */
        ELL_3MV_MUL(tang, mat, pld->tang + 3*vertIdx);
        ELL_3V_NORM(tang, tang, tlen);
        ELL_3V_COPY_TT(pld->tang + 3*vertIdx, float, tang);
      }
    }
  }
  return;
}

/* !!! COPY AND PASTE !!!  (except for double homat[16]) */
void
limnPolyDataTransform_d(limnPolyData *pld, const double homat[16]) {
  double hovec[4], mat[9], inv[9], norm[3], nmat[9];
  unsigned int vertIdx;

  if (pld && homat) {
    if (pld->norm) {
      ELL_34M_EXTRACT(mat, homat);
      ell_3m_inv_d(inv, mat);
      ELL_3M_TRANSPOSE(nmat, inv);
    } else {
      ELL_3M_IDENTITY_SET(nmat);  /* hush unused value compiler warnings */
    }
    for (vertIdx=0; vertIdx<pld->xyzwNum; vertIdx++) {
      ELL_4MV_MUL(hovec, homat, pld->xyzw + 4*vertIdx);
      ELL_4V_COPY_TT(pld->xyzw + 4*vertIdx, float, hovec);
      if (pld->norm) {
        ELL_3MV_MUL(norm, nmat, pld->norm + 3*vertIdx);
        ELL_3V_COPY_TT(pld->norm + 3*vertIdx, float, norm);
      }
    }
  }
  return;
}

unsigned int
limnPolyDataPolygonNumber(const limnPolyData *pld) {
  unsigned int ret, primIdx;

  ret = 0;
  if (pld) {
    for (primIdx=0; primIdx<pld->primNum; primIdx++) {
      switch(pld->type[primIdx]) {
      case limnPrimitiveNoop:
        /* no increment */
        break;
      case limnPrimitiveTriangles:
        ret += pld->icnt[primIdx]/3;
        break;
      case limnPrimitiveTriangleStrip:
      case limnPrimitiveTriangleFan:
        ret += pld->icnt[primIdx] - 2;
        break;
      case limnPrimitiveQuads:
        ret += pld->icnt[primIdx]/4;
        break;
      }
    }
  }
  return ret;
}

static int
limnPolyDataVertexNormals_(limnPolyData *pld, int nonorient) {
  static const char me[]="limnPolyDataVertexNormals_";
  unsigned int infoBitFlag, primIdx, triIdx, normIdx, baseVertIdx;
  double len, *matrix=NULL;
  airArray *mop;

  infoBitFlag = limnPolyDataInfoBitFlag(pld);
  if (limnPolyDataAlloc(pld,
                        infoBitFlag | (1 << limnPolyDataInfoNorm),
                        pld->xyzwNum,
                        pld->indxNum,
                        pld->primNum)) {
    biffAddf(LIMN, "%s: couldn't alloc polydata normals", me);
    return 1;
  }

  mop = airMopNew();
  if (nonorient) { /* we will accumulate outer products */
    matrix = (double *) malloc(sizeof(double)*9*pld->xyzwNum);
    if (matrix==NULL) {
      biffAddf(LIMN, "%s: couldn't allocate matrix buffer", me);
      return 1;
    }
    airMopAdd(mop, matrix, airFree, airMopAlways);
    for (normIdx=0; normIdx<pld->normNum; normIdx++) {
      ELL_3M_ZERO_SET(matrix + 9*normIdx);
    }
  } else {
    for (normIdx=0; normIdx<pld->normNum; normIdx++) {
      ELL_3V_SET(pld->norm + 3*normIdx, 0, 0, 0);
    }
  }

  baseVertIdx = 0;
  for (primIdx=0; primIdx<pld->primNum; primIdx++) {
    unsigned int triNum, indxLine[3]={0,0,0}, ii;
    float pos[3][3], edgeA[3], edgeB[3], norm[3];

    switch (pld->type[primIdx]) {
    case limnPrimitiveTriangles:
      triNum = pld->icnt[primIdx]/3;
      break;
    case limnPrimitiveTriangleStrip:
    case limnPrimitiveTriangleFan:
      triNum = pld->icnt[primIdx]-2;
      break;
    case limnPrimitiveNoop:
      triNum = 0;
      break;
    default:
      biffAddf(LIMN, "%s: came across unsupported limnPrimitive \"%s\"", me,
               airEnumStr(limnPrimitive, pld->type[primIdx]));
      airMopError(mop);
      return 1;
    }

    if (limnPrimitiveNoop != pld->type[primIdx]) {
      for (triIdx=0; triIdx<triNum; triIdx++) {
        switch (pld->type[primIdx]) {
        case limnPrimitiveTriangles:
          ELL_3V_COPY(indxLine, pld->indx + baseVertIdx + 3*triIdx);
          break;
        case limnPrimitiveTriangleStrip:
          if (triIdx%2==0) {
            ELL_3V_COPY(indxLine, pld->indx+baseVertIdx+triIdx);
          } else {
            ELL_3V_SET(indxLine, pld->indx[baseVertIdx+triIdx+1],
                       pld->indx[baseVertIdx+triIdx],
                       pld->indx[baseVertIdx+triIdx+2]);
          }
          break;
        case limnPrimitiveTriangleFan:
          ELL_3V_SET(indxLine, pld->indx[baseVertIdx],
                     pld->indx[baseVertIdx+triIdx+1],
                     pld->indx[baseVertIdx+triIdx+2]);
          break;
        }
        for (ii=0; ii<3; ii++) {
          ELL_34V_HOMOG(pos[ii], pld->xyzw + 4*indxLine[ii]);
        }
        ELL_3V_SUB(edgeA, pos[1], pos[0]);
        ELL_3V_SUB(edgeB, pos[2], pos[0]);
        ELL_3V_CROSS(norm, edgeA, edgeB);
        /* Adding cross products without any normalization is
         * equivalent to weighting by triangle area, as proposed
         * (among others) by G. Taubin ("Estimating the tensor of
         * curvature of a surface from a polyhedral approximation",
         * ICCV 1995). This is efficient, avoids trouble with
         * degenerate triangles and gives reasonable results in
         * practice. */
        if (nonorient) {
          double outer[9];
          ELL_3MV_OUTER(outer, norm, norm);
          len = ELL_3V_LEN(norm);
          /* re-scale so that we don't weight by square of area */
          for (ii=0; ii<3; ii++) {
            ELL_3M_SCALE_INCR(matrix+9*indxLine[ii], 1.0/len, outer);
          }
        } else {
          for (ii=0; ii<3; ii++) {
            ELL_3V_INCR(pld->norm + 3*indxLine[ii], norm);
          }
        }
      }
    }
    baseVertIdx += pld->icnt[primIdx];
  }

  if (nonorient) {
    double eval[3], evec[9];
    for (normIdx=0; normIdx<pld->normNum; normIdx++) {
      ell_3m_eigensolve_d(eval, evec, matrix+9*normIdx, AIR_TRUE);
      ELL_3V_COPY_TT(pld->norm + 3*normIdx, float, evec);
    }
  } else {
    for (normIdx=0; normIdx<pld->normNum; normIdx++) {
      ELL_3V_NORM_TT(pld->norm + 3*normIdx, float, pld->norm + 3*normIdx, len);
    }
  }

  airMopOkay(mop);
  return 0;
}

int
limnPolyDataVertexNormals(limnPolyData *pld) {
  return limnPolyDataVertexNormals_(pld, 0);
}

/* counterpart for non-orientable surfaces */
int
limnPolyDataVertexNormalsNO(limnPolyData *pld) {
  return limnPolyDataVertexNormals_(pld, 1);
}

unsigned int
limnPolyDataPrimitiveTypes(const limnPolyData *pld) {
  unsigned int ret, primIdx;

  ret = 0;
  if (pld) {
    for (primIdx=0; primIdx<pld->primNum; primIdx++) {
      ret |= (1 << pld->type[primIdx]);
    }
  }
  return ret;
}

int
limnPolyDataPrimitiveVertexNumber(Nrrd *nout, limnPolyData *pld) {
  static const char me[]="limnPolyDataPrimitiveVertexNumber";
  unsigned int *vnum, pidx;

  if (!(nout && pld)) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if (nrrdMaybeAlloc_va(nout, nrrdTypeUInt, 1,
                        AIR_CAST(size_t, pld->primNum))) {
    biffMovef(LIMN, NRRD, "%s: couldn't allocate output", me);
    return 1;
  }

  vnum = AIR_CAST(unsigned int *, nout->data);
  for (pidx=0; pidx<pld->primNum; pidx++) {
    vnum[pidx] = pld->icnt[pidx];
  }

  return 0;
}

int
limnPolyDataPrimitiveArea(Nrrd *nout, limnPolyData *pld) {
  static const char me[]="limnPolyDataPrimitiveArea";
  unsigned int primIdx, baseVertIdx;
  unsigned int triNum, triIdx, *indx, ii;
  float vert[3][3], edgeA[3], edgeB[3], cross[3];
  double *area;

  if (!(nout && pld)) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if (nrrdMaybeAlloc_va(nout, nrrdTypeDouble, 1,
                        AIR_CAST(size_t, pld->primNum))) {
    biffMovef(LIMN, NRRD, "%s: couldn't allocate output", me);
    return 1;
  }

  area = AIR_CAST(double *, nout->data);
  baseVertIdx = 0;
  for (primIdx=0; primIdx<pld->primNum; primIdx++) {
    area[primIdx] = 0;
    switch (pld->type[primIdx]) {
    case limnPrimitiveNoop:
      area[primIdx] = 0.0;
      break;
    case limnPrimitiveTriangles:
      triNum = pld->icnt[primIdx]/3;
      for (triIdx=0; triIdx<triNum; triIdx++) {
        indx = pld->indx + baseVertIdx + 3*triIdx;
        for (ii=0; ii<3; ii++) {
          ELL_34V_HOMOG(vert[ii], pld->xyzw + 4*indx[ii]);
        }
        ELL_3V_SUB(edgeA, vert[1], vert[0]);
        ELL_3V_SUB(edgeB, vert[2], vert[0]);
        ELL_3V_CROSS(cross, edgeA, edgeB);
        area[primIdx] += ELL_3V_LEN(cross)/2;
      }
      break;
    case limnPrimitiveTriangleStrip:
    case limnPrimitiveTriangleFan:
    case limnPrimitiveQuads:
      biffAddf(LIMN,
               "%s: sorry, haven't implemented area(prim[%u]=%s) yet", me,
               primIdx, airEnumStr(limnPrimitive, pld->type[primIdx]));
      return 1;
      break;
    case limnPrimitiveLineStrip:
      /* lines have no area */
      break;
    }
    baseVertIdx += pld->icnt[primIdx];
  }

  return 0;
}

/*
** I may regret making this only be axis-aligned ...
*/
int
limnPolyDataRasterize(Nrrd *nout, limnPolyData *pld,
                      double min[3], double max[3],
                      size_t size[3], int type) {
  static const char me[]="limnPolyDataRasterize";
  size_t xi, yi, zi;
  unsigned int vertIdx;
  double (*ins)(void *, size_t, double);

  if (!(nout && pld && min && max && size)) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(nrrdType, type)) {
    biffAddf(LIMN, "%s: got invalid %s %d", me, nrrdType->name, type);
    return 1;
  }
  if (nrrdTypeBlock == type) {
    biffAddf(LIMN, "%s: can't use output type %s", me,
             airEnumStr(nrrdType, type));
    return 1;
  }
  if (!( min[0] < max[0] &&
         min[1] < max[1] &&
         min[2] < max[2] )) {
    biffAddf(LIMN, "%s min (%g,%g,%g) not < max (%g,%g,%g)", me,
             min[0], min[1], min[2], max[0], max[1], max[2]);
    return 1;
  }

  if (nrrdMaybeAlloc_nva(nout, type, 3, size)) {
    biffMovef(LIMN, NRRD, "%s: trouble allocating output", me);
    return 1;
  }
  ins = nrrdDInsert[type];

  for (vertIdx=0; vertIdx<pld->xyzwNum; vertIdx++) {
    double xyz[3];

    ELL_34V_HOMOG(xyz, pld->xyzw + 4*vertIdx);
    if (!( AIR_IN_OP(min[0], xyz[0], max[0]) &&
           AIR_IN_OP(min[1], xyz[1], max[1]) &&
           AIR_IN_OP(min[2], xyz[2], max[2]) )) {
      continue;
    }
    xi = airIndex(min[0], xyz[0], max[0], size[0]);
    yi = airIndex(min[1], xyz[1], max[1], size[1]);
    zi = airIndex(min[2], xyz[2], max[2], size[2]);
    ins(nout->data, xi + size[0]*(yi + size[1]*zi), 1.0);
  }

  nrrdAxisInfoSet_nva(nout, nrrdAxisInfoMin, min);
  nrrdAxisInfoSet_nva(nout, nrrdAxisInfoMax, max);

  return 0;
}

void
limnPolyDataColorSet(limnPolyData *pld,
                     unsigned char RR, unsigned char GG,
                     unsigned char BB, unsigned char AA) {
  unsigned int vertIdx;

  if (pld && ((1 << limnPolyDataInfoRGBA) & limnPolyDataInfoBitFlag(pld))) {
    for (vertIdx=0; vertIdx<pld->rgbaNum; vertIdx++) {
      ELL_4V_SET(pld->rgba + 4*vertIdx, RR, GG, BB, AA);
    }
  }
  return;
}
