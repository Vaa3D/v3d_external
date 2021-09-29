/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2011, 2010, 2009, 2008  Thomas Schultz

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

/* This file collects functions that implement extraction of crease
 * surfaces as proposed in Schultz / Theisel / Seidel, "Crease
 * Surfaces: From Theory to Extraction and Application to Diffusion
 * Tensor MRI", IEEE TVCG 16(1):109-119, 2010 */

#include "seek.h"
#include "privateSeek.h"

/* private helper routines for the T-based extraction */

/* Converts a Hessian into the transformed tensor T (cf. paper)
 *
 * T is a 9-vector representing the output
 * evals is a 3-vector (eigenvalues of the Hessian)
 * evecs is a 9-vector (eigenvectors of the Hessian)
 * evalDiffThresh is the threshold parameter theta (cf. Eq. (4) in paper)
 * ridge is non-zero if we are looking for a ridge (zero for valley)
 */
void
_seekHess2T(double *T, const double *evals, const double *evecs,
            const double evalDiffThresh, const char ridge) {
  double lambdas[3]={0.0,0.0,0.0};
  double tmpMat[9], diag[9], evecsT[9];
  if (ridge) {
    double diff = evals[1]-evals[2];
    lambdas[0]=lambdas[1]=1.0;
    if (diff<evalDiffThresh)
      lambdas[2]=(1.0-diff/evalDiffThresh)*(1.0-diff/evalDiffThresh);
  } else {
    double diff = evals[0]-evals[1];
    lambdas[1]=lambdas[2]=1.0;
    if (diff<evalDiffThresh)
      lambdas[0]=(1.0-diff/evalDiffThresh)*(1.0-diff/evalDiffThresh);
  }
  ELL_3M_ZERO_SET(diag);
  ELL_3M_DIAG_SET(diag, lambdas[0], lambdas[1], lambdas[2]);
  ELL_3M_TRANSPOSE(evecsT, evecs);
  ELL_3M_MUL(tmpMat, diag, evecs);
  ELL_3M_MUL(T, evecsT, tmpMat);
}

/* Converts a Hessian derivative into a derivative of the transformed
 * tensor field T
 *
 * Tder is a 9-vector representing the output
 * hessder is a 9-vector (Hessian derivative)
 * evals is a 3-vector (eigenvalues of the Hessian value)
 * evecs is a 9-vector (eigenvectors of the Hessian value)
 * evalDiffThresh is the threshold parameter theta (cf. Eq. (4) in the paper)
 * ridge is non-zero if we are looking for a ridge (zero for valley)
 */
void
_seekHessder2Tder(double *Tder, const double *hessder, const double *evals,
                  const double *evecs, const double evalDiffThresh,
                  const char ridge) {
  double evecTrans[9];
  double hessderE[9]; /* Hessian derivative in eigenframe */
  double tmp[9];
  ELL_3M_TRANSPOSE(evecTrans,evecs);
  ell_3m_mul_d(tmp,hessder,evecTrans);
  ell_3m_mul_d(hessderE,evecs,tmp);
  
  if (ridge) {
    double diff=evals[1]-evals[2];
    double l3;
    double l3der;
    if (diff<evalDiffThresh)
      l3=(1.0-diff/evalDiffThresh)*(1.0-diff/evalDiffThresh);
    else l3=0.0;
    hessderE[2]*=(1.0-l3)/(evals[0]-evals[2]);
    hessderE[6]=hessderE[2];
    hessderE[5]*=(1.0-l3)/(evals[1]-evals[2]);
    hessderE[7]=hessderE[5];
    
    if (diff<evalDiffThresh)
      l3der = 2/evalDiffThresh*(1-diff/evalDiffThresh)*
        (hessderE[8]-hessderE[4]);
    else
      l3der=0;
    hessderE[8]=l3der;
    hessderE[0]=hessderE[1]=hessderE[3]=hessderE[4]=0.0;
  } else {
    double diff=evals[0]-evals[1];
    double l1;
    double l1der;
    if (diff<evalDiffThresh)
      l1=(1.0-diff/evalDiffThresh)*(1.0-diff/evalDiffThresh);
    else l1=0.0;
    hessderE[1]*=(l1-1.0)/(evals[0]-evals[1]);
    hessderE[3]=hessderE[1];
    hessderE[2]*=(l1-1.0)/(evals[0]-evals[2]);
    hessderE[6]=hessderE[2];
    
    if (diff<evalDiffThresh)
      l1der = 2/evalDiffThresh*(1-diff/evalDiffThresh)*
        (hessderE[4]-hessderE[0]);
    else
      l1der = 0;
    hessderE[0]=l1der;
    hessderE[4]=hessderE[5]=hessderE[7]=hessderE[8]=0.0;
  }
  
  ell_3m_mul_d(tmp,hessderE,evecs);
  ell_3m_mul_d(Tder,evecTrans,tmp);  
}

static int
findFeatureIntersection(double *results, double *Tleft,
                        double *hessleft, double *gleft,
                        double *Tright, double *hessright,
                        double *gright, double idxleft,
                        double idxright, char ridge,
                        const double evalDiffThresh,
                        const double dotThresh) {
  double Tdp = ELL_3V_DOT(Tleft, Tright) + ELL_3V_DOT(Tleft+3,Tright+3) +
    ELL_3V_DOT(Tleft+6,Tright+6);
  double denom_l = sqrt(ELL_3V_DOT(Tleft, Tleft) + ELL_3V_DOT(Tleft+3, Tleft+3)
                        + ELL_3V_DOT(Tleft+6, Tleft+6)),
    denom_r = sqrt(ELL_3V_DOT(Tright,Tright) + ELL_3V_DOT(Tright+3, Tright+3)
                   + ELL_3V_DOT(Tright+6, Tright+6));
  
  if (Tdp/(denom_l*denom_r)<dotThresh &&
      idxright-idxleft>0.24) { /* do a recursive step */
    double idxcenter = 0.5*(idxleft+idxright);
    /* simply interpolate Hessian linearly */
    double hessnew[9];
    double evals[3],evecs[9];
    double Tnew[9], gradnew[3];
    int retval;
    
    ELL_3M_LERP(hessnew,0.5,hessleft,hessright);
    ell_3m_eigensolve_d(evals, evecs, hessnew, AIR_TRUE);
    
    _seekHess2T(Tnew, evals, evecs, evalDiffThresh, ridge);
    
    ELL_3V_LERP(gradnew,0.5,gleft,gright);
    retval = findFeatureIntersection(results, Tleft, hessleft,
                                     gleft, Tnew, hessnew, gradnew,
                                     idxleft, idxcenter,
                                     ridge, evalDiffThresh, dotThresh);
    retval += findFeatureIntersection(results+retval, Tnew, hessnew,
                                      gradnew, Tright, hessright, gright,
                                      idxcenter, idxright,
                                      ridge, evalDiffThresh, dotThresh);
    return retval;    
  } else {
    double d1[3], d4[3];
    ell_3mv_mul_d(d1, Tleft, gleft);
    ELL_3V_SUB(d1,d1,gleft);
    ell_3mv_mul_d(d4, Tright, gright);
    ELL_3V_SUB(d4,d4,gright);
    
    if (ELL_3V_DOT(d1,d4)<0) { /* mark edge as crossed */
      /* find assumed intersection point */
      double diff[3], dlen, alpha;
      ELL_3V_SUB(diff,d4,d1);
      dlen=ELL_3V_LEN(diff);
      if (dlen>1e-5) {
        double ap=-ELL_3V_DOT(d1,diff)/dlen;
        alpha = ap/dlen;
      } else
        alpha = 0.5;
      
      *results = (1-alpha)*idxleft+alpha*idxright;
      return 1;
    }
  }
  return 0;
}

/* Assuming (simplistic) linearly interpolated Hessians and gradients,
 * computes the analytical normal of the crease surface.
 * The result is _not_ normalized. */
static void
computeGradientLin(double *result, double *T, double *g,
                   double *Txm, double *gxm, double *Txp, double *gxp,
                   double *Tym, double *gym, double *Typ, double *gyp,
                   double *Tzm, double *gzm, double *Tzp, double *gzp) {
  double Tder[9];
  double gder[3];
  double tmp[3], tmp1[3], tmp2[3];
  double derxv[3], deryv[3], derzv[3];
  ELL_3M_SUB(Tder,Txp,Txm);
  ELL_3V_SUB(gder,gxp,gxm);
  ell_3mv_mul_d(tmp,T,gder);
  ELL_3V_SUB(tmp,tmp,gder);
  ell_3mv_mul_d(derxv,Tder,g);
  ELL_3V_ADD2(derxv,derxv,tmp);
  
  ELL_3M_SUB(Tder,Typ,Tym);
  ELL_3V_SUB(gder,gyp,gym);
  ell_3mv_mul_d(tmp,T,gder);
  ELL_3V_SUB(tmp,tmp,gder);
  ell_3mv_mul_d(deryv,Tder,g);
  ELL_3V_ADD2(deryv,deryv,tmp);
  
  ELL_3M_SUB(Tder,Tzp,Tzm);
  ELL_3V_SUB(gder,gzp,gzm);
  ell_3mv_mul_d(tmp,T,gder);
  ELL_3V_SUB(tmp,tmp,gder);
  ell_3mv_mul_d(derzv,Tder,g);
  ELL_3V_ADD2(derzv,derzv,tmp);
  
  /* accumulate a gradient */
  tmp1[0]=derxv[0]; tmp1[1]=deryv[0]; tmp1[2]=derzv[0];
  tmp2[0]=derxv[1]; tmp2[1]=deryv[1]; tmp2[2]=derzv[1];
  if (ELL_3V_DOT(tmp1,tmp2)<0)
    ELL_3V_SCALE(tmp2,-1.0,tmp2);
  ELL_3V_ADD2(tmp1,tmp1,tmp2);
  tmp2[0]=derxv[2]; tmp2[1]=deryv[2]; tmp2[2]=derzv[2];
  if (ELL_3V_DOT(tmp1,tmp2)<0)
    ELL_3V_SCALE(tmp2,-1.0,tmp2);
  ELL_3V_ADD2(result,tmp1,tmp2);
}

/* Given a unique edge ID and an intersection point given by some value
 * alpha \in [0,1], compute the crease surface normal at that point */
