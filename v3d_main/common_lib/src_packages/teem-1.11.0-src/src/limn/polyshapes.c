/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
  Copyright (C) 2010, 2009, 2008  Thomas Schultz
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


#include "limn.h"

int
limnPolyDataCube(limnPolyData *pld,
                 unsigned int infoBitFlag,
                 int sharpEdge) {
  static const char me[]="limnPolyDataCube";
  unsigned int vertNum, vertIdx, primNum, indxNum, cnum, ci;

  vertNum = sharpEdge ? 6*4 : 8;
  primNum = 1;
  indxNum = 6*4;
  if (limnPolyDataAlloc(pld, infoBitFlag, vertNum, indxNum, primNum)) {
    biffAddf(LIMN, "%s: couldn't allocate output", me);
    return 1;
  }
  pld->type[0] = limnPrimitiveQuads;
  pld->icnt[0] = indxNum;

  vertIdx = 0;
  cnum = sharpEdge ? 3 : 1;
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,  -1,  -1,  -1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,  -1,   1,  -1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,   1,   1,  -1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,   1,  -1,  -1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,  -1,  -1,   1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,  -1,   1,   1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,   1,   1,   1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,   1,  -1,   1,  1); vertIdx++;
  }

  vertIdx = 0;
  if (sharpEdge) {
    ELL_4V_SET(pld->indx + vertIdx,  0,  3,  6,  9); vertIdx += 4;
    ELL_4V_SET(pld->indx + vertIdx,  2, 14, 16,  5); vertIdx += 4;
    ELL_4V_SET(pld->indx + vertIdx,  4, 17, 18,  8); vertIdx += 4;
    ELL_4V_SET(pld->indx + vertIdx,  7, 19, 21, 10); vertIdx += 4;
    ELL_4V_SET(pld->indx + vertIdx,  1, 11, 23, 13); vertIdx += 4;
    ELL_4V_SET(pld->indx + vertIdx, 12, 22, 20, 15); vertIdx += 4;
  } else {
    ELL_4V_SET(pld->indx + vertIdx,  0,  1,  2,  3); vertIdx += 4;
    ELL_4V_SET(pld->indx + vertIdx,  0,  4,  5,  1); vertIdx += 4;
    ELL_4V_SET(pld->indx + vertIdx,  1,  5,  6,  2); vertIdx += 4;
    ELL_4V_SET(pld->indx + vertIdx,  2,  6,  7,  3); vertIdx += 4;
    ELL_4V_SET(pld->indx + vertIdx,  0,  3,  7,  4); vertIdx += 4;
    ELL_4V_SET(pld->indx + vertIdx,  4,  7,  6,  5); vertIdx += 4;
  }

  if ((1 << limnPolyDataInfoNorm) & infoBitFlag) {
    if (sharpEdge) {
      ELL_3V_SET(pld->norm +  3*0,  0,  0, -1);
      ELL_3V_SET(pld->norm +  3*3,  0,  0, -1);
      ELL_3V_SET(pld->norm +  3*6,  0,  0, -1);
      ELL_3V_SET(pld->norm +  3*9,  0,  0, -1);
      ELL_3V_SET(pld->norm +  3*2, -1,  0,  0);
      ELL_3V_SET(pld->norm +  3*5, -1,  0,  0);
      ELL_3V_SET(pld->norm + 3*14, -1,  0,  0);
      ELL_3V_SET(pld->norm + 3*16, -1,  0,  0);
      ELL_3V_SET(pld->norm +  3*4,  0,  1,  0);
      ELL_3V_SET(pld->norm +  3*8,  0,  1,  0);
      ELL_3V_SET(pld->norm + 3*17,  0,  1,  0);
      ELL_3V_SET(pld->norm + 3*18,  0,  1,  0);
      ELL_3V_SET(pld->norm +  3*7,  1,  0,  0);
      ELL_3V_SET(pld->norm + 3*10,  1,  0,  0);
      ELL_3V_SET(pld->norm + 3*19,  1,  0,  0);
      ELL_3V_SET(pld->norm + 3*21,  1,  0,  0);
      ELL_3V_SET(pld->norm +  3*1,  0, -1,  0);
      ELL_3V_SET(pld->norm + 3*11,  0, -1,  0);
      ELL_3V_SET(pld->norm + 3*13,  0, -1,  0);
      ELL_3V_SET(pld->norm + 3*23,  0, -1,  0);
      ELL_3V_SET(pld->norm + 3*12,  0,  0,  1);
      ELL_3V_SET(pld->norm + 3*15,  0,  0,  1);
      ELL_3V_SET(pld->norm + 3*20,  0,  0,  1);
      ELL_3V_SET(pld->norm + 3*22,  0,  0,  1);
    } else {
      float cn;
      cn = AIR_CAST(float, sqrt(3.0));
      ELL_3V_SET(pld->norm + 3*0, -cn, -cn, -cn);
      ELL_3V_SET(pld->norm + 3*1, -cn,  cn, -cn);
      ELL_3V_SET(pld->norm + 3*2,  cn,  cn, -cn);
      ELL_3V_SET(pld->norm + 3*3,  cn, -cn, -cn);
      ELL_3V_SET(pld->norm + 3*4, -cn, -cn,  cn);
      ELL_3V_SET(pld->norm + 3*5, -cn,  cn,  cn);
      ELL_3V_SET(pld->norm + 3*6,  cn,  cn,  cn);
      ELL_3V_SET(pld->norm + 3*7,  cn, -cn,  cn);
    }
  }

  if ((1 << limnPolyDataInfoRGBA) & infoBitFlag) {
    for (vertIdx=0; vertIdx<pld->rgbaNum; vertIdx++) {
      ELL_4V_SET(pld->rgba + 4*vertIdx, 255, 255, 255, 255);
    }
  }

  return 0;
}

