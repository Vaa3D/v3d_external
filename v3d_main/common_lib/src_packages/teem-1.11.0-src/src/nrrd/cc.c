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
** learned: if you have globals, such as _nrrdCC_verb, which are
** defined and declared here, but which are NOT initialized, then
** C++ apps which are linking against Teem will have problems!!!
** This was first seen on the mac.
*/
int _nrrdCC_verb = 0;
int _nrrdCC_EqvIncr = 10000; /* HEY: this has to be big so that ccfind is not
                                stuck constantly re-allocating the eqv array.
                                This is will be one of the best places to
                                test the new multiplicative reallocation
                                strategy, planned for Teem 2.0 */

int
_nrrdCCFind_1(Nrrd *nout, unsigned int *numid, const Nrrd *nin) {
  /* static const char me[]="_nrrdCCFind_1"; */
  unsigned int sx, I, id, lval, val, *out, (*lup)(const void *, size_t);

  lup = nrrdUILookup[nin->type];
  out = AIR_CAST(unsigned int*, nout->data);
  out[0] = id = 0;
  *numid = 1;

  sx = AIR_CAST(unsigned int, nin->axis[0].size);
  lval = lup(nin->data, 0);
  for (I=1; I<sx; I++) {
    val = lup(nin->data, I);
    if (lval != val) {
      id++;
      (*numid)++;
    }
    out[I] = id;
    lval = val;
  }

  return 0;
}

/*
** layout of value (pvl) and index (pid) cache:
**
**  2  3  4 --> X
**  1  .  .     oddly, index 0 is never used
**  .  .  .
**  |
**  v Y
*/
int
_nrrdCCFind_2(Nrrd *nout, unsigned int *numid, airArray *eqvArr,
              const Nrrd *nin, unsigned int conny) {
  static const char me[]="_nrrdCCFind_2";
  double vl=0, pvl[5]={0,0,0,0,0};
  unsigned int id, pid[5]={0,0,0,0,0}, (*lup)(const void *, size_t), *out;
  unsigned int p, x, y, sx, sy;

  id = 0; /* sssh! compiler warnings */
  lup = nrrdUILookup[nin->type];
  out = AIR_CAST(unsigned int*, nout->data);
  sx = AIR_CAST(unsigned int, nin->axis[0].size);
  sy = AIR_CAST(unsigned int, nin->axis[1].size);
#define GETV_2(x,y) ((AIR_IN_CL(0, AIR_CAST(int, x), AIR_CAST(int, sx-1))     \
                      && AIR_IN_CL(0, AIR_CAST(int, y), AIR_CAST(int, sy-1))) \
                     ? lup(nin->data, (x) + sx*(y)) \
                     : 0.5) /* value that can't come from an array of uints */
#define GETI_2(x,y) ((AIR_IN_CL(0, AIR_CAST(int, x), AIR_CAST(int, sx-1))     \
                      && AIR_IN_CL(0, AIR_CAST(int, y), AIR_CAST(int, sy-1))) \
                     ? out[(x) + sx*(y)] \
                     : AIR_CAST(unsigned int, -1))  /* CC index (probably!)
                                                       never assigned */

  *numid = 0;
  for (y=0; y<sy; y++) {
    for (x=0; x<sx; x++) {
      if (_nrrdCC_verb) {
        fprintf(stderr, "%s(%d,%d) -----------\n", me, x, y);
      }
      if (!x) {
        pvl[1] = GETV_2(-1, y);   pid[1] = GETI_2(-1, y);
        pvl[2] = GETV_2(-1, y-1); pid[2] = GETI_2(-1, y-1);
        pvl[3] = GETV_2(0, y-1);  pid[3] = GETI_2(0, y-1);

      } else {
        pvl[1] = vl;              pid[1] = id;
        pvl[2] = pvl[3];          pid[2] = pid[3];
        pvl[3] = pvl[4];          pid[3] = pid[4];
      }
      pvl[4] = GETV_2(x+1, y-1);  pid[4] = GETI_2(x+1, y-1);
      vl = GETV_2(x, y);
      p = 0;
      if (vl == pvl[1]) {
        id = pid[p=1];
      }
#define TEST(P) \
      if (vl == pvl[(P)]) {                         \
        if (p) { /* we already had a value match */ \
          if (id != pid[(P)]) {                     \
            airEqvAdd(eqvArr, pid[(P)], id);        \
          }                                         \
        } else {                                    \
          id = pid[p=(P)];                          \
        }                                           \
      }
      TEST(3);
      if (2 == conny) {
        TEST(2);
        TEST(4);
      }
      if (!p) {
        /* didn't match anything previous */
        id = *numid;
        (*numid)++;
      }
      if (_nrrdCC_verb) {
        fprintf(stderr, "%s: pvl: %g %g %g %g (vl = %g)\n", me,
                pvl[1], pvl[2], pvl[3], pvl[4], vl);
        fprintf(stderr, "        pid: %d %d %d %d\n",
                pid[1], pid[2], pid[3], pid[4]);
        fprintf(stderr, "    --> p = %d, id = %d, *numid = %d\n",
                p, id, *numid);
      }
      out[x + sx*y] = id;
    }
  }

  return 0;
}