static void
computeEdgeGradient(seekContext *sctx, baggage *bag, double *res,
                    unsigned int xi, unsigned int yi, char edgeid, double alpha)
{
  double Txm[9], Txp[9], Tym[9], Typ[9], Tzm[9], Tzp[9], T[9],
    gxm[3], gxp[3], gym[3], gyp[3], gzm[3], gzp[3], g[3];
  
  unsigned int sx = AIR_CAST(unsigned int, sctx->sx);
  unsigned int sy = AIR_CAST(unsigned int, sctx->sy);
  unsigned int sz = AIR_CAST(unsigned int, sctx->sz);
  unsigned int si = xi + sx*yi;
  unsigned int six = xi + 1 + sx*yi, siX = xi - 1 + sx*yi;
  unsigned int siy = xi + sx*(yi+1), siY = xi + sx*(yi-1);
  unsigned int sixy = xi + 1 + sx*(yi+1),
    sixY = xi + 1 + sx*(yi-1),
    siXy = xi - 1 + sx*(yi+1);
  /* many special cases needed to fill Txm, gxm, etc. :-( */
  switch (edgeid) {
  case 0:
    ELL_3M_LERP(T, alpha, sctx->t + 9*(0+2*si), sctx->t + 9*(0+2*six));
    ELL_3V_LERP(g, alpha, sctx->grad + 3*(0+2*si), sctx->grad + 3*(0+2*six));
    ELL_3M_LERP(Tzp, alpha, sctx->t + 9*(1+2*si), sctx->t + 9*(1+2*six));
    ELL_3V_LERP(gzp, alpha, sctx->grad + 3*(1+2*si), sctx->grad + 3*(1+2*six));
    if (bag->zi==0) {
      ELL_3M_COPY(Tzm, T); ELL_3V_COPY(gzm, g);
    } else {
      ELL_3M_LERP(Tzm, alpha, sctx->tcontext + 9*(0+2*si),
                  sctx->tcontext + 9*(0+2*six));
      ELL_3V_LERP(gzm, alpha, sctx->gradcontext + 3*(0+2*si),
                  sctx->gradcontext + 3*(0+2*six));
      ELL_3M_SCALE(Tzm, 0.5, Tzm); ELL_3M_SCALE(Tzp, 0.5, Tzp);
      ELL_3V_SCALE(gzm, 0.5, gzm); ELL_3V_SCALE(gzp, 0.5, gzp);
    }
    if (yi==0) {
      ELL_3M_COPY(Tym, T); ELL_3V_COPY(gym, g);
    } else {
      ELL_3M_LERP(Tym, alpha, sctx->t + 9*(0+2*siY), sctx->t + 9*(0+2*sixY));
      ELL_3V_LERP(gym, alpha, sctx->grad + 3*(0+2*siY),
                  sctx->grad + 3*(0+2*sixY));
    }
    if (yi==sy-1) {
      ELL_3M_COPY(Typ, T); ELL_3V_COPY(gyp, g);
    } else {
      ELL_3M_LERP(Typ, alpha, sctx->t + 9*(0+2*siy), sctx->t + 9*(0+2*sixy));
      ELL_3V_LERP(gyp, alpha, sctx->grad + 3*(0+2*siy),
                  sctx->grad + 3*(0+2*sixy));
    }
    if (yi!=0 && yi!=sy-1) {
      ELL_3M_SCALE(Tym, 0.5, Tym); ELL_3M_SCALE(Typ, 0.5, Typ);
      ELL_3V_SCALE(gym, 0.5, gym); ELL_3V_SCALE(gyp, 0.5, gyp);
    }
    computeGradientLin(res, T, g,
                       sctx->t + 9*(0+2*si), sctx->grad + 3*(0+2*si),
                       sctx->t + 9*(0+2*six), sctx->grad + 3*(0+2*six),
                       Tym, gym, Typ, gyp,
                       Tzm, gzm, Tzp, gzp);
    break;
  case 1:
    ELL_3M_LERP(T, alpha, sctx->t + 9*(0+2*si), sctx->t + 9*(0+2*siy));
    ELL_3V_LERP(g, alpha, sctx->grad + 3*(0+2*si), sctx->grad + 3*(0+2*siy));
    ELL_3M_LERP(Tzp, alpha, sctx->t + 9*(1+2*si), sctx->t + 9*(1+2*siy));
    ELL_3V_LERP(gzp, alpha, sctx->grad + 3*(1+2*si), sctx->grad + 3*(1+2*siy));
    if (bag->zi==0) {
      ELL_3M_COPY(Tzm, T); ELL_3V_COPY(gzm, g);
    } else {
      ELL_3M_LERP(Tzm, alpha, sctx->tcontext + 9*(0+2*si),
                  sctx->tcontext + 9*(0+2*siy));
      ELL_3V_LERP(gzm, alpha, sctx->gradcontext + 3*(0+2*si),
                  sctx->gradcontext + 3*(0+2*siy));
      ELL_3M_SCALE(Tzm, 0.5, Tzm); ELL_3M_SCALE(Tzp, 0.5, Tzp);
      ELL_3V_SCALE(gzm, 0.5, gzm); ELL_3V_SCALE(gzp, 0.5, gzp);
    }
    if (xi==0) {
      ELL_3M_COPY(Txm, T); ELL_3V_COPY(gxm, g);
    } else {
      ELL_3M_LERP(Txm, alpha, sctx->t + 9*(0+2*siX), sctx->t + 9*(0+2*siXy));
      ELL_3V_LERP(gxm, alpha, sctx->grad + 3*(0+2*siX),
                  sctx->grad + 3*(0+2*siXy));
    }
    if (xi==sx-1) {
      ELL_3M_COPY(Txp, T); ELL_3V_COPY(gxm, g);
    } else {
      ELL_3M_LERP(Txp, alpha, sctx->t + 9*(0+2*six), sctx->t + 9*(0+2*sixy));
      ELL_3V_LERP(gxp, alpha, sctx->grad + 3*(0+2*six),
                  sctx->grad + 3*(0+2*sixy));
    }
    if (xi!=0 && xi!=sx-1) {
      ELL_3M_SCALE(Txm, 0.5, Txm); ELL_3M_SCALE(Txp, 0.5, Txp);
      ELL_3V_SCALE(gxm, 0.5, gxm); ELL_3V_SCALE(gxp, 0.5, gxp);
    }
    computeGradientLin(res, T, g,
                       Txm, gxm, Txp, gxp,
                       sctx->t + 9*(0+2*si), sctx->grad + 3*(0+2*si),
                       sctx->t + 9*(0+2*siy), sctx->grad + 3*(0+2*siy),
                       T, g, Tzp, gzp);
    break;
  case 2:
    ELL_3M_LERP(T, alpha, sctx->t + 9*(0+2*si), sctx->t + 9*(1+2*si));
    ELL_3V_LERP(g, alpha, sctx->grad + 3*(0+2*si), sctx->grad + 3*(1+2*si));
    if (xi==0) {
      ELL_3M_COPY(Txm, T); ELL_3V_COPY(gxm, g);
    } else {
      ELL_3M_LERP(Txm, alpha, sctx->t + 9*(0+2*siX), sctx->t + 9*(1+2*siX));
      ELL_3V_LERP(gxm, alpha, sctx->grad + 3*(0+2*siX),
                  sctx->grad + 3*(1+2*siX));
    }
    if (xi==sx-1) {
      ELL_3M_COPY(Txp, T); ELL_3V_COPY(gxp, g);
    } else {
      ELL_3M_LERP(Txp, alpha, sctx->t + 9*(0+2*six), sctx->t + 9*(1+2*six));
      ELL_3V_LERP(gxp, alpha, sctx->grad + 3*(0+2*six),
                  sctx->grad + 3*(1+2*six));
    }
    if (xi!=0 && xi!=sx-1) {
      ELL_3M_SCALE(Txm, 0.5, Txm); ELL_3M_SCALE(Txp, 0.5, Txp);
      ELL_3V_SCALE(gxm, 0.5, gxm); ELL_3V_SCALE(gxp, 0.5, gxp);
    }
    if (yi==0) {
      ELL_3M_COPY(Tym, T); ELL_3V_COPY(gym, g);
    } else {
      ELL_3M_LERP(Tym, alpha, sctx->t + 9*(0+2*siY), sctx->t + 9*(1+2*siY));
      ELL_3V_LERP(gym, alpha, sctx->grad + 3*(0+2*siY),
                  sctx->grad + 3*(1+2*siY));
    }
    if (yi==sy-1) {
      ELL_3M_COPY(Typ, T); ELL_3V_COPY(gyp, g);
    } else {
      ELL_3M_LERP(Typ, alpha, sctx->t + 9*(0+2*siy), sctx->t + 9*(1+2*siy));
      ELL_3V_LERP(gyp, alpha, sctx->grad + 3*(0+2*siy),
                  sctx->grad + 3*(1+2*siy));
    }
    if (yi!=0 && yi!=sy-1) {
      ELL_3M_SCALE(Tym, 0.5, Tym); ELL_3M_SCALE(Typ, 0.5, Typ);
      ELL_3V_SCALE(gym, 0.5, gym); ELL_3V_SCALE(gyp, 0.5, gyp);
    }
    if (bag->zi>0 && bag->zi<sz-2) {
      ELL_3M_LERP(Tzm, alpha, sctx->tcontext + 9*(0+2*si),
                  sctx->t + 9*(0+2*si));
      ELL_3V_LERP(gzm, alpha, sctx->gradcontext + 3*(0+2*si),
                  sctx->grad + 3*(0+2*si));
      ELL_3M_LERP(Tzp, alpha, sctx->t + 9*(1+2*si),
                  sctx->tcontext + 9*(1+2*si));
      ELL_3V_LERP(gzp, alpha, sctx->grad + 3*(1+2*si),
                  sctx->gradcontext + 3*(1+2*si));
      ELL_3M_SCALE(Tzm, 0.5, Tzm); ELL_3M_SCALE(Tzp, 0.5, Tzp);
      ELL_3V_SCALE(gzm, 0.5, gzm); ELL_3V_SCALE(gzp, 0.5, gzp);
    } else {
      ELL_3M_COPY(Tzm, sctx->t + 9*(0+2*si));
      ELL_3V_COPY(gzm, sctx->grad + 3*(0+2*si));
      ELL_3M_COPY(Tzp, sctx->t + 9*(1+2*si));
      ELL_3V_COPY(gzp, sctx->grad + 3*(1+2*si));
    }
    computeGradientLin(res, T, g,
                       Txm, gxm, Txp, gxp,
                       Tym, gym, Typ, gyp,
                       Tzm, gzm, Tzp, gzp);
    break;
  case 3:
    ELL_3M_LERP(T, alpha, sctx->t + 9*(1+2*si), sctx->t + 9*(1+2*six));
    ELL_3V_LERP(g, alpha, sctx->grad + 3*(1+2*si), sctx->grad + 3*(1+2*six));
    ELL_3M_LERP(Tzm, alpha, sctx->t + 9*(0+2*si), sctx->t + 9*(0+2*six));
    ELL_3V_LERP(gzm, alpha, sctx->grad + 3*(0+2*si), sctx->grad + 3*(0+2*six));
    if (bag->zi==sz-2) {
      ELL_3M_COPY(Tzp, T); ELL_3V_COPY(gzp, g);
    } else {
      ELL_3M_LERP(Tzp, alpha, sctx->tcontext + 9*(1+2*si),
                  sctx->tcontext + 9*(1+2*six));
      ELL_3V_LERP(gzp, alpha, sctx->gradcontext + 3*(1+2*si),
                  sctx->gradcontext + 3*(1+2*six));
      ELL_3M_SCALE(Tzm, 0.5, Tzm); ELL_3M_SCALE(Tzp, 0.5, Tzp);
      ELL_3V_SCALE(gzm, 0.5, gzm); ELL_3V_SCALE(gzp, 0.5, gzp);
    }
    if (xi>0 && xi<sx-2) {
      unsigned int sixx = xi + 2 + sx*yi;
      ELL_3M_LERP(Txm, alpha, sctx->t + 9*(1+2*siX), sctx->t + 9*(1+2*si));
      ELL_3V_LERP(gxm, alpha, sctx->grad + 3*(1+2*siX),
                  sctx->grad + 3*(1+2*si));
      ELL_3M_LERP(Txp, alpha, sctx->t + 9*(1+2*six), sctx->t + 9*(1+2*sixx));
      ELL_3V_LERP(gxp, alpha, sctx->grad + 3*(1+2*six),
                  sctx->grad + 3*(1+2*sixx));
      ELL_3M_SCALE(Txm, 0.5, Txm); ELL_3M_SCALE(Txp, 0.5, Txp);
      ELL_3V_SCALE(gxm, 0.5, gxm); ELL_3V_SCALE(gxp, 0.5, gxp);
    } else {
      ELL_3M_COPY(Txm, sctx->t + 9*(1+2*si));
      ELL_3V_COPY(gxm, sctx->grad + 3*(1+2*si));
      ELL_3M_COPY(Txp, sctx->t + 9*(1+2*six));
      ELL_3V_COPY(gxp, sctx->grad + 3*(1+2*six));
    }
    if (yi==0) {
      ELL_3M_COPY(Tym, T); ELL_3V_COPY(gym, g);
    } else {
      ELL_3M_LERP(Tym, alpha, sctx->t + 9*(1+2*siY), sctx->t + 9*(1+2*sixY));
      ELL_3V_LERP(gym, alpha, sctx->grad + 3*(1+2*siY),
                  sctx->grad + 3*(1+2*sixY));
    }
    if (yi==sy-1) {
      ELL_3M_COPY(Typ, T); ELL_3V_COPY(gyp, g);
    } else {
      ELL_3M_LERP(Typ, alpha, sctx->t + 9*(1+2*siy), sctx->t + 9*(1+2*sixy));
      ELL_3V_LERP(gyp, alpha, sctx->grad + 3*(1+2*siy),
                  sctx->grad + 3*(1+2*sixy));
    }
    if (yi!=0 && yi!=sy-1) {
      ELL_3M_SCALE(Tym, 0.5, Tym); ELL_3M_SCALE(Typ, 0.5, Typ);
      ELL_3V_SCALE(gym, 0.5, gym); ELL_3V_SCALE(gyp, 0.5, gyp);
    }
    computeGradientLin(res, T, g,
                       Txm, gxm, Txp, gxp,
                       Tym, gym, Typ, gyp,
                       Tzm, gzm, Tzp, gzp);
    break;
  case 4:
    ELL_3M_LERP(T, alpha, sctx->t + 9*(1+2*si), sctx->t + 9*(1+2*siy));
    ELL_3V_LERP(g, alpha, sctx->grad + 3*(1+2*si), sctx->grad + 3*(1+2*siy));
    ELL_3M_LERP(Tzm, alpha, sctx->t + 9*(0+2*si), sctx->t + 9*(0+2*siy));
    ELL_3V_LERP(gzm, alpha, sctx->grad + 3*(0+2*si), sctx->grad + 3*(0+2*siy));
    if (bag->zi==sz-2) {
      ELL_3M_COPY(Tzp, T); ELL_3V_COPY(gzp, g);
    } else {
      ELL_3M_LERP(Tzp, alpha, sctx->tcontext + 9*(1+2*si),
                  sctx->tcontext + 9*(1+2*siy));
      ELL_3V_LERP(gzp, alpha, sctx->gradcontext + 3*(1+2*si),
                  sctx->gradcontext + 3*(1+2*siy));
      ELL_3M_SCALE(Tzm, 0.5, Tzm); ELL_3M_SCALE(Tzp, 0.5, Tzp);
      ELL_3V_SCALE(gzm, 0.5, gzm); ELL_3V_SCALE(gzp, 0.5, gzp);
    }
    if (xi==0) {
      ELL_3M_COPY(Txm, T); ELL_3V_COPY(gxm, g);
    } else {
      ELL_3M_LERP(Txm, alpha, sctx->t + 9*(1+2*siX), sctx->t + 9*(1+2*siXy));
      ELL_3V_LERP(gxm, alpha, sctx->grad + 3*(1+2*siX),
                  sctx->grad + 3*(1+2*siXy));
    }
    if (xi==sx-1) {
      ELL_3M_COPY(Txp, T); ELL_3V_COPY(gxp, g);
    } else {
      ELL_3M_LERP(Txp, alpha, sctx->t + 9*(1+2*six), sctx->t + 9*(1+2*sixy));
      ELL_3V_LERP(gxp, alpha, sctx->grad + 3*(1+2*six),
                  sctx->grad + 3*(1+2*sixy));
    }
    if (xi!=0 && xi!=sx-1) {
      ELL_3M_SCALE(Txm, 0.5, Txm); ELL_3M_SCALE(Txp, 0.5, Txp);
      ELL_3V_SCALE(gxm, 0.5, gxm); ELL_3V_SCALE(gxp, 0.5, gxp);
    }
    if (yi>0 && yi<sy-2) {
      unsigned int siyy = xi + sx*(yi+2);
      ELL_3M_LERP(Tym, alpha, sctx->t + 9*(1+2*siY), sctx->t + 9*(1+2*si));
      ELL_3V_LERP(gym, alpha, sctx->grad + 3*(1+2*siY),
                  sctx->grad + 3*(1+2*si));
      ELL_3M_LERP(Typ, alpha, sctx->t + 9*(1+2*siy), sctx->t + 9*(1+2*siyy));
      ELL_3V_LERP(gyp, alpha, sctx->grad + 3*(1+2*siy),
                  sctx->grad + 3*(1+2*siyy));
      ELL_3M_SCALE(Tym, 0.5, Tym); ELL_3M_SCALE(Typ, 0.5, Typ);
      ELL_3V_SCALE(gym, 0.5, gym); ELL_3V_SCALE(gyp, 0.5, gyp);
    } else {
      ELL_3M_COPY(Tym, sctx->t + 9*(1+2*si));
      ELL_3V_COPY(gym, sctx->grad + 3*(1+2*si));
      ELL_3M_COPY(Typ, sctx->t + 9*(1+2*six));
      ELL_3V_COPY(gyp, sctx->grad + 3*(1+2*six));
    }
    computeGradientLin(res, T, g,
                       Txm, gxm, Txp, gxp,
                       Tym, gym, Typ, gyp,
                       Tzm, gzm, Tzp, gzp);
    break;
  }
}