int
limnPolyDataCubeTriangles(limnPolyData *pld,
                          unsigned int infoBitFlag,
                          int sharpEdge) {
  static const char me[]="limnPolyDataCubeTriangles";
  unsigned int vertNum, vertIdx, primNum, indxNum, cnum, ci;

  vertNum = sharpEdge ? 6*4 : 8;
  primNum = 1;
  indxNum = 6*6;
  if (limnPolyDataAlloc(pld, infoBitFlag, vertNum, indxNum, primNum)) {
    biffAddf(LIMN, "%s: couldn't allocate output", me);
    return 1;
  }
  pld->type[0] = limnPrimitiveTriangles;
  pld->icnt[0] = indxNum;

  vertIdx = 0;
  cnum = sharpEdge ? 3 : 1;
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,   1,  -1,   1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,  -1,  -1,   1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,   1,   1,   1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,  -1,   1,   1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,   1,   1,  -1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,  -1,   1,  -1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,   1,  -1,  -1,  1); vertIdx++;
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,  -1,  -1,  -1,  1); vertIdx++;
  }

  vertIdx = 0;
  if (sharpEdge) {
    ELL_3V_SET(pld->indx + vertIdx,  0,  6,  3); vertIdx += 3;
    ELL_3V_SET(pld->indx + vertIdx,  3,  6,  9); vertIdx += 3;

    ELL_3V_SET(pld->indx + vertIdx,  7, 12, 10); vertIdx += 3;
    ELL_3V_SET(pld->indx + vertIdx, 10, 12, 16); vertIdx += 3;

    ELL_3V_SET(pld->indx + vertIdx, 11, 15,  4); vertIdx += 3;
    ELL_3V_SET(pld->indx + vertIdx,  4, 15, 21); vertIdx += 3;

    ELL_3V_SET(pld->indx + vertIdx, 17, 14, 22); vertIdx += 3;
    ELL_3V_SET(pld->indx + vertIdx, 22, 14, 20); vertIdx += 3;

    ELL_3V_SET(pld->indx + vertIdx,  2,  5, 19); vertIdx += 3;
    ELL_3V_SET(pld->indx + vertIdx, 19,  5, 23); vertIdx += 3;

    ELL_3V_SET(pld->indx + vertIdx, 13,  8, 18); vertIdx += 3;
    ELL_3V_SET(pld->indx + vertIdx, 18,  8,  1); vertIdx += 3;
  } else {
    ELL_3V_SET(pld->indx + vertIdx, 0, 2, 1); vertIdx += 3;
    ELL_3V_SET(pld->indx + vertIdx, 1, 2, 3); vertIdx += 3;

    ELL_3V_SET(pld->indx + vertIdx, 2, 4, 3); vertIdx += 3;
    ELL_3V_SET(pld->indx + vertIdx, 3, 4, 5); vertIdx += 3;

    ELL_3V_SET(pld->indx + vertIdx, 3, 5, 1); vertIdx += 3;
    ELL_3V_SET(pld->indx + vertIdx, 1, 5, 7); vertIdx += 3;

    ELL_3V_SET(pld->indx + vertIdx, 5, 4, 7); vertIdx += 3;
    ELL_3V_SET(pld->indx + vertIdx, 7, 4, 6); vertIdx += 3;

    ELL_3V_SET(pld->indx + vertIdx, 7, 6, 1); vertIdx += 3;
    ELL_3V_SET(pld->indx + vertIdx, 1, 6, 0); vertIdx += 3;

    ELL_3V_SET(pld->indx + vertIdx, 4, 2, 6); vertIdx += 3;
    ELL_3V_SET(pld->indx + vertIdx, 2, 6, 0); vertIdx += 3;
  }

  if ((1 << limnPolyDataInfoNorm) & infoBitFlag) {
    if (sharpEdge) {
      ELL_3V_SET(pld->norm + 3*14,  0,  0, -1);
      ELL_3V_SET(pld->norm + 3*17,  0,  0, -1);
      ELL_3V_SET(pld->norm + 3*20,  0,  0, -1);
      ELL_3V_SET(pld->norm + 3*22,  0,  0, -1);
      ELL_3V_SET(pld->norm + 3* 4, -1,  0,  0);
      ELL_3V_SET(pld->norm + 3*11, -1,  0,  0);
      ELL_3V_SET(pld->norm + 3*15, -1,  0,  0);
      ELL_3V_SET(pld->norm + 3*21, -1,  0,  0);
      ELL_3V_SET(pld->norm + 3* 7,  0,  1,  0);
      ELL_3V_SET(pld->norm + 3*10,  0,  1,  0);
      ELL_3V_SET(pld->norm + 3*12,  0,  1,  0);
      ELL_3V_SET(pld->norm + 3*16,  0,  1,  0);
      ELL_3V_SET(pld->norm + 3* 1,  1,  0,  0);
      ELL_3V_SET(pld->norm + 3* 8,  1,  0,  0);
      ELL_3V_SET(pld->norm + 3*13,  1,  0,  0);
      ELL_3V_SET(pld->norm + 3*18,  1,  0,  0);
      ELL_3V_SET(pld->norm + 3* 2,  0, -1,  0);
      ELL_3V_SET(pld->norm + 3* 5,  0, -1,  0);
      ELL_3V_SET(pld->norm + 3*19,  0, -1,  0);
      ELL_3V_SET(pld->norm + 3*23,  0, -1,  0);
      ELL_3V_SET(pld->norm + 3* 0,  0,  0,  1);
      ELL_3V_SET(pld->norm + 3* 3,  0,  0,  1);
      ELL_3V_SET(pld->norm + 3* 6,  0,  0,  1);
      ELL_3V_SET(pld->norm + 3* 9,  0,  0,  1);
    } else {
      float cn;
      cn = AIR_CAST(float, 1.0/sqrt(3.0));
      ELL_3V_SET(pld->norm + 3*0,  cn, -cn,  cn);
      ELL_3V_SET(pld->norm + 3*1, -cn, -cn,  cn);
      ELL_3V_SET(pld->norm + 3*2,  cn,  cn,  cn);
      ELL_3V_SET(pld->norm + 3*3, -cn,  cn,  cn);
      ELL_3V_SET(pld->norm + 3*4,  cn,  cn, -cn);
      ELL_3V_SET(pld->norm + 3*5, -cn,  cn, -cn);
      ELL_3V_SET(pld->norm + 3*6,  cn, -cn, -cn);
      ELL_3V_SET(pld->norm + 3*7, -cn, -cn, -cn);
    }
  }

  if ((1 << limnPolyDataInfoTex2) & infoBitFlag) {
    if (sharpEdge) {
      ELL_2V_SET(pld->tex2 + 2*14,  1,  1);
      ELL_2V_SET(pld->tex2 + 2*17,  0,  1);
      ELL_2V_SET(pld->tex2 + 2*20,  1,  0);
      ELL_2V_SET(pld->tex2 + 2*22,  0,  0);
      ELL_2V_SET(pld->tex2 + 2* 4,  1,  0);
      ELL_2V_SET(pld->tex2 + 2*11,  0,  0);
      ELL_2V_SET(pld->tex2 + 2*15,  0,  1);
      ELL_2V_SET(pld->tex2 + 2*21,  1,  1);
      ELL_2V_SET(pld->tex2 + 2* 7,  0,  0);
      ELL_2V_SET(pld->tex2 + 2*10,  1,  0);
      ELL_2V_SET(pld->tex2 + 2*12,  0,  1);
      ELL_2V_SET(pld->tex2 + 2*16,  1,  1);
      ELL_2V_SET(pld->tex2 + 2* 1,  0,  0);
      ELL_2V_SET(pld->tex2 + 2* 8,  1,  0);
      ELL_2V_SET(pld->tex2 + 2*13,  1,  1);
      ELL_2V_SET(pld->tex2 + 2*18,  0,  1);
      ELL_2V_SET(pld->tex2 + 2* 2,  1,  0);
      ELL_2V_SET(pld->tex2 + 2* 5,  0,  0);
      ELL_2V_SET(pld->tex2 + 2*19,  1,  1);
      ELL_2V_SET(pld->tex2 + 2*23,  0,  1);
      ELL_2V_SET(pld->tex2 + 2* 0,  1,  1);
      ELL_2V_SET(pld->tex2 + 2* 3,  0,  1);
      ELL_2V_SET(pld->tex2 + 2* 6,  1,  0);
      ELL_2V_SET(pld->tex2 + 2* 9,  0,  0);
    } else {
      ELL_2V_SET(pld->tex2 + 2*0, 1, 1);
      ELL_2V_SET(pld->tex2 + 2*1, 0, 1);
      ELL_2V_SET(pld->tex2 + 2*2, 1, 0);
      ELL_2V_SET(pld->tex2 + 2*3, 0, 0);
      ELL_2V_SET(pld->tex2 + 2*4, 1, 0);
      ELL_2V_SET(pld->tex2 + 2*5, 0, 0);
      ELL_2V_SET(pld->tex2 + 2*6, 1, 1);
      ELL_2V_SET(pld->tex2 + 2*7, 0, 1);
    }
  }

  if ((1 << limnPolyDataInfoTang) & infoBitFlag) {
    if (sharpEdge) {
      ELL_3V_SET(pld->tang + 3*14,  1,  0,  0);
      ELL_3V_SET(pld->tang + 3*17,  1,  0,  0);
      ELL_3V_SET(pld->tang + 3*20,  1,  0,  0);
      ELL_3V_SET(pld->tang + 3*22,  1,  0,  0);
      ELL_3V_SET(pld->tang + 3* 4,  0, -1,  0);
      ELL_3V_SET(pld->tang + 3*11,  0, -1,  0);
      ELL_3V_SET(pld->tang + 3*15,  0, -1,  0);
      ELL_3V_SET(pld->tang + 3*21,  0, -1,  0);
      ELL_3V_SET(pld->tang + 3* 7, -1,  0,  0);
      ELL_3V_SET(pld->tang + 3*10, -1,  0,  0);
      ELL_3V_SET(pld->tang + 3*12, -1,  0,  0);
      ELL_3V_SET(pld->tang + 3*16, -1,  0,  0);
      ELL_3V_SET(pld->tang + 3* 1,  0,  1,  0);
      ELL_3V_SET(pld->tang + 3* 8,  0,  1,  0);
      ELL_3V_SET(pld->tang + 3*13,  0,  1,  0);
      ELL_3V_SET(pld->tang + 3*18,  0,  1,  0);
      ELL_3V_SET(pld->tang + 3* 2,  1,  0,  0);
      ELL_3V_SET(pld->tang + 3* 5,  1,  0,  0);
      ELL_3V_SET(pld->tang + 3*19,  1,  0,  0);
      ELL_3V_SET(pld->tang + 3*23,  1,  0,  0);
      ELL_3V_SET(pld->tang + 3* 0,  1,  0,  0);
      ELL_3V_SET(pld->tang + 3* 3,  1,  0,  0);
      ELL_3V_SET(pld->tang + 3* 6,  1,  0,  0);
      ELL_3V_SET(pld->tang + 3* 9,  1,  0,  0);
    } else {
      float sn;
      sn = AIR_CAST(float, 1.0/sqrt(2.0));
      ELL_3V_SET(pld->tang + 3*0, -sn,  sn,  0);
      ELL_3V_SET(pld->tang + 3*1,  sn,  sn,  0);
      ELL_3V_SET(pld->tang + 3*2, -sn, -sn,  0);
      ELL_3V_SET(pld->tang + 3*3,  sn, -sn,  0);
      ELL_3V_SET(pld->tang + 3*4, -sn, -sn,  0);
      ELL_3V_SET(pld->tang + 3*5,  sn, -sn,  0);
      ELL_3V_SET(pld->tang + 3*6, -sn,  sn,  0);
      ELL_3V_SET(pld->tang + 3*7,  sn,  sn,  0);
    }
  }

  if ((1 << limnPolyDataInfoRGBA) & infoBitFlag) {
    for (vertIdx=0; vertIdx<pld->rgbaNum; vertIdx++) {
      ELL_4V_SET(pld->rgba + 4*vertIdx, 255, 255, 255, 255);
    }
  }

  return 0;
}