/*
**
**       5  6  7 --> X
**       8  9 10
**      11 12 13
**      |
**      v Y
**    2  3  4
**  / 1  .  .  again, 0 index never used, for reasons forgotten
** Z  .  .  .
*/
int
_nrrdCCFind_3(Nrrd *nout, unsigned int *numid, airArray *eqvArr,
              const Nrrd *nin, unsigned int conny) {
  /* static const char me[]="_nrrdCCFind_3" ; */
  double pvl[14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0}, vl=0;
  unsigned int id, *out, (*lup)(const void *, size_t),
    pid[14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  unsigned int p, x, y, z, sx, sy, sz;

  id = 0; /* sssh! compiler warnings */
  lup = nrrdUILookup[nin->type];
  out = AIR_CAST(unsigned int*, nout->data);
  sx = AIR_CAST(unsigned int, nin->axis[0].size);
  sy = AIR_CAST(unsigned int, nin->axis[1].size);
  sz = AIR_CAST(unsigned int, nin->axis[2].size);
#define GETV_3(x,y,z) ((AIR_IN_CL(0, AIR_CAST(int, x), AIR_CAST(int, sx-1))   \
                       && AIR_IN_CL(0, AIR_CAST(int, y), AIR_CAST(int, sy-1)) \
                       && AIR_IN_CL(0, AIR_CAST(int, z), AIR_CAST(int, sz-1)))\
                       ? lup(nin->data, (x) + sx*((y) + sy*(z)))              \
                       : 0.5)
#define GETI_3(x,y,z) ((AIR_IN_CL(0, AIR_CAST(int, x), AIR_CAST(int, sx-1))   \
                       && AIR_IN_CL(0, AIR_CAST(int, y), AIR_CAST(int, sy-1)) \
                       && AIR_IN_CL(0, AIR_CAST(int, z), AIR_CAST(int, sz-1)))\
                       ? out[(x) + sx*((y) + sy*(z))]                         \
                       : AIR_CAST(unsigned int, -1))

  *numid = 0;
  for (z=0; z<sz; z++) {
    for (y=0; y<sy; y++) {
      for (x=0; x<sx; x++) {
        if (!x) {
          pvl[ 1] = GETV_3( -1,   y,   z); pid[ 1] = GETI_3( -1,   y,   z);
          pvl[ 2] = GETV_3( -1, y-1,   z); pid[ 2] = GETI_3( -1, y-1,   z);
          pvl[ 3] = GETV_3(  0, y-1,   z); pid[ 3] = GETI_3(  0, y-1,   z);
          pvl[ 5] = GETV_3( -1, y-1, z-1); pid[ 5] = GETI_3( -1, y-1, z-1);
          pvl[ 8] = GETV_3( -1,   y, z-1); pid[ 8] = GETI_3( -1,   y, z-1);
          pvl[11] = GETV_3( -1, y+1, z-1); pid[11] = GETI_3( -1, y+1, z-1);
          pvl[ 6] = GETV_3(  0, y-1, z-1); pid[ 6] = GETI_3(  0, y-1, z-1);
          pvl[ 9] = GETV_3(  0,   y, z-1); pid[ 9] = GETI_3(  0,   y, z-1);
          pvl[12] = GETV_3(  0, y+1, z-1); pid[12] = GETI_3(  0, y+1, z-1);
        } else {
          pvl[ 1] = vl;                    pid[ 1] = id;
          pvl[ 2] = pvl[ 3];               pid[ 2] = pid[ 3];
          pvl[ 3] = pvl[ 4];               pid[ 3] = pid[ 4];
          pvl[ 5] = pvl[ 6];               pid[ 5] = pid[ 6];
          pvl[ 8] = pvl[ 9];               pid[ 8] = pid[ 9];
          pvl[11] = pvl[12];               pid[11] = pid[12];
          pvl[ 6] = pvl[ 7];               pid[ 6] = pid[ 7];
          pvl[ 9] = pvl[10];               pid[ 9] = pid[10];
          pvl[12] = pvl[13];               pid[12] = pid[13];
        }
        pvl[ 4] = GETV_3(x+1, y-1,   z);   pid[ 4] = GETI_3(x+1, y-1,   z);
        pvl[ 7] = GETV_3(x+1, y-1, z-1);   pid[ 7] = GETI_3(x+1, y-1, z-1);
        pvl[10] = GETV_3(x+1,   y, z-1);   pid[10] = GETI_3(x+1,   y, z-1);
        pvl[13] = GETV_3(x+1, y+1, z-1);   pid[13] = GETI_3(x+1, y+1, z-1);
        vl = GETV_3(x, y, z);
        p = 0;
        if (vl == pvl[1]) {
          id = pid[p=1];
        }
        TEST(3);
        TEST(9);
        if (2 <= conny) {
          TEST(2); TEST(4);
          TEST(6); TEST(8); TEST(10); TEST(12);
          if (3 == conny) {
            TEST(5); TEST(7); TEST(11); TEST(13);
          }
        }
        if (!p) {
          /* didn't match anything previous */
          id = *numid;
          (*numid)++;
        }
        out[x + sx*(y + sy*z)] = id;
      }
    }
  }

  return 0;
}