/* Given a unique face ID and coordinates, compute the crease surface
 * normal at the specified degenerate point */
static void
computeFaceGradient(seekContext *sctx, double *res,
                    unsigned int xi, unsigned int yi,
                    char faceid, double *coords) {
  double T[9], Txm[9], Txp[9], Tym[9], Typ[9], Tzm[9], Tzp[9],
    g[3], gxm[3], gxp[3], gym[3], gyp[3], gzm[3], gzp[3];
  unsigned int sx = AIR_CAST(unsigned int, sctx->sx);
  unsigned int sy = AIR_CAST(unsigned int, sctx->sy);
  unsigned int si = xi + sx*yi;
  unsigned int six = xi + 1 + sx*yi, siX = xi - 1 + sx*yi;
  unsigned int siy = xi + sx*(yi+1), siY = xi + sx*(yi-1);
  unsigned int sixy = xi + 1 + sx*(yi+1), sixY = xi + 1 + sx*(yi-1),
    siXy = xi - 1 + sx*(yi+1);
  
  /* Again, lots of special cases to fill Txm, gxm, etc. */
  switch (faceid) {
  case 0:
    /* bilinearly interpolate Tzp/gzp first */
    ELL_3M_LERP(Txm, coords[1], sctx->t + 9*(1+2*si), sctx->t + 9*(1+2*siy));
    ELL_3V_LERP(gxm, coords[1], sctx->grad + 3*(1+2*si),
                sctx->grad + 3*(1+2*siy));
    ELL_3M_LERP(Txp, coords[1], sctx->t + 9*(1+2*six), sctx->t + 9*(1+2*sixy));
    ELL_3V_LERP(gxp, coords[1], sctx->grad + 3*(1+2*six),
                sctx->grad + 3*(1+2*sixy));
    ELL_3M_LERP(Tzp, coords[0], Txm, Txp);
    ELL_3V_LERP(gzp, coords[0], gxm, gxp);
    /* now, compute all required points on the bottom face */
    ELL_3M_LERP(Txm, coords[1], sctx->t + 9*(0+2*si), sctx->t + 9*(0+2*siy));
    ELL_3V_LERP(gxm, coords[1], sctx->grad + 3*(0+2*si),
                sctx->grad + 3*(0+2*siy));
    ELL_3M_LERP(Txp, coords[1], sctx->t + 9*(0+2*six), sctx->t + 9*(0+2*sixy));
    ELL_3V_LERP(gxp, coords[1], sctx->grad + 3*(0+2*six),
                sctx->grad + 3*(0+2*sixy));
    
    ELL_3M_LERP(Tym, coords[0], sctx->t + 9*(0+2*si), sctx->t + 9*(0+2*six));
    ELL_3V_LERP(gym, coords[0], sctx->grad + 3*(0+2*si),
                sctx->grad + 3*(0+2*six));
    ELL_3M_LERP(Typ, coords[0], sctx->t + 9*(0+2*siy), sctx->t + 9*(0+2*sixy));
    ELL_3V_LERP(gyp, coords[0], sctx->grad + 3*(0+2*siy),
                sctx->grad + 3*(0+2*sixy));
    
    ELL_3M_LERP(T, coords[0], Txm, Txp);
    ELL_3V_LERP(g, coords[0], gxm, gxp);
    
    computeGradientLin(sctx->facenorm+3*(faceid+4*si), T, g,
                       Txm, gxm, Txp, gxp,
                       Tym, gym, Typ, gyp,
                       T, g, Tzp, gzp);
    break;
  case 1:
    /* bilinearly interpolate Typ/gyp first */
    if (yi!=sy-1) {
      ELL_3M_LERP(Txm, coords[1], sctx->t + 9*(0+2*siy), sctx->t + 9*(1+2*siy));
      ELL_3V_LERP(gxm, coords[1], sctx->grad + 3*(0+2*siy),
                  sctx->grad + 3*(1+2*siy));
      ELL_3M_LERP(Txp, coords[1], sctx->t + 9*(0+2*sixy),
                  sctx->t + 9*(1+2*sixy));
      ELL_3V_LERP(gxp, coords[1], sctx->grad + 3*(0+2*sixy),
                  sctx->grad + 3*(1+2*sixy));
      ELL_3M_LERP(Typ, coords[0], Txm, Txp);
      ELL_3V_LERP(gyp, coords[0], gxm, gxp);
    } else {
      ELL_3M_LERP(Txm, coords[1], sctx->t + 9*(0+2*siY), sctx->t + 9*(1+2*siY));
      ELL_3V_LERP(gxm, coords[1], sctx->grad + 3*(0+2*siY),
                  sctx->grad + 3*(1+2*siY));
      ELL_3M_LERP(Txp, coords[1], sctx->t + 9*(0+2*sixY),
                  sctx->t + 9*(1+2*sixY));
      ELL_3V_LERP(gxp, coords[1], sctx->grad + 3*(0+2*sixY),
                  sctx->grad + 3*(1+2*sixY));
      ELL_3M_LERP(Tym, coords[0], Txm, Txp);
      ELL_3V_LERP(gym, coords[0], gxm, gxp);
    }
    /* now, compute remaining points */
    ELL_3M_LERP(Txm, coords[1], sctx->t + 9*(0+2*si), sctx->t + 9*(1+2*si));
    ELL_3V_LERP(gxm, coords[1], sctx->grad + 3*(0+2*si),
                sctx->grad + 3*(1+2*si));
    ELL_3M_LERP(Txp, coords[1], sctx->t + 9*(0+2*six), sctx->t + 9*(1+2*six));
    ELL_3V_LERP(gxp, coords[1], sctx->grad + 3*(0+2*six),
                sctx->grad + 3*(1+2*six));
    
    ELL_3M_LERP(Tzm, coords[0], sctx->t + 9*(0+2*si), sctx->t + 9*(0+2*six));
    ELL_3V_LERP(gzm, coords[0], sctx->grad + 3*(0+2*si),
                sctx->grad + 3*(0+2*six));
    ELL_3M_LERP(Tzp, coords[0], sctx->t + 9*(1+2*si), sctx->t + 9*(1+2*six));
    ELL_3V_LERP(gzp, coords[0], sctx->grad + 3*(1+2*si),
                sctx->grad + 3*(1+2*six));
    
    ELL_3M_LERP(T, coords[0], Txm, Txp);
    ELL_3V_LERP(g, coords[0], gxm, gxp);
    
    if (yi!=sy-1) {
      computeGradientLin(sctx->facenorm+3*(faceid+4*si), T, g,
                         Txm, gxm, Txp, gxp,
                         T, g, Typ, gyp,
                         Tzm, gzm, Tzp, gzp);
    } else {
      computeGradientLin(sctx->facenorm+3*(faceid+4*si), T, g,
                         Txm, gxm, Txp, gxp,
                         Tym, gym, T, g,
                         Tzm, gzm, Tzp, gzp);
    }
    break;
  case 2:
    /* bilinearly interpolate Txp/gxp first */
    if (xi!=sx-1) {
      ELL_3M_LERP(Tym, coords[1], sctx->t + 9*(0+2*six), sctx->t + 9*(1+2*six));
      ELL_3V_LERP(gym, coords[1], sctx->grad + 3*(0+2*six),
                  sctx->grad + 3*(1+2*six));
      ELL_3M_LERP(Typ, coords[1], sctx->t + 9*(0+2*sixy),
                  sctx->t + 9*(1+2*sixy));
      ELL_3V_LERP(gyp, coords[1], sctx->grad + 3*(0+2*sixy),
                  sctx->grad + 3*(1+2*sixy));
      ELL_3M_LERP(Txp, coords[0], Tym, Typ);
      ELL_3V_LERP(gxp, coords[0], gym, gyp);
    } else {
      ELL_3M_LERP(Tym, coords[1], sctx->t + 9*(0+2*siX), sctx->t + 9*(1+2*siX));
      ELL_3V_LERP(gym, coords[1], sctx->grad + 3*(0+2*siX),
                  sctx->grad + 3*(1+2*siX));
      ELL_3M_LERP(Typ, coords[1], sctx->t + 9*(0+2*siXy),
                  sctx->t + 9*(1+2*siXy));
      ELL_3V_LERP(gyp, coords[1], sctx->grad + 3*(0+2*siXy),
                  sctx->grad + 3*(1+2*siXy));
      ELL_3M_LERP(Txm, coords[0], Tym, Typ);
      ELL_3V_LERP(gxm, coords[0], gym, gyp);
    }
    /* now, compute remaining points */
    ELL_3M_LERP(Tym, coords[1], sctx->t + 9*(0+2*si), sctx->t + 9*(1+2*si));
    ELL_3V_LERP(gym, coords[1], sctx->grad + 3*(0+2*si),
                sctx->grad + 3*(1+2*si));
    ELL_3M_LERP(Typ, coords[1], sctx->t + 9*(0+2*siy), sctx->t + 9*(1+2*siy));
    ELL_3V_LERP(gyp, coords[1], sctx->grad + 3*(0+2*siy),
                sctx->grad + 3*(1+2*siy));
    
    ELL_3M_LERP(Tzm, coords[0], sctx->t + 9*(0+2*si), sctx->t + 9*(0+2*siy));
    ELL_3V_LERP(gzm, coords[0], sctx->grad + 3*(0+2*si),
                sctx->grad + 3*(0+2*siy));
    ELL_3M_LERP(Tzp, coords[0], sctx->t + 9*(1+2*si), sctx->t + 9*(1+2*siy));
    ELL_3V_LERP(gzp, coords[0], sctx->grad + 3*(1+2*si),
                sctx->grad + 3*(1+2*siy));
    
    ELL_3M_LERP(T, coords[0], Tym, Typ);
    ELL_3V_LERP(g, coords[0], gym, gyp);
    
    if (xi!=sx-1) {
      computeGradientLin(sctx->facenorm+3*(faceid+4*si), T, g,
                         T, g, Txp, gxp,
                         Tym, gym, Typ, gyp,
                         Tzm, gzm, Tzp, gzp);
    } else {
      computeGradientLin(sctx->facenorm+3*(faceid+4*si), T, g,
                         Txm, gxm, T, g,
                         Tym, gym, Typ, gyp,
                         Tzm, gzm, Tzp, gzp);
    }
    break;
  case 3:
    /* bilinearly interpolate Tzm/gzm first */
    ELL_3M_LERP(Txm, coords[1], sctx->t + 9*(0+2*si), sctx->t + 9*(0+2*siy));
    ELL_3V_LERP(gxm, coords[1], sctx->grad + 3*(0+2*si),
                sctx->grad + 3*(0+2*siy));
    ELL_3M_LERP(Txp, coords[1], sctx->t + 9*(0+2*six), sctx->t + 9*(0+2*sixy));
    ELL_3V_LERP(gxp, coords[1], sctx->grad + 3*(0+2*six),
                sctx->grad + 3*(0+2*sixy));
    ELL_3M_LERP(Tzm, coords[0], Txm, Txp);
    ELL_3V_LERP(gzm, coords[0], gxm, gxp);
    /* now, compute all required points on the top face */
    ELL_3M_LERP(Txm, coords[1], sctx->t + 9*(1+2*si), sctx->t + 9*(1+2*siy));
    ELL_3V_LERP(gxm, coords[1], sctx->grad + 3*(1+2*si),
                sctx->grad + 3*(1+2*siy));
    ELL_3M_LERP(Txp, coords[1], sctx->t + 9*(1+2*six), sctx->t + 9*(1+2*sixy));
    ELL_3V_LERP(gxp, coords[1], sctx->grad + 3*(1+2*six),
                sctx->grad + 3*(1+2*sixy));
    
    ELL_3M_LERP(Tym, coords[0], sctx->t + 9*(1+2*si), sctx->t + 9*(1+2*six));
    ELL_3V_LERP(gym, coords[0], sctx->grad + 3*(1+2*si),
                sctx->grad + 3*(1+2*six));
    ELL_3M_LERP(Typ, coords[0], sctx->t + 9*(1+2*siy), sctx->t + 9*(1+2*sixy));
    ELL_3V_LERP(gyp, coords[0], sctx->grad + 3*(1+2*siy),
                sctx->grad + 3*(1+2*sixy));
    
    ELL_3M_LERP(T, coords[0], Txm, Txp);
    ELL_3V_LERP(g, coords[0], gxm, gxp);
    
    computeGradientLin(sctx->facenorm+3*(faceid+4*si), T, g,
                       Txm, gxm, Txp, gxp,
                       Tym, gym, Typ, gyp,
                       Tzm, gzm, T, g);
    break;
  }
  ELL_3V_COPY(res, sctx->facenorm+3*(faceid+4*si));
}

