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


#include "limn.h"

const char *
_limnSplineTypeStr[LIMN_SPLINE_TYPE_MAX+1] = {
  "(unknown_spline_type)",
  "linear",
  "timewarp",
  "hermite",
  "cubic-bezier",
  "BC"
};

const char *
_limnSplineTypeDesc[LIMN_SPLINE_TYPE_MAX+1] = {
  "unknown spline type",
  "simple linear interpolation between control points",
  "pseudo-Hermite spline for warping time to uniform (integral) "
    "control point locations",
  "Hermite cubic interpolating spline",
  "cubic Bezier spline",
  "Mitchell-Netravalli BC-family of cubic splines"
};

const char *
_limnSplineTypeStrEqv[] = {
  "linear", "lin", "line", "tent",
  "timewarp", "time-warp", "warp",
  "hermite",
  "cubicbezier", "cubic-bezier", "bezier", "bez",
  "BC", "BC-spline",
  ""
};

const int
_limnSplineTypeValEqv[] = {
  limnSplineTypeLinear, limnSplineTypeLinear, limnSplineTypeLinear,
      limnSplineTypeLinear,
  limnSplineTypeTimeWarp, limnSplineTypeTimeWarp, limnSplineTypeTimeWarp,
  limnSplineTypeHermite,
  limnSplineTypeCubicBezier, limnSplineTypeCubicBezier,
      limnSplineTypeCubicBezier, limnSplineTypeCubicBezier,
  limnSplineTypeBC, limnSplineTypeBC
};

const airEnum
_limnSplineType = {
  "spline-type",
  LIMN_SPLINE_TYPE_MAX,
  _limnSplineTypeStr,  NULL,
  _limnSplineTypeDesc,
  _limnSplineTypeStrEqv, _limnSplineTypeValEqv,
  AIR_FALSE
};
const airEnum *const
limnSplineType = &_limnSplineType;

const char *
_limnSplineInfoStr[LIMN_SPLINE_INFO_MAX+1] = {
  "(unknown_spline_info)",
  "scalar",
  "2vector",
  "3vector",
  "normal",
  "4vector",
  "quaternion"
};

const char *
_limnSplineInfoDesc[LIMN_SPLINE_INFO_MAX+1] = {
  "unknown spline info",
  "scalar",
  "2-vector",
  "3-vector",
  "surface normal, interpolated in S^2",
  "4-vector, interpolated in R^4",
  "quaternion, interpolated in S^3"
};

const char *
_limnSplineInfoStrEqv[] = {
  "scalar", "scale", "s", "t",
  "2-vector", "2vector", "2vec", "2v", "v2", "vec2", "vector2", "vector-2",
  "3-vector", "3vector", "3vec", "3v", "v3", "vec3", "vector3", "vector-3",
  "normal", "norm", "n",
  "4-vector", "4vector", "4vec", "4v", "v4", "vec4", "vector4", "vector-4",
  "quaternion", "quat", "q",
  ""
};

#define SISS limnSplineInfoScalar
#define SI2V limnSplineInfo2Vector
#define SI3V limnSplineInfo3Vector
#define SINN limnSplineInfoNormal
#define SI4V limnSplineInfo4Vector
#define SIQQ limnSplineInfoQuaternion

const int
_limnSplineInfoValEqv[] = {
  SISS, SISS, SISS, SISS,
  SI2V, SI2V, SI2V, SI2V, SI2V, SI2V, SI2V, SI2V,
  SI3V, SI3V, SI3V, SI3V, SI3V, SI3V, SI3V, SI3V,
  SINN, SINN, SINN,
  SI4V, SI4V, SI4V, SI4V, SI4V, SI4V, SI4V, SI4V,
  SIQQ, SIQQ, SIQQ
};

const airEnum
_limnSplineInfo = {
  "spline-info",
  LIMN_SPLINE_INFO_MAX,
  _limnSplineInfoStr,  NULL,
  _limnSplineInfoDesc,
  _limnSplineInfoStrEqv, _limnSplineInfoValEqv,
  AIR_FALSE
};
const airEnum *const
limnSplineInfo = &_limnSplineInfo;

/*
******** limnSplineInfoSize[]
**
** gives the number of scalars per "value" for each splineInfo
*/
unsigned int
limnSplineInfoSize[LIMN_SPLINE_INFO_MAX+1] = {
  0,  /* limnSplineInfoUnknown */
  1,  /* limnSplineInfoScalar */
  2,  /* limnSplineInfo2Vector */
  3,  /* limnSplineInfo3Vector */
  3,  /* limnSplineInfoNormal */
  4,  /* limnSplineInfo4Vector */
  4   /* limnSplineInfoQuaternion */
};