int
limnPolyDataOctahedron(limnPolyData *pld,
                       unsigned int infoBitFlag,
                       int sharpEdge) {
  static const char me[]="limnPolyDataOctahedron";
  unsigned int vertNum, vertIdx, primNum, indxNum, cnum, ci;

  vertNum = sharpEdge ? 4*6 : 6;
  primNum = 1;
  indxNum = 8*3;
  if (limnPolyDataAlloc(pld, infoBitFlag, vertNum, indxNum, primNum)) {
    biffAddf(LIMN, "%s: couldn't allocate output", me);
    return 1;
  }
  pld->type[0] = limnPrimitiveTriangles;
  pld->icnt[0] = indxNum;

  vertIdx = 0;
  cnum = sharpEdge ? 4 : 1;
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,  0,  0,  1,  1); vertIdx++; /* 0 */
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,  0,  1,  0,  1); vertIdx++; /* 1 */
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,  1,  0,  0,  1); vertIdx++; /* 2 */
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,  0, -1,  0,  1); vertIdx++; /* 3 */
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx, -1,  0,  0,  1); vertIdx++; /* 4 */
  }
  for (ci=0; ci<cnum; ci++) {
    ELL_4V_SET(pld->xyzw + 4*vertIdx,  0,  0, -1,  1); vertIdx++; /* 5 */
  }

  vertIdx = 0;
  if (sharpEdge) {
    ELL_3V_SET(pld->indx + vertIdx,  0,  8,  4); vertIdx += 3; /* 0 */
    ELL_3V_SET(pld->indx + vertIdx,  1, 15,  9); vertIdx += 3; /* 1 */
    ELL_3V_SET(pld->indx + vertIdx,  2, 16, 12); vertIdx += 3; /* 2 */
    ELL_3V_SET(pld->indx + vertIdx,  3,  6, 19); vertIdx += 3; /* 3 */
    ELL_3V_SET(pld->indx + vertIdx,  5, 11, 23); vertIdx += 3; /* 4 */
    ELL_3V_SET(pld->indx + vertIdx, 10, 14, 20); vertIdx += 3; /* 5 */
    ELL_3V_SET(pld->indx + vertIdx, 17, 21, 13); vertIdx += 3; /* 6 */
    ELL_3V_SET(pld->indx + vertIdx,  7, 22, 18); vertIdx += 3; /* 7 */
  } else {
    ELL_3V_SET(pld->indx + vertIdx,  0,  2,  1); vertIdx += 3; /* 0 */
    ELL_3V_SET(pld->indx + vertIdx,  0,  3,  2); vertIdx += 3; /* 1 */
    ELL_3V_SET(pld->indx + vertIdx,  0,  4,  3); vertIdx += 3; /* 2 */
    ELL_3V_SET(pld->indx + vertIdx,  0,  1,  4); vertIdx += 3; /* 3 */
    ELL_3V_SET(pld->indx + vertIdx,  1,  2,  5); vertIdx += 3; /* 4 */
    ELL_3V_SET(pld->indx + vertIdx,  2,  3,  5); vertIdx += 3; /* 5 */
    ELL_3V_SET(pld->indx + vertIdx,  4,  5,  3); vertIdx += 3; /* 6 */
    ELL_3V_SET(pld->indx + vertIdx,  1,  5,  4); vertIdx += 3; /* 7 */
  }

  if ((1 << limnPolyDataInfoNorm) & infoBitFlag) {
    if (sharpEdge) {
      float cn;
      cn = AIR_CAST(float, 1.0/sqrt(3));
      /* 0 */
      ELL_3V_SET(pld->norm +  3*0,  cn,  cn,  cn);
      ELL_3V_SET(pld->norm +  3*8,  cn,  cn,  cn);
      ELL_3V_SET(pld->norm +  3*4,  cn,  cn,  cn);
      /* 1 */
      ELL_3V_SET(pld->norm +  3*1,  cn, -cn,  cn);
      ELL_3V_SET(pld->norm + 3*15,  cn, -cn,  cn);
      ELL_3V_SET(pld->norm +  3*9,  cn, -cn,  cn);
      /* 2 */
      ELL_3V_SET(pld->norm +  3*2, -cn, -cn,  cn);
      ELL_3V_SET(pld->norm + 3*16, -cn, -cn,  cn);
      ELL_3V_SET(pld->norm + 3*12, -cn, -cn,  cn);
      /* 3 */
      ELL_3V_SET(pld->norm +  3*3, -cn,  cn,  cn);
      ELL_3V_SET(pld->norm +  3*6, -cn,  cn,  cn);
      ELL_3V_SET(pld->norm + 3*19, -cn,  cn,  cn);
      /* 4 */
      ELL_3V_SET(pld->norm +  3*5,  cn,  cn, -cn);
      ELL_3V_SET(pld->norm + 3*11,  cn,  cn, -cn);
      ELL_3V_SET(pld->norm + 3*23,  cn,  cn, -cn);
      /* 5 */
      ELL_3V_SET(pld->norm + 3*10,  cn, -cn, -cn);
      ELL_3V_SET(pld->norm + 3*14,  cn, -cn, -cn);
      ELL_3V_SET(pld->norm + 3*20,  cn, -cn, -cn);
      /* 6 */
      ELL_3V_SET(pld->norm + 3*17, -cn, -cn, -cn);
      ELL_3V_SET(pld->norm + 3*21, -cn, -cn, -cn);
      ELL_3V_SET(pld->norm + 3*13, -cn, -cn, -cn);
      /* 7 */
      ELL_3V_SET(pld->norm +  3*7, -cn,  cn, -cn);
      ELL_3V_SET(pld->norm + 3*22, -cn,  cn, -cn);
      ELL_3V_SET(pld->norm + 3*18, -cn,  cn, -cn);
    } else {
      ELL_3V_SET(pld->norm + 3*0,  0,  0,  1); /* 0 */
      ELL_3V_SET(pld->norm + 3*1,  0,  1,  0); /* 1 */
      ELL_3V_SET(pld->norm + 3*2,  1,  0,  0); /* 2 */
      ELL_3V_SET(pld->norm + 3*3,  0, -1,  0); /* 3 */
      ELL_3V_SET(pld->norm + 3*4, -1,  0,  0); /* 4 */
      ELL_3V_SET(pld->norm + 3*5,  0,  0, -1); /* 5 */
    }
  }

  if ((1 << limnPolyDataInfoRGBA) & infoBitFlag) {
    for (vertIdx=0; vertIdx<pld->rgbaNum; vertIdx++) {
      ELL_4V_SET(pld->rgba + 4*vertIdx, 255, 255, 255, 255);
    }
  }
  return 0;
}