int
_nrrdCCFind_N(Nrrd *nout, unsigned int *numid, airArray *eqvArr,
              const Nrrd *nin, unsigned int conny) {
  static const char me[]="_nrrdCCFind_N";

  AIR_UNUSED(nout);
  AIR_UNUSED(numid);
  AIR_UNUSED(eqvArr);
  AIR_UNUSED(nin);
  AIR_UNUSED(conny);
  biffAddf(NRRD, "%s: sorry, not implemented yet", me);
  return 1;
}

/*
******** nrrdCCFind
**
** finds connected components (CCs) in given integral type nrrd "nin",
** according to connectivity "conny", putting the results in "nout".
** The "type" argument controls what type the output will be.  If
** type == nrrdTypeDefault, the type used will be the smallest that
** can contain the CC id values.  Otherwise, the specified type "type"
** will be used, assuming that it is large enough to hold the CC ids.
**
** "conny": the number of coordinates that need to varied together in
** order to reach all the samples that are to consitute the neighborhood
** around a sample.  For 2-D, conny==1 specifies the 4 edge-connected
** pixels, and 2 specifies the 8 edge- and corner-connected.
**
** The caller can get a record of the values in each CC by passing a
** non-NULL nval, which will be allocated to an array of the same type
** as nin, so that nval->data[I] is the value in nin inside CC #I.
*/
int
nrrdCCFind(Nrrd *nout, Nrrd **nvalP, const Nrrd *nin, int type,
           unsigned int conny) {
  static const char me[]="nrrdCCFind", func[]="ccfind";
  Nrrd *nfpid;  /* first-pass IDs */
  airArray *mop, *eqvArr;
  unsigned int *fpid, numid, numsettleid, *map,
    (*lup)(const void *, size_t), (*ins)(void *, size_t, unsigned int);
  int ret;
  size_t I, NN;
  void *val;

  if (!(nout && nin)) {
    /* NULL nvalP okay */
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nout == nin) {
    biffAddf(NRRD, "%s: nout == nin disallowed", me);
    return 1;
  }
  if (!( nrrdTypeIsIntegral[nin->type]
         && nrrdTypeIsUnsigned[nin->type]
         && nrrdTypeSize[nin->type] <= 4 )) {
    biffAddf(NRRD, "%s: can only find connected components in "
             "1, 2, or 4 byte unsigned integral values (not %s)",
             me, airEnumStr(nrrdType, nin->type));
    return 1;
  }
  if (nrrdTypeDefault != type) {
    if (!( AIR_IN_OP(nrrdTypeUnknown, type, nrrdTypeLast) )) {
      biffAddf(NRRD, "%s: got invalid target type %d", me, type);
      return 1;
    }
    if (!( nrrdTypeIsIntegral[type]
           && nrrdTypeIsUnsigned[nin->type]
           && nrrdTypeSize[type] <= 4 )) {
      biffAddf(NRRD,
               "%s: can only save connected components to 1, 2, or 4 byte "
               "unsigned integral values (not %s)",
               me, airEnumStr(nrrdType, type));
      return 1;
    }
  }
  if (!( conny <= nin->dim )) {
    biffAddf(NRRD, "%s: connectivity value must be in [1..%d] for %d-D "
             "data (not %d)", me, nin->dim, nin->dim, conny);
    return 1;
  }
  if (nrrdConvert(nfpid=nrrdNew(), nin, nrrdTypeUInt)) {
    biffAddf(NRRD, "%s: couldn't allocate fpid %s array to match input size",
             me, airEnumStr(nrrdType, nrrdTypeUInt));
    return 1;
  }

  mop = airMopNew();
  airMopAdd(mop, nfpid, (airMopper)nrrdNuke, airMopAlways);
  eqvArr = airArrayNew(NULL, NULL, 2*sizeof(unsigned int), _nrrdCC_EqvIncr);
  airMopAdd(mop, eqvArr, (airMopper)airArrayNuke, airMopAlways);
  ret = 0;
  switch(nin->dim) {
  case 1:
    ret = _nrrdCCFind_1(nfpid, &numid, nin);
    break;
  case 2:
    ret = _nrrdCCFind_2(nfpid, &numid, eqvArr, nin, conny);
    break;
  case 3:
    ret = _nrrdCCFind_3(nfpid, &numid, eqvArr, nin, conny);
    break;
  default:
    ret = _nrrdCCFind_N(nfpid, &numid, eqvArr, nin, conny);
    break;
  }
  if (ret) {
    biffAddf(NRRD, "%s: initial pass failed", me);
    airMopError(mop); return 1;
  }

  map = AIR_MALLOC(numid, unsigned int);
  airMopAdd(mop, map, airFree, airMopAlways);
  numsettleid = airEqvMap(eqvArr, map, numid);
  /* convert fpid values to final id values */
  fpid = (unsigned int*)(nfpid->data);
  NN = nrrdElementNumber(nfpid);
  for (I=0; I<NN; I++) {
    fpid[I] = map[fpid[I]];
  }
  if (nvalP) {
    if (!(*nvalP)) {
      *nvalP = nrrdNew();
    }
    if (nrrdMaybeAlloc_va(*nvalP, nin->type, 1,
                          AIR_CAST(size_t, numsettleid))) {
      biffAddf(NRRD, "%s: couldn't allocate output value list", me);
      airMopError(mop); return 1;
    }
    airMopAdd(mop, nvalP, (airMopper)airSetNull, airMopOnError);
    airMopAdd(mop, *nvalP, (airMopper)nrrdNuke, airMopOnError);
    val = (*nvalP)->data;
    lup = nrrdUILookup[nin->type];
    ins = nrrdUIInsert[nin->type];
    /* I'm not sure if its more work to do all the redundant assignments
       or to check whether or not to do them */
    for (I=0; I<NN; I++) {
      ins(val, fpid[I], lup(nin->data, I));
    }
  }

  if (nrrdTypeDefault != type) {
    if (numsettleid-1 > nrrdTypeMax[type]) {
      biffAddf(NRRD,
               "%s: max cc id %u is too large to fit in output type %s",
               me, numsettleid-1, airEnumStr(nrrdType, type));
      airMopError(mop); return 1;
    }
  } else {
    type = (numsettleid-1 <= nrrdTypeMax[nrrdTypeUChar]
            ? nrrdTypeUChar
            : (numsettleid-1 <= nrrdTypeMax[nrrdTypeUShort]
               ? nrrdTypeUShort
               : nrrdTypeUInt));
  }
  if (nrrdConvert(nout, nfpid, type)) {
    biffAddf(NRRD, "%s: trouble converting to final output", me);
    airMopError(mop); return 1;
  }
  if (nrrdContentSet_va(nout, func, nin, "%s,%d",
                        airEnumStr(nrrdType, type), conny)) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  if (nout != nin) {
    nrrdAxisInfoCopy(nout, nin, NULL, NRRD_AXIS_INFO_NONE);
  }
  /* basic info handled by nrrdConvert */

  airMopOkay(mop);
  return 0;
}

