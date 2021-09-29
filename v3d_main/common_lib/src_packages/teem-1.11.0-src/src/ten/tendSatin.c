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

#define INFO "Generate a pretty synthetic DT volume"
static const char *_tend_satinInfoL =
  (INFO
   ".  The surface of a sphere or torus is covered with either linear or "
   "planar anisotropic tensors, or somewhere in between.");

void
tend_satinSphereEigen(float *eval, float *evec, float x, float y, float z,
                      float parm, float mina, float maxa,
                      float thick, float bnd, float evsc) {
  float aniso, bound1, bound2, r, norm, tmp[3];

  r = AIR_CAST(float, sqrt(x*x + y*y + z*z));
  /* 1 on inside, 0 on outside */
  bound1 = AIR_CAST(float, 0.5 - 0.5*airErf((r-0.9)/(bnd + 0.0001)));
  /* other way around */
  bound2 = AIR_CAST(float, 0.5 - 0.5*airErf((0.9-thick-r)/(bnd + 0.0001)));
  aniso = AIR_CAST(float, AIR_AFFINE(0.0, AIR_MIN(bound1, bound2), 1.0,
                                     mina, maxa));

  ELL_3V_SET_TT(eval, float,
                AIR_LERP(aniso, 1.0/3.0, AIR_AFFINE(0.0, parm, 2.0, 1.0, 0.0)),
                AIR_LERP(aniso, 1.0/3.0, AIR_AFFINE(0.0, parm, 2.0, 0.0, 1.0)),
                AIR_LERP(aniso, 1.0/3.0, 0));
  ELL_3V_SCALE(eval, evsc, eval);

  /* v0: looking down positive Z, points counter clockwise */
  if (x || y) {
    ELL_3V_SET(evec + 3*0, y, -x, 0);
    ELL_3V_NORM_TT(evec + 3*0, float, evec + 3*0, norm);

    /* v1: points towards pole at positive Z */
    ELL_3V_SET(tmp, -x, -y, -z);
    ELL_3V_NORM_TT(tmp, float, tmp, norm);
    ELL_3V_CROSS(evec + 3*1, tmp, evec + 3*0);

    /* v2: v0 x v1 */
    ELL_3V_CROSS(evec + 3*2, evec + 3*0, evec + 3*1);
  } else {
    /* not optimal, but at least it won't show up in glyph visualizations */
    ELL_3M_IDENTITY_SET(evec);
  }
  return;
}

void
tend_satinTorusEigen(float *eval, float *evec, float x, float y, float z,
                     float parm, float mina, float maxa,
                     float thick, float bnd, float evsc) {
  float bound, R, r, norm, out[3], up[3], aniso;

  thick *= 2;
  R = AIR_CAST(float, sqrt(x*x + y*y));
  r = AIR_CAST(float, sqrt((R-1)*(R-1) + z*z));
  /* 1 on inside, 0 on outside */
  bound = AIR_CAST(float, 0.5 - 0.5*airErf((r-thick)/(bnd + 0.0001)));
  aniso = AIR_CAST(float, AIR_AFFINE(0, bound, 1, mina, maxa));

  ELL_3V_SET_TT(eval, float,
                AIR_LERP(aniso, 1.0/3.0, AIR_AFFINE(0.0, parm, 2.0, 1.0, 0.0)),
                AIR_LERP(aniso, 1.0/3.0, AIR_AFFINE(0.0, parm, 2.0, 0.0, 1.0)),
                AIR_LERP(aniso, 1.0/3.0, 0));
  ELL_3V_SCALE(eval, evsc, eval);

  ELL_3V_SET(up, 0, 0, 1);

  if (x || y) {
    /* v0: looking down positive Z, points counter clockwise */
    ELL_3V_SET(evec + 3*0, y, -x, 0);
    ELL_3V_NORM_TT(evec + 3*0, float, evec + 3*0, norm);

    /* v2: points into core of torus */
    /* out: points away from (x,y)=(0,0) */
    ELL_3V_SET(out, x, y, 0);
    ELL_3V_NORM_TT(out, float, out, norm);
    ELL_3V_SCALE_ADD2(evec + 3*2, -z, up, (1-R), out);
    ELL_3V_NORM_TT(evec + 3*2, float, evec + 3*2, norm);

    /* v1: looking at right half of cross-section, points counter clockwise */
    ELL_3V_CROSS(evec + 3*1, evec + 3*0, evec + 3*2);
  } else {
    /* not optimal, but at least it won't show up in glyph visualizations */
    ELL_3M_IDENTITY_SET(evec);
  }
  return;
}