int
limnPolyDataCylinder(limnPolyData *pld,
                     unsigned int infoBitFlag,
                     unsigned int thetaRes, int sharpEdge) {
  static const char me[]="limnPolyDataCylinder";
  unsigned int vertNum, primNum, primIdx, indxNum, thetaIdx, vertIdx, blah;
  double theta, cth, sth, sq2;

  /* sanity bounds */
  thetaRes = AIR_MAX(3, thetaRes);

  vertNum = sharpEdge ? 4*thetaRes : 2*thetaRes;
  primNum = 3;
  indxNum = 2*thetaRes + 2*(thetaRes+1);  /* 2 fans + 1 strip */
  if (limnPolyDataAlloc(pld, infoBitFlag, vertNum, indxNum, primNum)) {
    biffAddf(LIMN, "%s: couldn't allocate output", me);
    return 1;
  }

  vertIdx = 0;
  for (blah=0; blah < (sharpEdge ? 2u : 1u); blah++) {
    for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
      theta = AIR_AFFINE(0, thetaIdx, thetaRes, 0, 2*AIR_PI);
      ELL_4V_SET_TT(pld->xyzw + 4*vertIdx, float,
                    cos(theta), sin(theta), 1, 1);
      /*
      fprintf(stderr, "!%s: vert[%u] = %g %g %g\n", me, vertIdx,
              (pld->xyzw + 4*vertIdx)[0],
              (pld->xyzw + 4*vertIdx)[1],
              (pld->xyzw + 4*vertIdx)[2]);
      */
      ++vertIdx;
    }
  }
  for (blah=0; blah < (sharpEdge ? 2u : 1u); blah++) {
    for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
      theta = AIR_AFFINE(0, thetaIdx, thetaRes, 0, 2*AIR_PI);
      ELL_4V_SET_TT(pld->xyzw + 4*vertIdx, float,
                    cos(theta), sin(theta), -1, 1);
      /*
      fprintf(stderr, "!%s: vert[%u] = %g %g %g\n", me, vertIdx,
              (pld->xyzw + 4*vertIdx)[0],
              (pld->xyzw + 4*vertIdx)[1],
              (pld->xyzw + 4*vertIdx)[2]);
      */
      ++vertIdx;
    }
  }

  primIdx = 0;
  vertIdx = 0;
  /* fan on top */
  /* fprintf(stderr, "!%s: fan on top:\n", me); */
  for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
    /* fprintf(stderr, "!%s:   indx[%u] = %u\n", me, vertIdx, thetaIdx); */
    pld->indx[vertIdx++] = thetaIdx;
  }
  pld->type[primIdx] = limnPrimitiveTriangleFan;
  pld->icnt[primIdx] = thetaRes;
  primIdx++;

  /* single strip around */
  /* fprintf(stderr, "!%s: strip around:\n", me); */
  for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
    /*
    fprintf(stderr, "!%s:   indx[%u] = %u\n", me, vertIdx,
            (sharpEdge ? 1 : 0)*thetaRes + thetaIdx);
    */
    pld->indx[vertIdx++] = (sharpEdge ? 1 : 0)*thetaRes + thetaIdx;
    /*
    fprintf(stderr, "!%s:   indx[%u] = %u\n", me, vertIdx,
            (sharpEdge ? 2 : 1)*thetaRes + thetaIdx);
    */
    pld->indx[vertIdx++] = (sharpEdge ? 2 : 1)*thetaRes + thetaIdx;
  }
  /*
  fprintf(stderr, "!%s:   indx[%u] = %u\n", me, vertIdx,
          (sharpEdge ? 1 : 0)*thetaRes);
  */
  pld->indx[vertIdx++] = (sharpEdge ? 1 : 0)*thetaRes;
  /*
  fprintf(stderr, "!%s:   indx[%u] = %u\n", me, vertIdx,
          (sharpEdge ? 2 : 1)*thetaRes);
  */
  pld->indx[vertIdx++] = (sharpEdge ? 2 : 1)*thetaRes;
  pld->type[primIdx] = limnPrimitiveTriangleStrip;
  pld->icnt[primIdx] = 2*(thetaRes+1);
  primIdx++;

  /* fan on bottom */
  /* fprintf(stderr, "!%s: fan on bottom:\n", me); */
  for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
    /*
    fprintf(stderr, "!%s:   indx[%u] = %u\n", me, vertIdx,
            (sharpEdge ? 3 : 1)*thetaRes + thetaIdx);
    */
    pld->indx[vertIdx++] = (sharpEdge ? 3 : 1)*thetaRes + thetaIdx;
  }
  pld->type[primIdx] = limnPrimitiveTriangleFan;
  pld->icnt[primIdx] = thetaRes;
  primIdx++;

  if ((1 << limnPolyDataInfoNorm) & infoBitFlag) {
    sq2 = sqrt(2.0);
    if (sharpEdge) {
      for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
        theta = AIR_AFFINE(0, thetaIdx, thetaRes, 0, 2*AIR_PI);
        cth = cos(theta);
        sth = sin(theta);
        ELL_3V_SET_TT(pld->norm + 3*(thetaIdx + 0*thetaRes),
                      float, 0, 0, 1);
        ELL_3V_SET_TT(pld->norm + 3*(thetaIdx + 1*thetaRes),
                      float, cth, sth, 0);
        ELL_3V_SET_TT(pld->norm + 3*(thetaIdx + 2*thetaRes),
                      float, cth, sth, 0);
        ELL_3V_SET_TT(pld->norm + 3*(thetaIdx + 3*thetaRes),
                      float, 0, 0, -1);
      }
    } else {
      for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
        theta = AIR_AFFINE(0, thetaIdx, thetaRes, 0, 2*AIR_PI);
        cth = sq2*cos(theta);
        sth = sq2*sin(theta);
        ELL_3V_SET_TT(pld->norm + 3*(thetaIdx + 0*thetaRes), float,
                      cth, sth, sq2);
        ELL_3V_SET_TT(pld->norm + 3*(thetaIdx + 1*thetaRes), float,
                      cth, sth, -sq2);
      }
    }
  }

  if ((1 << limnPolyDataInfoTex2) & infoBitFlag) {
    if (sharpEdge) {
      for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
        theta = AIR_AFFINE(0, thetaIdx, thetaRes, 0, 1);
        ELL_2V_SET_TT(pld->tex2 + 2*(thetaIdx + 0*thetaRes), float, theta, 0);
        ELL_2V_SET_TT(pld->tex2 + 2*(thetaIdx + 1*thetaRes), float, theta, 0);
        ELL_2V_SET_TT(pld->tex2 + 2*(thetaIdx + 2*thetaRes), float, theta, 1);
        ELL_2V_SET_TT(pld->tex2 + 2*(thetaIdx + 3*thetaRes), float, theta, 1);
      }
    } else {
      for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
        theta = AIR_AFFINE(0, thetaIdx, thetaRes, 0, 1);
        ELL_2V_SET_TT(pld->tex2 + 2*(thetaIdx + 0*thetaRes), float, theta, 0);
        ELL_2V_SET_TT(pld->tex2 + 2*(thetaIdx + 1*thetaRes), float, theta, 1);
      }
    }
  }

  if ((1 << limnPolyDataInfoTang) & infoBitFlag) {
    float xx, yy, tang[3], tlen;
    if (sharpEdge) {
      for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
        xx = (pld->xyzw + 4*(thetaIdx + 0*thetaRes))[0];
        yy = (pld->xyzw + 4*(thetaIdx + 0*thetaRes))[1];
        ELL_3V_SET(tang, -yy, xx, 0.0);
        ELL_3V_NORM_TT(tang, float, tang, tlen);
        ELL_3V_COPY(pld->tang + 3*(thetaIdx + 0*thetaRes), tang);
        ELL_3V_COPY(pld->tang + 3*(thetaIdx + 1*thetaRes), tang);
        ELL_3V_COPY(pld->tang + 3*(thetaIdx + 2*thetaRes), tang);
        ELL_3V_COPY(pld->tang + 3*(thetaIdx + 3*thetaRes), tang);
      }
    } else {
      for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
        xx = (pld->xyzw + 4*(thetaIdx + 0*thetaRes))[0];
        yy = (pld->xyzw + 4*(thetaIdx + 0*thetaRes))[1];
        ELL_3V_SET(tang, -yy, xx, 0.0);
        ELL_3V_NORM_TT(tang, float, tang, tlen);
        ELL_3V_COPY(pld->tang + 3*(thetaIdx + 0*thetaRes), tang);
        ELL_3V_COPY(pld->tang + 3*(thetaIdx + 1*thetaRes), tang);
      }
    }
  }

  if ((1 << limnPolyDataInfoRGBA) & infoBitFlag) {
    for (vertIdx=0; vertIdx<pld->rgbaNum; vertIdx++) {
      ELL_4V_SET(pld->rgba + 4*vertIdx, 255, 255, 255, 255);
    }
  }

  return 0;
}

int
limnPolyDataCone(limnPolyData *pld,
                 unsigned int infoBitFlag,
                 unsigned int thetaRes, int sharpEdge) {
  static const char me[]="limnPolyDataCone";
  unsigned int vertNum, primNum, primIdx, indxNum, thetaIdx, vertIdx, blah;
  double theta, cth, sth;

  /* sanity bounds */
  thetaRes = AIR_MAX(3, thetaRes);

  vertNum = sharpEdge ? 3*thetaRes : 1 + thetaRes;
  primNum = 2;
  indxNum = thetaRes + 2*(thetaRes+1);  /* 1 fans + 1 strip */
  if (limnPolyDataAlloc(pld, infoBitFlag, vertNum, indxNum, primNum)) {
    biffAddf(LIMN, "%s: couldn't allocate output", me);
    return 1;
  }

  /* top point(s) */
  vertIdx = 0;
  if (sharpEdge) {
    for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
      ELL_4V_SET_TT(pld->xyzw + 4*vertIdx, float,
                    0, 0, 1, 1);
      ++vertIdx;
    }
  } else {
    ELL_4V_SET_TT(pld->xyzw + 4*vertIdx, float,
                  0, 0, 1, 1);
    ++vertIdx;
  }
  /* bottom edge */
  for (blah=0; blah < (sharpEdge ? 2u : 1u); blah++) {
    for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
      theta = AIR_AFFINE(0, thetaIdx, thetaRes, 0, 2*AIR_PI);
      ELL_4V_SET_TT(pld->xyzw + 4*vertIdx, float,
                    cos(theta), sin(theta), -1, 1);
      ++vertIdx;
    }
  }

  primIdx = 0;
  vertIdx = 0;
  /* single strip around */
  for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
    pld->indx[vertIdx++] = sharpEdge ? thetaIdx : 0;
    pld->indx[vertIdx++] = (sharpEdge ? thetaRes : 1) + thetaIdx;
  }
  pld->indx[vertIdx++] = 0;
  pld->indx[vertIdx++] = sharpEdge ? thetaRes : 1;
  pld->type[primIdx] = limnPrimitiveTriangleStrip;
  pld->icnt[primIdx] = 2*(thetaRes+1);
  primIdx++;

  /* fan on bottom */
  for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
    pld->indx[vertIdx++] = (sharpEdge ? 2*thetaRes : 1) + thetaIdx;
  }
  pld->type[primIdx] = limnPrimitiveTriangleFan;
  pld->icnt[primIdx] = thetaRes;
  primIdx++;

  if ((1 << limnPolyDataInfoNorm) & infoBitFlag) {
    double isq3;
    isq3 = 1/sqrt(3.0);
    if (sharpEdge) {
      for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
        theta = AIR_AFFINE(0, thetaIdx, thetaRes, 0, 2*AIR_PI);
        cth = cos(theta);
        sth = sin(theta);
        ELL_3V_SET_TT(pld->norm + 3*(thetaIdx + 0*thetaRes),
                      float, cth*isq3, sth*isq3, isq3);
        ELL_3V_SET_TT(pld->norm + 3*(thetaIdx + 1*thetaRes),
                      float, cth*isq3, sth*isq3, isq3);
        ELL_3V_SET_TT(pld->norm + 3*(thetaIdx + 2*thetaRes),
                      float, 0, 0, -1);
      }
    } else {
      ELL_3V_SET_TT(pld->norm + 3*(0), float,
                    0, 0, 1);
      for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
        theta = AIR_AFFINE(0, thetaIdx, thetaRes, 0, 2*AIR_PI);
        cth = cos(theta);
        sth = sin(theta);
        ELL_3V_SET_TT(pld->norm + 3*(thetaIdx + 1),
                      float, cth*isq3, sth*isq3, -isq3); /* close enough */
      }
    }
  }

  if ((1 << limnPolyDataInfoRGBA) & infoBitFlag) {
    for (vertIdx=0; vertIdx<pld->rgbaNum; vertIdx++) {
      ELL_4V_SET(pld->rgba + 4*vertIdx, 255, 255, 255, 255);
    }
  }

  return 0;
}