int
_nrrdCCAdj_1(unsigned char *out, int numid, const Nrrd *nin) {

  AIR_UNUSED(out);
  AIR_UNUSED(numid);
  AIR_UNUSED(nin);
  return 0;
}

int
_nrrdCCAdj_2(unsigned char *out, unsigned int numid, const Nrrd *nin,
             unsigned int conny) {
  unsigned int (*lup)(const void *, size_t), x, y, sx, sy, id=0;
  double pid[5]={0,0,0,0,0};

  lup = nrrdUILookup[nin->type];
  sx = AIR_CAST(unsigned int, nin->axis[0].size);
  sy = AIR_CAST(unsigned int, nin->axis[1].size);
  for (y=0; y<sy; y++) {
    for (x=0; x<sx; x++) {
      if (!x) {
        pid[1] = GETV_2(-1, y);
        pid[2] = GETV_2(-1, y-1);
        pid[3] = GETV_2(0, y-1);
      } else {
        pid[1] = id;
        pid[2] = pid[3];
        pid[3] = pid[4];
      }
      pid[4] = GETV_2(x+1, y-1);
      id = AIR_CAST(unsigned int, GETV_2(x, y));
#define TADJ(P) \
      if (pid[(P)] != 0.5 && id != pid[(P)]) { \
        out[id + numid*AIR_CAST(unsigned int, pid[(P)])] = \
          out[AIR_CAST(unsigned int, pid[(P)]) + numid*id] = 1; \
      }
      TADJ(1);
      TADJ(3);
      if (2 == conny) {
        TADJ(2);
        TADJ(4);
      }
    }
  }

  return 0;
}

