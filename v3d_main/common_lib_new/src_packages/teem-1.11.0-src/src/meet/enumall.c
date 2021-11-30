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

#include "meet.h"

const int
meetPresent = 42;

const char *
meetBiffKey = "meet";

typedef union {
  const airEnum ***enm;
  void **v;
} foobarUnion;

/*
******** meetAirEnumAll
**
** ALLOCATES and returns a NULL-terminated array of
** pointers to all the airEnums in Teem
**
** It would be better if this array could be created at compile-time,
** but efforts at doing this resulted in lots of "initializer is not const"
** errors.
**
** NOTE: the order here reflects the library ordering of the LIBS
** variable in teem/src/GNUMakefile, which is the canonical dependency
** ordering of the libraries.  Can manually check completeness by:
** (TEEM_LIB_LIST)

grep "airEnum *" {air,hest,biff,nrrd,ell,unrrdu,alan,moss,tijk,gage,dye,bane,limn,echo,hoover,seek,ten,elf,pull,coil,push,mite}/?*.h | grep EXPORT | more

** (with the ? in "}/?*.h" to stop warnings about / * inside comment)
** We could grep specifically for "const airEnum *const", but its good to
** use this occasion to also make sure that all public airEnums are
** indeed const airEnum *const
*/
const airEnum **
meetAirEnumAll() {
  airArray *arr;
  const airEnum **enm;
  unsigned int ii;
  foobarUnion fbu;

  arr = airArrayNew((fbu.enm = &enm, fbu.v),
                    NULL, sizeof(airEnum *), 2);

  /* air */
  ii = airArrayLenIncr(arr, 1); enm[ii] = airEndian;
  ii = airArrayLenIncr(arr, 1); enm[ii] = airBool;

  /* hest: no airEnums */

  /* biff: no airEnums */

  /* nrrd */
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdFormatType;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdType;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdEncodingType;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdCenter;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdKind;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdField;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdSpace;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdSpacingStatus;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdBoundary;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdMeasure;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdUnaryOp;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdBinaryOp;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdTernaryOp;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdFFTWPlanRigor;
  ii = airArrayLenIncr(arr, 1); enm[ii] = nrrdResampleNonExistent;

  /* ell */
  ii = airArrayLenIncr(arr, 1); enm[ii] = ell_cubic_root;

  /* unrrdu: no airEnums */

#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
  /* alan */
  ii = airArrayLenIncr(arr, 1); enm[ii] = alanStop;
#endif

  /* moss: no airEnums */

#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
  ii = airArrayLenIncr(arr, 1); enm[ii] = tijk_class;
#endif

  /* gage */
  ii = airArrayLenIncr(arr, 1); enm[ii] = gageErr;
  ii = airArrayLenIncr(arr, 1); enm[ii] = gageKernel;
  ii = airArrayLenIncr(arr, 1); enm[ii] = gageItemPackPart;
  ii = airArrayLenIncr(arr, 1); enm[ii] = gageScl;
  ii = airArrayLenIncr(arr, 1); enm[ii] = gageVec;

  /* dye: no airEnums */

#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
  /* bane */
  ii = airArrayLenIncr(arr, 1); enm[ii] = baneGkmsMeasr;
#endif

  /* limn */
  ii = airArrayLenIncr(arr, 1); enm[ii] = limnSpace;
  ii = airArrayLenIncr(arr, 1); enm[ii] = limnPolyDataInfo;
  ii = airArrayLenIncr(arr, 1); enm[ii] = limnCameraPathTrack;
  ii = airArrayLenIncr(arr, 1); enm[ii] = limnPrimitive;
  ii = airArrayLenIncr(arr, 1); enm[ii] = limnSplineType;
  ii = airArrayLenIncr(arr, 1); enm[ii] = limnSplineInfo;

  /* echo */
  ii = airArrayLenIncr(arr, 1); enm[ii] = echoJitter;
  ii = airArrayLenIncr(arr, 1); enm[ii] = echoType;
  ii = airArrayLenIncr(arr, 1); enm[ii] = echoMatter;

  /* hoover */
  ii = airArrayLenIncr(arr, 1); enm[ii] = hooverErr;

  /* seek */
  ii = airArrayLenIncr(arr, 1); enm[ii] = seekType;

  /* ten */
  ii = airArrayLenIncr(arr, 1); enm[ii] = tenAniso;
  ii = airArrayLenIncr(arr, 1); enm[ii] = tenInterpType;
  ii = airArrayLenIncr(arr, 1); enm[ii] = tenGage;
  ii = airArrayLenIncr(arr, 1); enm[ii] = tenFiberType;
  ii = airArrayLenIncr(arr, 1); enm[ii] = tenDwiFiberType;
  ii = airArrayLenIncr(arr, 1); enm[ii] = tenFiberStop;
  ii = airArrayLenIncr(arr, 1); enm[ii] = tenFiberIntg;
  ii = airArrayLenIncr(arr, 1); enm[ii] = tenGlyphType;
  ii = airArrayLenIncr(arr, 1); enm[ii] = tenEstimate1Method;
  ii = airArrayLenIncr(arr, 1); enm[ii] = tenEstimate2Method;
  ii = airArrayLenIncr(arr, 1); enm[ii] = tenTripleType;
  ii = airArrayLenIncr(arr, 1); enm[ii] = tenDwiGage;

#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
  /* elf: no airEnums */
#endif

  /* pull */
  ii = airArrayLenIncr(arr, 1); enm[ii] = pullInterType;
  ii = airArrayLenIncr(arr, 1); enm[ii] = pullEnergyType;
  ii = airArrayLenIncr(arr, 1); enm[ii] = pullInfo;
  ii = airArrayLenIncr(arr, 1); enm[ii] = pullSource;
  ii = airArrayLenIncr(arr, 1); enm[ii] = pullProp;
  ii = airArrayLenIncr(arr, 1); enm[ii] = pullProcessMode;
  ii = airArrayLenIncr(arr, 1); enm[ii] = pullTraceStop;
  ii = airArrayLenIncr(arr, 1); enm[ii] = pullCount;
  ii = airArrayLenIncr(arr, 1); enm[ii] = pullConstraintFail;

#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
  /* coil */
  ii = airArrayLenIncr(arr, 1); enm[ii] = coilMethodType;
  ii = airArrayLenIncr(arr, 1); enm[ii] = coilKindType;

  /* push */
  ii = airArrayLenIncr(arr, 1); enm[ii] = pushEnergyType;
#endif

  /* mite */
  ii = airArrayLenIncr(arr, 1); enm[ii] = miteVal;
  ii = airArrayLenIncr(arr, 1); enm[ii] = miteStageOp;

  /* meet: no new airEnums of its own */

  /* NULL-terminate the list */
  ii = airArrayLenIncr(arr, 1); enm[ii] = NULL;

  /* nix, not nuke the airArray */
  airArrayNix(arr);
  return enm;
}