/*
******** limnPolyDataSuperquadric
**
** makes a superquadric parameterized around the Z axis
*/
int
limnPolyDataSuperquadric(limnPolyData *pld,
                         unsigned int infoBitFlag,
                         float alpha, float beta,
                         unsigned int thetaRes, unsigned int phiRes) {
  static const char me[]="limnPolyDataSuperquadric";
  unsigned int vertIdx, vertNum, fanNum, stripNum, primNum, indxNum,
    thetaIdx, phiIdx, primIdx;
  double theta, phi;

  /* sanity bounds */
  thetaRes = AIR_MAX(3u, thetaRes);
  phiRes = AIR_MAX(2u, phiRes);
  alpha = AIR_MAX(0.00001f, alpha);
  beta = AIR_MAX(0.00001f, beta);

  vertNum = 2 + thetaRes*(phiRes-1);
  fanNum = 2;
  stripNum = phiRes-2;
  primNum = fanNum + stripNum;
  indxNum = (thetaRes+2)*fanNum + 2*(thetaRes+1)*stripNum;
  if (limnPolyDataAlloc(pld, infoBitFlag, vertNum, indxNum, primNum)) {
    biffAddf(LIMN, "%s: couldn't allocate output", me);
    return 1;
  }

  vertIdx = 0;
  ELL_4V_SET(pld->xyzw + 4*vertIdx, 0, 0, 1, 1);
  if ((1 << limnPolyDataInfoNorm) & infoBitFlag) {
    ELL_3V_SET(pld->norm + 3*vertIdx, 0, 0, 1);
  }
  ++vertIdx;
  for (phiIdx=1; phiIdx<phiRes; phiIdx++) {
    double cost, sint, cosp, sinp;
    phi = AIR_AFFINE(0, phiIdx, phiRes, 0, AIR_PI);
    cosp = cos(phi);
    sinp = sin(phi);
    for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
      theta = AIR_AFFINE(0, thetaIdx, thetaRes, 0, 2*AIR_PI);
      cost = cos(theta);
      sint = sin(theta);
      ELL_4V_SET_TT(pld->xyzw + 4*vertIdx, float,
                    airSgnPow(cost,alpha) * airSgnPow(sinp,beta),
                    airSgnPow(sint,alpha) * airSgnPow(sinp,beta),
                    airSgnPow(cosp,beta),
                    1.0);
      if ((1 << limnPolyDataInfoNorm) & infoBitFlag) {
        if (1 == alpha && 1 == beta) {
          ELL_3V_COPY(pld->norm + 3*vertIdx, pld->xyzw + 4*vertIdx);
        } else {
          ELL_3V_SET_TT(pld->norm + 3*vertIdx, float,
                        2*airSgnPow(cost,2-alpha)*airSgnPow(sinp,2-beta)/beta,
                        2*airSgnPow(sint,2-alpha)*airSgnPow(sinp,2-beta)/beta,
                        2*airSgnPow(cosp,2-beta)/beta);
        }
      }
      ++vertIdx;
    }
  }
  ELL_4V_SET(pld->xyzw + 4*vertIdx, 0, 0, -1, 1);
  if ((1 << limnPolyDataInfoNorm) & infoBitFlag) {
    ELL_3V_SET(pld->norm + 3*vertIdx, 0, 0, -1);
  }
  ++vertIdx;

  /* triangle fan at top */
  vertIdx = 0;
  primIdx = 0;
  pld->indx[vertIdx++] = 0;
  for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
    pld->indx[vertIdx++] = thetaIdx + 1;
  }
  pld->indx[vertIdx++] = 1;
  pld->type[primIdx] = limnPrimitiveTriangleFan;
  pld->icnt[primIdx++] = thetaRes + 2;

  /* tristrips around */
  for (phiIdx=1; phiIdx<phiRes-1; phiIdx++) {
    /*
    fprintf(stderr, "!%s: prim[%u] = vert[%u] =", me, primIdx, vertIdx);
    */
    for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
      /*
      fprintf(stderr, " [%u %u] %u %u",
              vertIdx, vertIdx + 1,
              (phiIdx-1)*thetaRes + thetaIdx + 1,
              phiIdx*thetaRes + thetaIdx + 1);
      */
      pld->indx[vertIdx++] = (phiIdx-1)*thetaRes + thetaIdx + 1;
      pld->indx[vertIdx++] = phiIdx*thetaRes + thetaIdx + 1;
    }
    /*
    fprintf(stderr, " [%u %u] %u %u (%u verts)\n",
            vertIdx, vertIdx + 1,
            (phiIdx-1)*thetaRes + 1,
            phiIdx*thetaRes + 1, 2*(thetaRes+1));
    */
    pld->indx[vertIdx++] = (phiIdx-1)*thetaRes + 1;
    pld->indx[vertIdx++] = phiIdx*thetaRes + 1;
    pld->type[primIdx] = limnPrimitiveTriangleStrip;
    pld->icnt[primIdx++] = 2*(thetaRes+1);
  }

  /* triangle fan at bottom */
  pld->indx[vertIdx++] = vertNum-1;
  for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
    pld->indx[vertIdx++] = thetaRes*(phiRes-2) + thetaRes - thetaIdx;
  }
  pld->indx[vertIdx++] = thetaRes*(phiRes-2) + thetaRes;
  pld->type[primIdx] = limnPrimitiveTriangleFan;
  pld->icnt[primIdx++] = thetaRes + 2;

  if ((1 << limnPolyDataInfoRGBA) & infoBitFlag) {
    for (vertIdx=0; vertIdx<pld->rgbaNum; vertIdx++) {
      ELL_4V_SET(pld->rgba + 4*vertIdx, 255, 255, 255, 255);
    }
  }

  return 0;
}

