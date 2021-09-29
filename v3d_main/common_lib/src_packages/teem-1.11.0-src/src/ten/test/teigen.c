/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2011, 2010, 2009, University of Chicago
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


#include "../ten.h"

char *info = ("tests tenEigensolve_d and new stand-alone function.");

#define ROOT_TRIPLE 2           /* ell_cubic_root_triple */
#define ROOT_SINGLE_DOUBLE 3    /* ell_cubic_root_single_double */
#define ROOT_THREE 4            /* ell_cubic_root_three */

#define ABS(a) (((a) > 0.0f ? (a) : -(a)))
#define VEC_SET(v, a, b, c) \
  ((v)[0] = (a), (v)[1] = (b), (v)[2] = (c))
#define VEC_DOT(v1, v2) \
  ((v1)[0]*(v2)[0] + (v1)[1]*(v2)[1] + (v1)[2]*(v2)[2])
#define VEC_CROSS(v3, v1, v2) \
  ((v3)[0] = (v1)[1]*(v2)[2] - (v1)[2]*(v2)[1], \
   (v3)[1] = (v1)[2]*(v2)[0] - (v1)[0]*(v2)[2], \
   (v3)[2] = (v1)[0]*(v2)[1] - (v1)[1]*(v2)[0])
#define VEC_ADD(v1, v2)  \
  ((v1)[0] += (v2)[0], \
   (v1)[1] += (v2)[1], \
   (v1)[2] += (v2)[2])
#define VEC_SUB(v1, v2)  \
  ((v1)[0] -= (v2)[0], \
   (v1)[1] -= (v2)[1], \
   (v1)[2] -= (v2)[2])
#define VEC_SCL(v1, s)  \
  ((v1)[0] *= (s),      \
   (v1)[1] *= (s),      \
   (v1)[2] *= (s))
#define VEC_LEN(v) (sqrt(VEC_DOT(v,v)))
#define VEC_NORM(v, len) ((len) = VEC_LEN(v), VEC_SCL(v, 1.0/len))
#define VEC_SCL_SUB(v1, s, v2) \
  ((v1)[0] -= (s)*(v2)[0],      \
   (v1)[1] -= (s)*(v2)[1], \
   (v1)[2] -= (s)*(v2)[2])
#define VEC_COPY(v1, v2)  \
  ((v1)[0] = (v2)[0], \
   (v1)[1] = (v2)[1], \
   (v1)[2] = (v2)[2])

/*
** All the three given vectors span only a 2D space, and this finds
** the normal to that plane.  Simply sums up all the pair-wise
** cross-products to get a good estimate.  Trick is getting the cross
** products to line up before summing.
*/
void
nullspace1(double ret[3],
           const double r0[3], const double r1[3], const double r2[3]) {
  double crs[3];

  /* ret = r0 x r1 */
  VEC_CROSS(ret, r0, r1);
  /* crs = r1 x r2 */
  VEC_CROSS(crs, r1, r2);
  /* ret += crs or ret -= crs; whichever makes ret longer */
  if (VEC_DOT(ret, crs) > 0) {
    VEC_ADD(ret, crs);
  } else {
    VEC_SUB(ret, crs);
  }
  /* crs = r0 x r2 */
  VEC_CROSS(crs, r0, r2);
  /* ret += crs or ret -= crs; whichever makes ret longer */
  if (VEC_DOT(ret, crs) > 0) {
    VEC_ADD(ret, crs);
  } else {
    VEC_SUB(ret, crs);
  }

  return;
}

/*
** All vectors are in the same 1D space, we have to find two
** mutually vectors perpendicular to that span
*/
void
nullspace2(double reta[3], double retb[3],
           const double r0[3], const double r1[3], const double r2[3]) {
  double sqr[3], sum[3];
  int idx;

  VEC_COPY(sum, r0);
  if (VEC_DOT(sum, r1) > 0) {
    VEC_ADD(sum, r1);
  } else {
    VEC_SUB(sum, r1);
  }
  if (VEC_DOT(sum, r2) > 0) {
    VEC_ADD(sum, r2);
  } else {
    VEC_SUB(sum, r2);
  }
  /* find largest component, to get most stable expression for a
     perpendicular vector */
  sqr[0] = sum[0]*sum[0];
  sqr[1] = sum[1]*sum[1];
  sqr[2] = sum[2]*sum[2];
  idx = 0;
  if (sqr[0] < sqr[1])
    idx = 1;
  if (sqr[idx] < sqr[2])
    idx = 2;
  /* reta will be perpendicular to sum */
  if (0 == idx) {
    VEC_SET(reta, sum[1] - sum[2], -sum[0], sum[0]);
  } else if (1 == idx) {
    VEC_SET(reta, -sum[1], sum[0] - sum[2], sum[1]);
  } else {
    VEC_SET(reta, -sum[2], sum[2], sum[0] - sum[1]);
  }
  /* and now retb will be perpendicular to both reta and sum */
  VEC_CROSS(retb, reta, sum);
  return;
}

