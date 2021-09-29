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

#include <math.h>
#include "../nrrd.h"

char *genvolInfo = ("generates test volumes.  Not very flexible as long as "
                    "the \"funk\" library doesn't exist");

double
rho(double r) {
  return cos(2*AIR_PI*6.0*cos(AIR_PI*r/2));
}

double
genvolFunc(double x, double y, double z) {
  double mask, phi, R2, R3, Rbig, Rlit, sig0=0.17, sig1=0.04, a, b, w, ret;

  /* three bladed thing */
  R3 = sqrt(x*x + y*y + z*z);
  mask = AIR_AFFINE(-1.0, erf((R3 - 0.75)*15), 1.0, 1.0, 0.0);
  R2 = sqrt(x*x + y*y);
  phi = atan2(y+0.001,x+0.001) + z*1.2;
  w = pow((1+cos(3*phi))/2, R2*R2*90);
  return w*mask;

#if 0
  /* ridge surface is a Mobius aka Moebius strip */
  Rbig = sqrt(x*x + y*y);
  Rlit = sqrt(z*z + (Rbig-0.5)*(Rbig-0.5));
  phi = atan2(Rbig-0.5, z) - atan2(x, y)/2;
  a = Rlit*cos(phi);
  b = Rlit*sin(phi);
  /*
    ret = airGaussian(a, 0, sig0)*airGaussian(b, 0, sig1);
  */
  a = (a > sig0
       ? a - sig0
       : (a < -sig0
          ? a + sig0
          : 0));
  ret = airGaussian(a, 0, sig1)*airGaussian(b, 0, sig1);
  return ret;
#endif

  /*
  double A, B;

  A = 1;
  B = 1;
  */
  /* marschner-lobb, the real thing
  return ((1 - sin(AIR_PI*z/2))
          + 0.25*(1 + rho(sqrt(x*x + y*y))))/(2*(1 + 0.25));
           */
  /* marschner-lobb, linear variation in Z
  return (1 - (AIR_PI*z + 3)/5
          + 0.25*(1 + rho(sqrt(x*x + y*y)))/(2*(1 + 0.25)));
  */
  /* cone
  return z - 2*sqrt(x*x + y*y) + 0.5;
  */
  /* pin-cushion
  return x*x + y*y + z*z - x*x*x*x - y*y*y*y - z*z*z*z;
  */
  /* quadratic surface (moved to quadvol.c)
  return A*x*x + B*y*y - z;
  */
  /* torus
  A = sqrt(x*x + y*y) - 0.5;
  return 2 - sqrt(A*A + z*z);
  */
  /* return sqrt(x*x + y*y + z*z); */
  /* return sqrt(x*x + y*y); */
}

int
main(int argc, const char *argv[]) {
  const char *me;
  char *err, *out;
  int size[3], xi, yi, zi;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;
  double min[3], max[3], x, y, z, *data;
  Nrrd *nout;

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "s", "sx sy sz", airTypeInt, 3, 3, size, "128 128 128",
             "dimensions of output volume");
  hestOptAdd(&hopt, "min", "x y z", airTypeDouble, 3, 3, min, "-1 -1 -1",
             "lower bounding corner of volume");
  hestOptAdd(&hopt, "max", "x y z", airTypeDouble, 3, 3, max, "1 1 1",
             "upper bounding corner of volume");
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &out, "-",
             "file to write output nrrd to");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, genvolInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdAlloc_va(nout, nrrdTypeDouble, 3,
                   AIR_CAST(size_t, size[0]),
                   AIR_CAST(size_t, size[1]),
                   AIR_CAST(size_t, size[2]))) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: problem allocating volume:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  data = (double *)nout->data;
  for (zi=0; zi<size[2]; zi++) {
    z = AIR_AFFINE(0, zi, size[2]-1, min[2], max[2]);
    for (yi=0; yi<size[1]; yi++) {
      y = AIR_AFFINE(0, yi, size[1]-1, min[1], max[1]);
      for (xi=0; xi<size[0]; xi++) {
        x = AIR_AFFINE(0, xi, size[0]-1, min[0], max[0]);
        *data = genvolFunc(x,y,z);
        data += 1;
      }
    }
  }

  nrrdAxisInfoSet_va(nout, nrrdAxisInfoCenter,
                     nrrdCenterNode, nrrdCenterNode, nrrdCenterNode);
#if 0
  nrrdAxisInfoSet_va(nout, nrrdAxisInfoMin, min[0], min[1], min[2]);
  nrrdAxisInfoSet_va(nout, nrrdAxisInfoMax, max[0], max[1], max[2]);
  nrrdAxisInfoSpacingSet(nout, 0);
  nrrdAxisInfoSpacingSet(nout, 1);
  nrrdAxisInfoSpacingSet(nout, 2);
#else
  /* impatient, not using API, bad! */
#define ELL_3V_SET(v, a, b, c) \
  ((v)[0] = (a), (v)[1] = (b), (v)[2] = (c))
  nout->space = nrrdSpaceLeftPosteriorSuperior;
  nout->spaceDim = 3;
  ELL_3V_SET(nout->spaceOrigin, 0, 0, 0);
  ELL_3V_SET(nout->axis[0].spaceDirection, 1, 0, 0);
  ELL_3V_SET(nout->axis[1].spaceDirection, 0, 1, 0);
  ELL_3V_SET(nout->axis[2].spaceDirection, 0, 0, 1);
#endif

  if (nrrdSave(out, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: problem saving output:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  exit(0);
}
