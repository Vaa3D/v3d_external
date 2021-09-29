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

#include "teem/gage.h"

#define PROBE "probePolynomial"

/*
** Tests:
**
*/

/* the hope is that without too much work it would be possible to
   increase this to 4th-order polynomials and 4th-order derivatives */
#define POWER_MAX 3

/*
** polyeval takes a polynomial defined by coef --
** coef[i][j][k] is the coefficient for (x^i)*(y^j)*(z^k) --
** and evaluates at position pos[] the dx-th derivative along X,
** the dy-th derivative along Y, and the dz-th derivative along Z,
** dx = dy = dz = 0 to evaluate the polynomial itself
*/
static double
polyeval(double coef[POWER_MAX+1][POWER_MAX+1][POWER_MAX+1],
         unsigned int dx, unsigned int dy, unsigned int dz,
         const double pos[3]) {
  unsigned int xi, yi, zi;
  double tmp, ret,
    xp[POWER_MAX+1], yp[POWER_MAX+1], zp[POWER_MAX+1],
    dc[POWER_MAX+1][POWER_MAX+1] = {
    {1.0, 1.0, 1.0, 1.0},  /* how coeffs are scaled for 0th derivative */
    {0.0, 1.0, 2.0, 3.0},  /* how coeffs are scaled for 1st derivative */
    {0.0, 0.0, 2.0, 6.0},  /* how coeffs are scaled for 2nd derivative */
    {0.0, 0.0, 0.0, 6.0}}; /* how coeffs are scaled for 3rd derivative */

  tmp = 1.0;
  for (xi=0; xi<=POWER_MAX; xi++) {
    xp[xi] = tmp;
    tmp *= pos[0];
  }
  tmp = 1.0;
  for (yi=0; yi<=POWER_MAX; yi++) {
    yp[yi] = tmp;
    tmp *= pos[1];
  }
  tmp = 1.0;
  for (zi=0; zi<=POWER_MAX; zi++) {
    zp[zi] = tmp;
    tmp *= pos[2];
  }

  ret = 0.0;
  for (xi=dx; xi<=POWER_MAX; xi++) {
    for (yi=dy; yi<=POWER_MAX; yi++) {
      for (zi=dz; zi<=POWER_MAX; zi++) {
        ret += (coef[xi][yi][zi]*xp[xi-dx]*yp[yi-dy]*zp[zi-dz]
                *dc[dx][xi]*dc[dy][yi]*dc[dz][zi]);
      }
    }
  }

  return ret;
}

static void
randvec(double vec[NRRD_SPACE_DIM_MAX], double lenexp, double lensig,
        airRandMTState *rng) {
  double tmp, clen[2], len;

  airNormalRand_r(vec + 0, vec + 1, rng);
  airNormalRand_r(vec + 2, NULL, rng);
  ELL_3V_NORM(vec, vec, tmp);
  airNormalRand_r(clen + 0, clen + 1, rng);
  /* rician noise, actually */
  clen[0] = lenexp + lensig*clen[0];
  clen[1] = lensig*clen[1];
  len = ELL_2V_LEN(clen);
  ELL_3V_SCALE(vec, len, vec);
}