int
_nrrdCCAdj_3(unsigned char *out, int numid, const Nrrd *nin,
             unsigned int conny) {
  unsigned int (*lup)(const void *, size_t), x, y, z, sx, sy, sz, id=0;
  double pid[14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  lup = nrrdUILookup[nin->type];
  sx = AIR_CAST(unsigned int, nin->axis[0].size);
  sy = AIR_CAST(unsigned int, nin->axis[1].size);
  sz = AIR_CAST(unsigned int, nin->axis[2].size);
  for (z=0; z<sz; z++) {
    for (y=0; y<sy; y++) {
      for (x=0; x<sx; x++) {
        if (!x) {
          pid[ 1] = GETV_3(-1,   y,   z);
          pid[ 2] = GETV_3(-1, y-1,   z);
          pid[ 3] = GETV_3( 0, y-1,   z);
          pid[ 5] = GETV_3(-1, y-1, z-1);
          pid[ 8] = GETV_3(-1,   y, z-1);
          pid[11] = GETV_3(-1, y+1, z-1);
          pid[ 6] = GETV_3( 0, y-1, z-1);
          pid[ 9] = GETV_3( 0,   y, z-1);
          pid[12] = GETV_3( 0, y+1, z-1);
        } else {
          pid[ 1] = id;
          pid[ 2] = pid[ 3];
          pid[ 3] = pid[ 4];
          pid[ 5] = pid[ 6];
          pid[ 8] = pid[ 9];
          pid[11] = pid[12];
          pid[ 6] = pid[ 7];
          pid[ 9] = pid[10];
          pid[12] = pid[13];
        }
        pid[ 4] = GETV_3(x+1, y-1,   z);
        pid[ 7] = GETV_3(x+1, y-1, z-1);
        pid[10] = GETV_3(x+1,   y, z-1);
        pid[13] = GETV_3(x+1, y+1, z-1);
        id = AIR_CAST(unsigned int, GETV_3(x, y, z));
        TADJ(1);
        TADJ(3);
        TADJ(9);
        if (2 <= conny) {
          TADJ(2); TADJ(4);
          TADJ(6); TADJ(8); TADJ(10); TADJ(12);
          if (3 == conny) {
            TADJ(5); TADJ(7); TADJ(11); TADJ(13);
          }
        }
      }
    }
  }

  return 0;
}

int
_nrrdCCAdj_N(unsigned char *out, int numid, const Nrrd *nin,
             unsigned int conny) {
  static const char me[]="_nrrdCCAdj_N";

  AIR_UNUSED(out);
  AIR_UNUSED(numid);
  AIR_UNUSED(nin);
  AIR_UNUSED(conny);
  biffAddf(NRRD, "%s: sorry, not implemented", me);
  return 1;
}

int
nrrdCCAdjacency(Nrrd *nout, const Nrrd *nin, unsigned int conny) {
  static const char me[]="nrrdCCAdjacency", func[]="ccadj";
  int ret;
  unsigned int maxid;
  unsigned char *out;

  if (!( nout && nrrdCCValid(nin) )) {
    biffAddf(NRRD, "%s: invalid args", me);
    return 1;
  }
  if (nout == nin) {
    biffAddf(NRRD, "%s: nout == nin disallowed", me);
    return 1;
  }
  if (!( AIR_IN_CL(1, conny, nin->dim) )) {
    biffAddf(NRRD, "%s: connectivity value must be in [1..%d] for %d-D "
             "data (not %d)", me, nin->dim, nin->dim, conny);
    return 1;
  }
  maxid = nrrdCCMax(nin);
  if (nrrdMaybeAlloc_va(nout, nrrdTypeUChar, 2,
                        AIR_CAST(size_t, maxid+1),
                        AIR_CAST(size_t, maxid+1))) {
    biffAddf(NRRD, "%s: trouble allocating output", me);
    return 1;
  }
  out = (unsigned char *)(nout->data);

  switch(nin->dim) {
  case 1:
    ret = _nrrdCCAdj_1(out, maxid+1, nin);
    break;
  case 2:
    ret = _nrrdCCAdj_2(out, maxid+1, nin, conny);
    break;
  case 3:
    ret = _nrrdCCAdj_3(out, maxid+1, nin, conny);
    break;
  default:
    ret = _nrrdCCAdj_N(out, maxid+1, nin, conny);
    break;
  }
  if (ret) {
    biffAddf(NRRD, "%s: trouble", me);
    return 1;
  }
  /* this goofiness is just so that histo-based projections
     return the sorts of values that we expect */
  nout->axis[0].center = nout->axis[1].center = nrrdCenterCell;
  nout->axis[0].min = nout->axis[1].min = -0.5;
  nout->axis[0].max = nout->axis[1].max = maxid + 0.5;
  if (nrrdContentSet_va(nout, func, nin, "%d", conny)) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }

  return 0;
}

