/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah
  Copyright (C) 2012, 2011, 2010, 2009, 2008  Thomas Schultz

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

/*
** determines intersection of elements of srcA and srcB.
** assumes:
** - there are no repeats in either list
** - dstC is allocated for at least as long as the longer of srcA and srcB
*/
static unsigned int
flipListIntx(unsigned int *dstC,
             const unsigned int *_srcA, const unsigned int *_srcB) {
  const unsigned int *srcA, *srcB;
  unsigned int numA, numB, numC, idxA, idxB;

  numA = _srcA[0];
  srcA = _srcA + 1;
  numB = _srcB[0];
  srcB = _srcB + 1;
  numC = 0;
  for (idxA=0; idxA<numA; idxA++) {
    for (idxB=0; idxB<numB; idxB++) {
      if (srcA[idxA] == srcB[idxB]) {
        dstC[numC++] = srcA[idxA];
      }
    }
  }
  return numC;
}

/*
** given triangle identified by triIdx,
** set neighGot[] and neighInfo[][]
** neighbors are index 0,1,2;
** neighbor ii is on edge between vert ii and (ii+1)%3
** neighGot[ii] is non-zero iff there was such a neighbor
** neighInfo[ii][0]: index of the (triangle) neighbor
** neighInfo[ii][1], neighInfo[ii][2]: the two vertices shared with neighbor,
**    in the order that the *neighbor* should be traversing them
*/
static void
flipNeighborsGet(Nrrd *nTriWithVert, Nrrd *nVertWithTri,
                 unsigned int neighGot[3], unsigned int neighInfo[3][3],
                 unsigned int *intxBuff, unsigned int triIdx) {
  /* static const char me[]="flipNeighborsGet"; */
  unsigned int intxNum, vertA, vertB, neighIdx, maxTriPerVert,
    *vertWithTri, *triWithVert;
  int ii;

  vertWithTri = AIR_CAST(unsigned int*, nVertWithTri->data);
  triWithVert = AIR_CAST(unsigned int*, nTriWithVert->data);
  maxTriPerVert = nTriWithVert->axis[0].size - 1;
  for (ii=0; ii<3; ii++) {
    vertA = (vertWithTri + 3*triIdx)[ii];
    vertB = (vertWithTri + 3*triIdx)[AIR_MOD(ii+1, 3)];
    /*
    fprintf(stderr, "!%s: %u edge %u: vert{A,B} = %u %u\n", me,
            triIdx, ii, vertA, vertB);
    */
    /* find the intersection of the sets of {triangles using vertA}
       and {triangles using vertB}: for reasonable surfaces should
       be either 0 or 2 triangles, and if its 2, then triIdx
       should be one of them */
    intxNum = flipListIntx(intxBuff,
                           triWithVert + (1+maxTriPerVert)*vertA,
                           triWithVert + (1+maxTriPerVert)*vertB);
    if (2 == intxNum) {
      neighIdx = intxBuff[0];
      if (neighIdx == triIdx) {
        neighIdx = intxBuff[1];
      }
      neighGot[ii] = AIR_TRUE;
      neighInfo[ii][0] = neighIdx;
      neighInfo[ii][1] = vertB;
      neighInfo[ii][2] = vertA;
    } else {
      neighGot[ii] = AIR_FALSE;
    }
  }
  return;
}

/*
** determines if triIdx needs to be flipped, given that it should
** be seeing vertices vertA and vertB in that order
*/
static int
flipNeed(Nrrd *nVertWithTri, unsigned int triIdx,
         unsigned int vertA, unsigned int vertB) {
  unsigned int *vertWithTri, vert[3];
  int ai, bi;

  vertWithTri = AIR_CAST(unsigned int*, nVertWithTri->data);
  ELL_3V_COPY(vert, vertWithTri + 3*triIdx);
  for (ai=0; vert[ai] != vertA; ai++)
    ;
  for (bi=0; vert[bi] != vertB; bi++)
    ;
  return (1 != AIR_MOD(bi - ai, 3));
}

/*
** this is a weird dual-personality function that is the inner
** loop of both vertex winding fixing, and the learning stage of
** vertex splitting
**
** for flipping (!splitting)
** assumes that triIdx was just popped from "okay" stack
** (triIdx has just been fixed to have correct winding)
** then goes through the not-yet-done neighbors of triIdx,
** flipping them if needed, and
** then adding those neighbors to the stack.
** returns the number of tris added to stack
**
** NOTE: the "flipping" is done within the nVertWithTri representation,
** but *not* in the limnPolyData itself.
*/
static unsigned int
neighborsCheckPush(Nrrd *nTriWithVert, Nrrd *nVertWithTri,
                   unsigned char *triDone, airArray *okayArr,
                   unsigned int *intxBuff, airArray *splitArr,
                   unsigned int triIdx, int splitting) {
  /* static const char me[]="neighborsCheckPush"; */
  unsigned int neighGot[3], neighInfo[3][3], ii, *okay, okayIdx,
    *vertWithTri, pushedNum;

  vertWithTri = AIR_CAST(unsigned int*, nVertWithTri->data);
  flipNeighborsGet(nTriWithVert, nVertWithTri,
                   neighGot, neighInfo,
                   intxBuff, triIdx);
  /*
  for (ii=0; ii<3; ii++) {
    fprintf(stderr, "!%s: %u neigh[%u]: ", me, triIdx, ii);
    if (neighGot[ii]) {
      fprintf(stderr, "%u (%u %u) (done %u)\n",
              neighInfo[ii][0], neighInfo[ii][1], neighInfo[ii][2],
              triDone[neighInfo[ii][0]]);
    } else {
      fprintf(stderr, "nope\n");
    }
  }
  */
  pushedNum = 0;
  for (ii=0; ii<3; ii++) {
    /* WARNING: complicated logic WRT triDone, splitting, and need */
    if (neighGot[ii]) {
      unsigned int tmp, *idxLine, need;
      if (!splitting) {
        if (!triDone[neighInfo[ii][0]]) {
          /* we only take time to learn need if as yet undone */
          need = flipNeed(nVertWithTri, neighInfo[ii][0],
                          neighInfo[ii][1], neighInfo[ii][2]);
          if (need) {
            idxLine = vertWithTri + 3*neighInfo[ii][0];
            /* here is the vertex winding flip */
            ELL_SWAP2(idxLine[0], idxLine[1], tmp);
          }
        }
      } else {
        /* we're here for splitting  */
        /* we have to learn need regardless of done-ness */
        need = flipNeed(nVertWithTri, neighInfo[ii][0],
                        neighInfo[ii][1], neighInfo[ii][2]);
        if (need && triDone[neighInfo[ii][0]]) {
          /* we "need" to flip and yet we've already visited that triangle
             ==> edge between triIdx and neighInfo[ii][0] needs splitting.
             See if its a new split, and add it if so */
          unsigned int *split, splitIdx, splitNum, vert0, vert1;
          vert0 = AIR_MIN(neighInfo[ii][1], neighInfo[ii][2]);
          vert1 = AIR_MAX(neighInfo[ii][1], neighInfo[ii][2]);
          splitNum = splitArr->len;
          split = AIR_CAST(unsigned int*, splitArr->data);
          for (splitIdx=0; splitIdx<splitNum; splitIdx++) {
            if (split[2 + 5*splitIdx] == vert0
                && split[3 + 5*splitIdx] == vert1) {
              break;
            }
          }
          if (splitIdx == splitNum) {
            /* this is a new split, add it */
            /*
            fprintf(stderr, "!%s: new split(%u,%u) (have %u)\n",
                    me, vert0, vert1, splitArr->len);
            */
            splitIdx = airArrayLenIncr(splitArr, 1);
            split = AIR_CAST(unsigned int*, splitArr->data);
            split[0 + 5*splitIdx] = triIdx;
            split[1 + 5*splitIdx] = neighInfo[ii][0];
            split[2 + 5*splitIdx] = vert0;
            split[3 + 5*splitIdx] = vert1;
            split[4 + 5*splitIdx] = AIR_FALSE;
          }
        }
      }
      /* regardless of splitting, we push onto the okay stack all
         the un-done neighbors that we just processed */
      if (!triDone[neighInfo[ii][0]]) {
        triDone[neighInfo[ii][0]] = AIR_TRUE;
        okayIdx = airArrayLenIncr(okayArr, 1);
        okay = AIR_CAST(unsigned int*, okayArr->data);
        okay[okayIdx] = neighInfo[ii][0];
        ++pushedNum;
      }
    } /* if (neighGot[ii]) */
  } /* for ii */
  return pushedNum;
}

/*
** ONLY GOOD FOR limnPrimitiveTriangles!!
*/
static unsigned int
maxTrianglePerPrimitive(limnPolyData *pld) {
  unsigned int ret, primIdx;

  ret = 0;
  for (primIdx=0; primIdx<pld->primNum; primIdx++) {
    ret = AIR_MAX(ret, pld->icnt[primIdx]/3);
  }
  return ret;
}

/*
** fills nTriWithVert with 2D array about which triangles use which vertices
*/
static int
triangleWithVertex(Nrrd *nTriWithVert, limnPolyData *pld) {
  static const char me[]="triangleWithVertex";
  unsigned int *triWithVertNum,   /* vert ii has triWithVertNum[ii] tris */
    *triWithVert, baseVertIdx, primIdx, vertIdx,
    maxTriPerVert, totTriIdx;
  airArray *mop;

  if (!(nTriWithVert && pld)) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if ((1 << limnPrimitiveTriangles) != limnPolyDataPrimitiveTypes(pld)) {
    biffAddf(LIMN, "%s: sorry, can only handle %s primitives", me,
             airEnumStr(limnPrimitive, limnPrimitiveTriangles));
    return 1;
  }

  triWithVertNum = AIR_CAST(unsigned int*,
                            calloc(pld->xyzwNum, sizeof(unsigned int)));
  if (!triWithVertNum) {
    biffAddf(LIMN, "%s: couldn't allocate temp array", me);
    return 1;
  }
  mop = airMopNew();
  airMopAdd(mop, triWithVertNum, airFree, airMopAlways);

  /* fill in triWithVertNum */
  baseVertIdx = 0;
  for (primIdx=0; primIdx<pld->primNum; primIdx++) {
    unsigned int triNum, triIdx, *indxLine, ii;
    triNum = pld->icnt[primIdx]/3;
    for (triIdx=0; triIdx<triNum; triIdx++) {
      indxLine = pld->indx + baseVertIdx + 3*triIdx;
      for (ii=0; ii<3; ii++) {
        triWithVertNum[indxLine[ii]]++;
      }
    }
    baseVertIdx += pld->icnt[primIdx];
  }

  /* find max # tris per vert, allocate output */
  maxTriPerVert = 0;
  for (vertIdx=0; vertIdx<pld->xyzwNum; vertIdx++) {
    maxTriPerVert = AIR_MAX(maxTriPerVert, triWithVertNum[vertIdx]);
  }
  if (nrrdMaybeAlloc_va(nTriWithVert, nrrdTypeUInt, 2,
                        AIR_CAST(size_t, 1 + maxTriPerVert),
                        AIR_CAST(size_t, pld->xyzwNum))) {
    biffMovef(LIMN, NRRD, "%s: couldn't allocate output", me);
    airMopError(mop); return 1;
  }
  triWithVert = AIR_CAST(unsigned int*, nTriWithVert->data);

  baseVertIdx = 0;
  totTriIdx = 0;
  for (primIdx=0; primIdx<pld->primNum; primIdx++) {
    unsigned int triNum, *indxLine, *twvLine, ii, triIdx;
    triNum = pld->icnt[primIdx]/3;
    for (triIdx=0; triIdx<triNum; triIdx++) {
      indxLine = pld->indx + baseVertIdx + 3*triIdx;
      for (ii=0; ii<3; ii++) {
        twvLine = triWithVert + (1+maxTriPerVert)*indxLine[ii];
        twvLine[1+twvLine[0]] = totTriIdx;
        twvLine[0]++;
      }
      ++totTriIdx;
    }
    baseVertIdx += pld->icnt[primIdx];
  }

  airMopOkay(mop);
  return 0;
}

/*
** learns which (three vertices) are with which triangle
*/
static int
vertexWithTriangle(Nrrd *nVertWithTri, limnPolyData *pld) {
  static const char me[]="vertexWithTriangle";
  unsigned int baseVertIdx, primIdx, *vertWithTri, triNum;

  if (!(nVertWithTri && pld)) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if ((1 << limnPrimitiveTriangles) != limnPolyDataPrimitiveTypes(pld)) {
    biffAddf(LIMN, "%s: sorry, can only handle %s primitives", me,
             airEnumStr(limnPrimitive, limnPrimitiveTriangles));
    return 1;
  }

  triNum = limnPolyDataPolygonNumber(pld);
  if (nrrdMaybeAlloc_va(nVertWithTri, nrrdTypeUInt, 2,
                        AIR_CAST(size_t, 3),
                        AIR_CAST(size_t, triNum))) {
    biffMovef(LIMN, NRRD, "%s: couldn't allocate output", me);
    return 1;
  }
  vertWithTri = AIR_CAST(unsigned int*, nVertWithTri->data);

  baseVertIdx = 0;
  for (primIdx=0; primIdx<pld->primNum; primIdx++) {
    unsigned int triIdx, *indxLine, totTriIdx, ii;
    triNum = pld->icnt[primIdx]/3;
    for (triIdx=0; triIdx<triNum; triIdx++) {
      totTriIdx = triIdx + baseVertIdx/3;
      indxLine = pld->indx + baseVertIdx + 3*triIdx;
      for (ii=0; ii<3; ii++) {
        (vertWithTri + 3*totTriIdx)[ii] = indxLine[ii];
      }
    }
    baseVertIdx += pld->icnt[primIdx];
  }

  return 0;
}