/* small helper routines: intersection tests */

/* check if a given 2D triangle is oriented clockwise (-1)
 * or counter-clockwise (1).
 * returns 0 if given points are collinear */
static int
checkTriOrientation (double *p1, double *p2, double *p3) {
  double test = (((p2[0]-p1[0])*(p3[1]-p1[1])) - ((p3[0]-p1[0])*(p2[1]-p1[1])));
  if (test > 0) return 1;
  else if (test < 0) return -1;
  else return 0;
}

/* check if two given 2D lines intersect */
static int
lineIntersectionTest (double *l1p1, double *l1p2, double *l2p1, double *l2p2) {
  int or1 = checkTriOrientation(l1p1, l1p2, l2p1);
  int or2 = checkTriOrientation(l1p1, l1p2, l2p2);
  if (or1 != or2) {
    or1 = checkTriOrientation(l2p1, l2p2, l1p1);
    or2 = checkTriOrientation(l2p1, l2p2, l1p2);
    if (or1 != or2)
      return 1;
  }
  return 0;
}

/* check if two given 3D triangles intersect */
static int
triIntersectionTest (double *t1v1, double *t1v2, double *t1v3,
                     double *t2v1, double *t2v2, double *t2v3) {
  double n1[3], n2[3], d1, d2;
  double diff1[3], diff2[3];
  double t2sd1, t2sd2, t2sd3;
  ELL_3V_SUB(diff1, t1v2, t1v1);
  ELL_3V_SUB(diff2, t1v3, t1v1);
  ELL_3V_CROSS(n1,diff1,diff2);
  d1=-ELL_3V_DOT(n1,t1v1);
  
  /* compute scaled signed distances of t2 to plane of t1 */
  t2sd1 = ELL_3V_DOT(n1, t2v1)+d1;
  t2sd2 = ELL_3V_DOT(n1, t2v2)+d1;
  t2sd3 = ELL_3V_DOT(n1, t2v3)+d1;
  
  if (t2sd1==0 && t2sd2==0 && t2sd3==0) {
    /* coplanar case: handle in 2D */
    double t1v12d[2], t1v22d[2], t1v32d[2], t2v12d[2], t2v22d[2], t2v32d[2];
    if (fabs(n1[0])>=fabs(n1[1]) && fabs(n1[0])>=fabs(n1[2])) {
      t1v12d[0]=t1v1[1]; t1v12d[1]=t1v1[2];
      t1v22d[0]=t1v2[1]; t1v22d[1]=t1v2[2];
      t1v32d[0]=t1v3[1]; t1v32d[1]=t1v3[2];
      t2v12d[0]=t2v1[1]; t2v12d[1]=t2v1[2];
      t2v22d[0]=t2v2[1]; t2v22d[1]=t2v2[2];
      t2v32d[0]=t2v3[1]; t2v32d[1]=t2v3[2];
    } else if (fabs(n1[1])>=fabs(n1[0]) && fabs(n1[1])>=fabs(n1[2])) {
      t1v12d[0]=t1v1[0]; t1v12d[1]=t1v1[2];
      t1v22d[0]=t1v2[0]; t1v22d[1]=t1v2[2];
      t1v32d[0]=t1v3[0]; t1v32d[1]=t1v3[2];
      t2v12d[0]=t2v1[0]; t2v12d[1]=t2v1[2];
      t2v22d[0]=t2v2[0]; t2v22d[1]=t2v2[2];
      t2v32d[0]=t2v3[0]; t2v32d[1]=t2v3[2];
    } else {
      t1v12d[0]=t1v1[0]; t1v12d[1]=t1v1[1];
      t1v22d[0]=t1v2[0]; t1v22d[1]=t1v2[1];
      t1v32d[0]=t1v3[0]; t1v32d[1]=t1v3[1];
      t2v12d[0]=t2v1[0]; t2v12d[1]=t2v1[1];
      t2v22d[0]=t2v2[0]; t2v22d[1]=t2v2[1];
      t2v32d[0]=t2v3[0]; t2v32d[1]=t2v3[1];
    }
    /* we may assume that none of the triangles is fully contained
     * within the other. Thus, it suffices to do a lot of 2D line-line
     * intersections */
    if (lineIntersectionTest(t1v12d, t1v22d, t2v12d, t2v22d) ||
        lineIntersectionTest(t1v22d, t1v32d, t2v12d, t2v22d) ||
        lineIntersectionTest(t1v32d, t1v12d, t2v12d, t2v22d) ||
        lineIntersectionTest(t1v12d, t1v22d, t2v22d, t2v32d) ||
        lineIntersectionTest(t1v22d, t1v32d, t2v22d, t2v32d) ||
        lineIntersectionTest(t1v32d, t1v12d, t2v22d, t2v32d) ||
        lineIntersectionTest(t1v12d, t1v22d, t2v32d, t2v12d) ||
        lineIntersectionTest(t1v22d, t1v32d, t2v32d, t2v12d) ||
        lineIntersectionTest(t1v32d, t1v12d, t2v32d, t2v12d))
      return 1;
    return 0;
  } else {
    /* pointers to the vertices on the same side / opposite side of plane */
    double *t2s11, *t2s12, *t2s2, t2s11sd, t2s12sd, t2s2sd;
    double t1sd1, t1sd2, t1sd3;
    double *t1s11, *t1s12, *t1s2, t1s11sd, t1s12sd, t1s2sd;
    double t1p11, t1p12, t1p2, t2p11, t2p12, t2p2;
    double D[3]; /* direction vector of line */
    double t1t1, t1t2, t2t1, t2t2;
    if (t2sd1*t2sd2>=0 && t2sd1*t2sd3<=0) {
      t2s11=t2v1; t2s12=t2v2; t2s2=t2v3; t2s11sd=t2sd1;
      t2s12sd=t2sd2; t2s2sd=t2sd3;
    } else if (t2sd1*t2sd3>=0 && t2sd1*t2sd2<=0) {
      t2s11=t2v1; t2s12=t2v3; t2s2=t2v2; t2s11sd=t2sd1;
      t2s12sd=t2sd3; t2s2sd=t2sd2;
    } else if (t2sd2*t2sd3>=0 && t2sd1*t2sd2<=0) {
      t2s11=t2v2; t2s12=t2v3; t2s2=t2v1; t2s11sd=t2sd2;
      t2s12sd=t2sd3; t2s2sd=t2sd1;
    } else
      return 0; /* all on the same side; no intersection */
    
    /* same game for triangle 2 */
    ELL_3V_SUB(diff1, t2v2, t2v1);
    ELL_3V_SUB(diff2, t2v3, t2v1);
    ELL_3V_CROSS(n2, diff1, diff2);
    d2=-ELL_3V_DOT(n2,t2v1);
    t1sd1 = ELL_3V_DOT(n2, t1v1)+d2;
    t1sd2 = ELL_3V_DOT(n2, t1v2)+d2;
    t1sd3 = ELL_3V_DOT(n2, t1v3)+d2;
    if (t1sd1*t1sd2>=0 && t1sd1*t1sd3<=0) {
      t1s11=t1v1; t1s12=t1v2; t1s2=t1v3; t1s11sd=t1sd1;
      t1s12sd=t1sd2; t1s2sd=t1sd3;
    } else if (t1sd1*t1sd3>=0 && t1sd1*t1sd2<=0) {
      t1s11=t1v1; t1s12=t1v3; t1s2=t1v2; t1s11sd=t1sd1;
      t1s12sd=t1sd3; t1s2sd=t1sd2;
    } else if (t1sd2*t1sd3>=0 && t1sd1*t1sd2<=0) {
      t1s11=t1v2; t1s12=t1v3; t1s2=t1v1; t1s11sd=t1sd2;
      t1s12sd=t1sd3; t1s2sd=t1sd1;
    } else
      return 0; /* all on the same side; no intersection */
    
    /* both planes intersect in a line; check if the intervals on that
     * line intersect */
    ELL_3V_CROSS(D,n1,n2);
    /* we are only interested in component magnitudes */
    D[0]=fabs(D[0]); D[1]=fabs(D[1]); D[2]=fabs(D[2]);
    if (D[0]>=D[1] && D[0]>=D[2]) {
      t1p11=t1s11[0]; t1p12=t1s12[0]; t1p2=t1s2[0];
      t2p11=t2s11[0]; t2p12=t2s12[0]; t2p2=t2s2[0];
    } else if (D[1]>=D[0] && D[1]>=D[2]) {
      t1p11=t1s11[1]; t1p12=t1s12[1]; t1p2=t1s2[1];
      t2p11=t2s11[1]; t2p12=t2s12[1]; t2p2=t2s2[1];
    } else {
      t1p11=t1s11[2]; t1p12=t1s12[2]; t1p2=t1s2[2];
      t2p11=t2s11[2]; t2p12=t2s12[2]; t2p2=t2s2[2];
    }
    /* compute interval boundaries */
    t1t1=t1p11+(t1p2-t1p11)*t1s11sd/(t1s11sd-t1s2sd);
    t1t2=t1p12+(t1p2-t1p12)*t1s12sd/(t1s12sd-t1s2sd);
    if (t1t1>t1t2) {
      double help=t1t1;
      t1t1=t1t2;
      t1t2=help;
    }
    t2t1=t2p11+(t2p2-t2p11)*t2s11sd/(t2s11sd-t2s2sd);
    t2t2=t2p12+(t2p2-t2p12)*t2s12sd/(t2s12sd-t2s2sd);
    if (t2t1>t2t2) {
      double help=t2t1;
      t2t1=t2t2;
      t2t2=help;
    }
    /* test for interval intersection */
    if (t2t1>t1t2 || t1t1>t2t2) return 0;
    return 1;
  }
}

/* Score possible local topologies based on the agreement of
 * connecting lines with normal directions. Lower scores are
 * better. */

/* Connections between degenerate points on cell faces; if only four
 * degenerate points are present, set p31 to NULL */
static double
evaluateDegConnection(double *p11, double *p12, double *p13, double *p14,
                      double *p21, double *p22, double *p23, double *p24,
                      double *p31, double *p32, double *p33, double *p34,
                      double *n12, double *n13, double *n22, double *n23,
                      double *n32, double *n33) {
  double diff1[3], diff2[3], diff3[3], ret;
  /* first, perform intersection testing */
  if (triIntersectionTest(p11, p12, p13, p21, p22, p23) ||
      triIntersectionTest(p13, p14, p11, p21, p22, p23) ||
      triIntersectionTest(p11, p12, p13, p23, p24, p21) ||
      triIntersectionTest(p13, p14, p11, p23, p24, p21))
    return 1e20;
  if (p31 != NULL) { /* three pairs - some more to do */
    if (triIntersectionTest(p11, p12, p13, p31, p32, p33) ||
        triIntersectionTest(p11, p12, p13, p33, p34, p31) ||
        triIntersectionTest(p13, p14, p11, p31, p32, p33) ||
        triIntersectionTest(p13, p14, p11, p33, p34, p31) ||
        triIntersectionTest(p21, p22, p23, p31, p32, p33) ||
        triIntersectionTest(p21, p22, p23, p33, p34, p31) ||
        triIntersectionTest(p23, p24, p21, p31, p32, p33) ||
        triIntersectionTest(p23, p24, p21, p33, p34, p31))
      return 1e20;
  }
  ELL_3V_SUB(diff1,p13,p12);
  ELL_3V_SUB(diff2,p23,p22);
  ret=fabs(ELL_3V_DOT(diff1,n12))+fabs(ELL_3V_DOT(diff1,n13))+
    fabs(ELL_3V_DOT(diff2,n22))+fabs(ELL_3V_DOT(diff2,n23));
  if (p31 != NULL) {
    ELL_3V_SUB(diff3,p33,p32);
    ret+=fabs(ELL_3V_DOT(diff3,n32))+fabs(ELL_3V_DOT(diff3,n33));
  }
  return ret;
}