int
tend_satinGen(Nrrd *nout, float parm, float mina, float maxa, int wsize,
              float thick, float bnd, float bndRm, float evsc, int torus) {
  static const char me[]="tend_satinGen";
  char buff[AIR_STRLEN_SMALL];
  Nrrd *nconf, *neval, *nevec;
  float *conf, *eval, *evec;
  size_t xi, yi, zi, size[3];
  float x, y, z, min[3], max[3];

  if (torus) {
    ELL_3V_SET(size, 2*wsize, 2*wsize, wsize);
    ELL_3V_SET(min, -2, -2, -1);
    ELL_3V_SET(max, 2, 2, 1);
  } else {
    ELL_3V_SET(size, wsize, wsize, wsize);
    ELL_3V_SET(min, -1, -1, -1);
    ELL_3V_SET(max, 1, 1, 1);
  }
  if (nrrdMaybeAlloc_va(nconf=nrrdNew(), nrrdTypeFloat, 3,
                        size[0], size[1], size[2]) ||
      nrrdMaybeAlloc_va(neval=nrrdNew(), nrrdTypeFloat, 4,
                        AIR_CAST(size_t, 3), size[0], size[1], size[2]) ||
      nrrdMaybeAlloc_va(nevec=nrrdNew(), nrrdTypeFloat, 4,
                        AIR_CAST(size_t, 9), size[0], size[1], size[2])) {
    biffMovef(TEN, NRRD, "%s: trouble allocating temp nrrds", me);
    return 1;
  }

  conf = (float *)nconf->data;
  eval = (float *)neval->data;
  evec = (float *)nevec->data;
  for (zi=0; zi<size[2]; zi++) {
    z = AIR_CAST(float, AIR_AFFINE(0, zi, size[2]-1, min[2], max[2]));
    for (yi=0; yi<size[1]; yi++) {
      y = AIR_CAST(float, AIR_AFFINE(0, yi, size[1]-1, min[1], max[1]));
      for (xi=0; xi<size[0]; xi++) {
        x = AIR_CAST(float, AIR_AFFINE(0, xi, size[0]-1, min[0], max[0]));
        *conf = 1.0;
        if (torus) {
          float aff;
          aff = AIR_CAST(float, AIR_AFFINE(0, yi, size[1]-1, 0, 1));
          tend_satinTorusEigen(eval, evec, x, y, z, parm,
                               mina, maxa, thick, bnd + bndRm*aff, evsc);
        } else {
          float aff;
          aff = AIR_CAST(float, AIR_AFFINE(0,yi, size[1]-1, 0, 1));
          tend_satinSphereEigen(eval, evec, x, y, z, parm,
                                mina, maxa, thick, bnd + bndRm*aff, evsc);
        }
        conf += 1;
        eval += 3;
        evec += 9;
      }
    }
  }

  if (tenMake(nout, nconf, neval, nevec)) {
    biffAddf(TEN, "%s: trouble generating output", me);
    return 1;
  }

  nrrdNuke(nconf);
  nrrdNuke(neval);
  nrrdNuke(nevec);
  nrrdAxisInfoSet_va(nout, nrrdAxisInfoLabel, "tensor", "x", "y", "z");
  sprintf(buff, "satin(%g,%g,%g)", parm, mina, maxa);
  nout->content = airStrdup(buff);
  return 0;
}