static int
splitListExtract(unsigned int *listLenP,
                 airArray *edgeArr, unsigned char *hitCount,
                 unsigned int firstVertIdx, unsigned int edgeDoneNum) {
  static const char me[]="splitListExtract";
  unsigned int *edgeData, edgeNum, *edgeLine, edgeIdx, edgeTmp[5],
    tmp, nextVertIdx, listLen;

  edgeNum = edgeArr->len;
  edgeData = AIR_CAST(unsigned int*, edgeArr->data);
  edgeNum -= edgeDoneNum;
  edgeData += 5*edgeDoneNum;

  /* put first edge in first position */
  for (edgeIdx=0; edgeIdx<edgeNum; edgeIdx++) {
    edgeLine = edgeData + 5*edgeIdx;
    if (edgeLine[2] == firstVertIdx || edgeLine[3] == firstVertIdx) {
      break;
    }
  }
  if (edgeIdx == edgeNum) {
    biffAddf(LIMN, "%s: never found first vertex %u", me, firstVertIdx);
    return 1;
  }
  if (edgeLine[3] == firstVertIdx) {
    ELL_SWAP2(edgeLine[2], edgeLine[3], tmp);
  }
  ELL_5V_COPY(edgeTmp, edgeData);
  ELL_5V_COPY(edgeData, edgeLine);
  ELL_5V_COPY(edgeLine, edgeTmp);

  /* start looking for the rest */
  listLen = 1;
  hitCount[firstVertIdx]--;
  nextVertIdx = edgeData[3];
  hitCount[nextVertIdx]--;
  /*
  fprintf(stderr, "!%s: found first %u --> %u (tris %u %u)\n", me,
          firstVertIdx, nextVertIdx,
          edgeData[0], edgeData[1]);
  */

  /* the search start progresses so that we don't see the same edge twice */
#define SEARCH \
  for (edgeIdx=listLen; edgeIdx<edgeNum; edgeIdx++) { \
    edgeLine = edgeData + 5*edgeIdx; \
    if (edgeLine[2] == nextVertIdx || edgeLine[3] == nextVertIdx) { \
      break; \
    } \
  }
  SEARCH;
  while (edgeIdx < edgeNum) {
    if (edgeLine[3] == nextVertIdx) {
      ELL_SWAP2(edgeLine[2], edgeLine[3], tmp);
    }
    ELL_5V_COPY(edgeTmp, edgeData + 5*listLen);
    ELL_5V_COPY(edgeData + 5*listLen, edgeLine);
    ELL_5V_COPY(edgeLine, edgeTmp);
    hitCount[nextVertIdx]--;
    /*
    fprintf(stderr, "!%s: (len %u) found %u --> %u  (tris %u %u)\n", me,
            listLen, nextVertIdx,
            (edgeData + 5*listLen)[3],
            (edgeData + 5*listLen)[0],
            (edgeData + 5*listLen)[1]);
    */
    nextVertIdx = (edgeData + 5*listLen)[3];
    hitCount[nextVertIdx]--;
    listLen++;
    SEARCH;
  }
  /*
  fprintf(stderr, "!%s: finishing with Len %u, ended at %u\n", me,
          listLen, nextVertIdx);
  */
  *listLenP = listLen;
  return 0;
#undef SEARCH
}

/*
** returns the element of vert[] that is not v0 or v1
*/
static unsigned int
sweepVertNext(unsigned int *vert, unsigned int v0, unsigned int v1) {
  unsigned int v2;

  v2 = vert[0];
  if (v2 == v0 || v2 == v1) {
    v2 = vert[1];
  }
  if (v2 == v0 || v2 == v1) {
    v2 = vert[2];
  }
  return v2;
}

/*
** returns non-zero iff A and B are in {v[0],v[1],v[2]}
*/
static int
sweepHave2(unsigned int v[3], unsigned int A, unsigned B) {
  int haveA, haveB;

  haveA = (A == v[0] || A == v[1] || A == v[2]);
  haveB = (B == v[0] || B == v[1] || B == v[2]);
  return (haveA && haveB);
}

/*
** returns UINT_MAX if there is no other triangle
*/
static unsigned int
sweepTriNext(unsigned int *triLine, unsigned int v0, unsigned int v1,
             unsigned int triNot, Nrrd *nVertWithTri) {
  unsigned int triIdx, ret, *vertLine, *vertWithTri;

  vertWithTri = AIR_CAST(unsigned int*, nVertWithTri->data);

  for (triIdx=0; triIdx<triLine[0]; triIdx++) {
    if (triLine[1+triIdx] == triNot) {
      continue;
    }
    vertLine = vertWithTri + 3*triLine[1+triIdx];
    if (sweepHave2(vertLine, v0, v1)) {
      break;
    }
  }
  if (triIdx == triLine[0]) {
    ret = UINT_MAX;
  } else {
    ret = triLine[1+triIdx];
  }
  return ret;
}

/*
** the sweep does NOT include triStart, but it does include whichever
**  triStop it hit (if any)
** returns: length of sweep
** sweep: output (does not include triStart)
** triStartIdx: what triangle to start at
** vertPivotIdx, vertStartIdx: two vertices of start triangle; sweep
**     proceeds around the pivot index
** triStop{0,1}Idx: triangles to stop sweeping at
*/
static unsigned int
splitTriSweep(unsigned int *sweep,
              unsigned int triStart,
              unsigned int vertPivot, unsigned int vertStart,
              unsigned int triStop0, unsigned int triStop1,
              Nrrd *nTriWithVert, Nrrd *nVertWithTri) {
  /* static const char me[]="splitTriSweep"; */
  unsigned int sweepLen;
  unsigned int maxTriPerVert, *triWithVert,
    *vertWithTri, *triLine, *vertLine, triCurr, vertLast, vertNext;

  maxTriPerVert = AIR_CAST(unsigned int, nTriWithVert->axis[0].size-1);
  triWithVert = AIR_CAST(unsigned int*, nTriWithVert->data);
  vertWithTri = AIR_CAST(unsigned int*, nVertWithTri->data);

  /*
  fprintf(stderr, "!%s:  hi,  triStart %u, pivot %u, start %u, "
          "stop = %u, %u\n", me,
          triStart, vertPivot, vertStart, triStop0, triStop1);
  */
  if (triStart == triStop0 || triStart == triStop1) {
    /* nowhere to go */
    return 0;
  }

  triLine = triWithVert + (1+maxTriPerVert)*vertPivot;
  vertLast = vertStart;
  triCurr = triStart;
  sweepLen = 0;
  do {
    if (!(triCurr == triStart)) {
      sweep[sweepLen++] = triCurr;
      /*
      fprintf(stderr, "!%s:       saving sweep[%u] = %u\n", me,
              sweepLen-1, triCurr);
      */
    }
    vertLine = vertWithTri + 3*triCurr;
    vertNext = sweepVertNext(vertLine, vertPivot, vertLast);
    /*
    fprintf(stderr, "!%s:       vertNext(%u,%u) = %u\n", me,
            vertPivot, vertLast, vertNext);
    */
    triCurr = sweepTriNext(triLine, vertPivot, vertNext,
                           triCurr, nVertWithTri);
    /*
    fprintf(stderr, "!%s:       triNext(%u,%u) = %u\n", me,
            vertPivot, vertNext, triCurr);
    */
    vertLast = vertNext;
  } while (!( UINT_MAX == triCurr
              || triStart == triCurr
              || triStop0 == triCurr
              || triStop1 == triCurr ));
  if (!( UINT_MAX == triCurr )) {
    sweep[sweepLen++] = triCurr;
    /*
    fprintf(stderr, "!%s:       saving sweep[%u] = %u\n", me,
            sweepLen-1, triCurr);
    */
  }

  return sweepLen;
}

/*
** track0: first triangle track, length *track0LenP
** track1: first triangle track, length *track1LenP
** sweep: buffer for sweep
**
** NOTE: triangles may be internally repeated in a track
**
** when vert path a loop on a non-orientable surface (e.g. mobius strip),
** then track0 will NOT include the endpoint triangles
** (or its not supposed to), and track1 will include them.
*/
static int
splitTriTrack(unsigned int *track0, unsigned int *track0LenP,
              unsigned int *track1, unsigned int *track1LenP,
              unsigned int *sweep,
              Nrrd *nTriWithVert, Nrrd *nVertWithTri,
              airArray *edgeArr, unsigned startIdx, unsigned int listLen,
              int looping) {
  static const char me[]="splitTriTrack";
  unsigned int len0, len1, *edgeData, *edgeLine, edgeIdx, triIdx,
    /* maxTriPerVert, *triWithVert, *vertWithTri, */
    sweepLen, loopEnd0, loopEnd1, loopStart0, loopStart1;
  int doBack0, doBack1;

  len0 = len1 = 0;
  edgeData = AIR_CAST(unsigned int*, edgeArr->data);
  edgeData += 5*startIdx;
  /* maxTriPerVert = AIR_CAST(unsigned int, nTriWithVert->axis[0].size-1); */
  /* triWithVert = AIR_CAST(unsigned int*, nTriWithVert->data); */
  /* vertWithTri = AIR_CAST(unsigned int*, nVertWithTri->data); */

  if (looping) {
    loopStart0 = (edgeData)[0];
    loopStart1 = (edgeData)[1];
    loopEnd0 = (edgeData + 5*(listLen - 1))[0];
    loopEnd1 = (edgeData + 5*(listLen - 1))[1];
    /*
    fprintf(stderr, "!%s: loop start = %u, %u, end = %u,%u\n", me,
            loopStart0, loopStart1, loopEnd0, loopEnd1);
    */
  } else {
    loopStart0 = loopStart1 = UINT_MAX;
    loopEnd0 = loopEnd1 = UINT_MAX;
  }

  /* ,,,,,,,,,,,,,,,,,,,,,
  fprintf(stderr, "!%s: 1st 2 tris %u %u, verts %u %u\n", me,
          edgeData[0], edgeData[1], edgeData[2], edgeData[3]);
  fprintf(stderr, "!%s: triangles at start vert %u:\n", me, edgeData[2]);
  triLine = triWithVert + edgeData[2]*(1+maxTriPerVert);
  for (triIdx=0; triIdx<triLine[0]; triIdx++) {
    unsigned int *vertLine;
    vertLine = vertWithTri + 3*triLine[1+triIdx];
    fprintf(stderr, "!%s: %u:  %u  (verts %u %u %u)\n",
            me, triIdx, triLine[1+triIdx],
            vertLine[0], vertLine[1], vertLine[2]);
  }
  ````````````````````` */

  /* we turn on backward sweeping for the initial edge;
     doBack{0,1} will be set explicitly at each edge thereafter */
  doBack0 = doBack1 = AIR_TRUE;
  for (edgeIdx=0; edgeIdx<(looping
                           ? listLen-1
                           : listLen); edgeIdx++) {
    unsigned int stop0, stop1;
    edgeLine = edgeData + 5*edgeIdx;
    /* ,,,,,,,,,,,,,,,,,,,,,
    fprintf(stderr, "!%s: edge %u: vert %u->%u, tris %u, %u\n", me,
            edgeIdx, edgeLine[2], edgeLine[3],
            edgeLine[0], edgeLine[1]);
    fprintf(stderr, "!%s:   triangles at next vert %u:\n", me, edgeLine[3]);
    triLine = triWithVert + edgeLine[3]*(1+maxTriPerVert);
    for (triIdx=0; triIdx<triLine[0]; triIdx++) {
      vertLine = vertWithTri + 3*triLine[1+triIdx];
      fprintf(stderr, "!%s:   %u:  %u  (verts %u %u %u)\n",
              me, triIdx, triLine[1+triIdx],
              vertLine[0], vertLine[1], vertLine[2]);
    }
    ````````````````````` */
    if (0 == edgeIdx && looping) {
      /* sweeps from 1st link on loop are stopped by a tris on last edge */
      stop0 = loopEnd0;
      stop1 = loopEnd1;
    } else {
      stop0 = UINT_MAX;
      stop1 = UINT_MAX;
    }
    if (doBack0) {
      sweepLen = splitTriSweep(sweep, edgeLine[0], edgeLine[2], edgeLine[3],
                               stop0, stop1, nTriWithVert, nVertWithTri);
      if (0 == edgeIdx && looping && sweepLen > 0) {
        /* don't include either stop triangle on track 0 */
        for (triIdx=0; triIdx<sweepLen-1; triIdx++) {
          track0[len0++] = sweep[sweepLen-2-triIdx];
        }
      } else {
        for (triIdx=0; triIdx<sweepLen; triIdx++) {
          track0[len0++] = sweep[sweepLen-1-triIdx];
        }
      }
      track0[len0++] = edgeLine[0];
    }
    if (doBack1) {
      sweepLen = splitTriSweep(sweep, edgeLine[1], edgeLine[2], edgeLine[3],
                               stop0, stop1, nTriWithVert, nVertWithTri);
      /* on this side we *do* include the stop triangle */
      for (triIdx=0; triIdx<sweepLen; triIdx++) {
        track1[len1++] = sweep[sweepLen-1-triIdx];
      }
      track1[len1++] = edgeLine[1];
    }

    if (edgeIdx<listLen-1) {
      stop0 = (edgeLine + 5)[0];
      stop1 = (edgeLine + 5)[1];
    } else {
      if (looping) {
        stop0 = loopStart0;
        stop1 = loopStart1;
      } else {
        stop0 = UINT_MAX;
        stop1 = UINT_MAX;
      }
    }
    sweepLen = splitTriSweep(sweep, edgeLine[0], edgeLine[3], edgeLine[2],
                             stop0, stop1, nTriWithVert, nVertWithTri);
    for (triIdx=0; triIdx<sweepLen; triIdx++) {
      track0[len0++] = sweep[triIdx];
    }
    sweepLen = splitTriSweep(sweep, edgeLine[1], edgeLine[3], edgeLine[2],
                             stop0, stop1, nTriWithVert, nVertWithTri);
    for (triIdx=0; triIdx<sweepLen; triIdx++) {
      track1[len1++] = sweep[triIdx];
    }
    if (edgeIdx<listLen-1) {
      unsigned int *nextLine, tmp;
      /* re-arrange the next edgeLine according to sweep results */
      nextLine = edgeData + 5*(1 + edgeIdx);
      if (track0[len0-1] == nextLine[0]
          && track1[len1-1] == nextLine[1]) {
        /* fprintf(stderr, "!%s: tracking went 0->0, 1->1\n", me); */
        doBack0 = doBack1 = AIR_FALSE;
      } else if (track0[len0-1] == nextLine[1]
                 && track1[len1-1] == nextLine[0]) {
        /* fprintf(stderr, "!%s: tracking went 0->1, 0->1\n", me); */
        ELL_SWAP2(nextLine[0], nextLine[1], tmp);
        doBack0 = doBack1 = AIR_FALSE;
      } else if (track0[len0-1] == nextLine[0]) {
        /* fprintf(stderr, "!%s: tracking went 0->0, 1->x\n", me); */
        doBack0 = AIR_FALSE;
        doBack1 = AIR_TRUE;
      } else if (track1[len1-1] == nextLine[1]) {
        /* fprintf(stderr, "!%s: tracking went 0->x, 1->1\n", me); */
        doBack0 = AIR_TRUE;
        doBack1 = AIR_FALSE;
      } else if (track0[len0-1] == nextLine[1]) {
        /* fprintf(stderr, "!%s: tracking went 0->1, 1->x\n", me); */
        ELL_SWAP2(nextLine[0], nextLine[1], tmp);
        doBack0 = AIR_FALSE;
        doBack1 = AIR_TRUE;
      } else if (track1[len1-1] == nextLine[0]) {
        /* fprintf(stderr, "!%s: tracking went 0->x, 1->0\n", me); */
        ELL_SWAP2(nextLine[0], nextLine[1], tmp);
        doBack0 = AIR_TRUE;
        doBack1 = AIR_FALSE;
      } else {
        biffAddf(LIMN, "%s: edge %u/%u, sweep ends %u,%u != want %u,%u", me,
                 edgeIdx, listLen, track0[len0-1], track1[len1-1],
                 nextLine[0], nextLine[1]);
        return 1;
      }
    } else {
      doBack0 = doBack1 = AIR_FALSE;
    }
  }
  if (looping) {
    /* the end of track0 shouldn't include the stop */
    len0--;
  }

  *track0LenP = len0;
  *track1LenP = len1;
  return 0;
}