/*
******** limnPolyDataSpiralBetterquadric
**
** puts a "betterquadric" into a single spiral triangle strip
*/
int
limnPolyDataSpiralBetterquadric(limnPolyData *pld,
                                unsigned int infoBitFlag,
                                float alpha, float beta, float cee,
                                float minRad,
                                unsigned int thetaRes, unsigned int phiRes) {
  static const char me[]="limnPolyDataSpiralBetterquadric";
  unsigned int vertIdx, vertNum, indxNum, thetaIdx, phiIdx;

  /* sanity bounds */
  thetaRes = AIR_MAX(3u, thetaRes);
  phiRes = AIR_MAX(2u, phiRes);
  alpha = AIR_MAX(0.00001f, alpha);
  beta = AIR_MAX(0.00001f, beta);

  vertNum = thetaRes*phiRes + 1;
  indxNum = 2*thetaRes*(phiRes+1) - 2;
  if (limnPolyDataAlloc(pld, infoBitFlag, vertNum, indxNum, 1)) {
    biffAddf(LIMN, "%s: couldn't allocate output", me);
    return 1;
  }

  vertIdx = 0;
  for (phiIdx=0; phiIdx<phiRes; phiIdx++) {
    for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
      double cost, sint, cosp, sinp, xx, yy, zz;
      double phi = (AIR_AFFINE(0, phiIdx, phiRes, 0, AIR_PI)
                    + AIR_AFFINE(0, thetaIdx, thetaRes, 0, AIR_PI)/phiRes);
      double theta = AIR_AFFINE(0, thetaIdx, thetaRes, 0.0, 2*AIR_PI);
      cosp = cos(phi);
      sinp = sin(phi);
      cost = cos(theta);
      sint = sin(theta);
      xx = airSgnPow(cost,alpha) * airSgnPow(sinp,beta);
      yy = airSgnPow(sint,alpha) * airSgnPow(sinp,beta);
      zz = airSgnPow(cosp,beta);
      if (cee != beta) {
        /* expand profile along y axis to match having beta=cee */
        double yp, ymax;
        yp = airSgnPow(sin(acos(airSgnPow(zz, 1/cee))), cee);
        ymax = airSgnPow(sinp, beta);
        if (ymax) {
          yy *= yp/ymax;
        }
      }
      ELL_4V_SET_TT(pld->xyzw + 4*vertIdx, float, xx, yy, zz, 1.0);
      if (minRad > 0.0) {
        /* add thickness to small radius */
        double rr;
        xx = (pld->xyzw + 4*vertIdx)[0];
        yy = (pld->xyzw + 4*vertIdx)[1];
        rr = sqrt(xx*xx + yy*yy);
        if (rr) {
          (pld->xyzw + 4*vertIdx)[0] *= AIR_CAST(float, AIR_AFFINE(0, rr, 1, minRad/rr, 1/rr));
          (pld->xyzw + 4*vertIdx)[1] *= AIR_CAST(float, AIR_AFFINE(0, rr, 1, minRad/rr, 1/rr));
        }
      }
      if (((1 << limnPolyDataInfoNorm) & infoBitFlag)
          || ((1 << limnPolyDataInfoTang) & infoBitFlag)) {
        double norm[3], nlen;
        if (1 == alpha && 1 == beta) {
          ELL_3V_COPY(norm, pld->xyzw + 4*vertIdx);
        } else {
          if (!vertIdx) {
            ELL_3V_SET(norm, 0.0, 0.0, 1.0);
          } else {
            ELL_3V_SET(norm,
                       (2*airSgnPow(cost,2-alpha)*airSgnPow(sinp,2-beta)/beta),
                       (2*airSgnPow(sint,2-alpha)*airSgnPow(sinp,2-beta)/beta),
                       2*airSgnPow(cosp,2-beta)/beta);
          }
        }
        if ((nlen = ELL_3V_LEN(norm))) {
          ELL_3V_SCALE(norm, 1.0/nlen, norm);
        } else {
          ELL_3V_SET(norm, 0.0, 0.0, 1.0);
        }
        if ((1 << limnPolyDataInfoNorm) & infoBitFlag) {
          ELL_3V_COPY_TT(pld->norm + 3*vertIdx, float, norm);
        }
        if ((1 << limnPolyDataInfoTang) & infoBitFlag) {
          double tang[3], tlen;
          ELL_3V_SET(tang, -norm[1], norm[0], 0.0);
          if ((tlen = ELL_3V_LEN(tang))) {
            ELL_3V_SCALE(tang, 1.0/tlen, tang);
          } else {
            ELL_3V_SET(tang, 1.0, 0.0, 0.0);
          }
          ELL_3V_COPY_TT(pld->tang + 3*vertIdx, float, tang);
        }
      }
      if ((1 << limnPolyDataInfoTex2) & infoBitFlag) {
        ELL_2V_SET_TT(pld->tex2 + 2*vertIdx, float,
                      AIR_AFFINE(0.0, theta, 2*AIR_PI, 0.0, 1.0),
                      AIR_AFFINE(0.0, phi, AIR_PI, 0.0, 1.0));
      }
      ++vertIdx;
    }
  }
  ELL_4V_SET(pld->xyzw + 4*vertIdx, 0, 0, -1, 1);
  if ((1 << limnPolyDataInfoNorm) & infoBitFlag) {
    ELL_3V_SET(pld->norm + 3*vertIdx, 0.0, 0.0, -1.0);
  }
  if ((1 << limnPolyDataInfoTex2) & infoBitFlag) {
    ELL_2V_SET(pld->tex2 + 2*vertIdx, 0.5, 1.0);
  }
  if ((1 << limnPolyDataInfoTang) & infoBitFlag) {
    ELL_3V_SET(pld->tang + 3*vertIdx, 1.0, 0.0, 0.0);
  }
  ++vertIdx;

  /* single triangle strip */
  pld->type[0] = limnPrimitiveTriangleStrip;
  pld->icnt[0] = indxNum;
  vertIdx = 0;
  for (thetaIdx=1; thetaIdx<thetaRes; thetaIdx++) {
    pld->indx[vertIdx++] = 0;
    pld->indx[vertIdx++] = thetaIdx;
  }
  for (phiIdx=0; phiIdx<phiRes-1; phiIdx++) {
    for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
      pld->indx[vertIdx++] = ((phiIdx + 0) * thetaRes) + thetaIdx;
      pld->indx[vertIdx++] = ((phiIdx + 1) * thetaRes) + thetaIdx;
    }
  }
  for (thetaIdx=0; thetaIdx<thetaRes; thetaIdx++) {
    pld->indx[vertIdx++] = (phiRes - 1)*thetaRes + thetaIdx;
    pld->indx[vertIdx++] = (phiRes - 0)*thetaRes;
  }
#if 0
  if ( (cee != beta || minRad > 0.0)
       && ((1 << limnPolyDataInfoNorm) & infoBitFlag) ) {
    /* have deformed object in some way that confounds analytic normals */
    if (limnPolyDataVertexNormals(pld)) {
      biffAddf(LIMN, "%s: trouble getting normals", me); return 1;
    }
  }
#endif
  if ((1 << limnPolyDataInfoRGBA) & infoBitFlag) {
    for (vertIdx=0; vertIdx<pld->rgbaNum; vertIdx++) {
      ELL_4V_SET(pld->rgba + 4*vertIdx, 255, 255, 255, 255);
    }
  }

  return 0;
}

/*
******** limnPolyDataSpiralSuperquadric
**
** puts a superquadric into a single spiral triangle strip
*/
int
limnPolyDataSpiralSuperquadric(limnPolyData *pld,
                               unsigned int infoBitFlag,
                               float alpha, float beta,
                               unsigned int thetaRes, unsigned int phiRes) {
  static const char me[]="limnPolyDataSpiralSuperquadric";

  if (limnPolyDataSpiralBetterquadric(pld, infoBitFlag,
                                      alpha, beta, beta, 0.0,
                                      thetaRes, phiRes)) {
    biffAddf(LIMN, "%s: trouble", me); return 1;
  }
  return 0;
}

/*
******** limnPolyDataPolarSphere
**
** makes a unit sphere, centered at the origin, parameterized around Z axis
*/
int
limnPolyDataPolarSphere(limnPolyData *pld,
                        unsigned int infoBitFlag,
                        unsigned int thetaRes, unsigned int phiRes) {
  static const char me[]="limnPolyDataPolarSphere";

  if (limnPolyDataSuperquadric(pld, infoBitFlag,
                               1.0, 1.0, thetaRes, phiRes)) {
    biffAddf(LIMN, "%s: trouble", me);
    return 1;
  }
  return 0;
}

int
limnPolyDataSpiralSphere(limnPolyData *pld,
                         unsigned int infoBitFlag,
                         unsigned int thetaRes,
                         unsigned int phiRes) {
  static const char me[]="limnPolyDataSpiralSphere";

  if (limnPolyDataSpiralSuperquadric(pld, infoBitFlag,
                                     1.0, 1.0, thetaRes, phiRes)) {
    biffAddf(LIMN, "%s: trouble", me);
    return 1;
  }
  return 0;
}

/* Geometry for an icosahedron */
#define ICO_ONE 0.5257311121f
#define ICO_TAU 0.8506508084f
static float icovertices[36] = {
  0.0f, ICO_ONE, ICO_TAU,  0.0f, -ICO_ONE, -ICO_TAU,
  0.0f, -ICO_ONE, ICO_TAU,  0.0f, ICO_ONE, -ICO_TAU,
  ICO_ONE, ICO_TAU, 0.0f,  -ICO_ONE, -ICO_TAU, 0.0f,
  ICO_ONE, -ICO_TAU, 0.0f, -ICO_ONE, ICO_TAU, 0.0f,
  ICO_TAU, 0.0f, ICO_ONE,  -ICO_TAU, 0.0f, -ICO_ONE,
  -ICO_TAU, 0.0f, ICO_ONE,  ICO_TAU, 0.0f, -ICO_ONE
};
#undef ICO_ONE
#undef ICO_TAU

static unsigned int icoedges[60] = {
  0, 2,  1, 3,  0, 4,  1, 5,  0, 8,  1, 9,  0, 10,  1, 11, /* 0-7 */
  0, 7,  1, 6,  2, 6,  3, 7,  2, 8,  3, 9,  2, 10,  3, 11, /* 8-15 */
  2, 5,  3, 4,  4, 8,  5, 9,  4, 7,  5, 6,  4, 11,  5, 10, /* 16-23*/
  6, 8,  7, 9,  6, 11,  7, 10,  8, 11,  9, 10 /* 24-29 */
};

static unsigned int icofaces[60] = {
  0, 4, 12,   1, 5, 13,   0, 6, 14,   1, 7, 15,
  6, 8, 27,   7, 9, 26,   2, 4, 18,   3, 5, 19,
  10, 12, 24,   11, 13, 25,   14, 16, 23,   15, 17, 22,
  2, 8, 20,   3, 9, 21,   10, 16, 21,   11, 17, 20,
  24, 26, 28,   25, 27, 29,   18, 22, 28,   19, 23, 29
};