/*
******** nrrdCCMerge
**
** Slightly-too-multi-purpose tool for merging small connected components
** (CCs) into larger ones, according to a number of possible different
** constraints, as explained below.
**
** valDir: (value direction) uses information about the original values
** in the CC to constrain whether darker gets merged into brighter, or vice
** versa, or neither.  For non-zero valDir values, a non-NULL _nval (from
** nrrdCCFind) must be passed.
**   valDir > 0 : merge dark CCs into bright, but not vice versa
**   valDir = 0 : merge either way, values are irrelevant
**   valDir < 0 : merge bright CCs into dark, but not vice versa
** When merging with multiple neighbors (maxNeighbor > 1), the value
** of the largest neighbor is considered.
**
** maxSize: a cap on how large "small" is- CCs any larger than maxSize are
** not merged, as they are deemed too significant.  Or, a maxSize of 0 says
** size is no object for merging CCs.
**
** maxNeighbor: a maximum number of neighbors that a CC can have (either
** bigger than the CC or not) if it is to be merged.  Use 1 to merge
** isolated islands into their surrounds, 2 to merge CC with the larger
** of their two neighbors, etc., or 0 to allow any number of neighbors.
**
** conny: passed to nrrdCCAdjacency() when determining neighbors
**
** In order to prevent weirdness, the merging done in one call to this
** function is not transitive: if A is merged to B, then B will not be
** merged to anything else, even if meets all the requirements defined
** by the given parameters.  This is accomplished by working from the
** smallest CCs to the largest. Iterated calls may be needed to acheive
** the desired effect.
**
** Note: the output of this is not "settled"- the CC id values are not
** shiftward downwards to their lowest possible values, since this would
** needlessly invalidate the nval value store.
*/
int
nrrdCCMerge(Nrrd *nout, const Nrrd *nin, Nrrd *_nval,
            int valDir, unsigned int maxSize, unsigned int maxNeighbor,
            unsigned int conny) {
  static const char me[]="nrrdCCMerge", func[]="ccmerge";
  const char *valcnt;
  unsigned int _i, i, j, bigi=0, numid, *size, *sizeId,
    *nn,  /* number of neighbors */
    *val=NULL, *hit,
    (*lup)(const void *, size_t), (*ins)(void *, size_t, unsigned int);
  Nrrd *nadj, *nsize, *nval=NULL, *nnn;
  unsigned char *adj;
  unsigned int *map, *id;
  airArray *mop;
  size_t I, NN;

  mop = airMopNew();
  if (!( nout && nrrdCCValid(nin) )) {
    /* _nval can be NULL */
    biffAddf(NRRD, "%s: invalid args", me);
    airMopError(mop); return 1;
  }
  if (valDir) {
    airMopAdd(mop, nval = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    if (nrrdConvert(nval, _nval, nrrdTypeUInt)) {
      biffAddf(NRRD, "%s: value-directed merging needs usable nval", me);
      airMopError(mop); return 1;
    }
    val = (unsigned int*)(nval->data);
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      biffAddf(NRRD, "%s:", me);
      airMopError(mop); return 1;
    }
  }
  airMopAdd(mop, nadj = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nsize = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nnn = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);

  if (nrrdCCSize(nsize, nin)
      || nrrdCopy(nnn, nsize)  /* just to allocate to right size and type */
      || nrrdCCAdjacency(nadj, nin, conny)) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }
  size = (unsigned int*)(nsize->data);
  adj = (unsigned char*)(nadj->data);
  nn = (unsigned int*)(nnn->data);
  numid = AIR_CAST(unsigned int, nsize->axis[0].size);
  for (i=0; i<numid; i++) {
    nn[i] = 0;
    for (j=0; j<numid; j++) {
      nn[i] += adj[j + numid*i];
    }
  }
  map = AIR_MALLOC(numid, unsigned int);
  id = AIR_MALLOC(numid, unsigned int);
  hit = AIR_MALLOC(numid, unsigned int);
  sizeId = AIR_MALLOC(2*numid, unsigned int);
  /* we add to the mops BEFORE error checking so that anything non-NULL
     will get airFree'd, and happily airFree is a no-op on NULL */
  airMopAdd(mop, map, airFree, airMopAlways);
  airMopAdd(mop, id, airFree, airMopAlways);
  airMopAdd(mop, hit, airFree, airMopAlways);
  airMopAdd(mop, sizeId, airFree, airMopAlways);
  if (!(map && id && hit && sizeId)) {
    biffAddf(NRRD, "%s: couldn't allocate buffers", me);
    airMopError(mop); return 1;
  }

  /* store and sort size/id pairs */
  for (i=0; i<numid; i++) {
    sizeId[0 + 2*i] = size[i];
    sizeId[1 + 2*i] = i;
  }
  qsort(sizeId, numid, 2*sizeof(unsigned int), nrrdValCompare[nrrdTypeUInt]);
  for (i=0; i<numid; i++) {
    id[i] = sizeId[1 + 2*i];
  }

  /* initialize arrays */
  for (i=0; i<numid; i++) {
    map[i] = i;
    hit[i] = AIR_FALSE;
  }
  /* _i goes through 0 to numid-1,
     i goes through the CC ids in ascending order of size */
  for (_i=0; _i<numid; _i++) {
    i = id[_i];
    if (hit[i]) {
      continue;
    }
    if (maxSize && (size[i] > maxSize)) {
      continue;
    }
    if (maxNeighbor && (nn[i] > maxNeighbor)) {
      continue;
    }
    /* find biggest neighbor, exploiting the fact that we already
       sorted CC ids on size.  j descends through indices of id[],
       bigi goes through CC ids which are larger than CC i */
    for (j=numid-1; j>_i; j--) {
      bigi = id[j];
      if (adj[bigi + numid*i])
        break;
    }
    if (j == _i) {
      continue;   /* we had no neighbors ?!?! */
    }
    if (valDir && (AIR_CAST(int, val[bigi])
                   - AIR_CAST(int, val[i]))*valDir < 0 ) {
      continue;
    }
    /* else all criteria for merging have been met */
    map[i] = bigi;
    hit[bigi] = AIR_TRUE;
  }
  lup = nrrdUILookup[nin->type];
  ins = nrrdUIInsert[nout->type];
  NN = nrrdElementNumber(nin);
  for (I=0; I<NN; I++) {
    ins(nout->data, I, map[lup(nin->data, I)]);
  }

  valcnt = ((_nval && _nval->content)
            ? _nval->content
            : nrrdStateUnknownContent);
  if ( (valDir && nrrdContentSet_va(nout, func, nin, "%c(%s),%d,%d,%d",
                                    (valDir > 0 ? '+' : '-'), valcnt,
                                    maxSize, maxNeighbor, conny))
       ||
       (!valDir && nrrdContentSet_va(nout, func, nin, ".,%d,%d,%d",
                                     maxSize, maxNeighbor, conny)) ) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }
  /* basic info handled by nrrdCopy */
  airMopOkay(mop);
  return 0;
}

