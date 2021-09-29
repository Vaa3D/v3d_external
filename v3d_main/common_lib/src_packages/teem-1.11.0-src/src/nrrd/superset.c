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

#include "nrrd.h"
#include "privateNrrd.h"

/*
******** nrrdSplice()
**
** (opposite of nrrdSlice): replaces one slice of a nrrd with
** another nrrd.  Will allocate memory for output only if nout != nin.
*/
int
nrrdSplice(Nrrd *nout, const Nrrd *nin, const Nrrd *nslice,
           unsigned int axis, size_t pos) {
  static const char me[]="nrrdSplice", func[]="splice";
  size_t
    I,
    rowLen,                  /* length of segment */
    colStep,                 /* distance between start of each segment */
    colLen;                  /* number of periods */
  unsigned int ai;
  char *src, *dest, *sliceCont;
  char stmp[2][AIR_STRLEN_SMALL];

  if (!(nin && nout && nslice)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nout == nslice) {
    biffAddf(NRRD, "%s: nout==nslice disallowed", me);
    return 1;
  }

  /* check that desired slice location is legit */
  if (!( axis < nin->dim )) {
    biffAddf(NRRD, "%s: slice axis %d out of bounds (0 to %d)",
             me, axis, nin->dim-1);
    return 1;
  }
  if (!( pos < nin->axis[axis].size )) {
    biffAddf(NRRD, "%s: position %s out of bounds (0 to %s)", me,
             airSprintSize_t(stmp[0], pos),
             airSprintSize_t(stmp[1], nin->axis[axis].size-1));
    return 1;
  }

  /* check that slice will fit in nin */
  if (nrrdCheck(nslice) || nrrdCheck(nin)) {
    biffAddf(NRRD, "%s: input or slice not valid nrrd", me);
    return 1;
  }
  if (!( nin->dim-1 == nslice->dim )) {
    biffAddf(NRRD, "%s: dim of slice (%d) not one less than "
             "dim of input (%d)", me, nslice->dim, nin->dim);
    return 1;
  }
  if (!( nin->type == nslice->type )) {
    biffAddf(NRRD, "%s: type of slice (%s) != type of input (%s)",
             me, airEnumStr(nrrdType, nslice->type),
             airEnumStr(nrrdType, nin->type));
    return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    if (!( nin->blockSize == nslice->blockSize )) {
      biffAddf(NRRD, "%s: input's blockSize (%s) != subvolume's (%s)", me,
               airSprintSize_t(stmp[0], nin->blockSize),
               airSprintSize_t(stmp[1], nslice->blockSize));
      return 1;
    }
  }
  for (ai=0; ai<nslice->dim; ai++) {
    if (!( nin->axis[ai + (ai >= axis)].size == nslice->axis[ai].size )) {
      biffAddf(NRRD, "%s: input ax %d size (%s) != slices ax %d size (%s)",
               me, ai + (ai >= axis),
               airSprintSize_t(stmp[0], nin->axis[ai + (ai >= axis)].size), ai,
               airSprintSize_t(stmp[1], nslice->axis[ai].size));
      return 1;
    }
  }

  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      biffAddf(NRRD, "%s:", me);
      return 1;
    }
  }
  /* else we're going to splice in place */

  /* the following was copied from nrrdSlice() */
  /* set up control variables */
  rowLen = colLen = 1;
  for (ai=0; ai<nin->dim; ai++) {
    if (ai < axis) {
      rowLen *= nin->axis[ai].size;
    } else if (ai > axis) {
      colLen *= nin->axis[ai].size;
    }
  }
  rowLen *= nrrdElementSize(nin);
  colStep = rowLen*nin->axis[axis].size;

  /* the skinny */
  src = (char *)nout->data;    /* switched src,dest from nrrdSlice() */
  dest = (char *)nslice->data;
  src += rowLen*pos;
  for (I=0; I<colLen; I++) {
    /* HEY: replace with AIR_MEMCPY() or similar, when applicable */
    memcpy(src, dest, rowLen);  /* switched src,dest from nrrdSlice() */
    src += colStep;
    dest += rowLen;
  }

  sliceCont = _nrrdContentGet(nslice);
  if (nrrdContentSet_va(nout, func, nin, "%s,%d,%s", sliceCont, axis,
                        airSprintSize_t(stmp[0], pos))) {
    biffAddf(NRRD, "%s:", me);
    free(sliceCont); return 1;
  }
  free(sliceCont);
  /* basic info copied by nrrdCopy above */

  return 0;
}