/*
******** limnPolyDataIcoSphere
**
** Makes a unit sphere, centered at the origin, by refining an icosahedron
** level times. Each refinement step subdivides each edge at its center,
** turning each triangle into four smaller ones.
**
** In the output, vertex 2*i and 2*i+1 are antipodal points, which allows for
** a more efficient implementation of operations that produce the same or
** a mirrored result for antipodal points (e.g., tensor glyphs).
*/

int
limnPolyDataIcoSphere(limnPolyData *pld,
                      unsigned int infoBitFlag,
                      unsigned int level) {
  static const char me[]="limnPolyDataIcoSphere";
  unsigned int vertNum=12, edgeNum=30, faceNum=20, center;
  float *verts, *xyzwp, *vertp;
  unsigned char *rgbap;
  unsigned int *edges, *faces;
  unsigned int i,e,f; /* loop counters */
  airArray *mop; /* free memory in case of allocation error */

  /* sanity checks */
  if (!pld) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  mop = airMopNew();

  /* x/y/z positions */
  verts = (float *) malloc (sizeof(float)*12*3);
  if (verts==NULL) goto error_and_exit;
  airMopAdd(mop, verts, airFree, airMopAlways);
  memcpy(verts,icovertices,sizeof(float)*12*3);
  /* endpoints for each edge */
  edges = (unsigned int *) malloc (sizeof(unsigned int)*30*2);
  if (edges==NULL) goto error_and_exit;
  airMopAdd(mop, edges, airFree, airMopAlways);
  memcpy(edges,icoedges,sizeof(unsigned int)*60);
  /* vertices of each face */
  faces = (unsigned int *) malloc (sizeof(unsigned int)*20*3);
  if (faces==NULL) goto error_and_exit;
  airMopAdd(mop, faces, airFree, airMopAlways);
  memcpy(faces,icofaces,sizeof(unsigned int)*60);

  for (i=0; i<level; ++i) {
    /* subdivision step */
    unsigned int nvertNum=vertNum+edgeNum;
    unsigned int nedgeNum=2*edgeNum+3*faceNum;
    unsigned int nfaceNum=4*faceNum;
    float *newverts;
    unsigned int *newedges, *newfaces;
    newverts = (float *) malloc(sizeof(float)*3*nvertNum);
    if (newverts==NULL) goto error_and_exit;
    airMopAdd(mop, newverts, airFree, airMopAlways);
    newedges = (unsigned int *) malloc(sizeof(unsigned int)*2*nedgeNum);
    if (newedges==NULL) goto error_and_exit;
    airMopAdd(mop, newedges, airFree, airMopAlways);
    newfaces = (unsigned int *) malloc(sizeof(unsigned int)*3*nfaceNum);
    if (newfaces==NULL) goto error_and_exit;
    airMopAdd(mop, newfaces, airFree, airMopAlways);
    memcpy(newverts, verts, sizeof(float)*3*vertNum);
    for (e=0; e<edgeNum; e+=2) { /* split both edge and anti-edge */
      float norm;
      ELL_3V_ADD2(newverts+3*(vertNum+e), verts+3*(edges[2*e]),
                  verts+3*(edges[2*e+1]));
      /* project new vertex to unit sphere */
      ELL_3V_NORM_TT(newverts+3*(vertNum+e),float,newverts+3*(vertNum+e),norm);
      ELL_3V_SCALE_TT(newverts+3*(vertNum+e+1),float,-1.0,newverts+3*(vertNum+e));
      /* split the edges such that anti-edge follows edge */
      newedges[4*e]=edges[2*e];
      newedges[4*e+1]=vertNum+e;
      newedges[4*e+2]=edges[2*e+2];
      newedges[4*e+3]=vertNum+e+1;

      newedges[4*e+4]=vertNum+e;
      newedges[4*e+5]=edges[2*e+1];
      newedges[4*e+6]=vertNum+e+1;
      newedges[4*e+7]=edges[2*e+3];
    }
    for (f=0; f<faceNum; f+=2) { /* split both face and anti-face */
      unsigned int oldedge11=faces[3*f], oldedge12=faces[3*f+1],
        oldedge13=faces[3*f+2], oldedge21=faces[3*f+3],
        oldedge22=faces[3*f+4], oldedge23=faces[3*f+5];
      unsigned int eidx=2*edgeNum+3*f; /* index of the first edge to add */
      char pol11=0, pol12=0, pol13=0, pol21=0, pol22=0, pol23=0; /* polarity */
      /* add three edges per face - anti-edge has to follow edge! */
      newedges[2*eidx]=newedges[2*eidx+9]=vertNum+oldedge11;
      newedges[2*eidx+1]=newedges[2*eidx+4]=vertNum+oldedge12;
      newedges[2*eidx+5]=newedges[2*eidx+8]=vertNum+oldedge13;
      newedges[2*eidx+2]=newedges[2*eidx+11]=vertNum+oldedge21;
      newedges[2*eidx+3]=newedges[2*eidx+6]=vertNum+oldedge22;
      newedges[2*eidx+7]=newedges[2*eidx+10]=vertNum+oldedge23;
      /* split the faces - we do not have directed half-edges!
       * determine the "polarity" of the edges (0 forward / 2 backward) */
      if (edges[2*oldedge11+1]==edges[2*oldedge13+1] ||
          edges[2*oldedge11+1]==edges[2*oldedge13])
        pol11=2;
      if (edges[2*oldedge12+1]==edges[2*oldedge11+1] ||
          edges[2*oldedge12+1]==edges[2*oldedge11])
        pol12=2;
      if (edges[2*oldedge13+1]==edges[2*oldedge12+1] ||
          edges[2*oldedge13+1]==edges[2*oldedge12])
        pol13=2;
      if (edges[2*oldedge21+1]==edges[2*oldedge23+1] ||
          edges[2*oldedge21+1]==edges[2*oldedge23])
        pol21=2;
      if (edges[2*oldedge22+1]==edges[2*oldedge21+1] ||
          edges[2*oldedge22+1]==edges[2*oldedge21])
        pol22=2;
      if (edges[2*oldedge23+1]==edges[2*oldedge22+1] ||
          edges[2*oldedge23+1]==edges[2*oldedge22])
        pol23=2;

      newfaces[12*f] = 2*oldedge11-(oldedge11%2)+pol11; /* bottom/left */
      newfaces[12*f+1] = eidx+4;
      newfaces[12*f+2] = 2*oldedge13-(oldedge13%2)+2-pol13;
      newfaces[12*f+3] = 2*oldedge21-(oldedge21%2)+pol21; /* anti */
      newfaces[12*f+4] = eidx+5;
      newfaces[12*f+5] = 2*oldedge23-(oldedge23%2)+2-pol23;

      newfaces[12*f+6] = 2*oldedge11-(oldedge11%2)+2-pol11; /* bottom/right */
      newfaces[12*f+7] = 2*oldedge12-(oldedge12%2)+pol12;
      newfaces[12*f+8] = eidx;
      newfaces[12*f+9] = 2*oldedge21-(oldedge21%2)+2-pol21; /* anti */
      newfaces[12*f+10]= 2*oldedge22-(oldedge22%2)+pol22;
      newfaces[12*f+11]= eidx+1;

      newfaces[12*f+12]= 2*oldedge12-(oldedge12%2)+2-pol12; /* top */
      newfaces[12*f+13]= 2*oldedge13-(oldedge13%2)+pol13;
      newfaces[12*f+14]= eidx+2;
      newfaces[12*f+15]= 2*oldedge22-(oldedge22%2)+2-pol22; /* anti */
      newfaces[12*f+16]= 2*oldedge23-(oldedge23%2)+pol23;
      newfaces[12*f+17]= eidx+3;

      newfaces[12*f+18]= eidx; /* center */
      newfaces[12*f+19]= eidx+2;
      newfaces[12*f+20]= eidx+4;
      newfaces[12*f+21]= eidx+1; /* anti */
      newfaces[12*f+22]= eidx+3;
      newfaces[12*f+23]= eidx+5;
    }
    /* make subdivided mesh the current one */
    airMopSub(mop, verts, airFree);
    airMopSub(mop, edges, airFree);
    airMopSub(mop, faces, airFree);
    free(verts); free(edges); free(faces);
    verts=newverts; edges=newedges; faces=newfaces;
    vertNum=nvertNum; edgeNum=nedgeNum; faceNum=nfaceNum;
  }
  /* done; now copy to a limnPolyData struct */
  if (limnPolyDataAlloc(pld, infoBitFlag, vertNum, 3*faceNum, 1)) {
    biffAddf(LIMN, "%s: couldn't allocate output", me);
    return 1;
  }
  xyzwp=pld->xyzw; vertp=verts;
  for (i=0; i<vertNum; i++) {
    ELL_4V_SET(xyzwp, vertp[0], vertp[1], vertp[2], 1.0);
    xyzwp+=4; vertp+=3;
  }
  if ((1 << limnPolyDataInfoNorm) & infoBitFlag) {
    /* normals equal the vertex coordinates */
    memcpy(pld->norm, verts, sizeof(float)*3*vertNum);
  }
  if ((1 << limnPolyDataInfoRGBA) & infoBitFlag) {
    rgbap=pld->rgba; vertp=verts;
    for (i=0; i<vertNum; i++) {
      ELL_4V_SET_TT(rgbap, unsigned char, 255, 255, 255, 255);
      rgbap+=4; vertp+=3;
    }
  }
  if ((1 << limnPolyDataInfoTex2) & infoBitFlag) {
    for (i=0; i<vertNum; i++) {
      double rr, xyz[3], phi, theta;
      ELL_3V_COPY(xyz, pld->xyzw + 4*i);
      rr = sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1]);
      phi = atan2(rr, xyz[2]);
      theta = atan2(xyz[1], xyz[0]);
      ELL_2V_SET_TT(pld->tex2 + 2*i, float,
                    AIR_AFFINE(-AIR_PI, theta, AIR_PI, 0.0, 1.0),
                    AIR_AFFINE(0.0, phi, AIR_PI, 0.0, 1.0));
    }
  }
  if ((1 << limnPolyDataInfoTang) & infoBitFlag) {
    for (i=0; i<vertNum; i++) {
      double rr, xyz[3], tang[3], len;
      ELL_3V_COPY(xyz, pld->xyzw + 4*i);
      rr = sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1]);
      if (rr) {
        ELL_3V_SET(tang, -xyz[1], xyz[0], 0.0);
        ELL_3V_NORM_TT(pld->tang + 3*i, float, tang, len);
      } else {
        ELL_3V_SET(pld->tang + 3*i, 1.0f, 0.0f, 0.0f);
      }
    }
  }

  /* We need to replace reference to edges in faces with references to
   * vertices. Make sure that they are ordered CCW */
  pld->type[0] = limnPrimitiveTriangles;
  pld->icnt[0] = faceNum*3;
  for (f=0; f<faceNum; ++f) {
    unsigned int vertices[3]; /* find the right vertices */
    float diff1[3],diff2[3],cross[3];
    vertices[0]=edges[2*faces[3*f]];
    vertices[1]=edges[2*faces[3*f]+1];
    if (edges[2*faces[3*f+1]]==vertices[0] ||
        edges[2*faces[3*f+1]]==vertices[1])
      vertices[2]=edges[2*faces[3*f+1]+1];
    else
      vertices[2]=edges[2*faces[3*f+1]];
    /* put them into correct order */
    ELL_3V_SUB(diff1,verts+3*vertices[1],verts+3*vertices[0]);
    ELL_3V_SUB(diff2,verts+3*vertices[2],verts+3*vertices[0]);
    ELL_3V_CROSS(cross,diff1,diff2);
    pld->indx[3*f]=vertices[0];
    if (ELL_3V_DOT(cross,verts+3*vertices[0])<0) {
      pld->indx[3*f+1]=vertices[2];
      pld->indx[3*f+2]=vertices[1];
    } else {
      pld->indx[3*f+1]=vertices[1];
      pld->indx[3*f+2]=vertices[2];
    }
  }
  /* re-order the triangles */
  center=3*(faceNum/2);
  for (i=0; i<faceNum/2; i++) {
    ELL_3V_COPY(faces+3*i,pld->indx+6*i);
    ELL_3V_COPY(faces+center+3*i,pld->indx+6*i+3);
  }
  airMopAdd(mop, pld->indx, airFree, airMopAlways);
  airMopSub(mop, faces, airFree);
  pld->indx=faces;
  /* done */
  airMopOkay(mop);
  return 0;
 error_and_exit:
  biffAddf(LIMN, "%s: memory allocation failed", me);
  airMopError(mop);
  return 1;
}

