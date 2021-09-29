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


#include "ell.h"

int
ell_Nm_check(Nrrd *mat, int doNrrdCheck) {
  static const char me[]="ell_Nm_check";

  if (doNrrdCheck) {
    if (nrrdCheck(mat)) {
      biffMovef(ELL, NRRD, "%s: basic nrrd validity check failed", me);
      return 1;
    }
  } else {
    if (!mat) {
      biffAddf(ELL, "%s: got NULL pointer", me);
      return 1;
    }
  }
  if (!( 2 == mat->dim )) {
    biffAddf(ELL, "%s: nrrd must be 2-D (not %d-D)", me, mat->dim);
    return 1;
  }
  if (!( nrrdTypeDouble == mat->type )) {
    biffAddf(ELL, "%s: nrrd must be type %s (not %s)", me,
             airEnumStr(nrrdType, nrrdTypeDouble),
             airEnumStr(nrrdType, mat->type));
    return 1;
  }

  return 0;
}

/*
******** ell_Nm_tran
**
**     M             N
** N [trn]  <--  M [mat]
*/
int
ell_Nm_tran(Nrrd *ntrn, Nrrd *nmat) {
  static const char me[]="ell_Nm_tran";
  double *mat, *trn;
  size_t MM, NN, mm, nn;

  if (!( ntrn && !ell_Nm_check(nmat, AIR_FALSE) )) {
    biffAddf(ELL, "%s: NULL or invalid args", me);
    return 1;
  }
  if (ntrn == nmat) {
    biffAddf(ELL, "%s: sorry, can't work in-place yet", me);
    return 1;
  }
  /*
  if (nrrdAxesSwap(ntrn, nmat, 0, 1)) {
    biffMovef(ELL, NRRD, "%s: trouble", me);
    return 1;
    }
  */
  NN = nmat->axis[0].size;
  MM = nmat->axis[1].size;
  if (nrrdMaybeAlloc_va(ntrn, nrrdTypeDouble, 2,
                        MM, NN)) {
    biffMovef(ELL, NRRD, "%s: trouble", me);
    return 1;
  }
  mat = AIR_CAST(double *, nmat->data);
  trn = AIR_CAST(double *, ntrn->data);
  for (nn=0; nn<NN; nn++) {
    for (mm=0; mm<MM; mm++) {
      trn[mm + MM*nn] = mat[nn + NN*mm];
    }
  }

  return 0;
}

/*
******** ell_Nm_mul
**
** Currently, only useful for matrix-matrix multiplication
**
** matrix-matrix:      M       N
**                  L [A] . M [B]
*/
int
ell_Nm_mul(Nrrd *nAB, Nrrd *nA, Nrrd *nB) {
  static const char me[]="ell_Nm_mul";
  double *A, *B, *AB, tmp;
  size_t LL, MM, NN, ll, mm, nn;
  char stmp[4][AIR_STRLEN_SMALL];

  if (!( nAB && !ell_Nm_check(nA, AIR_FALSE)
         && !ell_Nm_check(nB, AIR_FALSE) )) {
    biffAddf(ELL, "%s: NULL or invalid args", me);
    return 1;
  }
  if (nAB == nA || nAB == nB) {
    biffAddf(ELL, "%s: can't do in-place multiplication", me);
    return 1;
  }
  LL = nA->axis[1].size;
  MM = nA->axis[0].size;
  NN = nB->axis[0].size;
  if (MM != nB->axis[1].size) {
    biffAddf(ELL, "%s: size mismatch: %s-by-%s times %s-by-%s", me,
             airSprintSize_t(stmp[0], LL),
             airSprintSize_t(stmp[1], MM),
             airSprintSize_t(stmp[2], nB->axis[1].size),
             airSprintSize_t(stmp[3], NN));
    return 1;
  }
  if (nrrdMaybeAlloc_va(nAB, nrrdTypeDouble, 2,
                        NN, LL)) {
    biffMovef(ELL, NRRD, "%s: trouble", me);
    return 1;
  }
  A = (double*)(nA->data);
  B = (double*)(nB->data);
  AB = (double*)(nAB->data);
  for (ll=0; ll<LL; ll++) {
    for (nn=0; nn<NN; nn++) {
      tmp = 0;
      for (mm=0; mm<MM; mm++) {
        tmp += A[mm + MM*ll]*B[nn + NN*mm];
      }
      AB[ll*NN + nn] = tmp;
    }
  }

  return 0;
}