/*
******** nrrdInset()
**
** (opposite of nrrdCrop()) replace some sub-volume inside a nrrd with
** another given nrrd.
**
*/
int
nrrdInset(Nrrd *nout, const Nrrd *nin, const Nrrd *nsub, const size_t *min) {
  static const char me[]="nrrdInset", func[] = "inset";
  char buff1[NRRD_DIM_MAX*30], buff2[AIR_STRLEN_SMALL];
  unsigned int ai;
  size_t I,
    lineSize,                /* #bytes in one scanline to be copied */
    typeSize,                /* size of data type */
    cIn[NRRD_DIM_MAX],       /* coords for line start, in input */
    cOut[NRRD_DIM_MAX],      /* coords for line start, in output */
    szIn[NRRD_DIM_MAX],
    szOut[NRRD_DIM_MAX],
    idxIn, idxOut,           /* linear indices for input and output */
    numLines;                /* number of scanlines in output nrrd */
  char *dataIn, *dataOut, *subCont, stmp[3][AIR_STRLEN_SMALL];

  /* errors */
  if (!(nout && nin && nsub && min)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nout == nsub) {
    biffAddf(NRRD, "%s: nout==nsub disallowed", me);
    return 1;
  }
  if (nrrdCheck(nin)) {
    biffAddf(NRRD, "%s: input not valid nrrd", me);
    return 1;
  }
  if (nrrdCheck(nsub)) {
    biffAddf(NRRD, "%s: subvolume not valid nrrd", me);
    return 1;
  }
  if (!( nin->dim == nsub->dim )) {
    biffAddf(NRRD, "%s: input's dim (%d) != subvolume's dim (%d)",
             me, nin->dim, nsub->dim);
    return 1;
  }
  if (!( nin->type == nsub->type )) {
    biffAddf(NRRD, "%s: input's type (%s) != subvolume's type (%s)", me,
             airEnumStr(nrrdType, nin->type),
             airEnumStr(nrrdType, nsub->type));
    return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    if (!( nin->blockSize == nsub->blockSize )) {
      biffAddf(NRRD, "%s: input's blockSize (%s) != subvolume's (%s)", me,
               airSprintSize_t(stmp[0], nin->blockSize),
               airSprintSize_t(stmp[1], nsub->blockSize));
      return 1;
    }
  }
  for (ai=0; ai<nin->dim; ai++) {
    if (!( min[ai] + nsub->axis[ai].size - 1 <= nin->axis[ai].size - 1)) {
      biffAddf(NRRD, "%s: axis %d range of inset indices [%s,%s] not within "
               "input indices [0,%s]", me, ai,
               airSprintSize_t(stmp[0], min[ai]),
               airSprintSize_t(stmp[1], min[ai] + nsub->axis[ai].size - 1),
               airSprintSize_t(stmp[2], nin->axis[ai].size - 1));
      return 1;
    }
  }

  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      biffAddf(NRRD, "%s:", me);
      return 1;
    }
  }
  /* else we're going to inset in place */

  /* WARNING: following code copied/modified from nrrdCrop(),
     so the meanings of "in"/"out", "src"/"dest" are all messed up */

  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, szIn);
  nrrdAxisInfoGet_nva(nsub, nrrdAxisInfoSize, szOut);
  numLines = 1;
  for (ai=1; ai<nin->dim; ai++) {
    numLines *= szOut[ai];
  }
  lineSize = szOut[0]*nrrdElementSize(nin);

  /* the skinny */
  typeSize = nrrdElementSize(nin);
  dataIn = (char *)nout->data;
  dataOut = (char *)nsub->data;
  for (ai=0; ai<NRRD_DIM_MAX; ai++) {
    cOut[ai] = 0;
  }
  for (I=0; I<numLines; I++) {
    for (ai=0; ai<nin->dim; ai++) {
      cIn[ai] = cOut[ai] + min[ai];
    }
    NRRD_INDEX_GEN(idxOut, cOut, szOut, nin->dim);
    NRRD_INDEX_GEN(idxIn, cIn, szIn, nin->dim);
    memcpy(dataIn + idxIn*typeSize, dataOut + idxOut*typeSize, lineSize);
    /* the lowest coordinate in cOut[] will stay zero, since we are
       copying one (1-D) scanline at a time */
    NRRD_COORD_INCR(cOut, szOut, nin->dim, 1);
  }

  /* HEY: before Teem version 2.0 figure out nrrdKind stuff here */

  strcpy(buff1, "[");
  for (ai=0; ai<nin->dim; ai++) {
    sprintf(buff2, "%s%s", (ai ? "," : ""),
            airSprintSize_t(stmp[0], min[ai]));
    strcat(buff1, buff2);
  }
  strcat(buff1, "]");
  subCont = _nrrdContentGet(nsub);
  if (nrrdContentSet_va(nout, func, nin, "%s,%s", subCont, buff1)) {
    biffAddf(NRRD, "%s:", me);
    free(subCont); return 1;
  }
  free(subCont);
  /* basic info copied by nrrdCopy above */

  return 0;
}