void
meetAirEnumAllPrint(FILE *file) {
  const airEnum **enm, *ee;
  unsigned int ei;

  if (!file) {
    return;
  }
  enm = meetAirEnumAll();
  ei = 0;
  while ((ee = enm[ei])) {
    airEnumPrint(file, ee);
    fprintf(file, "\n");
    ei++;
  }
  free(AIR_CAST(void *, enm));
  return;
}

int
meetAirEnumAllCheck(void) {
  static const char me[]="meetAirEnumAllCheck";
  const airEnum **enm, *ee;
  char err[AIR_STRLEN_LARGE];
  unsigned int ei;
  airArray *mop;

  mop = airMopNew();
  enm = meetAirEnumAll();
  airMopAdd(mop, (void*)enm, airFree, airMopAlways);
  ei = 0;
  while ((ee = enm[ei])) {
    /* fprintf(stderr, "!%s: %u %s\n", me, ei, ee->name); */
    if (airEnumCheck(err, ee)) {
      biffAddf(MEET, "%s: problem with enum %u", me, ei);
      biffAddf(MEET, "%s", err); /* kind of a hack */
      airMopError(mop);
      return 1;
    }
    ei++;
  }
  airMopOkay(mop);
  return 0;
}

const char *const
meetTeemLibs[] = {
  /* TEEM_LIB_LIST */
  "air",
  "hest",
  "biff",
  "nrrd",
  "ell",
  "unrrdu",
#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
  "alan",
#endif
  "moss",
#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
  "tijk",
#endif
  "gage",
  "dye",
#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
  "bane",
#endif
  "limn",
  "echo",
  "hoover",
  "seek",
  "ten",
#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
  "elf",
#endif
  "pull",
#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
  "coil",
  "push",
#endif
  "mite",
  "meet",
  NULL
};