/*
** _ell_LU_decomp()
**
** in-place LU decomposition
*/
int
_ell_LU_decomp(double *aa, size_t *indx, size_t NN)  {
  static const char me[]="_ell_LU_decomp";
  int ret=0;
  size_t ii, imax=0, jj, kk;
  double big, sum, tmp;
  double *vv;

  if (!( vv = (double*)calloc(NN, sizeof(double)) )) {
    biffAddf(ELL, "%s: couldn't allocate vv[]!", me);
    ret = 1; goto seeya;
  }

  /* find vv[i]: max of abs of everything in column i */
  for (ii=0; ii<NN; ii++) {
    big = 0.0;
    for (jj=0; jj<NN; jj++) {
      if ((tmp=AIR_ABS(aa[ii*NN + jj])) > big) {
        big = tmp;
      }
    }
    if (!big) {
      char stmp[AIR_STRLEN_SMALL];
      biffAddf(ELL, "%s: singular matrix since column %s all zero", me,
               airSprintSize_t(stmp, ii));
      ret = 1; goto seeya;
    }
    vv[ii] = big;
  }

  for (jj=0; jj<NN; jj++) {
    /* for aa[ii][jj] in lower triangle (below diagonal), subtract from
       aa[ii][jj] the dot product of all elements to its left with elements
       above it (starting at the top) */
    for (ii=0; ii<jj; ii++) {
      sum = aa[ii*NN + jj];
      for (kk=0; kk<ii; kk++) {
        sum -= aa[ii*NN + kk]*aa[kk*NN + jj];
      }
      aa[ii*NN + jj] = sum;
    }

    /* for aa[ii][jj] in upper triangle (including diagonal), subtract from
       aa[ii][jj] the dot product of all elements above it with elements to
       its left (starting from the left) */
    big = 0.0;
    for (ii=jj; ii<NN; ii++) {
      sum = aa[ii*NN + jj];
      for (kk=0; kk<jj; kk++) {
        sum -= aa[ii*NN + kk]*aa[kk*NN + jj];
      }
      aa[ii*NN + jj] = sum;
      /* imax column is one in which abs(aa[i][j])/vv[i] */
      if ((tmp = AIR_ABS(sum)/vv[ii]) >= big) {
        big = tmp;
        imax = ii;
      }
    }

    /* unless we're on the imax column, swap this column the with imax column,
       and permute vv[] accordingly */
    if (jj != imax) {
      /* could record parity # of permutes here */
      for (kk=0; kk<NN; kk++) {
        tmp = aa[imax*NN + kk];
        aa[imax*NN + kk] = aa[jj*NN + kk];
        aa[jj*NN + kk] = tmp;
      }
      tmp = vv[imax];
      vv[imax] = vv[jj];
      vv[jj] = tmp;
    }

    indx[jj] = imax;

    if (aa[jj*NN + jj] == 0.0) {
      aa[jj*NN + jj] = ELL_EPS;
    }

    /* divide everything right of a[jj][jj] by a[jj][jj] */
    if (jj != NN) {
      tmp = 1.0/aa[jj*NN + jj];
      for (ii=jj+1; ii<NN; ii++) {
        aa[ii*NN + jj] *= tmp;
      }
    }
  }
 seeya:
  airFree(vv);
  return ret;
}

/*
** _ell_LU_back_sub
**
** given the matrix and index array from _ellLUDecomp generated from
** some matrix M, solves for x in the linear equation Mx = b, and
** puts the result back into b
*/
void
_ell_LU_back_sub(double *aa, size_t *indx, double *bb, size_t NN) {
  size_t ii, jj;
  double sum;

  /* Forward substitution, with lower triangular matrix */
  for (ii=0; ii<NN; ii++) {
    sum = bb[indx[ii]];
    bb[indx[ii]] = bb[ii];
    for (jj=0; jj<ii; jj++) {
      sum -= aa[ii*NN + jj]*bb[jj];
    }
    bb[ii] = sum;
  }

  /* Backward substitution, with upper triangular matrix */
  for (ii=NN; ii>0; ii--) {
    sum = bb[ii-1];
    for (jj=ii; jj<NN; jj++) {
      sum -= aa[(ii-1)*NN + jj]*bb[jj];
    }
    bb[ii-1] = sum / aa[(ii-1)*NN + (ii-1)];
  }
  return;
}

/*
** _ell_inv
**
** Invert NNxNN matrix based on LU-decomposition
**
** The given matrix is copied, turned into its LU-decomposition, and
** then repeated backsubstitution is used to get successive columns of
** the inverse.
*/
int
_ell_inv(double *inv, double *_mat, size_t NN) {
  static const char me[]="_ell_inv";
  size_t ii, jj, *indx=NULL;
  double *col=NULL, *mat=NULL;
  int ret=0;

  if (!( (col = (double*)calloc(NN, sizeof(double))) &&
         (mat = (double*)calloc(NN*NN, sizeof(double))) &&
         (indx = (size_t*)calloc(NN, sizeof(size_t))) )) {
    biffAddf(ELL, "%s: couldn't allocate all buffers", me);
    ret = 1; goto seeya;
  }

  memcpy(mat, _mat, NN*NN*sizeof(double));

  if (_ell_LU_decomp(mat, indx, NN)) {
    biffAddf(ELL, "%s: trouble", me);
    ret = 1; goto seeya;
  }

  for (jj=0; jj<NN; jj++) {
    memset(col, 0, NN*sizeof(double));
    col[jj] = 1.0;
    _ell_LU_back_sub(mat, indx, col, NN);
    /* set column jj of inv to result of backsub */
    for (ii=0; ii<NN; ii++) {
      inv[ii*NN + jj] = col[ii];
    }
  }
 seeya:
  airFree(col); airFree(mat); airFree(indx);
  return ret;
}