#define MIRROR(N, I, M) \
  M = (I < 0 ? -I : I); \
  M = M % (2*N);        \
  M = (M >= N           \
       ? 2*N - 1 - M    \
       : M)             \

size_t
_nrrdMirror_64(size_t N, ptrdiff_t I) {
  size_t M;

  M = (I < 0 ? -I : I);
  M = M % (2*N);
  M = (M >= N
       ? 2*N - 1 - M
       : M);
  return M;
}

unsigned int
_nrrdMirror_32(unsigned int N, int I) {
  unsigned int M;

  M = (I < 0 ? -I : I);
  M = M % (2*N);
  M = (M >= N
       ? 2*N - 1 - M
       : M);
  return M;
}

/*
******** nrrdPad_va()
**
** strictly for padding
*/
int
nrrdPad_va(Nrrd *nout, const Nrrd *nin,
           const ptrdiff_t *min, const ptrdiff_t *max, int boundary, ...) {
  static const char me[]="nrrdPad_va", func[]="pad";
  char buff1[NRRD_DIM_MAX*30], buff2[AIR_STRLEN_MED];
  double padValue=AIR_NAN;
  int outside; /* whether current sample in output has any coordinates
                  that are outside the input volume (this is per-sample,
                  not per-axis) */
  unsigned int ai;
  ptrdiff_t
    cIn[NRRD_DIM_MAX];       /* coords for line start, in input */
  size_t
    typeSize,
    idxIn, idxOut,           /* linear indices for input and output */
    numOut,                  /* number of elements in output nrrd */
    szIn[NRRD_DIM_MAX],
    szOut[NRRD_DIM_MAX],
    cOut[NRRD_DIM_MAX];      /* coords for line start, in output */
  va_list ap;
  char *dataIn, *dataOut;
  char stmp[2][AIR_STRLEN_SMALL];

  if (!(nout && nin && min && max)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nout == nin) {
    biffAddf(NRRD, "%s: nout==nin disallowed", me);
    return 1;
  }
  if (!AIR_IN_OP(nrrdBoundaryUnknown, boundary, nrrdBoundaryLast)) {
    biffAddf(NRRD, "%s: boundary behavior %d invalid", me, boundary);
    return 1;
  }
  if (nrrdBoundaryWeight == boundary) {
    biffAddf(NRRD, "%s: boundary strategy %s not applicable here", me,
             airEnumStr(nrrdBoundary, boundary));
    return 1;
  }
  if (nrrdTypeBlock == nin->type && nrrdBoundaryPad == boundary) {
    biffAddf(NRRD, "%s: with nrrd type %s, boundary %s not valid", me,
             airEnumStr(nrrdType, nrrdTypeBlock),
             airEnumStr(nrrdBoundary, nrrdBoundaryPad));
    return 1;
  }
  va_start(ap, boundary);
  if (nrrdBoundaryPad == boundary) {
    padValue = va_arg(ap, double);
  }
  va_end(ap);
  switch(boundary) {
  case nrrdBoundaryPad:
  case nrrdBoundaryBleed:
  case nrrdBoundaryWrap:
  case nrrdBoundaryMirror:
    break;
  default:
    biffAddf(NRRD, "%s: sorry boundary %s (%d) unimplemented\n",
             me, airEnumStr(nrrdBoundary, boundary), boundary);
    return 1;
  }
  /*
  printf("!%s: boundary = %d, padValue = %g\n", me, boundary, padValue);
  */

  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, szIn);
  for (ai=0; ai<nin->dim; ai++) {
    if (!( min[ai] <= 0 )) {
      biffAddf(NRRD, "%s: axis %d min (%s) not <= 0", me, ai,
               airSprintPtrdiff_t(stmp[0], min[ai]));
      return 1;
    }
    if (!( (size_t)max[ai] >= szIn[ai]-1)) {
      biffAddf(NRRD, "%s: axis %d max (%s) not >= size-1 (%s)", me, ai,
               airSprintPtrdiff_t(stmp[0], max[ai]),
               airSprintSize_t(stmp[1], szIn[ai]-1));
      return 1;
    }
  }
  /* this shouldn't actually be necessary .. */
  if (!nrrdElementSize(nin)) {
    biffAddf(NRRD, "%s: nrrd reports zero element size!", me);
    return 1;
  }

  /* allocate */
  numOut = 1;
  for (ai=0; ai<nin->dim; ai++) {
    numOut *= (szOut[ai] = -min[ai] + max[ai] + 1);
  }
  nout->blockSize = nin->blockSize;
  if (nrrdMaybeAlloc_nva(nout, nin->type, nin->dim, szOut)) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }

  /* the skinny */
  typeSize = nrrdElementSize(nin);
  dataIn = (char *)nin->data;
  dataOut = (char *)nout->data;
  for (ai=0; ai<NRRD_DIM_MAX; ai++) {
    cOut[ai] = 0;
  }
  for (idxOut=0; idxOut<numOut; idxOut++) {
    outside = 0;
    for (ai=0; ai<nin->dim; ai++) {
      cIn[ai] = cOut[ai] + min[ai];
      switch(boundary) {
      case nrrdBoundaryPad: /* HEY, this shouldn't result in any
                               input coordinate being set */
      case nrrdBoundaryBleed:
        if (!AIR_IN_CL(0, cIn[ai], (ptrdiff_t)szIn[ai]-1)) {
          cIn[ai] = AIR_CLAMP(0, cIn[ai], (ptrdiff_t)szIn[ai]-1);
          outside = 1;
        }
        break;
      case nrrdBoundaryWrap:
        if (!AIR_IN_CL(0, cIn[ai], (ptrdiff_t)szIn[ai]-1)) {
          /* HEY: shouldn't have to cast szIn[ai] to ptrdiff_t */
          cIn[ai] = AIR_MOD(cIn[ai], (ptrdiff_t)szIn[ai]);
          outside = 1;
        }
        break;
      case nrrdBoundaryMirror:
        if (!AIR_IN_CL(0, cIn[ai], (ptrdiff_t)szIn[ai]-1)) {
          cIn[ai] = _nrrdMirror_64(szIn[ai], cIn[ai]);
          outside = 1;
        }
        break;
      }
    }
    NRRD_INDEX_GEN(idxIn, cIn, szIn, nin->dim);
    if (!outside) {
      /* the cIn coords are within the input nrrd: do memcpy() of whole
         1-D scanline, then artificially bump for-loop to the end of
         the scanline */
      memcpy(dataOut + idxOut*typeSize, dataIn + idxIn*typeSize,
             szIn[0]*typeSize);
      idxOut += nin->axis[0].size-1;
      cOut[0] += nin->axis[0].size-1;
    } else {
      /* we copy only a single value */
      if (nrrdBoundaryPad == boundary) {
        nrrdDInsert[nout->type](dataOut, idxOut, padValue);
      } else {
        memcpy(dataOut + idxOut*typeSize, dataIn + idxIn*typeSize, typeSize);
      }
    }
    NRRD_COORD_INCR(cOut, szOut, nin->dim, 0);
  }
  if (nrrdAxisInfoCopy(nout, nin, NULL, (NRRD_AXIS_INFO_SIZE_BIT |
                                         NRRD_AXIS_INFO_MIN_BIT |
                                         NRRD_AXIS_INFO_MAX_BIT ))) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  for (ai=0; ai<nin->dim; ai++) {
    nrrdAxisInfoPosRange(&(nout->axis[ai].min), &(nout->axis[ai].max),
                         nin, ai, AIR_CAST(double, min[ai]),
                         AIR_CAST(double, max[ai]));
    if (!nrrdStateKindNoop && nout->axis[ai].size == nin->axis[ai].size) {
      /* nothing changed along this axis; the kind should be preserved */
      nout->axis[ai].kind = nin->axis[ai].kind;
    } else {
      nout->axis[ai].kind = _nrrdKindAltered(nin->axis[ai].kind, AIR_FALSE);
    }
  }
  strcpy(buff1, "");
  for (ai=0; ai<nin->dim; ai++) {
    sprintf(buff2, "%s[%s,%s]", (ai ? "x" : ""),
            airSprintPtrdiff_t(stmp[0], min[ai]),
            airSprintPtrdiff_t(stmp[1], max[ai]));
    strcat(buff1, buff2);
  }
  if (nrrdBoundaryPad == boundary) {
    sprintf(buff2, "%s(%g)", airEnumStr(nrrdBoundary, nrrdBoundaryPad),
            padValue);
  } else {
    strcpy(buff2, airEnumStr(nrrdBoundary, boundary));
  }
  if (nrrdContentSet_va(nout, func, nin, "%s,%s", buff1, buff2)) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  if (nrrdBasicInfoCopy(nout, nin,
                        NRRD_BASIC_INFO_DATA_BIT
                        | NRRD_BASIC_INFO_TYPE_BIT
                        | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                        | NRRD_BASIC_INFO_DIMENSION_BIT
                        | NRRD_BASIC_INFO_CONTENT_BIT
                        | NRRD_BASIC_INFO_COMMENTS_BIT
                        | (nrrdStateKeyValuePairsPropagate
                           ? 0
                           : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  /* copy origin, then shift it along the spatial axes */
  nrrdSpaceVecCopy(nout->spaceOrigin, nin->spaceOrigin);
  for (ai=0; ai<nin->dim; ai++) {
    if (AIR_EXISTS(nin->axis[ai].spaceDirection[0])) {
      nrrdSpaceVecScaleAdd2(nout->spaceOrigin,
                            1.0, nout->spaceOrigin,
                            AIR_CAST(double, min[ai]),
                            nin->axis[ai].spaceDirection);
    }
  }

  return 0;
}

/*
******** nrrdPad_nva()
**
** unlike other {X_va,X_nva} pairs, nrrdPad_nva() is a wrapper around
** nrrdPad_va() instead of the other way around.
*/
int
nrrdPad_nva(Nrrd *nout, const Nrrd *nin,
            const ptrdiff_t *min, const ptrdiff_t *max,
            int boundary, double padValue) {
  static const char me[]="nrrdPad_nva";
  int E;

  if (!AIR_IN_OP(nrrdBoundaryUnknown, boundary, nrrdBoundaryLast)) {
    biffAddf(NRRD, "%s: boundary behavior %d invalid", me, boundary);
    return 1;
  }
  if (nrrdBoundaryPad == boundary) {
    E = nrrdPad_va(nout, nin, min, max, boundary, padValue);
  } else {
    E = nrrdPad_va(nout, nin, min, max, boundary);
  }
  if (E) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }

  return 0;
}

/*
******** nrrdSimplePad_va()
**
** pads by a given amount on top and bottom of EVERY axis
*/
int
nrrdSimplePad_va(Nrrd *nout, const Nrrd *nin, unsigned int pad,
                 int boundary, ...) {
  static const char me[]="nrrdSimplePad_va";
  unsigned ai;
  int ret;
  ptrdiff_t min[NRRD_DIM_MAX], max[NRRD_DIM_MAX];
  double padValue;
  va_list ap;

  if (!(nout && nin)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  for (ai=0; ai<nin->dim; ai++) {
    min[ai] = -AIR_CAST(ptrdiff_t, pad);
    max[ai] = nin->axis[ai].size-1 + pad;
  }
  va_start(ap, boundary);
  if (nrrdBoundaryPad == boundary) {
    padValue = va_arg(ap, double);
    ret = nrrdPad_va(nout, nin, min, max, boundary, padValue);
  } else {
    ret = nrrdPad_va(nout, nin, min, max, boundary);
  }
  va_end(ap);
  if (ret) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  return 0;
}

/*
******** nrrdSimplePad_nva()
**
** unlike other {X_va,X_nva} pairs, nrrdSimplePad_nva() is a wrapper
** around nrrdSimplePad_va() instead of the other way around.
*/
int
nrrdSimplePad_nva(Nrrd *nout, const Nrrd *nin, unsigned int pad,
                  int boundary, double padValue) {
  static const char me[]="nrrdSimplePad_nva";
  int E;

  if (!AIR_IN_OP(nrrdBoundaryUnknown, boundary, nrrdBoundaryLast)) {
    biffAddf(NRRD, "%s: boundary behavior %d invalid", me, boundary);
    return 1;
  }
  if (nrrdBoundaryPad == boundary) {
    E = nrrdSimplePad_va(nout, nin, pad, boundary, padValue);
  } else {
    E = nrrdSimplePad_va(nout, nin, pad, boundary);
  }
  if (E) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }

  return 0;
}