int
tend_satinMain(int argc, const char **argv, const char *me,
               hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  int wsize, torus;
  float parm, maxa, mina, thick, bnd, bndRm, evsc;
  Nrrd *nout;
  char *outS;
  gageShape *shape;
  double spd[4][4], orig[4];

  hestOptAdd(&hopt, "t", "do torus", airTypeInt, 0, 0, &torus, NULL,
             "generate a torus dataset, instead of the default spherical");
  hestOptAdd(&hopt, "p", "aniso parm", airTypeFloat, 1, 1, &parm, NULL,
             "anisotropy parameter.  0.0 for one direction of linear (along "
             "the equator for spheres, or along the larger circumference for "
             "toruses), 1.0 for planar, 2.0 for the other direction of linear "
             "(from pole to pole for spheres, or along the smaller "
             "circumference for toruses)");
  hestOptAdd(&hopt, "max", "max ca1", airTypeFloat, 1, 1, &maxa, "1.0",
             "maximum anisotropy in dataset, according to the \"ca1\" "
             "anisotropy metric.  \"1.0\" means "
             "completely linear or completely planar anisotropy");
  hestOptAdd(&hopt, "min", "min ca1", airTypeFloat, 1, 1, &mina, "0.0",
             "minimum anisotropy in dataset");
  hestOptAdd(&hopt, "b", "boundary", airTypeFloat, 1, 1, &bnd, "0.05",
             "parameter governing how fuzzy the boundary between high and "
             "low anisotropy is. Use \"-b 0\" for no fuzziness");
  hestOptAdd(&hopt, "br", "ramp", airTypeFloat, 1, 1, &bndRm, "0.0",
             "how much to ramp upeffective \"b\" along Y axis. "
             "Use \"-b 0\" for no such ramping.");
  hestOptAdd(&hopt, "th", "thickness", airTypeFloat, 1, 1, &thick, "0.3",
             "parameter governing how thick region of high anisotropy is");
  hestOptAdd(&hopt, "evsc", "eval scale", airTypeFloat, 1, 1, &evsc, "1.0",
             "scaling of eigenvalues");
  hestOptAdd(&hopt, "s", "size", airTypeInt, 1, 1, &wsize, "32",
             "dimensions of output volume.  For size N, the output is "
             "N\tx\tN\tx\tN for spheres, and 2N\tx\t2N\tx\tN for toruses");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output filename");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_satinInfoL);
  JUSTPARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (tend_satinGen(nout, parm, mina, maxa, wsize, thick,
                    bnd, bndRm, evsc, torus)) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble making volume:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  /* use gageShape to determine orientation info */
  nrrdAxisInfoSet_va(nout, nrrdAxisInfoCenter,
                     nrrdCenterUnknown, nrrdCenterCell,
                     nrrdCenterCell, nrrdCenterCell);
  shape = gageShapeNew();
  airMopAdd(mop, shape, (airMopper)gageShapeNix, airMopAlways);
  /* this is a weird mix of new and legacy code.  At some point
     prior to Wed May 27 19:23:55 CDT 2009, it was okay to pass
     in a volume to gageShapeSet that had absolutely no notion
     of spacing or orientation.  Then gageShapeSet was used to
     get a plausible set of space directions and space origin.
     Now, we're setting some spacings, so that gageShapeSet can
     do its thing, then (below) nan-ing out those spacings so
     that the nrrd is self-consistent */
  nout->axis[1].spacing = 1.0;
  nout->axis[2].spacing = 1.0;
  nout->axis[3].spacing = 1.0;
  if (gageShapeSet(shape, nout, tenGageKind->baseDim)) {
    airMopAdd(mop, err=biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble doing shape:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  /* the ItoW is a 4x4 matrix, but
     we really only care about the first three rows */
  ELL_4V_SET(spd[0], AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_4MV_COL0_GET(spd[1], shape->ItoW); ELL_4V_SCALE(spd[1], 32, spd[1]);
  ELL_4MV_COL1_GET(spd[2], shape->ItoW); ELL_4V_SCALE(spd[2], 32, spd[2]);
  ELL_4MV_COL2_GET(spd[3], shape->ItoW); ELL_4V_SCALE(spd[3], 32, spd[3]);
  ELL_4MV_COL3_GET(orig, shape->ItoW);   ELL_4V_SCALE(orig, 32, orig);
  nrrdSpaceSet(nout, nrrdSpaceRightAnteriorSuperior);
  nrrdSpaceOriginSet(nout, orig);
  nrrdAxisInfoSet_va(nout, nrrdAxisInfoSpaceDirection,
                     spd[0], spd[1], spd[2], spd[3]);
  nout->axis[1].spacing = AIR_NAN;
  nout->axis[2].spacing = AIR_NAN;
  nout->axis[3].spacing = AIR_NAN;
  nrrdAxisInfoSet_va(nout, nrrdAxisInfoKind,
                     nrrdKind3DMaskedSymMatrix, nrrdKindSpace,
                     nrrdKindSpace, nrrdKindSpace);
  nout->measurementFrame[0][0] = 1;
  nout->measurementFrame[1][0] = 0;
  nout->measurementFrame[2][0] = 0;
  nout->measurementFrame[0][1] = 0;
  nout->measurementFrame[1][1] = 1;
  nout->measurementFrame[2][1] = 0;
  nout->measurementFrame[0][2] = 0;
  nout->measurementFrame[1][2] = 0;
  nout->measurementFrame[2][2] = 1;

  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
/* TEND_CMD(satin, INFO); */
unrrduCmd tend_satinCmd = { "satin", INFO, tend_satinMain };