static int
splitVertDup(limnPolyData *pld, airArray *edgeArr,
             unsigned int edgeDoneNum, unsigned int listLen,
             unsigned int *track, unsigned int trackLen,
             int looping) {
  static const char me[]="splitVertDup";
  unsigned int *vixLut, ii, vixLutLen, oldVertNum, newVertNum, *edgeData,
    bitflag, trackIdx, vert0, vert1;
  airArray *mop;
  limnPolyData pldTmp;

  mop = airMopNew();
  edgeData = AIR_CAST(unsigned int*, edgeArr->data);
  edgeData += 5*edgeDoneNum;
  oldVertNum = pld->xyzwNum;
  vixLutLen = looping ? listLen : listLen+1;
  newVertNum = oldVertNum + vixLutLen;

  /* quiet compiler warnings */
  pldTmp.rgba = NULL;
  pldTmp.norm = NULL;
  pldTmp.tex2 = NULL;
  pldTmp.tang = NULL;

  if (looping) {
    vert0 = edgeData[2]; /* don't use dupe of this on first triangle */
    vert1 = edgeData[3]; /* don't use dupe of this on last triangle */
  } else {
    vert0 = vert1 = UINT_MAX;
  }

  /* HEY: sneakily preserve the old per-vertex arrays; we own them now */
  pldTmp.xyzw = pld->xyzw;
  airMopAdd(mop, pldTmp.xyzw, airFree, airMopAlways);
  pld->xyzw = NULL;
  pld->xyzwNum = 0;
  bitflag = limnPolyDataInfoBitFlag(pld);
  if ((1 << limnPolyDataInfoRGBA) & bitflag) {
    pldTmp.rgba = pld->rgba;
    airMopAdd(mop, pldTmp.rgba, airFree, airMopAlways);
    pld->rgba = NULL;
    pld->rgbaNum = 0;
  }
  if ((1 << limnPolyDataInfoNorm) & bitflag) {
    pldTmp.norm = pld->norm;
    airMopAdd(mop, pldTmp.norm, airFree, airMopAlways);
    pld->norm = NULL;
    pld->normNum = 0;
  }
  if ((1 << limnPolyDataInfoTex2) & bitflag) {
    pldTmp.tex2 = pld->tex2;
    airMopAdd(mop, pldTmp.tex2, airFree, airMopAlways);
    pld->tex2 = NULL;
    pld->tex2Num = 0;
  }
  if ((1 << limnPolyDataInfoTang) & bitflag) {
    pldTmp.tang = pld->tang;
    airMopAdd(mop, pldTmp.tang, airFree, airMopAlways);
    pld->tang = NULL;
    pld->tangNum = 0;
  }
  if (limnPolyDataAlloc(pld, bitflag, newVertNum,
                        pld->indxNum, pld->primNum)) {
    biffAddf(LIMN, "%s: couldn't allocate new vert # %u", me, newVertNum);
    airMopError(mop); return 1;
  }

  /* copy old data */
  memcpy(pld->xyzw, pldTmp.xyzw, oldVertNum*4*sizeof(float));
  if ((1 << limnPolyDataInfoRGBA) & bitflag) {
    memcpy(pld->rgba, pldTmp.rgba, oldVertNum*4*sizeof(unsigned char));
  }
  if ((1 << limnPolyDataInfoNorm) & bitflag) {
    memcpy(pld->norm, pldTmp.norm, oldVertNum*3*sizeof(float));
  }
  if ((1 << limnPolyDataInfoTex2) & bitflag) {
    memcpy(pld->tex2, pldTmp.tex2, oldVertNum*2*sizeof(float));
  }
  if ((1 << limnPolyDataInfoTang) & bitflag) {
    memcpy(pld->tang, pldTmp.tang, oldVertNum*3*sizeof(float));
  }

  vixLut = AIR_CAST(unsigned int *, calloc(2*vixLutLen,
                                           sizeof(unsigned int)));
  airMopAdd(mop, vixLut, airFree, airMopAlways);
  if (looping) {
    for (ii=0; ii<vixLutLen; ii++) {
      vixLut[0 + 2*ii] = edgeData[2 + 5*ii];
      vixLut[1 + 2*ii] = oldVertNum + ii;
    }
  } else {
    for (ii=0; ii<vixLutLen-1; ii++) {
      vixLut[0 + 2*ii] = edgeData[2 + 5*ii];
      vixLut[1 + 2*ii] = oldVertNum + ii;
    }
    /* now ii == vixLutLen-1 == listLen */
    vixLut[0 + 2*ii] = edgeData[3 + 5*(ii-1)];
    vixLut[1 + 2*ii] = oldVertNum + ii;
  }

  /* copy pld's vertex information to duped vertices */
  for (ii=0; ii<vixLutLen; ii++) {
    ELL_4V_COPY(pld->xyzw + 4*vixLut[1 + 2*ii],
                pld->xyzw + 4*vixLut[0 + 2*ii]);
    if ((1 << limnPolyDataInfoRGBA) & bitflag) {
      ELL_4V_COPY(pld->rgba + 4*vixLut[1 + 2*ii],
                  pld->rgba + 4*vixLut[0 + 2*ii]);
    }
    if ((1 << limnPolyDataInfoNorm) & bitflag) {
      ELL_3V_COPY(pld->norm + 3*vixLut[1 + 2*ii],
                  pld->norm + 3*vixLut[0 + 2*ii]);
    }
    if ((1 << limnPolyDataInfoTex2) & bitflag) {
      ELL_2V_COPY(pld->tex2 + 2*vixLut[1 + 2*ii],
                  pld->tex2 + 2*vixLut[0 + 2*ii]);
    }
    if ((1 << limnPolyDataInfoTang) & bitflag) {
      ELL_3V_COPY(pld->tang + 3*vixLut[1 + 2*ii],
                  pld->tang + 3*vixLut[0 + 2*ii]);
    }
  }

  /* for triangles in track, update indices of duped vertices */
  /* we do this updating ONLY in the limnPolyData, and that's okay:
     the split information is computed entirely from nVertWithTri
     and nTriWithVert (which were based on the original polydata),
     but not the current polydata */
  /* HEY: this is one place where we really exploit the fact that we only
     have triangles: it makes the indxLine computation much much easier */
  for (trackIdx=0; trackIdx<trackLen; trackIdx++) {
    unsigned int *indxLine, jj;
    indxLine = pld->indx + 3*track[trackIdx];
    for (ii=0; ii<vixLutLen; ii++) {
      for (jj=0; jj<3; jj++) {
        if (indxLine[jj] == vixLut[0 + 2*ii]
            && !((0 == trackIdx && indxLine[jj] == vert0)
                 || (trackLen-1 == trackIdx && indxLine[jj] == vert1))) {
          indxLine[jj] = vixLut[1 + 2*ii];
        }
      }
    }
  }

  airMopOkay(mop);
  return 0;
}

