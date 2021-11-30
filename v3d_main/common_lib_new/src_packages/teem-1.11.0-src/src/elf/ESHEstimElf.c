/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2011, 2010, 2009, 2008 Thomas Schultz

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

#include "elf.h"

/* Routines for estimating even-order spherical harmonics coefficients */

/* elfCart2Thetaphi:
 *
 * Helper function to transform ct 3D Cartesian coordinates into
 * theta (polar angle from positive z) and phi (azimuth from positive x)
 * Input vectors should be non-zero, but do not need to be normalized
 * thetaphi needs to be pre-allocated to length 2*ct
 */
void elfCart2Thetaphi_f(float *thetaphi, const float *dirs, unsigned int ct)
{
  unsigned int i;
  for (i=0; i<ct; i++) {
    float r=AIR_CAST(float, ELL_3V_LEN(dirs+3*i));
    float z=dirs[3*i+2]/r;
    if (z>1) thetaphi[2*i]=0;
    else if (z<-1) thetaphi[2*i]=AIR_CAST(float, AIR_PI);
    else thetaphi[2*i]=AIR_CAST(float, acos(z));
    thetaphi[2*i+1]=AIR_CAST(float, atan2(dirs[3*i+1],dirs[3*i]));
  }
}

/* elfESHEstimMatrix:
 *
 * Computes a matrix T that can be used to transform a measurement vector v
 * of ct values on the sphere, at positions given by thetaphi, into a vector
 * c of ESH coefficients of the desired order (c=Tv) such that the ESH
 * series approximates the given values in a least squares sense.
 *
 * T needs to be pre-allocated to length len(c)*ct
 * If H is non-NULL, it should be pre-allocated to length ct*ct
 *   In this case, the "hat" matrix will be written to H, which maps the
 *   measurement vector v to the corresponding model predictions
 * If lambda>0, Laplace-Beltrami regularization is employed
 * If w is non-NULL, it is assumed to be a weight vector with len(w)=ct
 *
 * Returns 0 on success
 *         1 if order is unsupported
 *         2 if len(v)<len(c) (i.e., system is underdetermined)
 */
int elfESHEstimMatrix_f(float *T, float *H, unsigned int order,
                        const float *thetaphi,
                        unsigned int ct, float lambda, float *w)
{
  double *B, *M, *Minv;
  unsigned int N, i, j, k;
  Nrrd *nmat, *ninv;
  if (order>tijk_max_esh_order || order%2!=0)
    return 1;
  N=tijk_esh_len[order/2];
  if (ct<N) return 2;

  /* intermediate computations are done in double precision */
  B = (double*) malloc(sizeof(double)*N*ct);
  M = (double*) malloc(sizeof(double)*N*N);

  /* build the transformation matrix */
  /* B has row major format -> all SHs that belong to the same
   *                           thetaphi are stored sequentially */
  for (i=0; i<ct; i++) {
    tijk_eval_esh_basis_d(B+N*i, order, thetaphi[2*i], thetaphi[2*i+1]);
  }
  if (w!=NULL) { /* weighted fit */
    for (i=0; i<N; ++i) /* row index */
      for (j=0; j<N; ++j) { /* column index */
        M[N*i+j]=0.0;
        for (k=0; k<ct; ++k) {
          M[N*i+j]+=B[N*k+i]*B[N*k+j]*w[k];
        }
      }
  } else { /* unweighted fit */
    for (i=0; i<N; ++i) /* row index */
      for (j=0; j<N; ++j) { /* column index */
        M[N*i+j]=0.0;
        for (k=0; k<ct; ++k) {
          M[N*i+j]+=B[N*k+i]*B[N*k+j];
        }
      }
  }
  /* if desired, perform Laplace-Beltrami regularization */
  if (lambda>0) {
    unsigned int idx=0, o;
    for (o=0; o<=order; o+=2) { /* order */
      while (idx<tijk_esh_len[o/2]) {
        M[N*idx+idx]+=lambda*o*o*(o+1)*(o+1);
        idx++;
      }
    }
  }
  /* invert what we have up to now */
  nmat = nrrdNew();
  ninv = nrrdNew();
  nmat->dim=2;
  nmat->type=nrrdTypeDouble;
  nmat->axis[0].size=nmat->axis[1].size=N;
  nmat->data=M;
  ell_Nm_inv(ninv, nmat);
  Minv = (double*) ninv->data;

  /* create final transformation matrix */
  if (w!=NULL) { /* weighted */
    for (i=0; i<N; ++i) /* row index */
      for (j=0; j<ct; ++j) {
        unsigned int idx=ct*i+j;
        T[idx]=0.0;
        for (k=0; k<N; ++k) {
          T[idx]+=AIR_CAST(float, Minv[N*i+k]*B[N*j+k]*w[j]);
        }
      }
  } else { /* unweighted */
    for (i=0; i<N; ++i) /* row index */
      for (j=0; j<ct; ++j) {
        unsigned int idx=ct*i+j;
        T[idx]=0.0;
        for (k=0; k<N; ++k) {
          T[idx]+=AIR_CAST(float, Minv[N*i+k]*B[N*j+k]);
        }
      }
  }
  nmat = nrrdNix(nmat);
  ninv = nrrdNuke(ninv);

  if (H!=NULL) { /* H = BT */
    for (i=0; i<ct; i++) /* row index */
      for (j=0; j<ct; j++) {
        unsigned int idx=ct*i+j;
        H[idx]=0.0;
        for (k=0; k<N; k++) { /* sum over all SH coeffs */
          H[idx]+=AIR_CAST(float, B[N*i+k]*T[k*ct+j]);
        }
      }
  }

  free(M);
  free(B);
  return 0;
}