/*
** makeVolume allocates inside nin a new randomly oriented volume of
** size sx-by-sy-sz; the spaceDirection vectors are random but enforced
** to not be too parallel
*/
static gageContext *
makeVolume(Nrrd *nin, unsigned int sx, unsigned int sy, unsigned int sz,
           double coef[POWER_MAX+1][POWER_MAX+1][POWER_MAX+1],
           airRandMTState *rng) {
  static const char me[]="makeVolume";
  double spcOrig[NRRD_SPACE_DIM_MAX], spcVec[3][NRRD_SPACE_DIM_MAX],
    angle10, angle20, angle21, ooff[3], aperm=0.6, lexp=0.1, lstdv=0.04,
    kparm[NRRD_KERNEL_PARMS_NUM];
  gageContext *gctx;
  gagePerVolume *gpvl;
  unsigned int xi, yi, zi;
  airArray *submop;
  int EE;

  submop = airMopNew();
  randvec(spcVec[0], lexp, lstdv, rng);
  do {
    randvec(spcVec[1], lexp, lstdv, rng);
    angle10 = ell_3v_angle_d(spcVec[1], spcVec[0]);
  } while (!( AIR_IN_OP(AIR_PI*(1-aperm)/2, angle10, AIR_PI*(1+aperm)/2) ));
  do {
    randvec(spcVec[2], lexp, lstdv, rng);
    angle20 = ell_3v_angle_d(spcVec[2], spcVec[0]);
    angle21 = ell_3v_angle_d(spcVec[2], spcVec[1]);
  } while (!( AIR_IN_OP(AIR_PI*(1-aperm)/2, angle20, AIR_PI*(1+aperm)/2) &&
              AIR_IN_OP(AIR_PI*(1-aperm)/2, angle21, AIR_PI*(1+aperm)/2) ));

  /* center volume near origin */
  airNormalRand_r(ooff + 0, ooff + 1, rng);
  airNormalRand_r(ooff + 2, NULL, rng);
  ELL_3V_SET(spcOrig, 0.0, 0.0, 0.0);
  ELL_3V_SCALE_INCR(spcOrig, -AIR_CAST(double, sx)/2.0 + 2*ooff[0], spcVec[0]);
  ELL_3V_SCALE_INCR(spcOrig, -AIR_CAST(double, sy)/2.0 + 2*ooff[1], spcVec[1]);
  ELL_3V_SCALE_INCR(spcOrig, -AIR_CAST(double, sz)/2.0 + 2*ooff[1], spcVec[2]);

  /* allocate data */
  if (nrrdMaybeAlloc_va(nin, nrrdTypeDouble, 3,
                        AIR_CAST(size_t, sx),
                        AIR_CAST(size_t, sy),
                        AIR_CAST(size_t, sz))
      || nrrdSpaceSet(nin, nrrdSpaceRightAnteriorSuperior)
      || nrrdSpaceOriginSet(nin, spcOrig)) {
    biffMovef(PROBE, NRRD, "%s: trouble setting volume", me);
    airMopError(submop); return NULL;
  }
  nrrdAxisInfoSet_va(nin, nrrdAxisInfoSpaceDirection,
                     spcVec[0],
                     spcVec[1],
                     spcVec[2]);
  nrrdAxisInfoSet_va(nin, nrrdAxisInfoCenter,
                     nrrdCenterCell,
                     nrrdCenterCell,
                     nrrdCenterCell);

  /* set data */
  {
    double *ddata = AIR_CAST(double *, nin->data);
    for (zi=0; zi<sz; zi++) {
      double pos[3];
      for (yi=0; yi<sy; yi++) {
        for (xi=0; xi<sx; xi++) {
          ELL_3V_SCALE_ADD2(pos, 1.0, spcOrig, xi, spcVec[0]);
          ELL_3V_SCALE_ADD2(pos, 1.0, pos, yi, spcVec[1]);
          ELL_3V_SCALE_ADD2(pos, 1.0, pos, zi, spcVec[2]);
          ddata[xi + sx*(yi + sy*zi)] = polyeval(coef, 0, 0, 0, pos);
        }
      }
    }
  }

  /* create context, using the c4hexic kernel and its derivatives.  c4hexic
     is can reconstruct cubics without pre-filtering, and its analytical
     derivatives are readily available. tmf:n,3,4 can also reconstruct
     cubics but its not clear which other kernels are its derivatives */
  gctx = gageContextNew();
  airMopAdd(submop, gctx, (airMopper)gageContextNix, airMopOnError);
  gageParmSet(gctx, gageParmRenormalize, AIR_FALSE);
  gageParmSet(gctx, gageParmCheckIntegrals, AIR_TRUE);
  EE = 0;
  if (!EE) EE |= gageKernelSet(gctx, gageKernel00, nrrdKernelC4Hexic, kparm);
  if (!EE) EE |= gageKernelSet(gctx, gageKernel11, nrrdKernelC4HexicD, kparm);
  if (!EE) EE |= gageKernelSet(gctx, gageKernel22, nrrdKernelC4HexicDD, kparm);
  if (!EE) EE |= !(gpvl = gagePerVolumeNew(gctx, nin, gageKindScl));
  if (!EE) EE |= gagePerVolumeAttach(gctx, gpvl);
  if (!EE) EE |= gageQueryItemOn(gctx, gpvl, gageSclValue);
  if (!EE) EE |= gageQueryItemOn(gctx, gpvl, gageSclGradVec);
  if (!EE) EE |= gageQueryItemOn(gctx, gpvl, gageSclHessian);
  if (!EE) EE |= gageUpdate(gctx);
  if (EE) {
    biffMovef(PROBE, GAGE, "%s: trouble setting up gage", me);
    airMopError(submop); return NULL;
  }

  airMopOkay(submop);
  return gctx;
}