/*
**  edge[0, 1]: two neighboring triangles,
**  edge[2, 3]: their shared vertices
**  edge[4]: non-zero if this split has been processed
**
** this really makes no effort to be fast (or comprehensible)
**
** HEY should we be returning some statistics (e.g. how many points added)?
*/
static int
doSplitting(limnPolyData *pld, Nrrd *nTriWithVert, Nrrd *nVertWithTri,
            airArray *edgeArr) {
  static const char me[]="doSplitting";
  unsigned int edgeIdx, *edgeData,
    *edgeLine=NULL, vertIdx, vertNum, splitNum, edgeDoneNum, listLen=0,
    *track0, track0Len=0, *track1, *sweep, track1Len=0, maxTriPerVert;
  unsigned char *hitCount;
  airArray *mop;
  int passIdx;

  if (!edgeArr->len) {
    /* actually, no splitting was required! */
    return 0;
  }

  mop = airMopNew();
  /* NOTE: It is necessary to save out the number of (initial)
     number of vertices here, because as we do the splitting
     (which is done once per track, as tracks are computed),
     pld->xyzwNum will increase ... */
  vertNum = pld->xyzwNum;
  hitCount = AIR_CAST(unsigned char *, calloc(vertNum,
                                              sizeof(unsigned char)));
  maxTriPerVert = AIR_CAST(unsigned int, nTriWithVert->axis[0].size - 1);
  track0 = AIR_CAST(unsigned int *, calloc(maxTriPerVert*edgeArr->len,
                                           sizeof(unsigned int)));
  track1 = AIR_CAST(unsigned int *, calloc(maxTriPerVert*edgeArr->len,
                                           sizeof(unsigned int)));
  sweep = AIR_CAST(unsigned int *, calloc(maxTriPerVert,
                                          sizeof(unsigned int)));
  if (!(hitCount && track0 && track1 && sweep)) {
    biffAddf(LIMN, "%s: couldn't alloc buffers", me);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, hitCount, airFree, airMopAlways);
  airMopAdd(mop, track0, airFree, airMopAlways);
  airMopAdd(mop, track1, airFree, airMopAlways);
  airMopAdd(mop, sweep, airFree, airMopAlways);

  edgeData = AIR_CAST(unsigned int*, edgeArr->data);

  /* initialize hitCount */
  for (edgeIdx=0; edgeIdx<edgeArr->len; edgeIdx++) {
    unsigned int ha, hb;
    edgeLine = edgeData + 5*edgeIdx;
    ha = hitCount[edgeLine[2]]++;
    hb = hitCount[edgeLine[3]]++;
    if (ha > 2 || hb > 2) {
      biffAddf(LIMN, "%s: edge %u (vert %u %u) created hit counts %u %u", me,
               edgeIdx, edgeLine[2], edgeLine[3], ha, hb);
      airMopError(mop); return 1;
    }
  }

  /* scan hitCount */
#define SEARCH(x)                               \
  for (vertIdx=0; vertIdx<vertNum; vertIdx++) { \
    if ((x) == hitCount[vertIdx]) {             \
      break;                                    \
    }                                           \
  }

  splitNum = 0;
  edgeDoneNum = 0;
  /* pass 0: look for singleton hits ==> non-loop tracks
     pass 1: look for hitCount[2] ==> loop tracks
  */
  for (passIdx=0; passIdx<2; passIdx++) {
    if (0 == passIdx) {
      SEARCH(1);
    } else {
      SEARCH(2);
    }
    while (vertIdx < vertNum) {
      unsigned int E;
      E = 0;
      if (1) {
        unsigned int hitIdx, hitSum;
        hitSum = 0;
        for (hitIdx=0; hitIdx<vertNum; hitIdx++) {
          hitSum += hitCount[hitIdx];
        }
        /*
        fprintf(stderr, "!%s: PRE hitSum = %u (pass %u)\n", me,
                hitSum, passIdx);
        */
      }
      if (!E) E |= splitListExtract(&listLen, edgeArr, hitCount,
                                    vertIdx, edgeDoneNum);
      /* HEY: should do a splitListShorten() that cuts across repeated
         triangles, and then shifting downward the rest of the list.
         take care with loops.  iterate until there is no shortening */
      /*
      if (1) {
        unsigned int hitIdx, hitSum;
        hitSum = 0;
        for (hitIdx=0; hitIdx<vertNum; hitIdx++) {
          hitSum += hitCount[hitIdx];
        }
        fprintf(stderr, "!%s: (%d) POST hitSum = %u (pass %u)\n", me, E,
                hitSum, passIdx);
      }
      if (1 == passIdx) {
        fprintf(stderr, "!%s: loop len %u, verts %u,%u --- %u,%u\n"
                "         tris %u,%u --- %u,%u\n", me,
                listLen,
                (edgeData + 5*(edgeDoneNum + listLen - 1))[2],
                (edgeData + 5*(edgeDoneNum + listLen - 1))[3],
                (edgeData + 5*edgeDoneNum)[2],
                (edgeData + 5*edgeDoneNum)[3],
                (edgeData + 5*(edgeDoneNum + listLen - 1))[0],
                (edgeData + 5*(edgeDoneNum + listLen - 1))[1],
                (edgeData + 5*edgeDoneNum)[0],
                (edgeData + 5*edgeDoneNum)[1]);
      }
      */
      if (!E) E |= splitTriTrack(track0, &track0Len, track1, &track1Len,
                                 sweep, nTriWithVert, nVertWithTri,
                                 edgeArr, edgeDoneNum, listLen, passIdx);
      /* ,,,,,,,,,,,,,,,,,,,,,
      if (!E) {
        fprintf(stderr, "!%s: track0:\n", me);
        for (triIdx=0; triIdx<track0Len; triIdx++) {
          fprintf(stderr, "!%s:  %u: %u\n", me, triIdx, track0[triIdx]);
        }
        fprintf(stderr, "!%s: track1:\n", me);
        for (triIdx=0; triIdx<track1Len; triIdx++) {
          fprintf(stderr, "!%s:  %u: %u\n", me, triIdx, track1[triIdx]);
        }
      }
      ````````````````````` */
      /* see- this is the only time pld is used (so it can be modified) */
      /* HEY: we should be using track1, since that's the one that includes
         the endpoint triangles, but on a mobius strip demo it looked worse...
         this still needs debugging */
      if (!E) E |= splitVertDup(pld, edgeArr, edgeDoneNum, listLen,
                                track0, track0Len, passIdx);
      if (E) {
        biffAddf(LIMN, "%s: trouble on split %u (done %u/%u)", me,
                 splitNum, edgeDoneNum, AIR_CAST(unsigned int, edgeArr->len));
        return 1;
      }
      edgeDoneNum += listLen;
      /*
        fprintf(stderr, "!%s: edgeDoneNum now %u (%u)\n", me,
        edgeDoneNum, AIR_CAST(unsigned int, edgeArr->len));
      */
      if (0 == passIdx) {
        SEARCH(1);
      } else {
        SEARCH(2);
      }
    }
  }
#undef SEARCH
  airMopOkay(mop);
  return 0;
}

int
_limnPolyDataVertexWindingProcess(limnPolyData *pld, int splitting) {
  static const char me[]="limnPolyDataVertexWindingProcess";
  unsigned int
    primIdx,         /* for indexing through primitives */
    triIdx,          /* for indexing through triangles in each primitive */
    maxTriPerPrim,   /* max # triangles per primitive, which is essential for
                        the indexing of each triangle (in each primitive)
                        into a single triangle index */
    totTriIdx,       /* another triangle index */
    totTriNum,       /* total # triangles */
    trueTriNum,      /* correct total # triangles in all primitives */
    baseVertIdx,     /* first vertex for current primitive */
    maxTriPerVert,   /* max # of tris on single vertex */
    /* *triWithVert,    2D array ((1+maxTriPerVert) x pld->xyzwNum)
                        of per-vertex triangles */
    *vertWithTri,    /* 3D array (3 x maxTriPerPrim x pld->primNum)
                        of per-tri vertices (vertex indices), which is just
                        a repackaging of the information in the lpld */
    doneTriNum,      /* # triangles finished so far */
    *intxBuff,       /* stupid buffer */
    *okay,           /* the stack of triangles with okay (possibly fixed)
                        winding, but with some neighbors that may as yet
                        need fixing */
    *split;          /* stack of 5-tuples about edges needing vertex splits:
                        split[0, 1]: two neighboring triangles,
                        split[2, 3]: their shared vertices
                        split[4]: non-zero if this split has been processed */
  unsigned char
    *triDone;        /* 1D array (len totTriNum) record of done-ness */
  Nrrd *nTriWithVert, *nVertWithTri;
  airArray *mop,     /* house-keeping */
    *okayArr,        /* airArray around "okay" */
    *splitArr;       /* airArray around "split" */
  airPtrPtrUnion appu;
  /*
    fprintf(stderr, "!%s: hi\n", me);
  */
  if (!pld) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }

  if (!(pld->xyzwNum && pld->primNum)) {
    /* this is empty? */
    return 0;
  }

  if ((1 << limnPrimitiveTriangles) != limnPolyDataPrimitiveTypes(pld)) {
    biffAddf(LIMN, "%s: sorry, can only handle %s primitives", me,
             airEnumStr(limnPrimitive, limnPrimitiveTriangles));
    return 1;
  }

  maxTriPerPrim = maxTrianglePerPrimitive(pld);
  totTriNum = limnPolyDataPolygonNumber(pld);

  mop = airMopNew();
  triDone = AIR_CAST(unsigned char *, calloc(totTriNum,
                                             sizeof(unsigned char)));
  airMopAdd(mop, triDone, airFree, airMopAlways);
  if (!triDone) {
    biffAddf(LIMN, "%s: couldn't allocate temp array", me);
    airMopError(mop); return 1;
  }

  /* allocate TriWithVert, VertWithTri, intxBuff */
  nTriWithVert = nrrdNew();
  airMopAdd(mop, nTriWithVert, (airMopper)nrrdNuke, airMopAlways);
  nVertWithTri = nrrdNew();
  airMopAdd(mop, nVertWithTri, (airMopper)nrrdNuke, airMopAlways);
  if (triangleWithVertex(nTriWithVert, pld)
      || vertexWithTriangle(nVertWithTri, pld)) {
    biffAddf(LIMN, "%s: couldn't set nTriWithVert or nVertWithTri", me);
    airMopError(mop); return 1;
  }
  vertWithTri = AIR_CAST(unsigned int*, nVertWithTri->data);
  /* triWithVert = AIR_CAST(unsigned int*, nTriWithVert->data); */

  maxTriPerVert = nTriWithVert->axis[0].size - 1;
  intxBuff = AIR_CAST(unsigned int*, calloc(maxTriPerVert,
                                            sizeof(unsigned int)));
  if (!intxBuff) {
    biffAddf(LIMN, "%s: failed to alloc an itty bitty buffer", me);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, intxBuff, airFree, airMopAlways);

  /*
  nrrdSave("triWithVert.nrrd", nTriWithVert, NULL);
  nrrdSave("vertWithTri.nrrd", nVertWithTri, NULL);
  */

  /* create the stack of recently fixed triangles */
  appu.ui = &okay;
  okayArr = airArrayNew(appu.v, NULL, sizeof(unsigned int),
                        maxTriPerPrim);
  airMopAdd(mop, okayArr, (airMopper)airArrayNuke, airMopAlways);
  if (splitting) {
    appu.ui = &split;
    splitArr = airArrayNew(appu.v, NULL, 5*sizeof(unsigned int),
                           maxTriPerPrim);
    /* split set as it is used */
  } else {
    splitArr = NULL;
    split = NULL;
  }

  /* the skinny */
  doneTriNum = 0;
  trueTriNum = 0;
  for (primIdx=0; primIdx<pld->primNum; primIdx++) {
    trueTriNum += pld->icnt[primIdx]/3;
  }
  /*
  fprintf(stderr, "!%s: trueTriNum %u; other tri num %u\n", me,
          trueTriNum, limnPolyDataPolygonNumber(pld));
  */
  while (doneTriNum < trueTriNum) {
    /* find first undone triangle, which should be on a different
       connected component than any processed so far */
    for (totTriIdx=0; triDone[totTriIdx]; totTriIdx++)
      ;
    /* we use the winding of this triangle to determine the correct
       winding of all neighboring trianges, so this one is now done */
    triDone[totTriIdx] = AIR_TRUE;
    ++doneTriNum;
    /*
    fprintf(stderr, "!%s: considering tri %u done (%u)\n",
            me, totTriIdx, doneTriNum);
    */
    doneTriNum += neighborsCheckPush(nTriWithVert, nVertWithTri,
                                     triDone, okayArr, intxBuff, splitArr,
                                     totTriIdx, splitting);
    while (okayArr->len) {
      unsigned int popped;
      popped = okay[okayArr->len-1];
      airArrayLenIncr(okayArr, -1);
      /*
      fprintf(stderr, "!%s: popped %u\n", me, popped);
      */
      doneTriNum += neighborsCheckPush(nTriWithVert, nVertWithTri,
                                       triDone, okayArr, intxBuff, splitArr,
                                       popped, splitting);
    }
  }

  if (splitting) {
    if (doSplitting(pld, nTriWithVert, nVertWithTri, splitArr)) {
      biffAddf(LIMN, "%s: problem doing vertex splitting", me);
      return 1;
    }
  } else {
    /* Copy from nVertWithTri back into polydata */
    baseVertIdx = 0;
    for (primIdx=0; primIdx<pld->primNum; primIdx++) {
      unsigned int triNum, *indxLine, ii;
      triNum = pld->icnt[primIdx]/3;
      for (triIdx=0; triIdx<triNum; triIdx++) {
        totTriIdx = triIdx + baseVertIdx/3;
        indxLine = pld->indx + baseVertIdx + 3*triIdx;
        for (ii=0; ii<3; ii++) {
          indxLine[ii] = (vertWithTri + 3*totTriIdx)[ii];
        }
      }
      baseVertIdx += pld->icnt[primIdx];
    }
  }

  airMopOkay(mop);
  return 0;
}

/*
** with non-zero splitting, this does vertex splitting so that
** non-orientable surfaces can be rendered without seams. Took longer
** to implement than intended.
**
** HEY: still has a bug in handling which triangles get which
** (new) vertices when the seam in the non-orientable surface
** is a closed loop.  Can be debugged later...
*/
int
limnPolyDataVertexWindingFix(limnPolyData *pld, int splitting) {
  static const char me[]="limnPolyDataVertexWindingFix";

  if (!splitting) {
    if (_limnPolyDataVertexWindingProcess(pld, AIR_FALSE)) {
      biffAddf(LIMN, "%s: trouble", me);
      return 1;
    }
  } else {
    if (_limnPolyDataVertexWindingProcess(pld, AIR_FALSE)
        || _limnPolyDataVertexWindingProcess(pld, AIR_TRUE)) {
      biffAddf(LIMN, "%s: trouble", me);
      return 1;
    }
  }
  return 0;
}