/*
** Eigensolver for symmetric 3x3 matrix:
**
**  M00  M01  M02
**  M01  M11  M12
**  M02  M12  M22
**
** Must be passed eval[3] vector, and will compute eigenvalues
** only if evec[9] is non-NULL.  Computed eigenvectors are at evec+0,
** evec+3, and evec+6.
**
** Return value indicates something about the eigenvalue solution to
** the cubic characteristic equation; see ROOT_ #defines above
**
** Relies on the ABS and VEC_* macros above, as well as math functions
** atan2(), sin(), cos(), sqrt(), and airCbrt(), defined as:

  double
  airCbrt(double v) {
  #if defined(_WIN32) || defined(__STRICT_ANSI__)
    return (v < 0.0 ? -pow(-v,1.0/3.0) : pow(v,1.0/3.0));
  #else
    return cbrt(v);
  #endif
  }

**
** HEY: the numerical precision issues here are very subtle, and
** merit some more scrutiny.  With evals (1.000001, 1, 1), for example,
** whether it comes back as a single/double root, vs three distinct roots,
** is determines by the comparison between "D" and "epsilon", and the
** setting of epsilon seems pretty arbitrary at this point...
**
*/
int
evals(double eval[3],
      const double _M00, const double _M01, const double _M02,
      const double _M11, const double _M12,
      const double _M22) {

#include "teigen-evals-A.c"

#include "teigen-evals-B.c"

  return roots;
}