/* suggests a connectivity for a non-trivial combination of
 * intersection points in the plane.
 * pairs is output (permutation of idcs) 
 * bestval is input/output (best value so far, start with something big)
 * ct is the number of points (currently assumed even)
 * idcs is input (idcs that still need to be permuted)
 * fixedct is input (number of idcs that are already fixed at this depth)
 * coords is an array of 2D coordinates
 * norms is an array of 2D vectors */
static void
findConnectivity(signed char *pairs, double *bestval, int ct, char *idcs,
                 int fixedct, double *coords, double *norms) {
  int i,j;
  if (fixedct==ct) {
    double weight=0;
    for (i=0; i<ct-1; i+=2) {
      double diff[2];
      ELL_2V_SUB(diff,coords+2*idcs[i],coords+2*idcs[i+1]);
      weight+=fabs(ELL_2V_DOT(diff,norms+2*idcs[i]))+
        fabs(ELL_2V_DOT(diff,norms+2*idcs[i+1]));
    }
    if (weight<*bestval) {
      *bestval=weight;
      memcpy(pairs,idcs,sizeof(char)*ct);
    }
    return;
  }
  /* else: we need a recursion */
  for (i=fixedct+1; i<ct; i++) {
    int intersect=0;
    char *idxnew;
    if (NULL == (idxnew = (char*) malloc (sizeof(char)*ct)))
      return;
    memcpy(idxnew,idcs,sizeof(char)*ct);
    /* try any of the remaining indices as a pair */
    idxnew[fixedct+1]=idcs[i];
    idxnew[i]=idcs[fixedct+1];
    /* check if the resulting line causes an intersection */
    for (j=0;j<fixedct;j+=2) {
      if (lineIntersectionTest(coords+2*idxnew[fixedct],
                               coords+2*idxnew[fixedct+1],
                               coords+2*idxnew[j],coords+2*idxnew[j+1])) {
        intersect=1;
        break;
      }
    }
    if (!intersect) {
      findConnectivity(pairs, bestval, ct, idxnew, fixedct+2, coords, norms);
    }
    free(idxnew);
  }
}

#define _SEEK_TREATED_REQUEST 0x01 /* this voxel has to be treated */
#define _SEEK_TREATED_EDGE0 0x02 /* unique edge 0 has been treated */
#define _SEEK_TREATED_EDGE1 0x04 /* unique edge 1 has been treated */
#define _SEEK_TREATED_EDGE2 0x08 /* unique edge 2 has been treated */
#define _SEEK_TREATED_EDGE3 0x10 /* unique edge 3 has been treated */
#define _SEEK_TREATED_EDGE4 0x20 /* unique edge 4 has been treated */
#define _SEEK_TREATED_FACE3 0x40 /* unique face 3 has been treated */

/* find deg. points, normals, and connectivity on a given (unique) face
 * now refines the search if it couldn't find a degenerate point */
static void
connectFace(seekContext *sctx, baggage *bag,
            unsigned int xi, unsigned int yi, unsigned char faceid) {
  int edgeid[4][4]={{0, 2, 3, 1}, /* which edges belong to which unique face? */
                    {0, 5, 8, 4},
                    {1, 6, 9, 4},
                    {8,10,11, 9}};
  unsigned int sx = AIR_CAST(unsigned int, sctx->sx);
  unsigned int si = xi + sx*yi;
  unsigned int six = xi + 1 + sx*yi;
  unsigned int siy = xi + sx*(yi+1);
  unsigned int sixy = xi + 1 + sx*(yi+1);
  char inter[12]; /* indices of intersections */
  int pass; /* allow multiple refined passes */
  int i;
  /* voxel in which treated information is stored for local edge i */
  /* which vertices form which unique face? */
  int verti[4][4];
  int voxel[4][4];
  /* mask for treated information in the above voxel */
  char mask[4][4]={{_SEEK_TREATED_EDGE0, _SEEK_TREATED_EDGE1,
                    _SEEK_TREATED_EDGE0, _SEEK_TREATED_EDGE1},
                   {_SEEK_TREATED_EDGE0, _SEEK_TREATED_EDGE2,
                    _SEEK_TREATED_EDGE3, _SEEK_TREATED_EDGE2},
                   {_SEEK_TREATED_EDGE1, _SEEK_TREATED_EDGE2,
                    _SEEK_TREATED_EDGE4, _SEEK_TREATED_EDGE2},
                   {_SEEK_TREATED_EDGE3, _SEEK_TREATED_EDGE4,
                    _SEEK_TREATED_EDGE3, _SEEK_TREATED_EDGE4}};
  /* start- and endpoints of the edges */
  int verts[4][4], verte[4][4];
  char treat[4]={0,0,0,0};
  double dpthresh[3]={0.7,0.8,0.9};
  /* candidates for degenerate points */
  double candidates[18]={0.5,0.5,  0.25,0.25,  0.25,0.75,
                         0.75,0.25,  0.75,0.75,  0.5,0.25,
                         0.25,0.5,  0.75,0.5,  0.6,0.75};
  int cand_idx[4]={0, 1, 5, 9};
  int interct;
  
  /* apparently, some C compilers cannot make these initializations in-place */
  ELL_4V_SET(verti[0], 0+2*si, 0+2*six, 0+2*siy, 0+2*sixy);
  ELL_4V_SET(verti[1], 0+2*si, 0+2*six, 1+2*si, 1+2*six);
  ELL_4V_SET(verti[2], 0+2*si, 0+2*siy, 1+2*si, 1+2*siy);
  ELL_4V_SET(verti[3], 1+2*si, 1+2*six, 1+2*siy, 1+2*sixy);
  ELL_4V_SET(voxel[0], si, six, siy, si);
  ELL_4V_SET(voxel[1], si, six, si, si);
  ELL_4V_SET(voxel[2], si, siy, si, si);
  ELL_4V_SET(voxel[3], si, six, siy, si);
  ELL_4V_SET(verts[0], 0+2*si, 0+2*six, 0+2*siy, 0+2*si);
  ELL_4V_SET(verts[1], 0+2*si, 0+2*six, 1+2*si, 0+2*si);
  ELL_4V_SET(verts[2], 0+2*si, 0+2*siy, 1+2*si, 0+2*si);
  ELL_4V_SET(verts[3], 1+2*si, 1+2*six, 1+2*siy, 1+2*si);
  ELL_4V_SET(verte[0], 0+2*six, 0+2*sixy, 0+2*sixy, 0+2*siy);
  ELL_4V_SET(verte[1], 0+2*six, 1+2*six, 1+2*six, 1+2*si);
  ELL_4V_SET(verte[2], 0+2*siy, 1+2*siy, 1+2*siy, 1+2*si);
  ELL_4V_SET(verte[3], 1+2*six, 1+2*sixy, 1+2*sixy, 1+2*siy);
  
  /* find out which edges have not yet been treated */
  for (i=0; i<4; i++) {
    if (!(sctx->treated[voxel[faceid][i]]&mask[faceid][i])) {
      treat[i]=1; /* we need to treat this */
      sctx->treated[voxel[faceid][i]] |= mask[faceid][i];
    }
  }
  
  for (pass=0; pass<3; pass++) {
    /* first, find intersections for edges that need treatment */
    int j;
    for (j=0; j<4; j++) {
      double interpos[8];
      if (!treat[j]) continue;
      interct=findFeatureIntersection(interpos,
                                      sctx->t + 9*verts[faceid][j],
                                      sctx->hess + 9*verts[faceid][j],
                                      sctx->grad + 3*verts[faceid][j],
                                      sctx->t + 9*verte[faceid][j],
                                      sctx->hess + 9*verte[faceid][j],
                                      sctx->grad + 3*verte[faceid][j],
                                      0.0, 1.0, bag->esIdx==2,
                                      sctx->evalDiffThresh,
                                      dpthresh[pass]);
      if (interct>3) interct=3;
      for (i=0; i<interct; i++) {
        double x=0, y=0, z=0; unsigned int xb=0, yb=0, idb=0;
        sctx->edgealpha[3*(bag->evti[edgeid[faceid][j]]+5*si)+i] = interpos[i];
        switch (edgeid[faceid][j]) {
        case 0: x=xi+interpos[i]; y=yi; z=bag->zi;
          xb=xi; yb=yi; idb=0; break;
        case 1: x=xi; y=yi+interpos[i]; z=bag->zi;
          xb=xi; yb=yi; idb=1; break;
        case 2: x=xi+1; y=yi+interpos[i]; z=bag->zi;
          xb=xi+1; yb=yi; idb=1; break;
        case 3: x=xi+interpos[i]; y=yi+1; z=bag->zi;
          xb=xi; yb=yi+1; idb=0; break;
        case 4: x=xi; y=yi; z=bag->zi+interpos[i];
          xb=xi; yb=yi; idb=2; break;
        case 5: x=xi+1; y=yi; z=bag->zi+interpos[i];
          xb=xi+1; yb=yi; idb=2; break;
        case 6: x=xi; y=yi+1; z=bag->zi+interpos[i];
          xb=xi; yb=yi+1; idb=2; break;
        case 7: x=xi+1; y=yi+1; z=bag->zi+interpos[i];
          xb=xi+1; yb=yi+1; idb=2; break;
        case 8: x=xi+interpos[i]; y=yi; z=bag->zi+1;
          xb=xi; yb=yi; idb=3; break;
        case 9: x=xi; y=yi+interpos[i]; z=bag->zi+1;
          xb=xi; yb=yi; idb=4; break;
        case 10: x=xi+1; y=yi+interpos[i]; z=bag->zi+1;
          xb=xi+1; yb=yi; idb=4; break;
        case 11: x=xi+interpos[i]; y=yi+1; z=bag->zi+1;
          xb=xi; yb=yi+1; idb=3; break;
        }
        ELL_3V_SET(sctx->edgeicoord+9*(bag->evti[edgeid[faceid][j]]+5*si)+3*i,
                   x, y, z);
        computeEdgeGradient(sctx, bag, sctx->edgenorm+
                            9*(bag->evti[edgeid[faceid][j]]+5*si)+3*i,
                            xb, yb, idb, interpos[i]);
      }
    }
    
    interct=0; /* number of feature intersections */
    for (i=0; i<3; i++) {
      if (sctx->edgealpha[3*(bag->evti[edgeid[faceid][0]]+5*si)+i]>=0)
        inter[interct++]=i; /* numbering is local w.r.t. face */
      if (sctx->edgealpha[3*(bag->evti[edgeid[faceid][1]]+5*si)+i]>=0)
        inter[interct++]=3+i;
      if (sctx->edgealpha[3*(bag->evti[edgeid[faceid][2]]+5*si)+i]>=0)
        inter[interct++]=6+i;
      if (sctx->edgealpha[3*(bag->evti[edgeid[faceid][3]]+5*si)+i]>=0)
        inter[interct++]=9+i;
    }
    if (interct%2==1) { /* we need to look for a degeneracy */
      int k;
      for (k=cand_idx[pass]; k<cand_idx[pass+1]; k++) {
        ELL_2V_SET(sctx->facecoord+2*(faceid+4*si),
                   candidates[2*k], candidates[2*k+1]);
        if (!seekDescendToDeg(sctx->facecoord+2*(faceid+4*si),
                              sctx->hess + 9*verti[faceid][0],
                              sctx->hess + 9*verti[faceid][1],
                              sctx->hess + 9*verti[faceid][2],
                              sctx->hess + 9*verti[faceid][3],
                              50, 1e-9, (bag->esIdx==2)?'l':'p')) {
          inter[interct++]=12; /* 12 means "deg. point on this face */
          break;
        }
      }
      if ((pass==2) && (inter[interct-1]!=12)) {
        /* nothing helped, so insert a dummy vertex */
        ELL_2V_SET(sctx->facecoord+2*(faceid+4*si), 0.5, 0.5);
        inter[interct++]=12;
      }
      if (inter[interct-1]==12) {
        computeFaceGradient(sctx, sctx->facenorm+3*(faceid+4*si),
                            xi, yi, faceid, sctx->facecoord+2*(faceid+4*si));
        switch (faceid) {
        case 0: ELL_3V_SET(sctx->faceicoord+3*(faceid+4*si),
                           xi+sctx->facecoord[2*(faceid+4*si)],
                           yi+sctx->facecoord[2*(faceid+4*si)+1], bag->zi);
          break;
        case 1: ELL_3V_SET(sctx->faceicoord+3*(faceid+4*si),
                           xi+sctx->facecoord[2*(faceid+4*si)], yi,
                           bag->zi+sctx->facecoord[2*(faceid+4*si)+1]);
          break;
        case 2: ELL_3V_SET(sctx->faceicoord+3*(faceid+4*si), xi,
                           yi+sctx->facecoord[2*(faceid+4*si)],
                           bag->zi+sctx->facecoord[2*(faceid+4*si)+1]);
          break;
        case 3: ELL_3V_SET(sctx->faceicoord+3*(faceid+4*si),
                           xi+sctx->facecoord[2*(faceid+4*si)],
                           yi+sctx->facecoord[2*(faceid+4*si)+1], bag->zi+1);
          break;
        }
      }
    }
    if (interct%2==0) { /* we can break out */
      break;
    }
  }
  
  if (interct<=1) { /* there is no connectivity on this face */
    ELL_4V_SET(sctx->pairs+12*(faceid+4*si),-1,-1,-1,-1);
  } else if (interct==2) { /* connectivity is straightforward */
    ELL_4V_SET(sctx->pairs+12*(faceid+4*si),inter[0],inter[1],-1,-1);
  } else { /* we need gradients and coordinates to make a decision */
    double interc[24]; /* 2D coordinates of intersection points */
    double intern[24]; /* 2D normals at intersection points */
    /* used to find the best pairing without self-intersection */
    double bestscore=1e20;
    char idcs[12]; /* consider if we need to restrict this */
    for (i=0; i<interct; i++) {
      if (inter[i]<12) { /* edge feature */
        int resolved=edgeid[faceid][inter[i]/3];
        int offset=inter[i]%3;
        switch (faceid) {
        case 0: case 3:
          ELL_2V_SET(interc+2*i,
                     sctx->edgeicoord[9*(bag->evti[resolved]+5*si)+3*offset],
                     sctx->edgeicoord[9*(bag->evti[resolved]+5*si)+3*offset+1]);
          ELL_2V_SET(intern+2*i,
                     sctx->edgenorm[9*(bag->evti[resolved]+5*si)+3*offset],
                     sctx->edgenorm[9*(bag->evti[resolved]+5*si)+3*offset+1]);
          break;
        case 1:
          ELL_2V_SET(interc+2*i,
                     sctx->edgeicoord[9*(bag->evti[resolved]+5*si)+3*offset],
                     sctx->edgeicoord[9*(bag->evti[resolved]+5*si)+3*offset+2]);
          ELL_2V_SET(intern+2*i,
                     sctx->edgenorm[9*(bag->evti[resolved]+5*si)+3*offset],
                     sctx->edgenorm[9*(bag->evti[resolved]+5*si)+3*offset+2]);
          break;
        case 2:
          ELL_2V_SET(interc+2*i,
                     sctx->edgeicoord[9*(bag->evti[resolved]+5*si)+3*offset+1],
                     sctx->edgeicoord[9*(bag->evti[resolved]+5*si)+3*offset+2]);
          ELL_2V_SET(intern+2*i,
                     sctx->edgenorm[9*(bag->evti[resolved]+5*si)+3*offset+1],
                     sctx->edgenorm[9*(bag->evti[resolved]+5*si)+3*offset+2]);
          break;   
        }
      } else { /* face feature */
        switch (faceid) {
        case 0: case 3:
          ELL_2V_SET(interc+2*i, sctx->faceicoord[3*(faceid+4*si)],
                     sctx->faceicoord[3*(faceid+4*si)+1]);
          ELL_2V_SET(intern+2*i, sctx->facenorm[3*(faceid+4*si)],
                     sctx->facenorm[3*(faceid+4*si)+1]);
          break;
        case 1:
          ELL_2V_SET(interc+2*i, sctx->faceicoord[3*(faceid+4*si)],
                     sctx->faceicoord[3*(faceid+4*si)+2]);
          ELL_2V_SET(intern+2*i, sctx->facenorm[3*(faceid+4*si)],
                     sctx->facenorm[3*(faceid+4*si)+2]);
          break;
        case 2:
          ELL_2V_SET(interc+2*i, sctx->faceicoord[3*(faceid+4*si)+1],
                     sctx->faceicoord[3*(faceid+4*si)+2]);
          ELL_2V_SET(intern+2*i, sctx->facenorm[3*(faceid+4*si)+1],
                     sctx->facenorm[3*(faceid+4*si)+2]);
          break;   
        }
      }
    }
    for (i=0; i<interct; i++) {
      sctx->pairs[12*(faceid+4*si)+i]=i;
      idcs[i]=i;
    }
    findConnectivity(sctx->pairs+12*(faceid+4*si), &bestscore, interct,
                     idcs, 0, interc, intern);
    for (i=0; i<interct; i++)
      sctx->pairs[12*(faceid+4*si)+i]=inter[sctx->pairs[12*(faceid+4*si)+i]];
    for (i=interct; i<12; i++)
      sctx->pairs[12*(faceid+4*si)+i]=-1;
  }
}