/*
******** limnSplineTypeHasImplicitTangents[]
**
** this is non-zero when the spline path is determined solely the
** main control point values, without needing additional control
** points (as in cubic Bezier) or tangent information (as in Hermite)
*/
int
limnSplineTypeHasImplicitTangents[LIMN_SPLINE_TYPE_MAX+1] = {
  AIR_FALSE, /* limnSplineTypeUnknown */
  AIR_TRUE,  /* limnSplineTypeLinear */
  AIR_FALSE, /* limnSplineTypeTimeWarp */
  AIR_FALSE, /* limnSplineTypeHermite */
  AIR_FALSE, /* limnSplineTypeCubicBezier */
  AIR_TRUE   /* limnSplineTypeBC */
};

int
limnSplineNumPoints(limnSpline *spline) {
  int ret;

  ret = -1;
  if (spline) {
    ret = spline->ncpt->axis[2].size;
  }
  return ret;
}

double
limnSplineMinT(limnSpline *spline) {
  double ret;

  ret = AIR_NAN;
  if (spline) {
    ret = spline->time ? spline->time[0] : 0;
  }
  return ret;
}

double
limnSplineMaxT(limnSpline *spline) {
  double ret;
  int N;

  ret = AIR_NAN;
  if (spline) {
    N = spline->ncpt->axis[2].size;
    if (spline->time) {
      ret = spline->time[N-1];
    } else {
      ret = spline->loop ? N : N-1;
    }
  }
  return ret;
}

void
limnSplineBCSet(limnSpline *spline, double B, double C) {

  if (spline) {
    spline->B = B;
    spline->C = C;
  }
}

limnSplineTypeSpec *
limnSplineTypeSpecParse(char *_str) {
  static const char me[]="limnSplineTypeSpecParse";
  limnSplineTypeSpec *spec;
  int type;
  double B, C;
  char *str, *col, *bcS;
  airArray *mop;

  if (!( _str && airStrlen(_str) )) {
    biffAddf(LIMN, "%s: got NULL or emptry string", me);
    return NULL;
  }
  mop = airMopNew();
  airMopAdd(mop, str=airStrdup(_str), airFree, airMopAlways);
  col = strchr(str, ':');
  if (col) {
    *col = 0;
    bcS = col+1;
  } else {
    bcS = NULL;
  }
  if (limnSplineTypeUnknown == (type = airEnumVal(limnSplineType, str))) {
    biffAddf(LIMN, "%s: couldn't parse \"%s\" as spline type", me, str);
    airMopError(mop); return NULL;
  }

  if (!( (limnSplineTypeBC == type) == !!bcS )) {
    biffAddf(LIMN, "%s: spline type %s %s, but %s a parameter string %s%s%s",
             me,
             (limnSplineTypeBC == type) ? "is" : "is not",
             airEnumStr(limnSplineType, limnSplineTypeBC),
             !!bcS ? "got unexpected" : "did not get",
             !!bcS ? "\"" : "",
             !!bcS ? bcS : "",
             !!bcS ? "\"" : "");
    airMopError(mop); return NULL;
  }
  if (limnSplineTypeBC == type) {
    if (2 != sscanf(bcS, "%lg,%lg", &B, &C)) {
      biffAddf(LIMN, "%s: couldn't parse \"B,C\" parameters from \"%s\"", me,
               bcS);
      airMopError(mop); return NULL;
    }
  }
  spec = (limnSplineTypeBC == type
          ? limnSplineTypeSpecNew(type, B, C)
          : limnSplineTypeSpecNew(type));
  if (!spec) {
    biffAddf(LIMN, "%s: limnSplineTypeSpec allocation failed", me);
    airMopError(mop); return NULL;
  }

  airMopOkay(mop);
  return spec;
}