int
evals_evecs(double eval[3], double evec[9],
            const double _M00, const double _M01, const double _M02,
            const double _M11, const double _M12,
            const double _M22) {
  double r0[3], r1[3], r2[3], crs[3], len, dot;

  double mean, norm, rnorm, Q, R, QQQ, D, theta,
    M00, M01, M02, M11, M12, M22;
  double epsilon = 1.0E-12;
  int roots;

  /* copy the given matrix elements */
  M00 = _M00;
  M01 = _M01;
  M02 = _M02;
  M11 = _M11;
  M12 = _M12;
  M22 = _M22;

  /*
  ** subtract out the eigenvalue mean (will add back to evals later);
  ** helps with numerical stability
  */
  mean = (M00 + M11 + M22)/3.0;
  M00 -= mean;
  M11 -= mean;
  M22 -= mean;

  /*
  ** divide out L2 norm of eigenvalues (will multiply back later);
  ** this too seems to help with stability
  */
  norm = sqrt(M00*M00 + 2*M01*M01 + 2*M02*M02 +
              M11*M11 + 2*M12*M12 +
              M22*M22);
  rnorm = norm ? 1.0/norm : 1.0;
  M00 *= rnorm;
  M01 *= rnorm;
  M02 *= rnorm;
  M11 *= rnorm;
  M12 *= rnorm;
  M22 *= rnorm;

  /* this code is a mix of prior Teem code and ideas from Eberly's
     "Eigensystems for 3 x 3 Symmetric Matrices (Revisited)" */
  Q = (M01*M01 + M02*M02 + M12*M12 - M00*M11 - M00*M22 - M11*M22)/3.0;
  QQQ = Q*Q*Q;
  R = (M00*M11*M22 + M02*(2*M01*M12 - M02*M11)
       - M00*M12*M12 - M01*M01*M22)/2.0;
  D = QQQ - R*R;
  if (D > epsilon) {
    /* three distinct roots- this is the most common case */
    double mm, ss, cc;
    theta = atan2(sqrt(D), R)/3.0;
    mm = sqrt(Q);
    ss = sin(theta);
    cc = cos(theta);
    eval[0] = 2*mm*cc;
    eval[1] = mm*(-cc + sqrt(3.0)*ss);
    eval[2] = mm*(-cc - sqrt(3.0)*ss);
    roots = ROOT_THREE;
    /* else D is near enough to zero */
  } else if (R < -epsilon || epsilon < R) {
    double U;
    /* one double root and one single root */
    U = airCbrt(R); /* cube root function */
    if (U > 0) {
      eval[0] = 2*U;
      eval[1] = -U;
      eval[2] = -U;
    } else {
      eval[0] = -U;
      eval[1] = -U;
      eval[2] = 2*U;
    }
    roots = ROOT_SINGLE_DOUBLE;
  } else {
    /* a triple root! */
    eval[0] = eval[1] = eval[2] = 0.0;
    roots = ROOT_TRIPLE;
  }

  /* r0, r1, r2 are the vectors we manipulate to
     find the nullspaces of M - lambda*I */
  VEC_SET(r0, 0.0, M01, M02);
  VEC_SET(r1, M01, 0.0, M12);
  VEC_SET(r2, M02, M12, 0.0);
  if (ROOT_THREE == roots) {
    r0[0] = M00 - eval[0]; r1[1] = M11 - eval[0]; r2[2] = M22 - eval[0];
    nullspace1(evec+0, r0, r1, r2);
    r0[0] = M00 - eval[1]; r1[1] = M11 - eval[1]; r2[2] = M22 - eval[1];
    nullspace1(evec+3, r0, r1, r2);
    r0[0] = M00 - eval[2]; r1[1] = M11 - eval[2]; r2[2] = M22 - eval[2];
    nullspace1(evec+6, r0, r1, r2);
  } else if (ROOT_SINGLE_DOUBLE == roots) {
    if (eval[1] == eval[2]) {
      /* one big (eval[0]) , two small (eval[1,2]) */
      r0[0] = M00 - eval[0]; r1[1] = M11 - eval[0]; r2[2] = M22 - eval[0];
      nullspace1(evec+0, r0, r1, r2);
      r0[0] = M00 - eval[1]; r1[1] = M11 - eval[1]; r2[2] = M22 - eval[1];
      nullspace2(evec+3, evec+6, r0, r1, r2);
    }
    else {
      /* two big (eval[0,1]), one small (eval[2]) */
      r0[0] = M00 - eval[0]; r1[1] = M11 - eval[0]; r2[2] = M22 - eval[0];
      nullspace2(evec+0, evec+3, r0, r1, r2);
      r0[0] = M00 - eval[2]; r1[1] = M11 - eval[2]; r2[2] = M22 - eval[2];
      nullspace1(evec+6, r0, r1, r2);
    }
  } else {
    /* ROOT_TRIPLE == roots; use any basis for eigenvectors */
    VEC_SET(evec+0, 1, 0, 0);
    VEC_SET(evec+3, 0, 1, 0);
    VEC_SET(evec+6, 0, 0, 1);
  }
  /* we always make sure its really orthonormal; keeping fixed the
     eigenvector associated with the largest-magnitude eigenvalue */
  if (ABS(eval[0]) > ABS(eval[2])) {
    /* normalize evec+0 but don't move it */
    VEC_NORM(evec+0, len);
    dot = VEC_DOT(evec+0, evec+3); VEC_SCL_SUB(evec+3, dot, evec+0);
    VEC_NORM(evec+3, len);
    dot = VEC_DOT(evec+0, evec+6); VEC_SCL_SUB(evec+6, dot, evec+0);
    dot = VEC_DOT(evec+3, evec+6); VEC_SCL_SUB(evec+6, dot, evec+3);
    VEC_NORM(evec+6, len);
  } else {
    /* normalize evec+6 but don't move it */
    VEC_NORM(evec+6, len);
    dot = VEC_DOT(evec+6, evec+3); VEC_SCL_SUB(evec+3, dot, evec+6);
    VEC_NORM(evec+3, len);
    dot = VEC_DOT(evec+3, evec+0); VEC_SCL_SUB(evec+0, dot, evec+3);
    dot = VEC_DOT(evec+6, evec+0); VEC_SCL_SUB(evec+0, dot, evec+6);
    VEC_NORM(evec+0, len);
  }

  /* to be nice, make it right-handed */
  VEC_CROSS(crs, evec+0, evec+3);
  if (0 > VEC_DOT(crs, evec+6)) {
    VEC_SCL(evec+6, -1);
  }

  /* multiply back by eigenvalue L2 norm */
  eval[0] /= rnorm;
  eval[1] /= rnorm;
  eval[2] /= rnorm;

  /* add back in the eigenvalue mean */
  eval[0] += mean;
  eval[1] += mean;
  eval[2] += mean;

  return roots;
}