static void
intersectionShuffleProbe(seekContext *sctx, baggage *bag) {
  unsigned int xi, yi, sx, sy, si;
  int i;
  
  sx = AIR_CAST(unsigned int, sctx->sx);
  sy = AIR_CAST(unsigned int, sctx->sy);
  
  for (yi=0; yi<sy; yi++) {
    for (xi=0; xi<sx; xi++) {
      si = xi + sx*yi;
      /* take care of facevidx array */
      if (!bag->zi) { /* initialize, else copy over */
        sctx->facevidx[0 + 4*si] = -1;
      } else {
        sctx->facevidx[0 + 4*si] = sctx->facevidx[3 + 4*si];
      }
      sctx->facevidx[1 + 4*si] = sctx->facevidx[2 + 4*si] =
        sctx->facevidx[3 + 4*si] = -1;
      
      /* copy or reset data on the 5 unique edges */
      
      if (sctx->treated[si]&_SEEK_TREATED_EDGE3) {
        /* has been treated, just copy results */
        ELL_3V_COPY(sctx->edgealpha+3*(0+5*si), sctx->edgealpha+3*(3+5*si));
        ELL_3M_COPY(sctx->edgenorm+9*(0+5*si), sctx->edgenorm+9*(3+5*si));
        ELL_3M_COPY(sctx->edgeicoord+9*(0+5*si), sctx->edgeicoord+9*(3+5*si));
        sctx->treated[si]|=_SEEK_TREATED_EDGE0;
      } else if (xi!=sx-1) {
        ELL_3V_SET(sctx->edgealpha+3*(0+5*si),-1,-1,-1);
        sctx->treated[si]&=0xFF^_SEEK_TREATED_EDGE0;
      }
      
      if (sctx->treated[si]&_SEEK_TREATED_EDGE4) {
        /* has been treated, just copy results */
        ELL_3V_COPY(sctx->edgealpha+3*(1+5*si), sctx->edgealpha+3*(4+5*si));
        ELL_3M_COPY(sctx->edgenorm+9*(1+5*si), sctx->edgenorm+9*(4+5*si));
        ELL_3M_COPY(sctx->edgeicoord+9*(1+5*si), sctx->edgeicoord+9*(4+5*si));
        sctx->treated[si]|=_SEEK_TREATED_EDGE1;
      } else if (yi!=sy-1) {
        ELL_3V_SET(sctx->edgealpha+3*(1+5*si),-1,-1,-1);
        sctx->treated[si]&=0xFF^_SEEK_TREATED_EDGE1;
      }
      
      /* edges within and at top of the slab are new */
      ELL_3V_SET(sctx->edgealpha+3*(2+5*si),-1,-1,-1);
      sctx->treated[si]&=0xFF^_SEEK_TREATED_EDGE2;
      
      ELL_3V_SET(sctx->edgealpha+3*(3+5*si),-1,-1,-1);
      sctx->treated[si]&=0xFF^_SEEK_TREATED_EDGE3;
      
      ELL_3V_SET(sctx->edgealpha+3*(4+5*si),-1,-1,-1);
      sctx->treated[si]&=0xFF^_SEEK_TREATED_EDGE4;
    }
  }
  
  /* find missing deg. points, edge intersections, normals, and
   * connectivity on the four unique faces
   * this is done in a separate pass to make sure that all edge information
   * has been updated */
  for (yi=0; yi<sy; yi++) {
    for (xi=0; xi<sx; xi++) {
      si = xi + sx*yi;
      if (sctx->treated[si]&_SEEK_TREATED_FACE3) {
        /* we can copy previous results */
        ELL_2V_COPY(sctx->facecoord+2*(0+4*si), sctx->facecoord+2*(3+4*si));
        ELL_3V_COPY(sctx->faceicoord+3*(0+4*si), sctx->faceicoord+3*(3+4*si));
        ELL_3V_COPY(sctx->facenorm+3*(0+4*si), sctx->facenorm+3*(3+4*si));
        for (i=0; i<3; i++)
          ELL_4V_COPY(sctx->pairs+12*(0+4*si)+4*i, sctx->pairs+12*(3+4*si)+4*i);
      } else if (xi!=sx-1 && yi!=sy-1) {
        if (sctx->treated[si]&_SEEK_TREATED_REQUEST)
          connectFace(sctx, bag, xi, yi, 0);
        else
          ELL_4V_SET(sctx->pairs+12*(0+4*si),-1,-1,-1,-1);
      }
      if (xi!=sx-1) {
        if (sctx->treated[si]&_SEEK_TREATED_REQUEST ||
            (yi!=0 && sctx->treated[xi+sx*(yi-1)]&_SEEK_TREATED_REQUEST))
          connectFace(sctx, bag, xi, yi, 1);
        else ELL_4V_SET(sctx->pairs+12*(1+4*si),-1,-1,-1,-1);
      }      
      if (yi!=sy-1) {
        if (sctx->treated[si]&_SEEK_TREATED_REQUEST ||
            (xi!=0 && sctx->treated[xi-1+sx*yi]&_SEEK_TREATED_REQUEST))
          connectFace(sctx, bag, xi, yi, 2);
        else ELL_4V_SET(sctx->pairs+12*(2+4*si),-1,-1,-1,-1);
        if (xi!=sx-1) {
          if (sctx->treated[si]&_SEEK_TREATED_REQUEST) {
            connectFace(sctx, bag, xi, yi, 3);
            sctx->treated[si]|=_SEEK_TREATED_FACE3;
          } else {
            ELL_4V_SET(sctx->pairs+12*(3+4*si),-1,-1,-1,-1);
            sctx->treated[si]&=0xFF^_SEEK_TREATED_FACE3;
          }
        }
      }
    }
  }
}