/*
******** nrrdCCRevalue()
**
** assigns the original values back to the connected components
** obviously, this could be subsumed by nrrdApply1DLut(), but this
** is so special purpose that it seemed simpler to code from scratch
*/
int
nrrdCCRevalue (Nrrd *nout, const Nrrd *nin, const Nrrd *nval) {
  static const char me[]="nrrdCCRevalue";
  size_t I, NN;
  unsigned int (*vlup)(const void *, size_t), (*ilup)(const void *, size_t),
    (*ins)(void *, size_t, unsigned int);

  if (!( nout && nrrdCCValid(nin) && nval )) {
    biffAddf(NRRD, "%s: invalid args", me);
    return 1;
  }
  if (nrrdConvert(nout, nin, nval->type)) {
    biffAddf(NRRD, "%s: couldn't initialize output", me);
    return 1;
  }
  NN = nrrdElementNumber(nin);
  vlup = nrrdUILookup[nval->type];
  ilup = nrrdUILookup[nin->type];
  ins = nrrdUIInsert[nout->type];
  for (I=0; I<NN; I++) {
    ins(nout->data, I, vlup(nval->data, ilup(nin->data, I)));
  }
  /* basic info handled by nrrdConvert */

  return 0;
}

int
nrrdCCSettle(Nrrd *nout, Nrrd **nvalP, const Nrrd *nin) {
  static const char me[]="nrrdCCSettle", func[]="ccsettle";
  unsigned int numid, maxid, jd, id, *map,
    (*lup)(const void *, size_t), (*ins)(void *, size_t, unsigned int);
  size_t I, NN;
  airArray *mop;

  mop = airMopNew();
  if (!( nout && nrrdCCValid(nin) )) {
    /* nvalP can be NULL */
    biffAddf(NRRD, "%s: invalid args", me);
    airMopError(mop); return 1;
  }
  if (nrrdCopy(nout, nin)) {
    biffAddf(NRRD, "%s: initial copy failed", me);
    airMopError(mop); return 1;
  }
  maxid = nrrdCCMax(nin);
  lup = nrrdUILookup[nin->type];
  ins = nrrdUIInsert[nin->type];
  NN = nrrdElementNumber(nin);
  map = AIR_CALLOC(maxid+1, unsigned int);  /* we do need it zeroed out */
  if (!map) {
    biffAddf(NRRD, "%s: couldn't allocate internal LUT", me);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, map, airFree, airMopAlways);
  for (I=0; I<NN; I++) {
    map[lup(nin->data, I)] = 1;
  }
  numid = 0;
  for (jd=0; jd<=maxid; jd++) {
    numid += map[jd];
  }

  if (nvalP) {
    if (!(*nvalP)) {
      *nvalP = nrrdNew();
    }
    if (nrrdMaybeAlloc_va(*nvalP, nin->type, 1,
                          AIR_CAST(size_t, numid))) {
      biffAddf(NRRD, "%s: couldn't allocate output value list", me);
      airMopError(mop); return 1;
    }
    airMopAdd(mop, nvalP, (airMopper)airSetNull, airMopOnError);
    airMopAdd(mop, *nvalP, (airMopper)nrrdNuke, airMopOnError);
  }

  id = 0;
  for (jd=0; jd<=maxid; jd++) {
    if (map[jd]) {
      map[jd] = id;
      if (nvalP) {
        ins((*nvalP)->data, id, jd);
      }
      id++;
    }
  }
  for (I=0; I<NN; I++) {
    ins(nout->data, I, map[lup(nin->data, I)]);
  }

  if (nrrdContentSet_va(nout, func, nin, "")) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }
  /* basic info handled by nrrdCopy */
  airMopOkay(mop);
  return 0;
}