void
testeigen(double tt[7], double eval[3], double evec[9]) {
  double mat[9], dot[3], cross[3];
  unsigned int ii;

  TEN_T2M(mat, tt);
  printf("evals %g %g %g\n", eval[0], eval[1], eval[2]);
  printf("evec0 (%g) %g %g %g\n",
         ELL_3V_LEN(evec + 0), evec[0], evec[1], evec[2]);
  printf("evec1 (%g) %g %g %g\n",
         ELL_3V_LEN(evec + 3), evec[3], evec[4], evec[5]);
  printf("evec2 (%g) %g %g %g\n",
         ELL_3V_LEN(evec + 6), evec[6], evec[7], evec[8]);
  printf("Mv - lv: (len) X Y Z (should be ~zeros)\n");
  for (ii=0; ii<3; ii++) {
    double uu[3], vv[3], dd[3];
    ELL_3MV_MUL(uu, mat, evec + 3*ii);
    ELL_3V_SCALE(vv, eval[ii], evec + 3*ii);
    ELL_3V_SUB(dd, uu, vv);
    printf("%d: (%g) %g %g %g\n", ii, ELL_3V_LEN(dd), dd[0], dd[1], dd[2]);
  }
  dot[0] = ELL_3V_DOT(evec + 0, evec + 3);
  dot[1] = ELL_3V_DOT(evec + 0, evec + 6);
  dot[2] = ELL_3V_DOT(evec + 3, evec + 6);
  printf("pairwise dots: (%g) %g %g %g\n",
         ELL_3V_LEN(dot), dot[0], dot[1], dot[2]);
  ELL_3V_CROSS(cross, evec+0, evec+3);
  printf("right-handed: %g\n", ELL_3V_DOT(evec+6, cross));
  return;
}

int
main(int argc, const char *argv[]) {
  const char *me;
  hestOpt *hopt=NULL;
  airArray *mop;

  double _tt[6], tt[7], ss, pp[3], qq[4], rot[9], mat1[9], mat2[9], tmp,
    evalA[3], evecA[9], evalB[3], evecB[9];
  int roots;

  mop = airMopNew();
  me = argv[0];
  hestOptAdd(&hopt, NULL, "m00 m01 m02 m11 m12 m22",
             airTypeDouble, 6, 6, _tt, NULL, "symmtric matrix coeffs");
  hestOptAdd(&hopt, "p", "vec", airTypeDouble, 3, 3, pp, "0 0 0",
             "rotation as P vector");
  hestOptAdd(&hopt, "s", "scl", airTypeDouble, 1, 1, &ss, "1.0",
             "scaling");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  ELL_6V_COPY(tt + 1, _tt);
  tt[0] = 1.0;
  TEN_T_SCALE(tt, ss, tt);

  ELL_4V_SET(qq, 1, pp[0], pp[1], pp[2]);
  ELL_4V_NORM(qq, qq, tmp);
  ell_q_to_3m_d(rot, qq);
  printf("%s: rot\n", me);
  printf("  %g %g %g\n", rot[0], rot[1], rot[2]);
  printf("  %g %g %g\n", rot[3], rot[4], rot[5]);
  printf("  %g %g %g\n", rot[6], rot[7], rot[8]);

  TEN_T2M(mat1, tt);
  ell_3m_mul_d(mat2, rot, mat1);
  ELL_3M_TRANSPOSE_IP(rot, tmp);
  ell_3m_mul_d(mat1, mat2, rot);
  TEN_M2T(tt, mat1);

  printf("input matrix = \n %g %g %g\n %g %g\n %g\n",
          tt[1], tt[2], tt[3], tt[4], tt[5], tt[6]);

  printf("================== tenEigensolve_d ==================\n");
  roots = tenEigensolve_d(evalA, evecA, tt);
  printf("%s roots\n", airEnumStr(ell_cubic_root, roots));
  testeigen(tt, evalA, evecA);

  printf("================== new eigensolve ==================\n");
  roots = evals(evalB, tt[1], tt[2], tt[3], tt[4], tt[5], tt[6]);
  printf("%s roots: %g %g %g\n", airEnumStr(ell_cubic_root, roots),
         evalB[0], evalB[1], evalB[2]);
  roots = evals_evecs(evalB, evecB,
                      tt[1], tt[2], tt[3], tt[4], tt[5], tt[6]);
  printf("%s roots\n", airEnumStr(ell_cubic_root, roots));
  testeigen(tt, evalB, evecB);

  airMopOkay(mop);
  return 0;
}