/* special triangulation routine for use with T-based extraction */
int
_seekTriangulateT(seekContext *sctx, baggage *bag, limnPolyData *lpld) {
  unsigned xi, yi, sx, sy, si, i;
  
  /* map edge indices w.r.t. faces (as used in sctx->pairs) back to
   * edge indices w.r.t. voxel */
  char edges[6][5]={{0, 2, 3, 1,12},
                    {0, 5, 8, 4,13},
                    {2, 7,10, 5,14},
                    {3, 7,11, 6,15},
                    {1, 6, 9, 4,16},
                    {8,10,11, 9,17}};
  
  sx = AIR_CAST(unsigned int, sctx->sx);
  sy = AIR_CAST(unsigned int, sctx->sy);
  
  for (yi=0; yi<sy-1; yi++) {
    for (xi=0; xi<sx-1; xi++) {
      int fvti[6]; /* indices into unique face array */
      char connections[84];/* (12 edges * 3 possible intersections+6 faces)*2 */
      char degeneracies[6];
      int degct=0;
      
      unsigned int face;
      
      if (sctx->strengthUse && sctx->stng[0+2*(xi+sx*yi)] < sctx->strength &&
          sctx->stng[1+2*(xi+sx*yi)] < sctx->strength &&
          sctx->stng[0+2*(xi+1+sx*yi)] < sctx->strength &&
          sctx->stng[1+2*(xi+1+sx*yi)] < sctx->strength &&
          sctx->stng[0+2*(xi+sx*(yi+1))] < sctx->strength &&
          sctx->stng[1+2*(xi+sx*(yi+1))] < sctx->strength &&
          sctx->stng[0+2*(xi+1+sx*(yi+1))] < sctx->strength &&
          sctx->stng[1+2*(xi+1+sx*(yi+1))] < sctx->strength)
        continue;/* all vertices below strength limit, do not create geometry */
      
      si = xi + sx*yi;
      ELL_3V_SET(fvti, 0 + 4*si, 1 + 4*si, 2 + 4*(xi+1 + sx*yi));
      ELL_3V_SET(fvti+3, 1 + 4*(xi + sx*(yi+1)), 2 + 4*si, 3 + 4*si);
      
      /* collect all intersection + connectivity info for this voxel */
      memset(connections,-1,sizeof(connections));
      for (face=0; face<6; face++) {
        for (i=0; i<6; i++) {
          int idx1, offset1, idx2, offset2, idxmap1, idxmap2;
          if (sctx->pairs[12*fvti[face]+2*i]==-1) break;
          idx1=edges[face][sctx->pairs[12*fvti[face]+2*i]/3];
          offset1=sctx->pairs[12*fvti[face]+2*i]%3;
          idx2=edges[face][sctx->pairs[12*fvti[face]+2*i+1]/3];
          offset2=sctx->pairs[12*fvti[face]+2*i+1]%3;
          idxmap1=3*idx1+offset1;
          idxmap2=3*idx2+offset2;
          if (idx1>11) {
            idxmap1=idx1+24; /* +36-12 */
            degeneracies[degct++] = idxmap1;
          }
          if (idx2>11) {
            idxmap2=idx2+24;
            degeneracies[degct++] = idxmap2;
          }
          
          if (connections[2*idxmap1]==-1)
            connections[2*idxmap1]=idxmap2;
          else
            connections[2*idxmap1+1]=idxmap2;
          if (connections[2*idxmap2]==-1)
            connections[2*idxmap2]=idxmap1;
          else
            connections[2*idxmap2+1]=idxmap1;
        }
      }
      
      /* connect the degenerate points */
      if (degct==2) {
        connections[2*degeneracies[0]+1]=degeneracies[1];
        connections[2*degeneracies[1]+1]=degeneracies[0];
      } else if (degct==4) {
        int bestchoice=0;
        int eidcs[4], fidcs[4];
        int k;
        double bestscore, score;
        for (k=0; k<4; ++k) {
          eidcs[k]=3*(bag->evti[connections[2*degeneracies[k]]/3]+5*si)+
            connections[2*degeneracies[k]]%3;
          fidcs[k]=fvti[degeneracies[k]-36];
        }
        bestscore=evaluateDegConnection(sctx->edgeicoord+3*eidcs[0],
                                        sctx->faceicoord+3*fidcs[0],
                                        sctx->faceicoord+3*fidcs[1],
                                        sctx->edgeicoord+3*eidcs[1],
                                        sctx->edgeicoord+3*eidcs[2],
                                        sctx->faceicoord+3*fidcs[2],
                                        sctx->faceicoord+3*fidcs[3],
                                        sctx->edgeicoord+3*eidcs[3],
                                        NULL, NULL, NULL, NULL,
                                        sctx->facenorm+3*fidcs[0],
                                        sctx->facenorm+3*fidcs[1],
                                        sctx->facenorm+3*fidcs[2],
                                        sctx->facenorm+3*fidcs[3],
                                        NULL, NULL);
        score=evaluateDegConnection(sctx->edgeicoord+3*eidcs[0],
                                    sctx->faceicoord+3*fidcs[0],
                                    sctx->faceicoord+3*fidcs[2],
                                    sctx->edgeicoord+3*eidcs[2],
                                    sctx->edgeicoord+3*eidcs[1],
                                    sctx->faceicoord+3*fidcs[1],
                                    sctx->faceicoord+3*fidcs[3],
                                    sctx->edgeicoord+3*eidcs[3],
                                    NULL, NULL, NULL, NULL,
                                    sctx->facenorm+3*fidcs[0],
                                    sctx->facenorm+3*fidcs[2],
                                    sctx->facenorm+3*fidcs[1],
                                    sctx->facenorm+3*fidcs[3],
                                    NULL, NULL);
        if (score<bestscore) {
          bestscore=score;
          bestchoice=1;
        }
        score=evaluateDegConnection(sctx->edgeicoord+3*eidcs[0],
                                    sctx->faceicoord+3*fidcs[0],
                                    sctx->faceicoord+3*fidcs[3],
                                    sctx->edgeicoord+3*eidcs[3],
                                    sctx->edgeicoord+3*eidcs[1],
                                    sctx->faceicoord+3*fidcs[1],
                                    sctx->faceicoord+3*fidcs[2],
                                    sctx->edgeicoord+3*eidcs[2],
                                    NULL, NULL, NULL, NULL,
                                    sctx->facenorm+3*fidcs[0],
                                    sctx->facenorm+3*fidcs[3],
                                    sctx->facenorm+3*fidcs[1],
                                    sctx->facenorm+3*fidcs[2],
                                    NULL, NULL);
        if (score<bestscore) {
          bestscore=score;
          bestchoice=2;
        }
        switch (bestchoice) {
        case 0: connections[2*degeneracies[0]+1]=degeneracies[1];
          connections[2*degeneracies[1]+1]=degeneracies[0];
          connections[2*degeneracies[2]+1]=degeneracies[3];
          connections[2*degeneracies[3]+1]=degeneracies[2];
          break;
        case 1: connections[2*degeneracies[0]+1]=degeneracies[2];
          connections[2*degeneracies[2]+1]=degeneracies[0];
          connections[2*degeneracies[1]+1]=degeneracies[3];
          connections[2*degeneracies[3]+1]=degeneracies[1];
          break;
        case 2: connections[2*degeneracies[0]+1]=degeneracies[3];
          connections[2*degeneracies[3]+1]=degeneracies[0];
          connections[2*degeneracies[1]+1]=degeneracies[2];
          connections[2*degeneracies[2]+1]=degeneracies[1];
          break;
        }
      } else if (degct==6) {
        int bestchoice=0;
        int eidcs[6], fidcs[6];
        int k;
        double bestscore;
        int pairings[15][6]={{0,1,2,3,4,5},{0,1,2,4,3,5},{0,1,2,5,3,4},
                             {0,2,1,3,4,5},{0,2,1,4,3,5},{0,2,1,5,3,4},
                             {0,3,1,2,4,5},{0,3,1,4,2,5},{0,3,1,5,2,4},
                             {0,4,1,2,3,5},{0,4,1,3,2,5},{0,4,1,5,2,3},
                             {0,5,1,2,3,4},{0,5,1,3,2,4},{0,5,1,4,2,3}};
        for (k=0; k<6; ++k) {
          eidcs[k]=3*(bag->evti[connections[2*degeneracies[k]]/3]+5*si)+
            connections[2*degeneracies[k]]%3;
          fidcs[k]=fvti[degeneracies[k]-36];
        }
        bestscore=evaluateDegConnection(sctx->edgeicoord+3*eidcs[0],
                                        sctx->faceicoord+3*fidcs[0],
                                        sctx->faceicoord+3*fidcs[1],
                                        sctx->edgeicoord+3*eidcs[1],
                                        sctx->edgeicoord+3*eidcs[2],
                                        sctx->faceicoord+3*fidcs[2],
                                        sctx->faceicoord+3*fidcs[3],
                                        sctx->edgeicoord+3*eidcs[3],
                                        sctx->edgeicoord+3*eidcs[4],
                                        sctx->faceicoord+3*fidcs[4],
                                        sctx->faceicoord+3*fidcs[5],
                                        sctx->edgeicoord+3*eidcs[5],
                                        sctx->facenorm+3*fidcs[0],
                                        sctx->facenorm+3*fidcs[1],
                                        sctx->facenorm+3*fidcs[2],
                                        sctx->facenorm+3*fidcs[3],
                                        sctx->facenorm+3*fidcs[4],
                                        sctx->facenorm+3*fidcs[5]);
        for (k=1; k<15; ++k) {
          double score=evaluateDegConnection
            (sctx->edgeicoord+3*eidcs[pairings[k][0]],
             sctx->faceicoord+3*fidcs[pairings[k][0]],
             sctx->faceicoord+3*fidcs[pairings[k][1]],
             sctx->edgeicoord+3*eidcs[pairings[k][1]],
             sctx->edgeicoord+3*eidcs[pairings[k][2]],
             sctx->faceicoord+3*fidcs[pairings[k][2]],
             sctx->faceicoord+3*fidcs[pairings[k][3]],
             sctx->edgeicoord+3*eidcs[pairings[k][3]],
             sctx->edgeicoord+3*eidcs[pairings[k][4]],
             sctx->faceicoord+3*fidcs[pairings[k][4]],
             sctx->faceicoord+3*fidcs[pairings[k][5]],
             sctx->edgeicoord+3*eidcs[pairings[k][5]],
             sctx->facenorm+3*fidcs[pairings[k][0]],
             sctx->facenorm+3*fidcs[pairings[k][1]],
             sctx->facenorm+3*fidcs[pairings[k][2]],
             sctx->facenorm+3*fidcs[pairings[k][3]],
             sctx->facenorm+3*fidcs[pairings[k][4]],
             sctx->facenorm+3*fidcs[pairings[k][5]]);
          if (score<bestscore) {
            bestscore=score; bestchoice=k;
          }
        }
        connections[2*degeneracies[pairings[bestchoice][0]]+1]=
          degeneracies[pairings[bestchoice][1]];
        connections[2*degeneracies[pairings[bestchoice][1]]+1]=
          degeneracies[pairings[bestchoice][0]];
        connections[2*degeneracies[pairings[bestchoice][2]]+1]=
          degeneracies[pairings[bestchoice][3]];
        connections[2*degeneracies[pairings[bestchoice][3]]+1]=
          degeneracies[pairings[bestchoice][2]];
        connections[2*degeneracies[pairings[bestchoice][4]]+1]=
          degeneracies[pairings[bestchoice][5]];
        connections[2*degeneracies[pairings[bestchoice][5]]+1]=
          degeneracies[pairings[bestchoice][4]];
      }
      
      /* sufficient to run to 36: each polygon will contain at least
       * one edge vertex */
      for (i=0; i<36; i++) {
        if (connections[2*i]!=-1) {
          /* extract polygon from connections array */
          signed char polygon[42];
          unsigned char polyct=0;
          char thiz=i;
          char next=connections[2*i];
          polygon[polyct++]=i;
          connections[2*i]=-1;
          while (next!=-1) {
            char helpnext;
            polygon[polyct++]=next;
            if (connections[2*next]==thiz) {
              helpnext=connections[2*next+1];
            } else {
              helpnext=connections[2*next];
            }
            connections[2*next]=connections[2*next+1]=-1;
            thiz = next; next = helpnext;
            if (next==polygon[0])
              break; /* polygon is closed */
          }
          if (next!=-1) { /* else: discard unclosed polygon */
            /* make sure all required vertices are there */
            int j;
            for (j=0; j<polyct; ++j) {
              double tvertA[4], tvertB[4];
              if (polygon[j]<36) { /* we may need to insert an edge vertex */
                int eidx=3*(bag->evti[polygon[j]/3] + 5*si)+polygon[j]%3;
                if (-1 == sctx->vidx[eidx]) {
                  int ovi;
                  ELL_3V_COPY(tvertA, sctx->edgeicoord+3*eidx);
                  tvertA[3]=1.0;
                  
                  /* tvertB in input index space */
                  ELL_4MV_MUL(tvertB, sctx->txfIdx, tvertA);
                  /* tvertA in world space */
                  ELL_4MV_MUL(tvertA, sctx->shape->ItoW, tvertB);
                  ELL_4V_HOMOG(tvertA, tvertA);
                  
                  ovi = sctx->vidx[eidx] = 
                    airArrayLenIncr(bag->xyzwArr, 1);
                  ELL_4V_SET_TT(lpld->xyzw + 4*ovi, float,
                                tvertA[0], tvertA[1], tvertA[2], 1.0);
                  
                  if (sctx->normalsFind) {
                    double len=ELL_3V_LEN(sctx->edgenorm+3*eidx);
                    airArrayLenIncr(bag->normArr, 1);
                    ELL_3V_SCALE_TT(lpld->norm + 3*ovi, float, 1.0/len,
                                    sctx->edgenorm+3*eidx);
                  }
                  
                  sctx->vertNum++;
                }
              } else { /* we may need to insert a face vertex */
                int fidx=fvti[polygon[j]-36];
                if (-1 == sctx->facevidx[fidx]) {
                  int ovi;
                  ELL_3V_COPY(tvertA, sctx->faceicoord+3*fidx);
                  tvertA[3]=1.0;
                  /* tvertB in input index space */
                  ELL_4MV_MUL(tvertB, sctx->txfIdx, tvertA);
                  /* tvertA in world space */
                  ELL_4MV_MUL(tvertA, sctx->shape->ItoW, tvertB);
                  ELL_4V_HOMOG(tvertA, tvertA);
                  
                  ovi = sctx->facevidx[fidx] = 
                    airArrayLenIncr(bag->xyzwArr, 1);
                  ELL_4V_SET_TT(lpld->xyzw + 4*ovi, float,
                                tvertA[0], tvertA[1], tvertA[2], 1.0);
                  
                  if (sctx->normalsFind) {
                    double len=ELL_3V_LEN(sctx->facenorm+3*fidx);
                    airArrayLenIncr(bag->normArr, 1);
                    ELL_3V_SCALE_TT(lpld->norm + 3*ovi, float, 1.0/len,
                                    sctx->facenorm+3*fidx);
                  }
                  
                  sctx->vertNum++;    
                }
              }
            }
            if (polyct>4) { /* we need to insert a helper vertex */
              double tvertA[4], tvertB[4], tvertAsum[4]={0,0,0,0},
                normsum[3]={0,0,0};
                int ovi;
                unsigned int vii[3];
                
                for (j=0; j<polyct; j++) {
                  if (polygon[j]<36) {
                    int eidx=3*(bag->evti[polygon[j]/3] + 5*si)+polygon[j]%3;
                    ELL_3V_COPY(tvertA, sctx->edgeicoord+3*eidx);
                    tvertA[3]=1.0;
                    ELL_4V_INCR(tvertAsum,tvertA);
                    if (ELL_3V_DOT(normsum,sctx->edgenorm+3*eidx)<0)
                      ELL_3V_SUB(normsum,normsum,sctx->edgenorm+3*eidx);
                    else
                      ELL_3V_INCR(normsum,sctx->edgenorm+3*eidx);
                  } else {
                    int fidx=fvti[polygon[j]-36];
                    ELL_3V_COPY(tvertA, sctx->faceicoord+3*fidx);
                    tvertA[3]=1.0;
                    ELL_4V_INCR(tvertAsum,tvertA);
                    if (ELL_3V_DOT(normsum,sctx->facenorm+3*fidx)<0)
                      ELL_3V_SUB(normsum,normsum,sctx->facenorm+3*fidx);
                    else
                      ELL_3V_INCR(normsum,sctx->facenorm+3*fidx);
                  }
                }
                /* tvertB in input index space */
                ELL_4MV_MUL(tvertB, sctx->txfIdx, tvertAsum);
                /* tvertA in world space */
                ELL_4MV_MUL(tvertA, sctx->shape->ItoW, tvertB);
                ELL_4V_HOMOG(tvertA, tvertA);
                
                ovi = airArrayLenIncr(bag->xyzwArr, 1);
                ELL_4V_SET_TT(lpld->xyzw + 4*ovi, float,
                              tvertA[0], tvertA[1], tvertA[2], 1.0);
                if (sctx->normalsFind) {
                  double len=ELL_3V_LEN(normsum);
                  airArrayLenIncr(bag->normArr, 1);
                  ELL_3V_SCALE_TT(lpld->norm + 3*ovi, float, 1.0/len, normsum);
                }
                sctx->vertNum++;
                
                vii[0]=ovi;
                vii[1]=sctx->vidx[3*(bag->evti[polygon[0]/3]+5*si)+polygon[0]%3];
                for (j=0; j<polyct; ++j) {
                  double edgeA[3], edgeB[3];
                  double norm[3];
                  vii[2]=vii[1];
                  if (j==polyct-1) {
                    if (polygon[0]<36)
                      vii[1]=sctx->vidx[3*(bag->evti[polygon[0]/3] + 5*si)+
                                        polygon[0]%3];
                    else vii[1]=sctx->facevidx[fvti[polygon[0]-36]];
                  } else {
                    if (polygon[j+1]<36)
                      vii[1]=sctx->vidx[3*(bag->evti[polygon[j+1]/3] + 5*si)+
                                        polygon[j+1]%3];
                    else vii[1]=sctx->facevidx[fvti[polygon[j+1]-36]];
                  }
                  
                  /* check for degenerate tris */
                  ELL_3V_SUB(edgeA, lpld->xyzw+4*vii[1], lpld->xyzw+4*vii[0]);
                  ELL_3V_SUB(edgeB, lpld->xyzw+4*vii[2], lpld->xyzw+4*vii[0]);
                  ELL_3V_CROSS(norm, edgeA, edgeB);
                  if (ELL_3V_DOT(norm,norm)!=0) {
                    unsigned iii = airArrayLenIncr(bag->indxArr, 3);
                    ELL_3V_COPY(lpld->indx + iii, vii);
                    lpld->icnt[0] += 3;
                    sctx->faceNum++;
                  }
                  /* else: degeneracies are caused by intersections that
                   * more or less coincide with a grid vertex. They
                   * should be harmless, so just don't create
                   * deg. triangles in this case */
                }
            } else if (polyct>2) {
              /* insert the actual triangles */
              unsigned int vii[3], iii;
              if (polygon[0]<36)
                vii[0]=sctx->vidx[3*(bag->evti[polygon[0]/3] + 5*si)+
                                  polygon[0]%3];
              else vii[0]=sctx->facevidx[fvti[polygon[0]-36]];
              if (polygon[1]<36)
                vii[1]=sctx->vidx[3*(bag->evti[polygon[1]/3] + 5*si)+
                                  polygon[1]%3];
              else vii[1]=sctx->facevidx[fvti[polygon[1]-36]];
              if (polygon[2]<36)
                vii[2]=sctx->vidx[3*(bag->evti[polygon[2]/3] + 5*si)+
                                  polygon[2]%3];
              else vii[2]=sctx->facevidx[fvti[polygon[2]-36]];
              iii = airArrayLenIncr(bag->indxArr, 3);
              ELL_3V_COPY(lpld->indx + iii, vii);
              lpld->icnt[0] += 3;
              sctx->faceNum++;
              
              if (polyct==4) {
                vii[1]=vii[2];
                if (polygon[3]<36)
                  vii[2]=sctx->vidx[3*(bag->evti[polygon[3]/3] + 5*si)+
                                    polygon[3]%3];
                else vii[2]=sctx->facevidx[fvti[polygon[3]-36]];
                iii = airArrayLenIncr(bag->indxArr, 3);
                ELL_3V_COPY(lpld->indx + iii, vii);
                lpld->icnt[0] += 3;
                sctx->faceNum++;
              }
            }
          }
        }
      }
    }
  }
  return 0;
}