int
limnPolyDataCCFind(limnPolyData *pld) {
  static const char me[]="limnPolyDataCCFind";
  unsigned int realTriNum, *triMap, *triWithVert, vertIdx,
    *indxOld, *indxNew, primNumOld, *icntOld, *icntNew, *baseIndx,
    primIdxNew, primNumNew, passIdx, eqvNum=0;
  unsigned char *typeOld, *typeNew;
  Nrrd *nTriWithVert, *nccSize, *nTriMap;
  airArray *mop, *eqvArr;

  if (!pld) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(pld->xyzwNum && pld->primNum)) {
    /* this is empty? */
    return 0;
  }

  if ((1 << limnPrimitiveTriangles) != limnPolyDataPrimitiveTypes(pld)) {
    biffAddf(LIMN, "%s: sorry, can only handle %s primitives", me,
             airEnumStr(limnPrimitive, limnPrimitiveTriangles));
    return 1;
  }

  mop = airMopNew();

  realTriNum = limnPolyDataPolygonNumber(pld);

  eqvArr = airArrayNew(NULL, NULL, 2*sizeof(unsigned int),
                       /* this is only a heuristic */ pld->xyzwNum);
  airMopAdd(mop, eqvArr, (airMopper)airArrayNuke, airMopAlways);

  nTriWithVert = nrrdNew();
  airMopAdd(mop, nTriWithVert, (airMopper)nrrdNuke, airMopAlways);
  if (triangleWithVertex(nTriWithVert, pld)) {
    biffAddf(LIMN, "%s: couldn't set nTriWithVert", me);
    airMopError(mop); return 1;
  }

  /* simple profiling showed that stupid amount of time was spent
     adding the equivalences.  So we go in two passes- first two see
     how many equivalences are needed, and then actually adding them */
  /* yea, so, its like you don't really even need an airArray ... */
  triWithVert = AIR_CAST(unsigned int*, nTriWithVert->data);
  for (passIdx=0; passIdx<2; passIdx++) {
    if (0 == passIdx) {
      eqvNum = 0;
    } else {
      airArrayLenPreSet(eqvArr, eqvNum);
    }
    for (vertIdx=0; vertIdx<nTriWithVert->axis[1].size; vertIdx++) {
      unsigned int *triLine, triIdx;
      triLine = triWithVert + vertIdx*(nTriWithVert->axis[0].size);
      for (triIdx=1; triIdx<triLine[0]; triIdx++) {
        if (0 == passIdx) {
          ++eqvNum;
        } else {
          airEqvAdd(eqvArr, triLine[1], triLine[1+triIdx]);
        }
      }
    }
  }

  nTriMap = nrrdNew();
  airMopAdd(mop, nTriMap, (airMopper)nrrdNuke, airMopAlways);
  nccSize = nrrdNew();
  airMopAdd(mop, nccSize, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdMaybeAlloc_va(nTriMap, nrrdTypeUInt, 1,
                        AIR_CAST(size_t, realTriNum))) {
    biffMovef(LIMN, NRRD, "%s: couldn't allocate equivalence map", me);
    airMopError(mop); return 1;
  }
  triMap = AIR_CAST(unsigned int*, nTriMap->data);
  primNumNew = airEqvMap(eqvArr, triMap, realTriNum);
  if (nrrdHisto(nccSize, nTriMap, NULL, NULL, primNumNew, nrrdTypeUInt)) {
    biffMovef(LIMN, NRRD, "%s: couldn't histogram CC map", me);
    airMopError(mop); return 1;
  }

  /* indxNumOld == indxNumNew */
  indxOld = pld->indx;
  primNumOld = pld->primNum;
  if (1 != primNumOld) {
    biffAddf(LIMN, "%s: sorry! stupid implementation can't "
             "do primNum %u (only 1)",
             me, primNumOld);
    airMopError(mop); return 1;
  }
  typeOld = pld->type;
  icntOld = pld->icnt;
  indxNew = AIR_CAST(unsigned int*,
                     calloc(pld->indxNum, sizeof(unsigned int)));
  typeNew = AIR_CAST(unsigned char*,
                     calloc(primNumNew, sizeof(unsigned char)));
  icntNew = AIR_CAST(unsigned int*,
                     calloc(primNumNew, sizeof(unsigned int)));
  if (!(indxNew && typeNew && icntNew)) {
    biffAddf(LIMN, "%s: couldn't allocate new polydata arrays", me);
    airMopError(mop); return 1;
  }
  pld->indx = indxNew;
  pld->primNum = primNumNew;
  pld->type = typeNew;
  pld->icnt = icntNew;
  airMopAdd(mop, indxOld, airFree, airMopAlways);
  airMopAdd(mop, typeOld, airFree, airMopAlways);
  airMopAdd(mop, icntOld, airFree, airMopAlways);

  /* this multi-pass thing is really stupid
     (and assumes stupid primNumOld = 1) */
  baseIndx = pld->indx;
  for (primIdxNew=0; primIdxNew<pld->primNum; primIdxNew++) {
    unsigned int realTriIdx;
    pld->type[primIdxNew] = limnPrimitiveTriangles;
    pld->icnt[primIdxNew] = 0;
    for (realTriIdx=0; realTriIdx<realTriNum; realTriIdx++) {
      if (triMap[realTriIdx] == primIdxNew) {
        ELL_3V_COPY(baseIndx, indxOld + 3*realTriIdx);
        baseIndx += 3;
        pld->icnt[primIdxNew] += 3;
      }
    }
  }

  airMopOkay(mop);
  return 0;
}

int
limnPolyDataPrimitiveSort(limnPolyData *pld, const Nrrd *_nval) {
  static const char me[]="limnPolyDataPrimitiveSort";
  Nrrd *nval, *nrec;
  const Nrrd *ntwo[2];
  airArray *mop;
  double *rec;
  unsigned int primIdx, **startIndx, *indxNew, *baseIndx, *icntNew;
  unsigned char *typeNew;
  int E;

  if (!(pld && _nval)) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(1 == _nval->dim
        && nrrdTypeBlock != _nval->type
        && _nval->axis[0].size == pld->primNum)) {
    biffAddf(LIMN, "%s: need 1-D %u-len scalar nrrd "
             "(not %u-D type %s, axis[0].size %u)", me,
             pld->primNum,
             _nval->dim, airEnumStr(nrrdType, _nval->type),
             AIR_CAST(unsigned int, _nval->axis[0].size));
    return 1;
  }

  mop = airMopNew();
  nval = nrrdNew();
  airMopAdd(mop, nval, (airMopper)nrrdNuke, airMopAlways);
  nrec = nrrdNew();
  airMopAdd(mop, nrec, (airMopper)nrrdNuke, airMopAlways);
  E = 0;
  if (!E) E |= nrrdConvert(nval, _nval, nrrdTypeDouble);
  ntwo[0] = nval;
  ntwo[1] = nval;
  if (!E) E |= nrrdJoin(nrec, ntwo, 2, 0, AIR_TRUE);
  if (E) {
    biffMovef(LIMN, NRRD, "%s: problem creating records", me);
    airMopError(mop); return 1;
  }
  rec = AIR_CAST(double *, nrec->data);
  for (primIdx=0; primIdx<pld->primNum; primIdx++) {
    rec[1 + 2*primIdx] = primIdx;
  }
  qsort(rec, pld->primNum, 2*sizeof(double),
        nrrdValCompareInv[nrrdTypeDouble]);

  startIndx = AIR_CAST(unsigned int**, calloc(pld->primNum,
                                              sizeof(unsigned int*)));
  indxNew = AIR_CAST(unsigned int*, calloc(pld->indxNum,
                                           sizeof(unsigned int)));
  icntNew = AIR_CAST(unsigned int*, calloc(pld->primNum,
                                           sizeof(unsigned int)));
  typeNew = AIR_CAST(unsigned char*, calloc(pld->primNum,
                                            sizeof(unsigned char)));
  if (!(startIndx && indxNew && icntNew && typeNew)) {
    biffAddf(LIMN, "%s: couldn't allocated temp buffers", me);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, startIndx, airFree, airMopAlways);

  baseIndx = pld->indx;
  for (primIdx=0; primIdx<pld->primNum; primIdx++) {
    startIndx[primIdx] = baseIndx;
    baseIndx += pld->icnt[primIdx];
  }
  baseIndx = indxNew;
  for (primIdx=0; primIdx<pld->primNum; primIdx++) {
    unsigned int sortIdx;
    sortIdx = AIR_CAST(unsigned int, rec[1 + 2*primIdx]);
    memcpy(baseIndx, startIndx[sortIdx],
           pld->icnt[sortIdx]*sizeof(unsigned int));
    icntNew[primIdx] = pld->icnt[sortIdx];
    typeNew[primIdx] = pld->type[sortIdx];
    baseIndx += pld->icnt[sortIdx];
  }

  airFree(pld->indx);
  pld->indx = indxNew;
  airFree(pld->type);
  pld->type = typeNew;
  airFree(pld->icnt);
  pld->icnt = icntNew;

  airMopOkay(mop);
  return 0;
}

int
limnPolyDataVertexWindingFlip(limnPolyData *pld) {
  static const char me[]="limnPolyDataVertexWindingFlip";
  unsigned int baseVertIdx, primIdx;

  if (!pld) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if ((1 << limnPrimitiveTriangles) != limnPolyDataPrimitiveTypes(pld)) {
    biffAddf(LIMN, "%s: sorry, can only handle %s primitives", me,
             airEnumStr(limnPrimitive, limnPrimitiveTriangles));
    return 1;
  }

  baseVertIdx = 0;
  for (primIdx=0; primIdx<pld->primNum; primIdx++) {
    unsigned int triNum, triIdx, *indxLine, tmpIdx;
    triNum = pld->icnt[primIdx]/3;
    for (triIdx=0; triIdx<triNum; triIdx++) {
      indxLine = pld->indx + baseVertIdx + 3*triIdx;
      ELL_SWAP2(indxLine[0], indxLine[2], tmpIdx);
    }
    baseVertIdx += pld->icnt[primIdx];
  }

  return 0;
}

int
limnPolyDataPrimitiveSelect(limnPolyData *pldOut,
                            const limnPolyData *pldIn,
                            const Nrrd *_nmask) {
  static const char me[]="limnPolyDataPrimitiveSelect";
  Nrrd *nmask;
  double *mask;
  unsigned int oldBaseVertIdx, oldPrimIdx, oldVertIdx, bitflag,
    *old2newMap, *new2oldMap,
    newPrimNum, newBaseVertIdx, newPrimIdx, newIndxNum, newVertIdx, newVertNum;
  unsigned char *vertUsed;
  airArray *mop;

  if (!(pldOut && pldIn && _nmask)) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(1 == _nmask->dim
        && nrrdTypeBlock != _nmask->type
        && _nmask->axis[0].size == pldIn->primNum)) {
    biffAddf(LIMN, "%s: need 1-D %u-len scalar nrrd "
             "(not %u-D type %s, axis[0].size %u)", me,
             pldIn->primNum, _nmask->dim, airEnumStr(nrrdType, _nmask->type),
             AIR_CAST(unsigned int, _nmask->axis[0].size));
    return 1;
  }

  mop = airMopNew();
  nmask = nrrdNew();
  airMopAdd(mop, nmask, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(nmask, _nmask, nrrdTypeDouble)) {
    biffMovef(LIMN, NRRD, "%s: trouble converting mask to %s", me,
              airEnumStr(nrrdType, nrrdTypeDouble));
    return 1;
  }
  mask = AIR_CAST(double *, nmask->data);

  old2newMap = AIR_CAST(unsigned int *, calloc(pldIn->xyzwNum,
                                               sizeof(unsigned int)));
  airMopAdd(mop, old2newMap, airFree, airMopAlways);
  vertUsed = AIR_CAST(unsigned char *, calloc(pldIn->xyzwNum,
                                              sizeof(unsigned char)));
  airMopAdd(mop, vertUsed, airFree, airMopAlways);

  /* initialize all verts as unused */
  for (oldVertIdx=0; oldVertIdx<pldIn->xyzwNum; oldVertIdx++) {
    vertUsed[oldVertIdx] = AIR_FALSE;
  }
  /* mark the used verts, and count # new indices and primitives  */
  oldBaseVertIdx = 0;
  newPrimNum = 0;
  newIndxNum = 0;
  for (oldPrimIdx=0; oldPrimIdx<pldIn->primNum; oldPrimIdx++) {
    unsigned indxIdx;
    if (mask[oldPrimIdx]) {
      for (indxIdx=0; indxIdx<pldIn->icnt[oldPrimIdx]; indxIdx++) {
        vertUsed[(pldIn->indx + oldBaseVertIdx)[indxIdx]] = AIR_TRUE;
      }
      newIndxNum += pldIn->icnt[oldPrimIdx];
      newPrimNum++;
    }
    oldBaseVertIdx += pldIn->icnt[oldPrimIdx];
  }
  /* count the used verts, and set up map from old to new indices */
  newVertNum = 0;
  for (oldVertIdx=0; oldVertIdx<pldIn->xyzwNum; oldVertIdx++) {
    if (vertUsed[oldVertIdx]) {
      old2newMap[oldVertIdx] = newVertNum++;
    }
  }
  /* allocate and fill reverse map */
  new2oldMap = AIR_CAST(unsigned int *, calloc(newVertNum,
                                               sizeof(unsigned int)));
  airMopAdd(mop, new2oldMap, airFree, airMopAlways);
  newVertIdx = 0;
  for (oldVertIdx=0; oldVertIdx<pldIn->xyzwNum; oldVertIdx++) {
    if (vertUsed[oldVertIdx]) {
      new2oldMap[newVertIdx++] = oldVertIdx;
    }
  }

  /* allocate output polydata */
  bitflag = limnPolyDataInfoBitFlag(pldIn);
  if (limnPolyDataAlloc(pldOut, bitflag, newVertNum, newIndxNum, newPrimNum)) {
    biffAddf(LIMN, "%s: trouble allocating output", me);
    return 1;
  }

  /* transfer per-primitive information from old to new */
  oldBaseVertIdx = 0;
  newBaseVertIdx = 0;
  newPrimIdx = 0;
  for (oldPrimIdx=0; oldPrimIdx<pldIn->primNum; oldPrimIdx++) {
    if (mask[oldPrimIdx]) {
      unsigned indxIdx;
      pldOut->icnt[newPrimIdx] = pldIn->icnt[oldPrimIdx];
      pldOut->type[newPrimIdx] = pldIn->type[oldPrimIdx];
      for (indxIdx=0; indxIdx<pldIn->icnt[oldPrimIdx]; indxIdx++) {
        oldVertIdx = (pldIn->indx + oldBaseVertIdx)[indxIdx];
        (pldOut->indx + newBaseVertIdx)[indxIdx] = old2newMap[oldVertIdx];
      }
      newBaseVertIdx += pldIn->icnt[oldPrimIdx];
      newPrimIdx++;
    }
    oldBaseVertIdx += pldIn->icnt[oldPrimIdx];
  }
  /* transfer per-vertex info */
  for (newVertIdx=0; newVertIdx<newVertNum; newVertIdx++) {
    oldVertIdx = new2oldMap[newVertIdx];
    ELL_4V_COPY(pldOut->xyzw + 4*newVertIdx, pldIn->xyzw + 4*oldVertIdx);
    if ((1 << limnPolyDataInfoRGBA) & bitflag) {
      ELL_4V_COPY(pldOut->rgba + 4*newVertIdx, pldIn->rgba + 4*oldVertIdx);
    }
    if ((1 << limnPolyDataInfoNorm) & bitflag) {
      ELL_3V_COPY(pldOut->norm + 3*newVertIdx, pldIn->norm + 3*oldVertIdx);
    }
    if ((1 << limnPolyDataInfoTex2) & bitflag) {
      ELL_3V_COPY(pldOut->tex2 + 2*newVertIdx, pldIn->tex2 + 2*oldVertIdx);
    }
    if ((1 << limnPolyDataInfoTang) & bitflag) {
      ELL_3V_COPY(pldOut->tang + 3*newVertIdx, pldIn->tang + 3*oldVertIdx);
    }
  }

  airMopOkay(mop);
  return 0;
}