limnSpline *
limnSplineParse(char *_str) {
  static const char me[]="limnSplineParse";
  char *str, *col, *fnameS, *infoS, *typeS, *tmpS;
  int info;
  limnSpline *spline;
  limnSplineTypeSpec *spec;
  Nrrd *ninA, *ninB;
  airArray *mop;

  if (!( _str && airStrlen(_str) )) {
    biffAddf(LIMN, "%s: got NULL or empty string", me);
    return NULL;
  }
  mop = airMopNew();
  airMopAdd(mop, str=airStrdup(_str), airFree, airMopAlways);

  /* find separation between filename and "<splineInfo>:<splineType>[:B,C]" */
  col = strchr(str, ':');
  if (!col) {
    biffAddf(LIMN, "%s: saw no colon separator (between nrrd filename and "
             "spline info) in \"%s\"", me, _str);
    airMopError(mop); return NULL;
  }
  fnameS = str;
  *col = 0;
  tmpS = col+1;
  airMopAdd(mop, ninA = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdLoad(ninA, fnameS, NULL)) {
    biffMovef(LIMN, NRRD, "%s: couldn't read control point nrrd:\n", me);
    airMopError(mop); return NULL;
  }

  /* find separation between splineInfo and "<splineType>[:B,C]" */
  col = strchr(tmpS, ':');
  if (!col) {
    biffAddf(LIMN, "%s: saw no colon separator (between spline info "
             "and spline type) in \"%s\"", me, tmpS);
    airMopError(mop); return NULL;
  }
  infoS = tmpS;
  *col = 0;
  typeS = col+1;
  if (limnSplineInfoUnknown == (info = airEnumVal(limnSplineInfo, infoS))) {
    biffAddf(LIMN, "%s: couldn't parse \"%s\" as spline info", me, infoS);
    airMopError(mop); return NULL;
  }

  /* now parse <splineType>[:B,C] */
  if (!( spec = limnSplineTypeSpecParse(typeS) )) {
    biffAddf(LIMN, "%s: couldn't parse spline type in \"%s\":\n", me, typeS);
    airMopError(mop); return NULL;
  }
  if (limnSplineTypeTimeWarp == spec->type
      && limnSplineInfoScalar != info) {
    biffAddf(LIMN, "%s: can only time-warp %s info, not %s", me,
             airEnumStr(limnSplineInfo, limnSplineInfoScalar),
             airEnumStr(limnSplineInfo, info));
    airMopError(mop); return NULL;
  }

  airMopAdd(mop, ninB = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (limnSplineNrrdCleverFix(ninB, ninA, info, spec->type)) {
    biffAddf(LIMN, "%s: couldn't reshape given nrrd:\n", me);
    airMopError(mop); return NULL;
  }
  if (!( spline = limnSplineNew(ninB, info, spec) )) {
    biffAddf(LIMN, "%s: couldn't create spline:\n", me);
    airMopError(mop); return NULL;
  }

  airMopOkay(mop);
  return spline;
}

/*
** the spline command-line spline type specification is of the form
** <splineType>[:B,C]
*/
int
_limnHestSplineTypeSpecParse(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  static const char me[] = "_limnHestSplineTypeSpecParse";
  char *err2;
  limnSplineTypeSpec **specP;

  if (!(ptr && str && airStrlen(str))) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  specP = (limnSplineTypeSpec **)ptr;

  if (!( *specP = limnSplineTypeSpecParse(str) )) {
    err2 = biffGetDone(LIMN);
    sprintf(err, "%s: couldn't parse \"%s\":\n", me, str);
    strncat(err, err2, AIR_STRLEN_HUGE-1-strlen(err));
    free(err2); return 1;
  }

  return 0;
}


hestCB
_limnHestSplineTypeSpec = {
  sizeof(limnSplineTypeSpec *),
  "spline type specification",
  _limnHestSplineTypeSpecParse,
  (airMopper)limnSplineTypeSpecNix
};

hestCB *
limnHestSplineTypeSpec = &_limnHestSplineTypeSpec;

/*
** the spline command-line specification is of the form
** <nrrdFileName>:<splineInfo>:<splineType>[:B,C]
*/
int
_limnHestSplineParse(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  static const char me[] = "_limnHestSplineParse";
  char *err2;
  limnSpline **splineP;

  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  splineP = (limnSpline **)ptr;
  if (!airStrlen(str)) {
    /* got an empty string, which for now we take as an okay way
       to ask for NO spline */
    *splineP = NULL;
    return 0;
  }

  if (!( *splineP = limnSplineParse(str) )) {
    err2 = biffGetDone(LIMN);
    sprintf(err, "%s: couldn't parse \"%s\":\n", me, str);
    strncat(err, err2, AIR_STRLEN_HUGE-1-strlen(err));
    free(err2); return 1;
  }

  return 0;
}


hestCB
_limnHestSpline = {
  sizeof(limnSpline *),
  "spline specification",
  _limnHestSplineParse,
  (airMopper)limnSplineNix
};

hestCB *
limnHestSpline = &_limnHestSpline;