static void
shuffleT(seekContext *sctx, baggage *bag) {
  unsigned int xi, yi, sx, sy, si;
  
  sx = AIR_CAST(unsigned int, sctx->sx);
  sy = AIR_CAST(unsigned int, sctx->sy);
  
  if (sctx->strengthUse) { /* requests need to be cleared initially */
    for (yi=0; yi<sy; yi++) {
      for (xi=0; xi<sx; xi++) {
        sctx->treated[xi+sx*yi] &= 0xFF^_SEEK_TREATED_REQUEST;
      }
    }
  } /* else, the request bits are always on */
  
  for (yi=0; yi<sy; yi++) {
    for (xi=0; xi<sx; xi++) {
      si = xi + sx*yi;
      
      /* vidx neither needs past nor future context */
      if (!bag->zi) {
        ELL_3V_SET(sctx->vidx+3*(0+5*si), -1, -1, -1);
        ELL_3V_SET(sctx->vidx+3*(1+5*si), -1, -1, -1);
      } else {
        ELL_3V_COPY(sctx->vidx+3*(0+5*si), sctx->vidx+3*(3+5*si));
        ELL_3V_COPY(sctx->vidx+3*(1+5*si), sctx->vidx+3*(4+5*si));
      }
      ELL_3V_SET(sctx->vidx+3*(2+5*si),-1,-1,-1);
      ELL_3V_SET(sctx->vidx+3*(3+5*si),-1,-1,-1);
      ELL_3V_SET(sctx->vidx+3*(4+5*si),-1,-1,-1);
      
      /* strength only has future context */
      if (sctx->strengthUse) {
        sctx->stng[0 + 2*si] = sctx->stng[1 + 2*si];
        sctx->stng[1 + 2*si] = sctx->stngcontext[si];
        if (sctx->stng[0+2*si]>sctx->strength ||
            sctx->stng[1+2*si]>sctx->strength) {
          /* set up to four request bits */
          sctx->treated[si] |= _SEEK_TREATED_REQUEST;
          if (xi!=0) sctx->treated[xi-1+sx*yi] |= _SEEK_TREATED_REQUEST;
          if (yi!=0) sctx->treated[xi+sx*(yi-1)] |= _SEEK_TREATED_REQUEST;
          if (xi!=0 && yi!=0)
            sctx->treated[xi-1+sx*(yi-1)] |= _SEEK_TREATED_REQUEST;
        }
      }
      
      /* shuffle grad, hess and t in three steps: move to past context,
       * shuffle in slab itself, move from future context */
      
      ELL_3V_COPY(sctx->gradcontext + 3*(0+2*si), sctx->grad + 3*(0+2*si));
      ELL_3V_COPY(sctx->grad + 3*(0+2*si), sctx->grad + 3*(1+2*si));
      ELL_3V_COPY(sctx->grad + 3*(1+2*si), sctx->gradcontext + 3*(1+2*si));
      
      ELL_3M_COPY(sctx->hesscontext + 9*(0+2*si), sctx->hess + 9*(0+2*si));
      ELL_3M_COPY(sctx->hess + 9*(0+2*si), sctx->hess + 9*(1+2*si));
      ELL_3M_COPY(sctx->hess + 9*(1+2*si), sctx->hesscontext + 9*(1+2*si));
      
      ELL_3M_COPY(sctx->tcontext + 9*(0+2*si), sctx->t + 9*(0+2*si));
      ELL_3M_COPY(sctx->t + 9*(0+2*si), sctx->t + 9*(1+2*si));
      ELL_3M_COPY(sctx->t + 9*(1+2*si), sctx->tcontext + 9*(1+2*si));
    }
  }
}

static void
probeT(seekContext *sctx, baggage *bag, double zi) {
  unsigned int xi, yi, sx, sy, si;
  
  sx = AIR_CAST(unsigned int, sctx->sx);
  sy = AIR_CAST(unsigned int, sctx->sy);
  
  for (yi=0; yi<sy; yi++) {
    for (xi=0; xi<sx; xi++) {
      si = xi + sx*yi;
      
      if (sctx->gctx) { /* HEY: need this check, what's the right way? */
        _seekIdxProbe(sctx, bag, xi, yi, zi);
      }
      if (sctx->strengthUse) {
        sctx->stngcontext[si] = sctx->strengthSign*sctx->stngAns[0];
        if (sctx->strengthSeenMax==AIR_NAN) {
          sctx->strengthSeenMax = sctx->stngcontext[si];
        }
        sctx->strengthSeenMax = AIR_MAX(sctx->strengthSeenMax,
                                        sctx->stngcontext[si]);
      }
      ELL_3V_COPY(sctx->gradcontext + 3*(1 + 2*si), sctx->gradAns);
      ELL_3M_COPY(sctx->hesscontext + 9*(1 + 2*si), sctx->hessAns);
      _seekHess2T(sctx->tcontext + 9*(1 + 2*si), sctx->evalAns, sctx->evecAns,
                  sctx->evalDiffThresh, (sctx->type==seekTypeRidgeSurfaceT));
    }
  }
}

/* it has now become much easier to make this its own routine
 * (vs. adding many more case distinctions to shuffleProbe)
 * this only duplicates little (and trivial) code */
int
_seekShuffleProbeT(seekContext *sctx, baggage *bag) {
  /* for high-quality normal estimation, we need two slices of data
   * context; to keep the code simple, separate shuffle and probe
   * operations - let's hope this doesn't destroy cache performance */
  
  if (!bag->zi) {
    if (sctx->strengthUse)
      /* before the first round, initialize treated to zero */
      memset(sctx->treated, 0, sizeof(char)*sctx->sx*sctx->sy);
    else /* request all edges */
      memset(sctx->treated, _SEEK_TREATED_REQUEST,
             sizeof(char)*sctx->sx*sctx->sy);
    probeT(sctx, bag, 0);
    shuffleT(sctx, bag);
    probeT(sctx, bag, 1);
  }
  shuffleT(sctx, bag);
  if (bag->zi!=sctx->sz-2)
    probeT(sctx, bag, bag->zi+2);
  
  intersectionShuffleProbe(sctx, bag);
  
  return 0;
}

/* For all vertices in pld, use sctx to probe the strength measure,
 * and return the answer (times strengthSign) in nval. The intended
 * use is postfiltering (with limnPolyDataClip), which is obligatory
 * when using seekType*SurfaceT
 *
 * Returns 1 and adds a message to biff upon error
 * Returns 0 on success, -n when probing n vertices failed (strength
 * is set to AIR_NAN for those); note that positions outside the field
 * are clamped to lie on its boundary.
 *
 * This routine assumes that a strength has been set in sctx and
 * seekUpdate() has been run.
 * This routine does not modify sctx->strengthSeenMax.
 */
int
seekVertexStrength(Nrrd *nval, seekContext *sctx, limnPolyData *pld) {
  static const char me[]="seekVertexStrength";
  unsigned int i;
  double *data;
  int E=0;
  
  if (!(nval && sctx && pld)) {
    biffAddf(SEEK, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(sctx->gctx && sctx->pvl)) {
    biffAddf(SEEK, "%s: need sctx with attached gageContext", me);
    return 1;
  }
  if (!sctx->stngAns) {
    biffAddf(SEEK, "%s: no strength item found. Did you enable strengthUse?",
             me);
    return 1;
  }
  if (nrrdAlloc_va(nval, nrrdTypeDouble, 1, (size_t) pld->xyzwNum)) {
    biffAddf(SEEK, "%s: could not allocate output", me);
    return 1;
  }
  
  data = (double*) nval->data;
  for (i=0; i<pld->xyzwNum; i++) {
    float homog[4];
    ELL_4V_HOMOG(homog, pld->xyzw+4*i);
    if (!gageProbeSpace(sctx->gctx,homog[0],homog[1],homog[2],0,1)) {
      *(data++)=*(sctx->stngAns)*sctx->strengthSign;
    } else {
      *(data++)=AIR_NAN;
      E--;
    }
  }
  return E;
}