/* Helper function for limnPolyDataClipMulti - clips the edge between
 * disc and kept that partially fulfills the thresholds and maintains
 * a data structure that keeps track of edges we have clipped already,
 * to avoid creating duplicate vertices.
 */
static int
clipEdge(int disc, int kept, Nrrd *nval, double *thresh, int *newIdx,
         airArray *llistArr, limnPolyData *pld, unsigned int bitflag,
         limnPolyData *newpld, airArray *xyzwArr, airArray *rgbaArr,
         airArray *normArr, airArray *tex2Arr, airArray *tangArr) {
  int ref=-1, *llist=(int*)llistArr->data;
  int next=newIdx[disc];
  double alpha=0;
  unsigned int i,q,p,nk;
  double (*lup)(const void *v, size_t I)=nrrdDLookup[nval->type];
  /* check if we clipped the edge previously */
  while (next!=-1) {
    if (llist[next]==kept) /* found the desired vertex */
      return llist[next+1];
    ref=next+2;
    next=llist[next+2];
  }
  /* we need to interpolate - find the weight */
  nk=(nval->dim==1)?1:nval->axis[0].size;
  for (i=0; i<nk; i++) {
    double discval = lup(nval->data, nk*disc+i);
    double keptval = lup(nval->data, nk*kept+i);
    double thisalpha = AIR_AFFINE(discval,thresh[i],keptval,0.0,1.0);
    if (thisalpha<1.0 && thisalpha>alpha)
      alpha=thisalpha;
  }
  /* add interpolated vertex */
  q=airArrayLenIncr(xyzwArr, 1);
  ELL_4V_LERP_TT(newpld->xyzw+4*q, float, alpha, pld->xyzw+4*disc, pld->xyzw+4*kept);
  if ((1 << limnPolyDataInfoRGBA) & bitflag) {
    airArrayLenIncr(rgbaArr, 1);
    ELL_4V_LERP_TT(newpld->rgba+4*q, unsigned char, alpha, pld->rgba+4*disc, pld->rgba+4*kept);
  }
  if ((1 << limnPolyDataInfoNorm) & bitflag) {
    float fnorm[3];
    double len;
    /* take special care to treat non-orientable surface normals correctly */
    if (ELL_3V_DOT(pld->norm+3*disc, pld->norm+3*kept)<0) {
      ELL_3V_SCALE_TT(fnorm, float, -1.0, pld->norm+3*kept);
    } else {
      ELL_3V_COPY(fnorm, pld->norm+3*kept);
    }
    airArrayLenIncr(normArr, 1);
    ELL_3V_LERP_TT(newpld->norm+3*q, float, alpha, pld->norm+3*disc, fnorm);
    /* re-normalize */
    len=ELL_3V_LEN(newpld->norm+3*q);
    if (len>1e-20) {
      ELL_3V_SCALE_TT(newpld->norm+3*q, float, 1.0/len, newpld->norm+3*q);
    }
  }
  if ((1 << limnPolyDataInfoTex2) & bitflag) {
    airArrayLenIncr(tex2Arr, 1);
    ELL_2V_LERP_TT(newpld->tex2+2*q, float, alpha, pld->tex2+2*disc, pld->tex2+2*kept);
  }
  if ((1 << limnPolyDataInfoTang) & bitflag) {
    airArrayLenIncr(tangArr, 1);
    ELL_3V_LERP_TT(newpld->tang+3*q, float, alpha, pld->tang+3*disc, pld->tang+3*kept);
  }
  /* add new vertex to linked list */
  p=airArrayLenIncr(llistArr, 1);
  llist=(int*)llistArr->data; /* update in case of re-allocation */
  llist[3*p]=kept;
  llist[3*p+1]=q;
  llist[3*p+2]=-1;
  if (ref==-1) newIdx[disc]=3*p;
  else llist[ref]=3*p;
  return q;
}

/*
 * Clips the given triangles (limnPrimitiveTriangles) according to the
 * input matrix nval and the threshold array thresh. First axis of
 * nval are different clipping criteria, second axis are vertex
 * indices. The length of thresh has to equal the size of the first
 * axis, the vertex count in pld has to equal the size of the second
 * axis. If nval is 1D, it is assumed to have a single criterion.
 *
 * A vertex is preserved if all values are >= the respective
 * threshold; triangles with partially discarded vertices are clipped,
 * potentially generating a quad that is then triangulated arbitrarily.
 */
int
limnPolyDataClipMulti(limnPolyData *pld, Nrrd *nval, double *thresh) {
  static const char me[]="limnPolyDataClipMulti";
  unsigned char *keepVert=NULL;
  airArray *mop;
  unsigned int E, i, idx=0;
  double (*lup)(const void *v, size_t I);
  airArray *xyzwArr, *rgbaArr=NULL, *normArr=NULL, *tex2Arr=NULL,
    *tangArr=NULL, *indxArr, *typeArr, *icntArr, *llistArr=NULL;
  limnPolyData *newpld=NULL;
  int *newIdx=NULL, *llist=NULL;
  unsigned int bitflag, nk, nvert;
  airPtrPtrUnion appu;

  if (!(pld && nval)) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }

  if (nrrdTypeBlock == nval->type) {
    biffAddf(LIMN, "%s: need scalar type (not %s)", me,
             airEnumStr(nrrdType, nval->type));
    return 1;
  }

  if (nval->dim==1) {
    nk=1; nvert=nval->axis[0].size;
  } else if (nval->dim==2) {
    nk=nval->axis[0].size; nvert=nval->axis[1].size;
  } else {
    biffAddf(LIMN, "%s: need 1D or 2D input array, got %uD", me, nval->dim);
    return 1;
  }
  if (nvert!=pld->xyzwNum) {
    biffAddf(LIMN, "%s: # verts %u != # values %u", me,
             pld->xyzwNum, nvert);
    return 1;
  }
  if ((1 << limnPrimitiveTriangles) != limnPolyDataPrimitiveTypes(pld)) {
    biffAddf(LIMN, "%s: sorry, can only handle %s primitives", me,
             airEnumStr(limnPrimitive, limnPrimitiveTriangles));
    return 1;
  }

  /* Memory allocation in C IS a headache */
  mop=airMopNew();
  E = AIR_FALSE;
  if (!E) {
    E|=!(keepVert = AIR_CAST(unsigned char *,
                             calloc(pld->xyzwNum, sizeof(char))));
  }
  if (!E) {
    airMopAdd(mop, keepVert, airFree, airMopAlways);
    E|=!(newIdx = AIR_CAST(int *, malloc(pld->xyzwNum*sizeof(int))));
  }
  if (!E) {
    unsigned int incr;
    airMopAdd(mop, newIdx, airFree, airMopAlways);
    memset(newIdx, -1, sizeof(int)*pld->xyzwNum);
    /* This setting of incr is arbitrary and was not optimized in any way: */
    incr = pld->xyzwNum/10; /* 10% of previous vertex count... */
    if (incr<50) incr=50; /* ...but at least 50. */
    appu.i = &llist;
    E|=!(llistArr=airArrayNew(appu.v, NULL, 3*sizeof(int), incr));
  }
  if (!E) {
    airMopAdd(mop, llistArr, (airMopper)airArrayNuke, airMopAlways);
    E|=!(newpld = limnPolyDataNew());
  }
  bitflag = limnPolyDataInfoBitFlag(pld);
  if (!E) {
    unsigned int incr;
    airMopAdd(mop, newpld, airFree, airMopAlways); /* "shallow" free */
    incr = pld->xyzwNum/20; /* 5% of previous vertex count... */
    if (incr<10) incr=10; /* ...but at least 10. */
    appu.f = &(newpld->xyzw);
    E|=!(xyzwArr=airArrayNew(appu.v, &(newpld->xyzwNum),
                             4*sizeof(float), incr));
    if (!E) {
      airMopAdd(mop, xyzwArr, (airMopper)airArrayNuke, airMopOnError);
      airMopAdd(mop, xyzwArr, (airMopper)airArrayNix, airMopOnOkay);
    }
    if (!E && (1 << limnPolyDataInfoRGBA) & bitflag) {
      appu.uc = &(newpld->rgba);
      E|=!(rgbaArr=airArrayNew(appu.v, &(newpld->rgbaNum),
                               4*sizeof(unsigned char), incr));
      if (!E) {
        airMopAdd(mop, rgbaArr, (airMopper)airArrayNuke, airMopOnError);
        airMopAdd(mop, rgbaArr, (airMopper)airArrayNix, airMopOnOkay);
      }
    }
    if (!E && (1 << limnPolyDataInfoNorm) & bitflag) {
      appu.f = &(newpld->norm);
      E|=!(normArr=airArrayNew(appu.v, &(newpld->normNum),
                               3*sizeof(float), incr));
      if (!E) {
        airMopAdd(mop, normArr, (airMopper)airArrayNuke, airMopOnError);
        airMopAdd(mop, normArr, (airMopper)airArrayNix, airMopOnOkay);
      }
    }
    if (!E && (1 << limnPolyDataInfoTex2) & bitflag) {
      appu.f = &(newpld->tex2);
      E|=!(tex2Arr=airArrayNew(appu.v, &(newpld->tex2Num),
                               2*sizeof(float), incr));
      if (!E) {
        airMopAdd(mop, tex2Arr, (airMopper)airArrayNuke, airMopOnError);
        airMopAdd(mop, tex2Arr, (airMopper)airArrayNix, airMopOnOkay);
      }
    }
    if (!E && (1 << limnPolyDataInfoTang) & bitflag) {
      appu.f = &(newpld->tang);
      E|=!(tangArr=airArrayNew(appu.v, &(newpld->tangNum),
                               3*sizeof(float), incr));
      if (!E) {
        airMopAdd(mop, tangArr, (airMopper)airArrayNuke, airMopOnError);
        airMopAdd(mop, tangArr, (airMopper)airArrayNix, airMopOnOkay);
      }
    }
    if (!E) {
      incr = pld->indxNum/20; /* 5% of previous index count... */
      if (incr<10) incr=10; /* ...but at least 10. */
      appu.ui = &(newpld->indx);
      E|=!(indxArr=airArrayNew(appu.v, &(newpld->indxNum),
                               sizeof(unsigned int), incr));
      if (!E) {
        airMopAdd(mop, indxArr, (airMopper)airArrayNuke, airMopOnError);
        airMopAdd(mop, indxArr, (airMopper)airArrayNix, airMopOnOkay);
      }
    }
    if (!E) {
      incr = pld->primNum/10; /* 10% of previous primNum... */
      if (incr<1) incr=1; /* ...but at least 1. */
      appu.uc = &(newpld->type);
      E|=!(typeArr=airArrayNew(appu.v, &(newpld->primNum),
                               sizeof(unsigned char), incr));
      if (!E) {
        airMopAdd(mop, typeArr, (airMopper)airArrayNuke, airMopOnError);
        airMopAdd(mop, typeArr, (airMopper)airArrayNix, airMopOnOkay);
      }
      appu.ui = &(newpld->icnt);
      E|=!(icntArr=airArrayNew(appu.v, NULL,
                               sizeof(unsigned int), incr));
      if (!E) {
        airMopAdd(mop, icntArr, (airMopper)airArrayNuke, airMopOnError);
        airMopAdd(mop, icntArr, (airMopper)airArrayNix, airMopOnOkay);
      }
    }
  }
  if (E) {
    biffAddf(LIMN, "%s: couldn't allocate buffers", me);
    airMopError(mop); return 1;
  }

  /* mark vertices, leaving at 0 means "discard" */
  lup = nrrdDLookup[nval->type];
  for (i=0; i<pld->xyzwNum; i++) {
    unsigned int j, keep = AIR_TRUE;
    for (j=0; j<nk; j++, idx++) {
      if (lup(nval->data, idx) < thresh[j])
        keep = AIR_FALSE;
    }
    if (keep) {
      keepVert[i]=AIR_TRUE;
    }
  }

  /* now, iterate over all primitives and triangles */

  /* Note: If keepVert[i]==AIR_TRUE, newIdx[i] is its new index; else, it is
   * an index j into llist, which is a linked list:
   * llist[j] == other (kept) end of the edge
   * llist[j+1] == index of new vertex for that edge
   * llist[j+2] == next index into llist
   */

  /* TODO: All the airArray stuff should have allocation error checking */
  idx=0;
  for (i=0; i<pld->primNum; i++) {
    int j, oldTriNum=pld->icnt[i]/3, newTriNum=0;
    unsigned int kept=0, disck=0; /* index of last kept / discarded vertex */
    for (j=0; j<oldTriNum; j++, idx+=3) {
      unsigned int p, quad[4];
      int k, keepN=0;
      for (k=0; k<3; k++) {
        unsigned int oldidx=pld->indx[idx+k];
        if (keepVert[oldidx]) {
          keepN++; kept=oldidx;
          /* make sure the vertex is copied over */
          if (newIdx[oldidx]==-1) {
            unsigned int q=newIdx[oldidx]=airArrayLenIncr(xyzwArr, 1);
            ELL_4V_COPY(newpld->xyzw+4*q, pld->xyzw+4*oldidx);
            if ((1 << limnPolyDataInfoRGBA) & bitflag) {
              airArrayLenIncr(rgbaArr, 1);
              ELL_4V_COPY(newpld->rgba+4*q, pld->rgba+4*oldidx);
            }
            if ((1 << limnPolyDataInfoNorm) & bitflag) {
              airArrayLenIncr(normArr, 1);
              ELL_3V_COPY(newpld->norm+3*q, pld->norm+3*oldidx);
            }
            if ((1 << limnPolyDataInfoTex2) & bitflag) {
              airArrayLenIncr(tex2Arr, 1);
              ELL_2V_COPY(newpld->tex2+2*q, pld->tex2+2*oldidx);
            }
            if ((1 << limnPolyDataInfoTang) & bitflag) {
              airArrayLenIncr(tangArr, 1);
              ELL_3V_COPY(newpld->tang+3*q, pld->tang+3*oldidx);
            }
          }
        } else {
          disck=k;
        }
      }
      switch (keepN) {
      case 0: /* nothing to be done; discard this triangle */
        break;
      case 1: /* result of clipping is a single triangle */
        newTriNum++;
        p=airArrayLenIncr(indxArr, 3);
        for (k=0; k<3; k++) {
          if (keepVert[pld->indx[idx+k]])
            newpld->indx[p+k]=newIdx[pld->indx[idx+k]];
          else
            newpld->indx[p+k]=clipEdge(pld->indx[idx+k], kept, nval, thresh,
                                       newIdx, llistArr, pld, bitflag, newpld,
                                       xyzwArr, rgbaArr, normArr,
                                       tex2Arr, tangArr);
        }
        break;
      case 2: /* result of clipping is a quad, triangulate */
        newTriNum+=2;
        p=0;
        for (k=0; k<3; k++) {
          if (keepVert[pld->indx[idx+k]]) quad[p++]=newIdx[pld->indx[idx+k]];
          else {
            quad[p++]=clipEdge(pld->indx[idx+k], pld->indx[idx+(disck+2)%3],
                               nval, thresh, newIdx, llistArr,
                               pld, bitflag, newpld, xyzwArr, rgbaArr,
                               normArr, tex2Arr, tangArr);
            quad[p++]=clipEdge(pld->indx[idx+k], pld->indx[idx+(disck+1)%3],
                               nval, thresh, newIdx, llistArr,
                               pld, bitflag, newpld, xyzwArr, rgbaArr,
                               normArr, tex2Arr, tangArr);
          }
        }
        p=airArrayLenIncr(indxArr, 6);
        ELL_3V_SET(newpld->indx+p, quad[0], quad[1], quad[3]);
        ELL_3V_SET(newpld->indx+p+3, quad[1], quad[2], quad[3]);
        break;
      case 3: /* simply copy the existing triangle */
        newTriNum++;
        p=airArrayLenIncr(indxArr, 3);
        for (k=0; k<3; k++) {
          newpld->indx[p+k]=newIdx[pld->indx[idx+k]];
        }
        break;
      }
    }
    if (newTriNum>0) {
      unsigned int p=airArrayLenIncr(typeArr, 1);
      airArrayLenIncr(icntArr, 1);
      newpld->type[p]=limnPrimitiveTriangles;
      newpld->icnt[p]=newTriNum*3;
    }
  }

  /* finally, replace contents of pld with new data */
  airFree(pld->xyzw);
  airFree(pld->rgba);
  airFree(pld->norm);
  airFree(pld->tex2);
  airFree(pld->tang);
  airFree(pld->indx);
  airFree(pld->type);
  airFree(pld->icnt);
  memcpy(pld, newpld, sizeof(limnPolyData));

  airMopOkay(mop);
  return 0;
}