int
main(int argc, const char **argv) {
  airArray *mop;
  char *err;

  airRandMTState *rng;
  Nrrd *nin;
  unsigned int xi, yi, zi, runNum, runIdx;
  double coef[POWER_MAX+1][POWER_MAX+1][POWER_MAX+1], pos[3], epsilon;
  const double *vmeas, *gmeas, *hmeas;
  gageContext *gctx;

  AIR_UNUSED(argc);
  mop = airMopNew();

  rng = airRandMTStateNew(429);
  airMopAdd(mop, rng, (airMopper)airRandMTStateNix, airMopAlways);

  nin = nrrdNew();
  airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);
  epsilon = 1e-8;
  runNum = 30;

  for (runIdx=0; runIdx<runNum; runIdx++) {
    unsigned int sx, sy, sz, probeNum;
    double avgDiff;
    airArray *submop;

    submop = airMopNew();
    /* set random coefficients in polynomial */
    for (xi=0; xi<=POWER_MAX; xi++) {
      for (yi=0; yi<=POWER_MAX; yi++) {
        for (zi=0; zi<=POWER_MAX; zi++) {
          if (xi + yi + zi > POWER_MAX) {
            coef[xi][yi][zi] = 0.0;
          } else {
            airNormalRand_r(&(coef[xi][yi][zi]), NULL, rng);
          }
        }
      }
    }

    sx = 20 + airRandInt_r(rng, 120);
    sy = 20 + airRandInt_r(rng, 120);
    sz = 20 + airRandInt_r(rng, 120);
    fprintf(stderr, "%u: %u %u %u: ", runIdx, sx, sy, sz); fflush(stderr);
    if (!(gctx = makeVolume(nin, sx, sy, sz, coef, rng))) {
      airMopAdd(mop, err = biffGetDone(PROBE), airFree, airMopAlways);
      fprintf(stderr, "trouble creating volume:\n%s", err);
      airMopError(submop); airMopError(mop); return 1;
    }
    airMopAdd(submop, gctx, (airMopper)gageContextNix, airMopAlways);
    vmeas = gageAnswerPointer(gctx, gctx->pvl[0], gageSclValue);
    gmeas = gageAnswerPointer(gctx, gctx->pvl[0], gageSclGradVec);
    hmeas = gageAnswerPointer(gctx, gctx->pvl[0], gageSclHessian);
    ELL_3V_SET(pos, 0, 0, 0);
    gageProbeSpace(gctx, pos[0], pos[1], pos[2],
                   AIR_FALSE /* indexSpace */,
                   AIR_TRUE /* clamp */);
    probeNum = 0;
    avgDiff = 0.0;
    /* take a random walk starting at (0,0,0), making sure that at the
       visited positions the gage-measured values, gradients, and
       hessians are the same as the analytical ones */
    do {
      double vanal, ganal[3], hanal[9], vdiff, gdiff[3], hdiff[9],
        step[3], stepSize = 0.2;
      probeNum++;
      vanal = polyeval(coef, 0, 0, 0, pos);
      ganal[0] = polyeval(coef, 1, 0, 0, pos);
      ganal[1] = polyeval(coef, 0, 1, 0, pos);
      ganal[2] = polyeval(coef, 0, 0, 1, pos);
      hanal[0] = polyeval(coef, 2, 0, 0, pos);
      hanal[1] = polyeval(coef, 1, 1, 0, pos);
      hanal[2] = polyeval(coef, 1, 0, 1, pos);
      hanal[3] = hanal[1];
      hanal[4] = polyeval(coef, 0, 2, 0, pos);
      hanal[5] = polyeval(coef, 0, 1, 1, pos);
      hanal[6] = hanal[2];
      hanal[7] = hanal[5];
      hanal[8] = polyeval(coef, 0, 0, 2, pos);
      vdiff = fabs(vmeas[0] - vanal);
      ELL_3V_SUB(gdiff, gmeas, ganal);
      ELL_3M_SUB(hdiff, hmeas, hanal);
      if (vdiff > epsilon ||
          ELL_3V_LEN(gdiff) > epsilon ||
          ELL_3M_FROB(hdiff) > epsilon) {
        fprintf(stderr, "at (%g,%g,%g) one diff %.17g %.17g %.17g "
                "> eps %.17g\n",
                pos[0], pos[1], pos[2], vdiff,
                ELL_3V_LEN(gdiff), ELL_3M_FROB(hdiff), epsilon);
        airMopError(submop); airMopError(mop); return 1;
      }
      avgDiff += vdiff + ELL_3V_LEN(gdiff) + ELL_3M_FROB(hdiff);
      airNormalRand_r(step + 0, step + 1, rng);
      airNormalRand_r(step + 2, NULL, rng);
      ELL_3V_SCALE_INCR(pos, stepSize, step);
      gageProbeSpace(gctx, pos[0], pos[1], pos[2],
                     AIR_FALSE /* indexSpace */,
                     AIR_TRUE /* clamp */);
    } while (!gctx->edgeFrac);
    avgDiff /= probeNum;
    fprintf(stderr, "%u (%.17g)\n", probeNum, avgDiff);
    airMopOkay(submop);
  }

  airMopOkay(mop);
  return 0;
}