/*
******** ell_Nm_inv
**
** computes the inverse of given matrix in nmat, and puts the
** inverse in the (maybe allocated) ninv.  Does not touch the
** values in nmat.
*/
int
ell_Nm_inv(Nrrd *ninv, Nrrd *nmat) {
  static const char me[]="ell_Nm_inv";
  double *mat, *inv;
  size_t NN;

  if (!( ninv && !ell_Nm_check(nmat, AIR_FALSE) )) {
    biffAddf(ELL, "%s: NULL or invalid args", me);
    return 1;
  }

  NN = nmat->axis[0].size;
  if (!( NN == nmat->axis[1].size )) {
    char stmp[2][AIR_STRLEN_SMALL];
    biffAddf(ELL, "%s: need a square matrix, not %s-by-%s", me,
             airSprintSize_t(stmp[0], nmat->axis[1].size),
             airSprintSize_t(stmp[1], NN));
    return 1;
  }
  if (nrrdMaybeAlloc_va(ninv, nrrdTypeDouble, 2,
                        NN, NN)) {
    biffMovef(ELL, NRRD, "%s: trouble", me);
    return 1;
  }
  inv = (double*)(ninv->data);
  mat = (double*)(nmat->data);
  if (_ell_inv(inv, mat, NN)) {
    biffAddf(ELL, "%s: trouble", me);
    return 1;
  }

  return 0;
}

/*
******** ell_Nm_pseudo_inv()
**
** determines the pseudoinverse of the given matrix M by using the formula
** P = (M^T * M)^(-1) * M^T
**
** I'll get an SVD-based solution working later, since that gives a more
** general solution
*/
int
ell_Nm_pseudo_inv(Nrrd *ninv, Nrrd *nA) {
  static const char me[]="ell_Nm_pseudo_inv";
  Nrrd *nAt, *nAtA, *nAtAi;
  int ret=0;

  if (!( ninv && !ell_Nm_check(nA, AIR_FALSE) )) {
    biffAddf(ELL, "%s: NULL or invalid args", me);
    return 1;
  }
  nAt = nrrdNew();
  nAtA = nrrdNew();
  nAtAi = nrrdNew();
  if (ell_Nm_tran(nAt, nA)
      || ell_Nm_mul(nAtA, nAt, nA)
      || ell_Nm_inv(nAtAi, nAtA)
      || ell_Nm_mul(ninv, nAtAi, nAt)) {
    biffAddf(ELL, "%s: trouble", me);
    ret = 1; goto seeya;
  }

 seeya:
  nrrdNuke(nAt); nrrdNuke(nAtA); nrrdNuke(nAtAi);
  return ret;
}

/*
******** ell_Nm_wght_pseudo_inv()
**
** determines a weighted least squares solution via
** P = (A^T * W * A)^(-1) * A^T * W
*/
int
ell_Nm_wght_pseudo_inv(Nrrd *ninv, Nrrd *nA, Nrrd *nW) {
  static const char me[]="ell_Nm_wght_pseudo_inv";
  Nrrd *nAt, *nAtW, *nAtWA, *nAtWAi;
  int ret=0;

  if (!( ninv && !ell_Nm_check(nA, AIR_FALSE)
         && !ell_Nm_check(nW, AIR_FALSE) )) {
    biffAddf(ELL, "%s: NULL or invalid args", me);
    return 1;
  }
  nAt = nrrdNew();
  nAtW = nrrdNew();
  nAtWA = nrrdNew();
  nAtWAi = nrrdNew();
  if (ell_Nm_tran(nAt, nA)
      || ell_Nm_mul(nAtW, nAt, nW)
      || ell_Nm_mul(nAtWA, nAtW, nA)
      || ell_Nm_inv(nAtWAi, nAtWA)
      || ell_Nm_mul(ninv, nAtWAi, nAtW)) {
    biffAddf(ELL, "%s: trouble", me);
    ret = 1; goto seeya;
  }

 seeya:
  nrrdNuke(nAt); nrrdNuke(nAtW); nrrdNuke(nAtWA); nrrdNuke(nAtWAi);
  return ret;
}