/* Simple wrapper around limnPolyDataClipMulti, in case of only one
 * clipping criterion.
 */
int
limnPolyDataClip(limnPolyData *pld, Nrrd *nval, double thresh) {
  return limnPolyDataClipMulti(pld, nval, &thresh);
}

/* limnPolyDataCompress:
 * returns a "compressed" copy of the given limnPolyData pld that only
 *  contains vertices that are referenced by some primitive
 * returns NULL and adds a message to biff upon error
 */
limnPolyData *
limnPolyDataCompress(const limnPolyData *pld) {
  static const char me[]="limnPolyDataCompress";
  limnPolyData *ret = NULL;
  unsigned int infoBitFlag=0, vertNum=0, i, used_indxNum=0;
  int *vertMap;
  if (pld==NULL) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return NULL;
  }
  infoBitFlag=limnPolyDataInfoBitFlag(pld);
  vertMap=(int*) calloc(pld->xyzwNum,sizeof(int));
  if (vertMap==NULL) {
    biffAddf(LIMN, "%s: could not allocate memory", me);
    return NULL;
  }
  /* how many indices are actually used? */
  for (i=0; i<pld->primNum; i++) {
    used_indxNum+=pld->icnt[i];
  }
  /* loop over all indices and mark referenced vertices in vertMap */
  for (i=0; i<used_indxNum; i++) {
    vertMap[pld->indx[i]]=1;
  }
  /* turn vertMap into an index map */
  for (i=0; i<pld->xyzwNum; i++) {
    if (vertMap[i]==0)
      vertMap[i]=-1;
    else
      vertMap[i]=vertNum++;
  }
  /* allocate new limnPolyData */
  if (NULL == (ret=limnPolyDataNew()) ||
      0!=limnPolyDataAlloc(ret, infoBitFlag, vertNum,
                           used_indxNum, pld->primNum)) {
    biffAddf(LIMN, "%s: Could not allocate result", me);
    free(vertMap);
    return NULL;
  }
  /* fill the newly allocated structure */
  for (i=0; i<pld->xyzwNum; i++) {
    if (vertMap[i]>=0) {
      ELL_4V_COPY(ret->xyzw+4*vertMap[i], pld->xyzw+4*i);
    }
  }
  if (ret->rgba!=NULL) {
    for (i=0; i<pld->xyzwNum; i++) {
      if (vertMap[i]>=0) {
        ELL_4V_COPY(ret->rgba+4*vertMap[i], pld->rgba+4*i);
      }
    }
  }
  if (ret->norm!=NULL) {
    for (i=0; i<pld->xyzwNum; i++) {
      if (vertMap[i]>=0) {
        ELL_3V_COPY(ret->norm+3*vertMap[i], pld->norm+3*i);
      }
    }
  }
  if (ret->tex2!=NULL) {
    for (i=0; i<pld->xyzwNum; i++) {
      if (vertMap[i]>=0) {
        ELL_2V_COPY(ret->tex2+2*vertMap[i], pld->tex2+2*i);
      }
    }
  }
  if (ret->tang!=NULL) {
    for (i=0; i<pld->xyzwNum; i++) {
      if (vertMap[i]>=0) {
        ELL_3V_COPY(ret->tang+3*vertMap[i], pld->tang+3*i);
      }
    }
  }
  for (i=0; i<used_indxNum; i++) {
    ret->indx[i]=vertMap[pld->indx[i]];
  }
  memcpy(ret->type, pld->type, sizeof(char)*pld->primNum);
  memcpy(ret->icnt, pld->icnt, sizeof(int)*pld->primNum);

  free(vertMap);

  return ret;
}

/* limnPolyDataJoin:
 * concatenates the primitives in all num limnPolyDatas given in plds
 *  and returns the result as a newly allocated limnPolyData
 * the new limnPolyData will only have color/normals/texture coordinates
 *  if _all_ input limnPolyDatas had the respective attribute
 * returns NULL and adds a message to biff upon error
 */
limnPolyData *limnPolyDataJoin(const limnPolyData **plds,
                               unsigned int num) {
  static const char me[]="limnPolyDataJoin";
  limnPolyData *ret = NULL;
  unsigned int infoBitFlag=(1 << limnPolyDataInfoRGBA) |
    (1 << limnPolyDataInfoNorm) |
    (1 << limnPolyDataInfoTex2) |
    (1 << limnPolyDataInfoTang); /* by default, assume we have all these */
  unsigned int vertNum=0, indxNum=0, primNum=0;
  unsigned int i;
  if (plds==NULL) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return NULL;
  }
  /* loop over all input plds to find infoBitFlag and the total number of
   * vertices / indices / primitives */
  for (i=0; i<num; i++) {
    if (plds[i]==NULL) {
      biffAddf(LIMN, "%s: plds[%d] is a NULL pointer", me, i);
      return NULL;
    }
    infoBitFlag &= limnPolyDataInfoBitFlag(plds[i]);
    vertNum += plds[i]->xyzwNum;
    indxNum += plds[i]->indxNum;
    primNum += plds[i]->primNum;
  }
  if (NULL == (ret=limnPolyDataNew()) ||
      0!=limnPolyDataAlloc(ret, infoBitFlag, vertNum, indxNum, primNum)) {
    biffAddf(LIMN, "%s: Could not allocate result", me);
    return NULL;
  }
  /* loop again over all input plds and fill the newly allocated structure */
  vertNum=indxNum=primNum=0;
  for (i=0; i<num; i++) {
    unsigned int j, used_indxNum=0;
    memcpy(ret->xyzw+4*vertNum, plds[i]->xyzw,
           sizeof(float)*4*plds[i]->xyzwNum);
    if (ret->rgba!=NULL) {
      memcpy(ret->rgba+4*vertNum, plds[i]->rgba,
             sizeof(unsigned char)*4*plds[i]->xyzwNum);
    }
    if (ret->norm!=NULL) {
      memcpy(ret->norm+3*vertNum, plds[i]->norm,
             sizeof(float)*3*plds[i]->xyzwNum);
    }
    if (ret->tex2!=NULL) {
      memcpy(ret->tex2+2*vertNum, plds[i]->tex2,
             sizeof(float)*2*plds[i]->xyzwNum);
    }
    if (ret->tang!=NULL) {
      memcpy(ret->tang+3*vertNum, plds[i]->tang,
             sizeof(float)*3*plds[i]->xyzwNum);
    }
    for (j=0; j<plds[i]->indxNum; j++) {
      ret->indx[indxNum+j]=vertNum+plds[i]->indx[j];
    }
    for (j=0; j<plds[i]->primNum; j++) {
      ret->type[primNum+j]=plds[i]->type[j];
      ret->icnt[primNum+j]=plds[i]->icnt[j];
      /* need to keep track of how many indices are actually used */
      used_indxNum+=plds[i]->icnt[j];
    }
    vertNum+=plds[i]->xyzwNum;
    indxNum+=used_indxNum;
    primNum+=plds[i]->primNum;
  }
  return ret;
}