int
limnPolyDataPlane(limnPolyData *pld,
                  unsigned int infoBitFlag,
                  unsigned int uRes, unsigned int vRes) {
  static const char me[]="limnPolyDataPlane";
  unsigned int vertNum, indxNum, primNum, uIdx, vIdx, vertIdx, primIdx;
  float uu, vv;

  /* sanity */
  uRes = AIR_MAX(2, uRes);
  vRes = AIR_MAX(2, vRes);

  vertNum = uRes*vRes;
  primNum = vRes-1;
  indxNum = primNum*2*uRes;
  if (limnPolyDataAlloc(pld, infoBitFlag, vertNum, indxNum, primNum)) {
    biffAddf(LIMN, "%s: couldn't allocate output", me);
    return 1;
  }

  vertIdx = 0;
  for (vIdx=0; vIdx<vRes; vIdx++) {
    vv = AIR_CAST(float, AIR_AFFINE(0, vIdx, vRes-1, -1.0, 1.0));
    for (uIdx=0; uIdx<uRes; uIdx++) {
      uu = AIR_CAST(float, AIR_AFFINE(0, uIdx, uRes-1, -1.0, 1.0));
      ELL_4V_SET(pld->xyzw + 4*vertIdx, uu, vv, 0.0, 1.0);
      if ((1 << limnPolyDataInfoNorm) & infoBitFlag) {
        ELL_3V_SET_TT(pld->norm + 3*vertIdx, float, 0.0, 0.0, 1.0);
      }
      if ((1 << limnPolyDataInfoRGBA) & infoBitFlag) {
        ELL_4V_SET(pld->rgba + 4*vertIdx, 255, 255, 255, 255);
      }
      if ((1 << limnPolyDataInfoTex2) & infoBitFlag) {
        ELL_2V_SET_TT(pld->tex2 + 2*vertIdx, float, (uu+1.0)/2.0, (vv+1.0)/2.0);
      }
      if ((1 << limnPolyDataInfoTang) & infoBitFlag) {
        ELL_3V_SET_TT(pld->tang + 3*vertIdx, float, 1.0, 0.0, 0.0);
      }
      ++vertIdx;
    }
  }

  vertIdx = 0;
  for (primIdx=0; primIdx<primNum; primIdx++) {
    for (uIdx=0; uIdx<uRes; uIdx++) {
      pld->indx[vertIdx++] = uIdx + uRes*(primIdx+1);
      pld->indx[vertIdx++] = uIdx + uRes*(primIdx);
    }
    pld->type[primIdx] = limnPrimitiveTriangleStrip;
    pld->icnt[primIdx] = 2*uRes;
  }

  return 0;
}

int
limnPolyDataSquare(limnPolyData *pld, unsigned int infoBitFlag) {
  static const char me[]="limnPolyDataSquare";

  if (limnPolyDataAlloc(pld, infoBitFlag, 4, 4, 1)) {
    biffAddf(LIMN, "%s: couldn't allocate output", me);
    return 1;
  }
  ELL_4V_SET(pld->xyzw + 4*0, -1.0, -1.0, 0.0, 1.0);
  ELL_4V_SET(pld->xyzw + 4*1,  1.0, -1.0, 0.0, 1.0);
  ELL_4V_SET(pld->xyzw + 4*2, -1.0,  1.0, 0.0, 1.0);
  ELL_4V_SET(pld->xyzw + 4*3,  1.0,  1.0, 0.0, 1.0);
  if ((1 << limnPolyDataInfoNorm) & infoBitFlag) {
    ELL_3V_SET(pld->norm + 3*0, 0.0, 0.0, 1.0);
    ELL_3V_SET(pld->norm + 3*1, 0.0, 0.0, 1.0);
    ELL_3V_SET(pld->norm + 3*2, 0.0, 0.0, 1.0);
    ELL_3V_SET(pld->norm + 3*3, 0.0, 0.0, 1.0);
  }
  if ((1 << limnPolyDataInfoRGBA) & infoBitFlag) {
    ELL_4V_SET(pld->rgba + 4*0, 255, 255, 255, 255);
    ELL_4V_SET(pld->rgba + 4*1, 255, 255, 255, 255);
    ELL_4V_SET(pld->rgba + 4*2, 255, 255, 255, 255);
    ELL_4V_SET(pld->rgba + 4*3, 255, 255, 255, 255);
  }
  if ((1 << limnPolyDataInfoTex2) & infoBitFlag) {
    ELL_2V_SET(pld->tex2 + 2*0, 0.0, 1.0);
    ELL_2V_SET(pld->tex2 + 2*1, 1.0, 1.0);
    ELL_2V_SET(pld->tex2 + 2*2, 0.0, 0.0);
    ELL_2V_SET(pld->tex2 + 2*3, 1.0, 0.0);
  }
  if ((1 << limnPolyDataInfoTang) & infoBitFlag) {
    ELL_3V_SET(pld->tang + 3*0, 1.0, 0.0, 0.0);
    ELL_3V_SET(pld->tang + 3*1, 1.0, 0.0, 0.0);
    ELL_3V_SET(pld->tang + 3*2, 1.0, 0.0, 0.0);
    ELL_3V_SET(pld->tang + 3*3, 1.0, 0.0, 0.0);
  }
  pld->type[0] = limnPrimitiveTriangleStrip;
  ELL_4V_SET(pld->indx, 0, 1, 2, 3);
  pld->icnt[0] = 4;

  return 0;
}