int
limnPolyDataEdgeHalve(limnPolyData *pldOut,
                      const limnPolyData *pldIn) {
  static const char me[]="limnPolyDataEdgeHalve";
  Nrrd *nnewvert;
  unsigned int *newvert, nvold, nvidx, triidx, trinum, vlo, vhi, bitflag;
  airArray *mop;

  if ((1 << limnPrimitiveTriangles) != limnPolyDataPrimitiveTypes(pldIn)) {
    biffAddf(LIMN, "%s: sorry, can only handle %s primitives", me,
             airEnumStr(limnPrimitive, limnPrimitiveTriangles));
    return 1;
  }
  if (1 != pldIn->primNum) {
    biffAddf(LIMN, "%s: sorry, can only handle a single primitive", me);
    return 1;
  }
  mop = airMopNew();
  nnewvert = nrrdNew();
  airMopAdd(mop, nnewvert, AIR_CAST(airMopper, nrrdNuke), airMopAlways);
  nvold = pldIn->xyzwNum;
  if (nrrdMaybeAlloc_va(nnewvert, nrrdTypeUInt, 2, nvold, nvold)) {
    biffMovef(LIMN, NRRD, "%s: couldn't allocate buffer", me);
    airMopError(mop); return 1;
  }
  newvert = AIR_CAST(unsigned int*, nnewvert->data);

  /* run through triangles, recording edges with the new vertex index */
  nvidx = nvold;
  trinum = pldIn->indxNum/3;
  for (triidx=0; triidx<trinum; triidx++) {
    vlo = pldIn->indx[0 + 3*triidx];
    vhi = pldIn->indx[1 + 3*triidx];
    if (!newvert[vlo + nvold*vhi]) {
      newvert[vlo + nvold*vhi] = newvert[vhi + nvold*vlo] = nvidx++;
    }
    vlo = pldIn->indx[1 + 3*triidx];
    vhi = pldIn->indx[2 + 3*triidx];
    if (!newvert[vlo + nvold*vhi]) {
      newvert[vlo + nvold*vhi] = newvert[vhi + nvold*vlo] = nvidx++;
    }
    vlo = pldIn->indx[2 + 3*triidx];
    vhi = pldIn->indx[0 + 3*triidx];
    if (!newvert[vlo + nvold*vhi]) {
      newvert[vlo + nvold*vhi] = newvert[vhi + nvold*vlo] = nvidx++;
    }
  }

  /* allocate output */
  bitflag = limnPolyDataInfoBitFlag(pldIn);
  if (limnPolyDataAlloc(pldOut, bitflag, nvidx, 3*4*trinum, 1)) {
    biffAddf(LIMN, "%s: trouble allocating output", me);
    airMopError(mop); return 1;
  }
  pldOut->type[0] = limnPrimitiveTriangles;
  pldOut->icnt[0] = 3*4*trinum;

  /* set output indx */
  for (triidx=0; triidx<trinum; triidx++) {
    unsigned int aa, ab, bb, bc, cc, ac;
    aa = pldIn->indx[0 + 3*triidx];
    bb = pldIn->indx[1 + 3*triidx];
    cc = pldIn->indx[2 + 3*triidx];
    ab = newvert[aa + nvold*bb];
    bc = newvert[bb + nvold*cc];
    ac = newvert[aa + nvold*cc];
    ELL_3V_SET(pldOut->indx + 3*(0 + 4*triidx), aa, ab, ac);
    ELL_3V_SET(pldOut->indx + 3*(1 + 4*triidx), ab, bc, ac);
    ELL_3V_SET(pldOut->indx + 3*(2 + 4*triidx), ab, bb, bc);
    ELL_3V_SET(pldOut->indx + 3*(3 + 4*triidx), ac, bc, cc);
  }

  /* set output vertex info */
  for (vlo=0; vlo<nvold; vlo++) {
    ELL_4V_COPY(pldOut->xyzw + 4*vlo, pldIn->xyzw + 4*vlo);
    if ((1 << limnPolyDataInfoRGBA) & bitflag) {
      ELL_4V_COPY(pldOut->rgba + 4*vlo, pldIn->rgba + 4*vlo);
    }
    if ((1 << limnPolyDataInfoNorm) & bitflag) {
      ELL_3V_COPY(pldOut->norm + 3*vlo, pldIn->norm + 3*vlo);
    }
    if ((1 << limnPolyDataInfoTex2) & bitflag) {
      ELL_2V_COPY(pldOut->tex2 + 2*vlo, pldIn->tex2 + 2*vlo);
    }
    if ((1 << limnPolyDataInfoTang) & bitflag) {
      ELL_3V_COPY(pldOut->tang + 3*vlo, pldIn->tang + 3*vlo);
    }
    for (vhi=vlo+1; vhi<nvold; vhi++) {
      unsigned int mid;
      mid = newvert[vlo + nvold*vhi];
      if (!mid) {
        continue;
      }
      ELL_4V_LERP(pldOut->xyzw + 4*mid, 0.5f,
                  pldIn->xyzw + 4*vlo,
                  pldIn->xyzw + 4*vhi);
      if ((1 << limnPolyDataInfoRGBA) & bitflag) {
        ELL_4V_LERP_TT(pldOut->rgba + 4*mid, unsigned char, 0.5f,
                       pldIn->rgba + 4*vlo,
                       pldIn->rgba + 4*vhi);
      }
      if ((1 << limnPolyDataInfoNorm) & bitflag) {
        float tmp;
        ELL_3V_LERP(pldOut->norm + 3*mid, 0.5f,
                    pldIn->norm + 3*vlo,
                    pldIn->norm + 3*vhi);
        ELL_3V_NORM_TT(pldOut->norm + 3*mid, float,
                       pldOut->norm + 3*mid, tmp);
      }
      if ((1 << limnPolyDataInfoTex2) & bitflag) {
        ELL_2V_LERP(pldOut->tex2 + 2*mid, 0.5f,
                    pldIn->tex2 + 2*vlo,
                    pldIn->tex2 + 2*vhi);
      }
      if ((1 << limnPolyDataInfoTang) & bitflag) {
        float tmp;
        ELL_3V_LERP(pldOut->tang + 3*mid, 0.5f,
                    pldIn->tang + 3*vlo,
                    pldIn->tang + 3*vhi);
        ELL_3V_NORM_TT(pldOut->tang + 3*mid, float,
                       pldOut->tang + 3*mid, tmp);
      }
    }
  }

  airMopOkay(mop);
  return 0;
}

/* helper function for the limnPolyDataNeighborList below */
static void
registerNeighbor(unsigned int *nblist, size_t *len, unsigned int *maxnb,
                 unsigned int u, unsigned int v) {
  unsigned int idx=nblist[u], pointer=u, depth=1;
  while (idx!=0) {
    if (nblist[idx]==v)
      return; /* has already been registered */
    pointer=idx+1;
    idx=nblist[pointer];
    depth++;
  }
  if (depth>*maxnb)
    *maxnb=depth;
  /* do the registration */
  nblist[pointer]=*len;
  nblist[*len]=v;
  nblist[*len+1]=0;
  (*len)+=2;
  /* now the other way around */
  idx=nblist[v]; pointer=v;
  while (idx!=0) {
    /* do not have to check nblist[idx]==u due to symmetry */
    pointer=idx+1;
    idx=nblist[pointer];
  }
  nblist[pointer]=*len;
  nblist[*len]=u;
  nblist[*len+1]=0;
  (*len)+=2;
}

/* Here's the thing with all these limnPolyDataNeighbor* functions:
 *
 * *List is used for building up the information and called by all others
 * *Array is a great representation if all vertices have a similar number
 *        of neighbors (in particular, no gross outliers)
 *        The output of this is used by glyph coloring in elf.
 * *ArrayComp is a compressed representation that is best if vertices have
 *            a more variable number of neighbors
 *            The output of this is used by surface smoothing in limn.
 */

/* Mallocs *nblist and fills it with a linked list that contains all neighbors
 * of all n vertices in the given limnPolyData. The format is as follows:
 * For v<n, (*nblist)[v] is an index i (i>=n) into *nblist, or 0 if vertex v
 *   does not have any neighbors.
 * For an index i obtained this way, (*nblist)[i] is a neighbor of v,
 *   (*nblist)[i+1] is the index of the next list element (or 0).
 * If non-NULL, *len is set to the required size of *nblist - the initial malloc
 *   makes a conservative guess and you may want to realloc the result to *len
 *   bytes in order to free memory that ended up unused
 * If non-NULL, *m is set to the maximum number of neighbors (over all vertices)
 * Return value is 0 upon success, -1 upon error. Biff is used for errors.
 */
int
limnPolyDataNeighborList(unsigned int **nblist, size_t *len,
                         unsigned int *maxnb, const limnPolyData *pld) {
  static const char me[]="limnPolyDataNeighborList";
  unsigned int i, j, m, estimate=0, *indx;
  size_t last;
  /* estimate the maximum number of neighborhood relations */
  for (i=0; i<pld->primNum; i++) {
    switch (pld->type[i]) {
    case limnPrimitiveTriangles:
    case limnPrimitiveQuads:
      estimate+=pld->icnt[i]*2;
      break;
    case limnPrimitiveTriangleStrip:
    case limnPrimitiveTriangleFan:
      estimate+=4*(pld->icnt[i]-2)+2;
      break;
    case limnPrimitiveLineStrip:
      estimate+=2*(pld->icnt[i]-1);
      break;
    case limnPrimitiveLines:
      estimate+=pld->icnt[i];
      break;
    default: /* should be a noop; silently ignore it */
      break;
    }
  }
  /* allocate *nblist */
  *nblist = (unsigned int*) malloc(sizeof(unsigned int)*
                                   (pld->xyzwNum+2*estimate));
  if (*nblist==NULL) {
    biffAddf(LIMN, "%s: couldn't allocate nblist buffer", me);
    return -1;
  }
  /* populate the list */
  memset(*nblist, 0, sizeof(unsigned int)*pld->xyzwNum);
  last=pld->xyzwNum; m=0; indx=pld->indx;
  for (i=0; i<pld->primNum; i++) {
    switch (pld->type[i]) {
    case limnPrimitiveTriangles:
      for (j=0; j<pld->icnt[i]; j+=3) { /* go through all triangles */
        registerNeighbor(*nblist, &last, &m, indx[j], indx[j+1]);
        registerNeighbor(*nblist, &last, &m, indx[j+1], indx[j+2]);
        registerNeighbor(*nblist, &last, &m, indx[j+2], indx[j]);
      }
      break;
    case limnPrimitiveTriangleStrip:
      if (pld->icnt[i]>0)
        registerNeighbor(*nblist, &last, &m, indx[0], indx[1]);
      for (j=0; j<pld->icnt[i]-2; j++) {
        registerNeighbor(*nblist, &last, &m, indx[j], indx[j+2]);
        registerNeighbor(*nblist, &last, &m, indx[j+1], indx[j+2]);
      }
      break;
    case limnPrimitiveTriangleFan:
      if (pld->icnt[i]>0)
        registerNeighbor(*nblist, &last, &m, indx[0], indx[1]);
      for (j=0; j<pld->icnt[i]-2; j++) {
        registerNeighbor(*nblist, &last, &m, indx[0], indx[j+2]);
        registerNeighbor(*nblist, &last, &m, indx[j+1], indx[j+2]);
      }
      break;
    case limnPrimitiveQuads:
      for (j=0; j<pld->icnt[i]; j+=4) { /* go through all quads */
        registerNeighbor(*nblist, &last, &m, indx[j], indx[j+1]);
        registerNeighbor(*nblist, &last, &m, indx[j+1], indx[j+2]);
        registerNeighbor(*nblist, &last, &m, indx[j+2], indx[j+3]);
        registerNeighbor(*nblist, &last, &m, indx[j+3], indx[j]);
      }
      break;
    case limnPrimitiveLineStrip:
      for (j=0; j<pld->icnt[i]-1; j++) {
        registerNeighbor(*nblist, &last, &m, indx[j], indx[j+1]);
      }
      break;
    case limnPrimitiveLines:
      for (j=0; j<pld->icnt[i]; j+=2) {
        registerNeighbor(*nblist, &last, &m, indx[j], indx[j+1]);
      }
      break;
    default: /* should be a noop; silently ignore it */
      break;
    }
    indx+=pld->icnt[i];
  }
  if (len!=NULL) *len=last*sizeof(unsigned int);
  if (maxnb!=NULL) *maxnb=m;
  return 0;
}

/* Over the set of all n vertices in a given limnPolyData, finds the
 * maximum number m of neighbors. Sets *neighbors to a malloc'ed block
 * of (n*m) indices and lists the neighbors of vertex v at position
 * (v*m) of the list, padded with -1s.
 * *maxnb will be set to m (and is assumed to be non-NULL!)
 * Returns -1 and adds a biff error if there was a problem allocating memory.
 */
int
limnPolyDataNeighborArray(int **neighbors, unsigned int *maxnb,
                          const limnPolyData *pld) {
  static const char me[]="limnPolyDataNeighborArray";
  unsigned int i, *nblist, m;
  /* get the neighbors as a linked list */
  if (-1==limnPolyDataNeighborList(&nblist, NULL, &m, pld)) {
    return -1;
  }
  /* convert the result into an array */
  *neighbors = (int *) malloc(sizeof(int)*m*pld->xyzwNum);
  if (NULL==*neighbors) {
    biffAddf(LIMN, "%s: couldn't allocate neighbors buffer", me);
    free(nblist); return -1;
  }
  for (i=0; i<pld->xyzwNum; i++) {
    unsigned int aidx=0, lidx=nblist[i];
    while (lidx!=0) {
      (*neighbors)[m*i+aidx]=nblist[lidx];
      lidx=nblist[lidx+1];
      aidx++;
    }
    while (aidx<m) {
      (*neighbors)[m*i+aidx]=-1;
      aidx++;
    }
  }
  *maxnb=m;
  free(nblist);
  return 0;
}

/* Returns a compressed form of the above, rather than padding with -1s.
 * *neighbors is malloc'ed to an array that holds the indices of all neighbors
 * *idx is malloc'ed to an array of length pld->xyzwNum+1;
 * (*idx)[i] will give you the position in *neighbors at which the neighbors of
 * vertex i start
 * Returns -1 and adds a biff error if there was a problem allocating memory.
 */
int
limnPolyDataNeighborArrayComp(int **neighbors, int **idx,
                              const limnPolyData *pld) {
  static const char me[]="limnPolyDataNeighborArrayComp";
  unsigned int i, ct, *nblist;
  size_t len;
  airArray *mop;
  mop = airMopNew();
  /* get the neighbors as a linked list */
  if (-1==limnPolyDataNeighborList(&nblist, &len, NULL, pld)) {
    return -1;
  }
  airMopAdd(mop, nblist, airFree, airMopAlways);
  /* convert the result into compressed form */
  *neighbors = (int *) malloc(sizeof(int)*(len-pld->xyzwNum)/2);
  if (NULL==*neighbors) {
    biffAddf(LIMN, "%s: couldn't allocate neighbors buffer", me);
    airMopError(mop); return -1;
  }
  airMopAdd(mop, neighbors, airFree, airMopOnError);
  *idx = (int *) malloc(sizeof(int)*(pld->xyzwNum+1));
  if (NULL==*idx) {
    biffAddf(LIMN, "%s: couldn't allocate index buffer", me);
    airMopError(mop); return -1;
  }
  airMopAdd(mop, idx, airFree, airMopOnError);
  for (ct=i=0; i<pld->xyzwNum; i++) {
    unsigned int lidx=nblist[i];
    (*idx)[i]=ct;
    while (lidx!=0) {
      (*neighbors)[ct]=nblist[lidx];
      lidx=nblist[lidx+1];
      ct++;
    }
  }
  (*idx)[pld->xyzwNum]=ct;
  airMopOkay(mop);
  return 0;
}
