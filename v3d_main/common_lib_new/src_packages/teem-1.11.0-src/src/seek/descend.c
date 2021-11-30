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

/* This file collects functions that implement various gradient
 * descents required in the context of extracting crease surfaces */

#include "seek.h"
#include "privateSeek.h"

/* Tries to find a degenerate point on a bilinearly interpolated face
 * of symmetric second-order tensors. Uses the discriminant constraint
 * functions from Zheng/Parlett/Pang, TVCG 2005, and the
 * Newton-Raphson method with Armijo stepsize control.
 *
 * coord are coordinates relative to the given face and will be
 *      updated in each iteration.
 * botleft - topright are 9-vectors, representing the symmetric
 *      matrices at the corners of the face (input only)
 * maxiter is the maximum number of iterations allowed
 * eps is the accuracy up to which the discriminant should be zero
 *     The discriminant scales with the Frobenius norm to the sixth power,
 *     so the exact constraint is disc/|T|^6<eps
 * type can be 'l', 'p', or something else (=both)
 *
 * Returns 0 if the point was found up to the given accuracy
 * Returns 1 if we left the face
 * Returns 2 if we hit maxiter
 * Returns 3 if we could not invert a matrix to find next gradient dir
 * Returns 4 if we found a point, but it does not have the desired type
 * Returns 5 if Armijo rule failed to find a valid stepsize
 * Returns 6 if we hit a zero tensor (|T|<1e-300)
 */
int seekDescendToDeg(double *coord, double *botleft, double *botright,
                     double *topleft, double *topright,
                     int maxiter, double eps, char type)
{
  double discr; /* store discriminant value of previous iteration */
  double hesstop[9]; /* used to interpolate Hessian */
  double hessbot[9];
  double hess[9];
  double ten[6]; /* makes access more convenient */
  double tsqr[6]; /* squared tensor values, are used repeatedly */
  double cf[7]; /* constraint function vector */
  double norm; /* Frobenius norm, for normalization */
  int i, j, iter; /* counting variables for loops */

  /* check initial point */
  ELL_3M_LERP(hessbot,coord[0],botleft,botright);
  ELL_3M_LERP(hesstop,coord[0],topleft,topright);
  ELL_3M_LERP(hess,coord[1],hessbot,hesstop);

  /* normalize for scale invariance & to avoid numerical problems */
  norm = sqrt(hess[0]*hess[0]+hess[4]*hess[4]+hess[8]*hess[8]+
              2*(hess[1]*hess[1]+hess[2]*hess[2]+hess[5]*hess[5]));
  if (norm<1e-300) return 6;
  ten[0] = hess[0]/norm; ten[1] = hess[1]/norm; ten[2] = hess[2]/norm;
  ten[3] = hess[4]/norm; ten[4] = hess[5]/norm; ten[5] = hess[8]/norm;
  for (i=0; i<6; i++)
    tsqr[i] = ten[i]*ten[i];

  /* evaluate the constraint function vector */
  cf[0] = ten[0]*(tsqr[3]-tsqr[5])+ten[0]*(tsqr[1]-tsqr[2])+
    ten[3]*(tsqr[5]-tsqr[0])+ten[3]*(tsqr[4]-tsqr[1])+
    ten[5]*(tsqr[0]-tsqr[3])+ten[5]*(tsqr[2]-tsqr[4]); /* fx */
  cf[1] = ten[4]*(2*(tsqr[4]-tsqr[0])-(tsqr[2]+tsqr[1])+
                  2*(ten[3]*ten[0]+ten[5]*ten[0]-ten[3]*ten[5]))+
    ten[1]*ten[2]*(2*ten[0]-ten[5]-ten[3]); /* fy1 */
  cf[2] = ten[2]*(2*(tsqr[2]-tsqr[3])-(tsqr[1]+tsqr[4])+
                  2*(ten[5]*ten[3]+ten[0]*ten[3]-ten[5]*ten[0]))+
    ten[4]*ten[1]*(2*ten[3]-ten[0]-ten[5]); /* fy2 */
  cf[3] = ten[1]*(2*(tsqr[1]-tsqr[5])-(tsqr[4]+tsqr[2])+
                  2*(ten[0]*ten[5]+ten[3]*ten[5]-ten[0]*ten[3]))+
    ten[2]*ten[4]*(2*ten[5]-ten[3]-ten[0]); /* fy3 */
  cf[4] = ten[4]*(tsqr[2]-tsqr[1])+ten[1]*ten[2]*(ten[3]-ten[5]); /* fz1 */
  cf[5] = ten[2]*(tsqr[1]-tsqr[4])+ten[4]*ten[1]*(ten[5]-ten[0]); /* fz2 */
  cf[6] = ten[1]*(tsqr[4]-tsqr[2])+ten[2]*ten[4]*(ten[0]-ten[3]); /* fz3 */

  discr = cf[0]*cf[0]+cf[1]*cf[1]+cf[2]*cf[2]+cf[3]*cf[3]+
    15*(cf[4]*cf[4]+cf[5]*cf[5]+cf[6]*cf[6]);
  if (discr<eps) {
    if (type!='l' && type!='p') return 0;
    else {
      /* check if type is correct */
      double dev[9], det;
      double mean = (ten[0]+ten[3]+ten[5])/3;
      dev[0] = ten[0]-mean; dev[1] = ten[1]; dev[2] = ten[2];
      dev[3] = ten[1]; dev[4]=ten[3]-mean; dev[5] = ten[4];
      dev[6] = ten[2]; dev[7]=ten[4]; dev[8]=ten[5]-mean;
      det = ELL_3M_DET(dev);
      if ((type=='l' && det>0) || (type=='p' && det<0)) return 0;
      else return 4; /* sufficient accuracy, but wrong type  */
    }
  }

  for (iter=0; iter<maxiter; iter++) {
    /* find derivative of constraint function vector using the chain rule */
    double cft[42]; /* derive relative to tensor values, 7x6 matrix */
    double tx[12]; /* spatial derivative of tensor values, 6x2 matrix */
    double cfx[14]; /* spatial derivative of constraint funct., 7x2 matrix */
    double denom[3], det; /* symmetric 2x2 matrix that is to be inverted */
    double inv[3]; /* inverse of that matrix */
    double nom[2], dx[2]; /* more helpers to compute next step */
    /* variables needed for Armijo stepsize rule */
    double beta=1, gamma=0.5, alpha=beta; /* parameters */
    int accept=0, safetyct=0, maxct=30; /* counters */
    double dxsqr; /* squared length of stepsize */
    double hessleft[9], hessright[9]; /* used to compute Hessian derivative */
    double hessder[9];
    int row, col;

    cft [0] = tsqr[3]-tsqr[5]+tsqr[1]-tsqr[2]-2*ten[0]*ten[3]+2*ten[0]*ten[5];
    /* fx/T00 */
    cft [1] = 2*ten[0]*ten[1]-2*ten[3]*ten[1]; /* fx/T01 */
    cft [2] = -2*ten[0]*ten[2]+2*ten[5]*ten[2]; /* fx/T02 */
    cft [3] = 2*ten[0]*ten[3]+tsqr[5]-tsqr[0]+tsqr[4]-tsqr[1]-2*ten[5]*ten[3];
    /* fx/T11 */
    cft [4] = 2*ten[3]*ten[4]-2*ten[5]*ten[4]; /* fx/T12 */
    cft [5] = -2*ten[0]*ten[5]+2*ten[3]*ten[5]+tsqr[0]-tsqr[3]+tsqr[2]-tsqr[4];
    /* fx/T22 */

    cft [6] = -4*ten[0]*ten[4]+2*ten[4]*ten[3]+2*ten[4]*ten[5]+2*ten[1]*ten[2];
    /* fy1/T00 */
    cft [7] = -2*ten[4]*ten[1]+ten[2]*(2*ten[0]-ten[5]-ten[3]); /* fy1/T01 */
    cft [8] = -2*ten[4]*ten[2]+ten[1]*(2*ten[0]-ten[5]-ten[3]); /* fy1/T02 */
    cft [9] = 2*ten[4]*ten[0]-2*ten[4]*ten[5]-ten[1]*ten[2]; /* fy1/T11 */
    cft[10] = 6*tsqr[4]-2*tsqr[0]-(tsqr[2]+tsqr[1])+
      2*(ten[3]*ten[0]+ten[5]*ten[0]-ten[3]*ten[5]); /* fy1/T12 */
    cft[11] = 2*ten[4]*ten[0]-2*ten[4]*ten[3]-ten[1]*ten[2]; /* fy1/T22 */

    cft[12] = 2*ten[2]*ten[3]-2*ten[2]*ten[5]-ten[4]*ten[1]; /* fy2/T00 */
    cft[13] = -2*ten[2]*ten[1]+ten[4]*(2*ten[3]-ten[0]-ten[5]); /* fy2/T01 */
    cft[14] = 6*tsqr[2]-2*tsqr[3]-(tsqr[1]+tsqr[4])+
      2*(ten[5]*ten[3]+ten[0]*ten[3]-ten[5]*ten[0]); /* fy2/T02 */
    cft[15] = -4*ten[2]*ten[3]+2*ten[2]*ten[5]+2*ten[2]*ten[0]+2*ten[4]*ten[1];
    /* fy2/T11 */
    cft[16] = -2*ten[2]*ten[4]+ten[1]*(2*ten[3]-ten[0]-ten[5]); /* fy2/T12 */
    cft[17] = 2*ten[2]*ten[3]-2*ten[2]*ten[0]-ten[4]*ten[1]; /* fy2/T22 */

    cft[18] = 2*ten[1]*ten[5]-2*ten[1]*ten[3]-ten[2]*ten[4]; /* fy3/T00 */
    cft[19] = 6*tsqr[1]-2*tsqr[5]-(tsqr[4]+tsqr[2])+
      2*(ten[0]*ten[5]+ten[3]*ten[5]-ten[0]*ten[3]); /* fy3/T01 */
    cft[20] = -2*ten[1]*ten[2]+ten[4]*(2*ten[5]-ten[3]-ten[0]); /* fy3/T02 */
    cft[21] = 2*ten[1]*ten[5]-2*ten[1]*ten[0]-ten[2]*ten[4]; /* fy3/T11 */
    cft[22] = -2*ten[1]*ten[4]+ten[2]*(2*ten[5]-ten[3]-ten[0]); /* fy3/T12 */
    cft[23] = -4*ten[1]*ten[5]+2*ten[0]*ten[1]+2*ten[1]*ten[3]+2*ten[2]*ten[4];
    /* fy3/T22 */

    cft[24] = 0; /* fz1/T00 */
    cft[25] = -2*ten[4]*ten[1]+ten[2]*(ten[3]-ten[5]); /* fz1/T01 */
    cft[26] = 2*ten[4]*ten[2]+ten[1]*(ten[3]-ten[5]); /* fz1/T02 */
    cft[27] = ten[1]*ten[2]; /* fz1/T11 */
    cft[28] = tsqr[2]-tsqr[1]; /* fz1/T12 */
    cft[29] = -ten[1]*ten[2]; /* fz1/T22 */

    cft[30] = -ten[4]*ten[1]; /* fz2/T00 */
    cft[31] = 2*ten[2]*ten[1]+ten[4]*(ten[5]-ten[0]); /* fz2/T01 */
    cft[32] = tsqr[1]-tsqr[4]; /* fz2/T02 */
    cft[33] = 0; /* fz2/T11 */
    cft[34] = -2*ten[2]*ten[4]+ten[1]*(ten[5]-ten[0]); /* fz2/T12 */
    cft[35] = ten[4]*ten[1]; /* fz2/T22 */

    cft[36] = ten[2]*ten[4]; /* fz3/T00 */
    cft[37] = tsqr[4]-tsqr[2]; /* fz3/T01 */
    cft[38] = -2*ten[1]*ten[2]+ten[4]*(ten[0]-ten[3]); /* fz3/T02 */
    cft[39] = -ten[2]*ten[4]; /* fz3/T11 */
    cft[40] = 2*ten[1]*ten[4]+ten[2]*(ten[0]-ten[3]); /* fz3/T12 */
    cft[41] = 0; /* fz3/T22 */

    /* approximate Hessian derivative in x dir */
    ELL_3M_LERP(hessleft,coord[1],botleft,topleft);
    ELL_3M_LERP(hessright,coord[1],botright,topright);
    ELL_3M_SUB(hessder,hessright,hessleft);
    ELL_3M_SCALE(hessder,1.0/norm,hessder);

    tx[0] = hessder[0]; /* T00 / x */
    tx[2] = hessder[1]; /* T01 / x */
    tx[4] = hessder[2]; /* T02 / x */
    tx[6] = hessder[4]; /* T11 / x */
    tx[8] = hessder[5]; /* T12 / x */
    tx[10] = hessder[8]; /* T22 / x */

    /* approximate Hessian derivative in z dir */
    ELL_3M_SUB(hessder,hesstop,hessbot);
    ELL_3M_SCALE(hessder,1.0/norm,hessder);

    tx[1] = hessder[0]; /* T00 / z */
    tx[3] = hessder[1]; /* T01 / z */
    tx[5] = hessder[2]; /* T02 / z */
    tx[7] = hessder[4]; /* T11 / z */
    tx[9] = hessder[5]; /* T12 / z */
    tx[11] = hessder[8]; /* T22 / z */

    /* matrix multiply cft*tx */
    for (row=0; row<7; row++)
      for (col=0; col<2; col++) {
        i = row*2+col;
        cfx[i] = 0;
        for (j=0; j<6; j++) {
          cfx[i] += cft[row*6+j]*tx[j*2+col];
        }
      }

    for (i=0; i<3; i++)
      denom[i] = 0;
    for (j=0; j<7; j++) {
      denom[0] += cfx[j*2]*cfx[j*2];
      denom[1] += cfx[j*2+1]*cfx[j*2];
      denom[2] += cfx[j*2+1]*cfx[j*2+1];
    }

    det = denom[0]*denom[2]-denom[1]*denom[1];
    if (fabs(det)<DBL_EPSILON)
      return 3;

    inv[0] = denom[2] / det;
    inv[1] = -denom[1] / det;
    inv[2] = denom[0] / det;

    /* multiply transpose(cfx)*cf */
    nom[0]=0; nom[1]=0;
    for (j=0; j<7; j++) {
      nom[0] += cfx[j*2]   * cf[j];
      nom[1] += cfx[j*2+1] * cf[j];
    }

    /* calculate the coordinate offset dx = inv*nom */
    dx[0] = inv[0]*nom[0]+inv[1]*nom[1];
    dx[1] = inv[1]*nom[0]+inv[2]*nom[1];

    /* employ the Armijo stepsize rule for improved convergence */
    dxsqr = dx[0]*dx[0]+dx[1]*dx[1];
    while (!accept && safetyct++<maxct) {
      /* test discriminant at new position */
      double newcoord[2];
      double newdiscr;
      ELL_2V_SET(newcoord, coord[0]-alpha*dx[0], coord[1]-alpha*dx[1]);

      if (newcoord[0]<0 || newcoord[0]>1 ||
          newcoord[1]<0 || newcoord[1]>1) {
        if (safetyct==maxct)
          return 1; /* we left the cell */
        alpha*=gamma;
      }

      ELL_3M_LERP(hessbot,newcoord[0],botleft,botright);
      ELL_3M_LERP(hesstop,newcoord[0],topleft,topright);
      ELL_3M_LERP(hess,newcoord[1],hessbot,hesstop);

      norm = sqrt(hess[0]*hess[0]+hess[4]*hess[4]+hess[8]*hess[8]+
                  2*(hess[1]*hess[1]+hess[2]*hess[2]+hess[5]*hess[5]));
      if (norm<1e-300) return 6;
      /* copy over */
      ten[0] = hess[0]/norm; ten[1] = hess[1]/norm; ten[2] = hess[2]/norm;
      ten[3] = hess[4]/norm; ten[4] = hess[5]/norm; ten[5] = hess[8]/norm;
      for (i=0; i<6; i++)
        tsqr[i] = ten[i]*ten[i];

      /* evaluate the constraint function vector */
      cf[0] = ten[0]*(tsqr[3]-tsqr[5])+ten[0]*(tsqr[1]-tsqr[2])+
        ten[3]*(tsqr[5]-tsqr[0])+ten[3]*(tsqr[4]-tsqr[1])+
        ten[5]*(tsqr[0]-tsqr[3])+ten[5]*(tsqr[2]-tsqr[4]); /* fx */
      cf[1] = ten[4]*(2*(tsqr[4]-tsqr[0])-(tsqr[2]+tsqr[1])+
                      2*(ten[3]*ten[0]+ten[5]*ten[0]-ten[3]*ten[5]))+
        ten[1]*ten[2]*(2*ten[0]-ten[5]-ten[3]); /* fy1 */
      cf[2] = ten[2]*(2*(tsqr[2]-tsqr[3])-(tsqr[1]+tsqr[4])+
                      2*(ten[5]*ten[3]+ten[0]*ten[3]-ten[5]*ten[0]))+
        ten[4]*ten[1]*(2*ten[3]-ten[0]-ten[5]); /* fy2 */
      cf[3] = ten[1]*(2*(tsqr[1]-tsqr[5])-(tsqr[4]+tsqr[2])+
                      2*(ten[0]*ten[5]+ten[3]*ten[5]-ten[0]*ten[3]))+
        ten[2]*ten[4]*(2*ten[5]-ten[3]-ten[0]); /* fy3 */
      cf[4] = ten[4]*(tsqr[2]-tsqr[1])+ten[1]*ten[2]*(ten[3]-ten[5]); /* fz1 */
      cf[5] = ten[2]*(tsqr[1]-tsqr[4])+ten[4]*ten[1]*(ten[5]-ten[0]); /* fz2 */
      cf[6] = ten[1]*(tsqr[4]-tsqr[2])+ten[2]*ten[4]*(ten[0]-ten[3]); /* fz3 */

      newdiscr = cf[0]*cf[0]+cf[1]*cf[1]+cf[2]*cf[2]+cf[3]*cf[3]+
        15*(cf[4]*cf[4]+cf[5]*cf[5]+cf[6]*cf[6]);
      if (newdiscr<eps) {
        coord[0]=newcoord[0]; coord[1]=newcoord[1]; /* update coord! */
        if (type!='l' && type!='p') return 0;
        else {
          /* check if type is correct */
          double dev[9];
          double mean = (ten[0]+ten[3]+ten[5])/3;
          dev[0] = ten[0]-mean; dev[1] = ten[1]; dev[2] = ten[2];
          dev[3] = ten[1]; dev[4]=ten[3]-mean; dev[5] = ten[4];
          dev[6] = ten[2]; dev[7]=ten[4]; dev[8]=ten[5]-mean;
          det = ELL_3M_DET(dev);
          if ((type=='l' && det>0) || (type=='p' && det<0)) return 0;
          else return 4; /* sufficient accuracy, but wrong type  */
        }
      }

      if (newdiscr<=discr-0.5*alpha*dxsqr) {
        accept=1;
        discr = newdiscr;
      } else {
        alpha*=gamma;
      }
    }

    if (!accept)
      return 5;
    coord[0] -= alpha*dx[0];
    coord[1] -= alpha*dx[1];
  }
  return 2; /* hit maxiter */
}

/* Descends to the degenerate line in a trilinearly interpolated cell
 * using the discriminant constraint functions and the Newton-Raphson
 * method with Armijo stepsize control.
 *
 * This function is NOT part of the crease extraction, but has been
 * used for debugging it.
 *
 * coord are coordinates relative to the given cell and will be
 *      updated in each iteration.
 * Hbfl - Htbr are 9-vectors, representing the symmetric matrices at the
 *      corners of the face (input only)
 * maxiter is the maximum number of iterations allowed
 * eps is the accuracy up to which the discriminant should be zero
 *     The discriminant scales with the Frobenius norm to the sixth power,
 *     so the exact constraint is disc/|T|^6<eps
 * type can be 'l', 'p' or something else (=both)
 *
 * Returns 0 if the point was found up to the given accuracy
 * Returns 1 if we left the cell
 * Returns 2 if we hit maxiter
 * Returns 3 if we could not invert a matrix to find next gradient dir
 * Returns 4 if we found a point, but it does not have the desired type
 * Returns 5 if Armijo rule failed to find a valid stepsize
 * Returns 6 if we hit a zero tensor (|T|<1e-300)
 */
int seekDescendToDegCell(double *coord, double *Hbfl, double *Hbfr,
                         double *Hbbl, double *Hbbr,
                         double *Htfl, double *Htfr, double *Htbl, double *Htbr,
                         int maxiter, double eps, char type)
{
  double discr=0; /* store discriminant value for previous point */

  double Hfrontleft[9]={0,0,0,0,0,0,0,0,0}, Hbackleft[9]={0,0,0,0,0,0,0,0,0};
  double Hfrontright[9]={0,0,0,0,0,0,0,0,0}, Hbackright[9]={0,0,0,0,0,0,0,0,0};
  double Hleft[9]={0,0,0,0,0,0,0,0,0}, Hright[9]={0,0,0,0,0,0,0,0,0};
  double H[9]={0,0,0,0,0,0,0,0,0}; /* init takes care of compiler warnings */
  double optgrad[3]={0.0,0.0,0.0}; /* gradient for descent */

  int iter=0;
  do {
    /* on the first run, initialize discr; later, employ the Armijo
     * stepsize rule to guarantee convergence */
    double beta=1.0;
    double gamma=0.5;
    double alpha=beta;
    int accept=0;
    double optgradsqr = ELL_3V_DOT(optgrad,optgrad);
    unsigned int safetyct=0;
    const unsigned int maxct=30;
    double tsqr[6]={0,0,0,0,0,0}, /* only initialize to silence warning */
      ten[6]={0,0,0,0,0,0}, norm=0;
    double cf[7]={0,0,0,0,0,0,0}, /* only initialize to silence warning */
      cft[42]; /* derive relative to tensor values, 7x6 matrix */
    double cfx[21]; /* spatial derivative of constraint functions, 7x3 matrix */
    double tx[18]; /* spatial derivative of tensor values, 6x3 matrix */
    double Hder[9], Hfront[9], Hback[9]; /* used to approximate Hessian der. */
    double Htopleft[9], Htopright[9], Hbotleft[9], Hbotright[9],
      Htop[9], Hbot[9];
    double denom[9]; /* 3x3 matrix that is to be inverted */
    double inv[9]; /* inverse of that matrix */
    double nom[3]={0,0,0};

    int i, j, row, col; /* counters, used later on */

    while (!accept && safetyct++<maxct) {
      /* compute distance at new position */
      double newcoord[3];
      double newdiscr;
      ELL_3V_SET(newcoord, coord[0]-alpha*optgrad[0],
                 coord[1]-alpha*optgrad[1], coord[2]-alpha*optgrad[2]);

      if (newcoord[0]<0 || newcoord[0]>1 ||
          newcoord[1]<0 || newcoord[1]>1 ||
          newcoord[2]<0 || newcoord[2]>1) {
        if (safetyct==maxct) {
          ELL_3V_COPY(coord,newcoord); /* such that caller knows which
                                          dir was the culprit */
          return 1; /* we left the cell */
        }
        alpha*=gamma;
        continue;
      }

      ELL_3M_LERP(Hfrontleft, newcoord[2], Hbfl, Htfl);
      ELL_3M_LERP(Hbackleft, newcoord[2], Hbbl, Htbl);
      ELL_3M_LERP(Hleft, newcoord[1], Hfrontleft, Hbackleft);

      ELL_3M_LERP(Hfrontright, newcoord[2], Hbfr, Htfr);
      ELL_3M_LERP(Hbackright, newcoord[2], Hbbr, Htbr);
      ELL_3M_LERP(Hright, newcoord[1], Hfrontright, Hbackright);

      ELL_3M_LERP(H, newcoord[0], Hleft, Hright);
      norm = sqrt(H[0]*H[0]+H[4]*H[4]+H[8]*H[8]+
                  2*(H[1]*H[1]+H[2]*H[2]+H[5]*H[5]));
      if (norm<1e-300) return 6;
      ten[0]=H[0]/norm; ten[1]=H[1]/norm; ten[2]=H[2]/norm;
      ten[3]=H[4]/norm; ten[4]=H[5]/norm; ten[5]=H[7]/norm;

      for (i=0; i<6; i++)
        tsqr[i] = ten[i]*ten[i];

      /* evaluate the constraint function vector */
      cf[0] = ten[0]*(tsqr[3]-tsqr[5])+ten[0]*(tsqr[1]-tsqr[2])+
        ten[3]*(tsqr[5]-tsqr[0])+
        ten[3]*(tsqr[4]-tsqr[1])+ten[5]*(tsqr[0]-tsqr[3])+
        ten[5]*(tsqr[2]-tsqr[4]); /* fx */
      cf[1] = ten[4]*(2*(tsqr[4]-tsqr[0])-(tsqr[2]+tsqr[1])+
                      2*(ten[3]*ten[0]+ten[5]*ten[0]-ten[3]*ten[5]))+
        ten[1]*ten[2]*(2*ten[0]-ten[5]-ten[3]); /* fy1 */
      cf[2] = ten[2]*(2*(tsqr[2]-tsqr[3])-(tsqr[1]+tsqr[4])+
                      2*(ten[5]*ten[3]+ten[0]*ten[3]-ten[5]*ten[0]))+
        ten[4]*ten[1]*(2*ten[3]-ten[0]-ten[5]); /* fy2 */
      cf[3] = ten[1]*(2*(tsqr[1]-tsqr[5])-(tsqr[4]+tsqr[2])+
                      2*(ten[0]*ten[5]+ten[3]*ten[5]-ten[0]*ten[3]))+
        ten[2]*ten[4]*(2*ten[5]-ten[3]-ten[0]); /* fy3 */
      cf[4] = ten[4]*(tsqr[2]-tsqr[1])+ten[1]*ten[2]*(ten[3]-ten[5]); /* fz1 */
      cf[5] = ten[2]*(tsqr[1]-tsqr[4])+ten[4]*ten[1]*(ten[5]-ten[0]); /* fz2 */
      cf[6] = ten[1]*(tsqr[4]-tsqr[2])+ten[2]*ten[4]*(ten[0]-ten[3]); /* fz3 */

      newdiscr = cf[0]*cf[0]+cf[1]*cf[1]+cf[2]*cf[2]+cf[3]*cf[3]+
        15*(cf[4]*cf[4]+cf[5]*cf[5]+cf[6]*cf[6]);

      if (newdiscr<eps) {
        ELL_3V_COPY(coord, newcoord); /* update coord for output */
        if (type!='l' && type!='p') return 0;
        else {
          /* check if type is correct */
          double dev[9], det;
          double mean = (ten[0]+ten[3]+ten[5])/3;
          dev[0] = ten[0]-mean; dev[1] = ten[1]; dev[2] = ten[2];
          dev[3] = ten[1]; dev[4]=ten[3]-mean; dev[5] = ten[4];
          dev[6] = ten[2]; dev[7]=ten[4]; dev[8]=ten[5]-mean;
          det = ELL_3M_DET(dev);
          if ((type=='l' && det>0) || (type=='p' && det<0)) return 0;
          else return 4; /* sufficient accuracy, but wrong type  */
        }
      }

      if (iter==0 || newdiscr<=discr-0.5*alpha*optgradsqr) {
        accept=1;
        discr = newdiscr;
      } else {
        alpha*=gamma;
      }
    }

    if (!accept)
      return 5; /* could not find a valid stepsize */
    coord[0] -= alpha*optgrad[0];
    coord[1] -= alpha*optgrad[1];
    coord[2] -= alpha*optgrad[2];

    if (iter==maxiter-1)
      break;

    /* find derivative of constraint function vector using the chain rule */

    cft [0] = tsqr[3]-tsqr[5]+tsqr[1]-tsqr[2]-
      2*ten[0]*ten[3]+2*ten[0]*ten[5]; /* fx/T00 */
    cft [1] = 2*ten[0]*ten[1]-2*ten[3]*ten[1]; /* fx/T01 */
    cft [2] = -2*ten[0]*ten[2]+2*ten[5]*ten[2]; /* fx/T02 */
    cft [3] = 2*ten[0]*ten[3]+tsqr[5]-tsqr[0]+tsqr[4]-tsqr[1]-
      2*ten[5]*ten[3]; /* fx/T11 */
    cft [4] = 2*ten[3]*ten[4]-2*ten[5]*ten[4]; /* fx/T12 */
    cft [5] = -2*ten[0]*ten[5]+2*ten[3]*ten[5]+tsqr[0]-tsqr[3]+
      tsqr[2]-tsqr[4]; /* fx/T22 */

    cft [6] = -4*ten[0]*ten[4]+2*ten[4]*ten[3]+2*ten[4]*ten[5]+
      2*ten[1]*ten[2]; /* fy1/T00 */
    cft [7] = -2*ten[4]*ten[1]+ten[2]*(2*ten[0]-ten[5]-ten[3]); /* fy1/T01 */
    cft [8] = -2*ten[4]*ten[2]+ten[1]*(2*ten[0]-ten[5]-ten[3]); /* fy1/T02 */
    cft [9] = 2*ten[4]*ten[0]-2*ten[4]*ten[5]-ten[1]*ten[2]; /* fy1/T11 */
    cft[10] = 6*tsqr[4]-2*tsqr[0]-(tsqr[2]+tsqr[1])+
      2*(ten[3]*ten[0]+ten[5]*ten[0]-ten[3]*ten[5]); /* fy1/T12 */
    cft[11] = 2*ten[4]*ten[0]-2*ten[4]*ten[3]-ten[1]*ten[2]; /* fy1/T22 */

    cft[12] = 2*ten[2]*ten[3]-2*ten[2]*ten[5]-ten[4]*ten[1]; /* fy2/T00 */
    cft[13] = -2*ten[2]*ten[1]+ten[4]*(2*ten[3]-ten[0]-ten[5]); /* fy2/T01 */
    cft[14] = 6*tsqr[2]-2*tsqr[3]-(tsqr[1]+tsqr[4])+
      2*(ten[5]*ten[3]+ten[0]*ten[3]-ten[5]*ten[0]); /* fy2/T02 */
    cft[15] = -4*ten[2]*ten[3]+2*ten[2]*ten[5]+2*ten[2]*ten[0]+
      2*ten[4]*ten[1]; /* fy2/T11 */
    cft[16] = -2*ten[2]*ten[4]+ten[1]*(2*ten[3]-ten[0]-ten[5]); /* fy2/T12 */
    cft[17] = 2*ten[2]*ten[3]-2*ten[2]*ten[0]-ten[4]*ten[1]; /* fy2/T22 */

    cft[18] = 2*ten[1]*ten[5]-2*ten[1]*ten[3]-ten[2]*ten[4]; /* fy3/T00 */
    cft[19] = 6*tsqr[1]-2*tsqr[5]-(tsqr[4]+tsqr[2])+
      2*(ten[0]*ten[5]+ten[3]*ten[5]-ten[0]*ten[3]); /* fy3/T01 */
    cft[20] = -2*ten[1]*ten[2]+ten[4]*(2*ten[5]-ten[3]-ten[0]); /* fy3/T02 */
    cft[21] = 2*ten[1]*ten[5]-2*ten[1]*ten[0]-ten[2]*ten[4]; /* fy3/T11 */
    cft[22] = -2*ten[1]*ten[4]+ten[2]*(2*ten[5]-ten[3]-ten[0]); /* fy3/T12 */
    cft[23] = -4*ten[1]*ten[5]+2*ten[0]*ten[1]+2*ten[1]*ten[3]+
      2*ten[2]*ten[4]; /* fy3/T22 */

    cft[24] = 0; /* fz1/T00 */
    cft[25] = -2*ten[4]*ten[1]+ten[2]*(ten[3]-ten[5]); /* fz1/T01 */
    cft[26] = 2*ten[4]*ten[2]+ten[1]*(ten[3]-ten[5]); /* fz1/T02 */
    cft[27] = ten[1]*ten[2]; /* fz1/T11 */
    cft[28] = tsqr[2]-tsqr[1]; /* fz1/T12 */
    cft[29] = -ten[1]*ten[2]; /* fz1/T22 */

    cft[30] = -ten[4]*ten[1]; /* fz2/T00 */
    cft[31] = 2*ten[2]*ten[1]+ten[4]*(ten[5]-ten[0]); /* fz2/T01 */
    cft[32] = tsqr[1]-tsqr[4]; /* fz2/T02 */
    cft[33] = 0; /* fz2/T11 */
    cft[34] = -2*ten[2]*ten[4]+ten[1]*(ten[5]-ten[0]); /* fz2/T12 */
    cft[35] = ten[4]*ten[1]; /* fz2/T22 */

    cft[36] = ten[2]*ten[4]; /* fz3/T00 */
    cft[37] = tsqr[4]-tsqr[2]; /* fz3/T01 */
    cft[38] = -2*ten[1]*ten[2]+ten[4]*(ten[0]-ten[3]); /* fz3/T02 */
    cft[39] = -ten[2]*ten[4]; /* fz3/T11 */
    cft[40] = 2*ten[1]*ten[4]+ten[2]*(ten[0]-ten[3]); /* fz3/T12 */
    cft[41] = 0; /* fz3/T22 */

    /* approximate Hessian derivative in x dir */
    ELL_3M_SUB(Hder, Hright, Hleft);
    ELL_3M_SCALE(Hder,1.0/norm,Hder);

    tx[0] = Hder[0]; /* T00 / x */
    tx[3] = Hder[1]; /* T01 / x */
    tx[6] = Hder[2]; /* T02 / x */
    tx[9] = Hder[4]; /* T11 / x */
    tx[12] = Hder[5]; /* T12 / x */
    tx[15] = Hder[8]; /* T22 / x */

    ELL_3M_LERP(Hfront, coord[0], Hfrontleft, Hfrontright);
    ELL_3M_LERP(Hback, coord[0], Hbackleft, Hbackright);
    ELL_3M_SUB(Hder, Hback, Hfront); /* y dir */
    ELL_3M_SCALE(Hder,1.0/norm,Hder);

    tx[1] = Hder[0]; /* T00 / y */
    tx[4] = Hder[1]; /* T01 / y */
    tx[7] = Hder[2]; /* T02 / y */
    tx[10] = Hder[4]; /* T11 / y */
    tx[13] = Hder[5]; /* T12 / y */
    tx[16] = Hder[8]; /* T22 / y */

    /* approximate Hessian derivative in z dir */
    ELL_3M_LERP(Htopleft, coord[1], Htfl, Htbl);
    ELL_3M_LERP(Htopright, coord[1], Htfr, Htbr);
    ELL_3M_LERP(Hbotleft, coord[1], Hbfl, Hbbl);
    ELL_3M_LERP(Hbotright, coord[1], Hbfr, Hbbr);
    ELL_3M_LERP(Htop, coord[0], Htopleft, Htopright);
    ELL_3M_LERP(Hbot, coord[0], Hbotleft, Hbotright);
    ELL_3M_SUB(Hder, Htop, Hbot); /* z dir */
    ELL_3M_SCALE(Hder,1.0/norm,Hder);

    tx[2] = Hder[0]; /* T00 / z */
    tx[5] = Hder[1]; /* T01 / z */
    tx[8] = Hder[2]; /* T02 / z */
    tx[11] = Hder[4]; /* T11 / z */
    tx[14] = Hder[5]; /* T12 / z */
    tx[17] = Hder[8]; /* T22 / z */

    /* matrix multiply cft*tx */
    for (row=0; row<7; row++)
      for (col=0; col<3; col++) {
        i = row*3+col;
        cfx[i] = 0;
        for (j=0; j<6; j++) {
          cfx[i] += cft[row*6+j]*tx[j*3+col];
        }
      }

    for (row=0; row<3; row++)
      for (col=0; col<3; col++) {
        i = row*3+col;
        denom[i] = 0;
        for (j=0; j<7; j++) {
          denom[i] += cfx[j*3+row]*cfx[j*3+col];
        }
      }
    ell_3m_inv_d(inv, denom);

    /* multiply transpose(cfx)*cf */
    for (j=0; j<7; j++) {
      nom[0] += cfx[j*3]  * cf[j];
      nom[1] += cfx[j*3+1]* cf[j];
      nom[2] += cfx[j*3+2]* cf[j];
    }

    /* compute new optgrad = inv*nom */
    ELL_3MV_MUL(optgrad,inv,nom);
  } while (iter++<maxiter);

  return 2; /* hit maxiter */
}

/* Gradient descent to a point on a crease surface
 *
 * NOT used as part of the crease extraction, only for debugging
 *
 * coord are coordinates relative to the given cell and will be
 *      updated in each iteration.
 * Hbfl - Htbr are 9-vectors, representing the Hessian matrices at the
 *      corners of the face (input only)
 * gbfl - gbbr are 3-vectors, representing the gradient directions at the
 *      corners of the face (input only)
 * maxiter is the maximum number of iterations allowed
 * eps is the accuracy up to which |h| (|Tg-g|, cf. paper) must be zero
 * ridge is non-zero if we are looking for a ridge (zero for valley)
 *
 * Returns 0 if the point was found up to the given accuracy
 * Returns 1 if we left the cell
 * Returns 2 if we hit maxiter
 * Returns 3 if Armijo rule failed to find a valid stepsize
 */
int seekDescendToRidge(double *coord,
                       double *Hbfl, double *gbfl, double *Hbfr, double *gbfr,
                       double *Hbbl, double *gbbl, double *Hbbr, double *gbbr,
                       double *Htfl, double *gtfl, double *Htfr, double *gtfr,
                       double *Htbl, double *gtbl, double *Htbr, double *gtbr,
                       int maxiter, double eps, char ridge,
                       const double evalDiffThresh) {
  double dist=0; /* store distance value of previous iteration */

  double Hfrontleft[9], Hbackleft[9];
  double Hfrontright[9], Hbackright[9];
  double Hleft[9], Hright[9];
  double H[9], evals[3], evecs[9], T[9];

  double gfrontleft[3], gbackleft[3];
  double gfrontright[3], gbackright[3];
  double gleft[3], gright[3];
  double g[3];

  double optgrad[3]={0.0,0.0,0.0}; /* gradient for descent */

  int iter=0;
  do {
    double Tg[3];

    /* on the first run, initialize dist; later, employ the Armijo
     * stepsize rule to guarantee convergence */
    double beta=0.1;
    double gamma=0.5;
    double alpha=beta;
    int accept=0;
    double optgradsqr = ELL_3V_DOT(optgrad,optgrad);
    int safetyct=0;
    int maxct=30; /* avoid infinite loops when finding stepsize */
    /* variables used to compute the next step */
    double Hder[9], gder[3], Tder[9];
    double Tpg[3], Tgp[3];
    double Hfront[9], Hback[9], gfront[3], gback[3];
    double Htopleft[9], Htopright[9], Hbotleft[9], Hbotright[9],
      gtopleft[3], gtopright[3], gbotleft[3], gbotright[3],
      Htop[9], Hbot[9], gtop[3], gbot[3];
    while (!accept && safetyct++<maxct) {
      /* compute distance at new position */
      double newcoord[3];
      double diff[3], newdist;
      ELL_3V_SET(newcoord, coord[0]-alpha*optgrad[0],
                 coord[1]-alpha*optgrad[1], coord[2]-alpha*optgrad[2]);

      if (newcoord[0]<0 || newcoord[0]>1 ||
          newcoord[1]<0 || newcoord[1]>1 ||
          newcoord[2]<0 || newcoord[2]>1) {
        if (safetyct==maxct)
          return 1; /* we left the cell */
        alpha*=gamma;
      }

      ELL_3M_LERP(Hfrontleft, newcoord[2], Hbfl, Htfl);
      ELL_3M_LERP(Hbackleft, newcoord[2], Hbbl, Htbl);
      ELL_3M_LERP(Hleft, newcoord[1], Hfrontleft, Hbackleft);

      ELL_3M_LERP(Hfrontright, newcoord[2], Hbfr, Htfr);
      ELL_3M_LERP(Hbackright, newcoord[2], Hbbr, Htbr);
      ELL_3M_LERP(Hright, newcoord[1], Hfrontright, Hbackright);

      ELL_3M_LERP(H, newcoord[0], Hleft, Hright);
      ell_3m_eigensolve_d(evals, evecs, H, AIR_TRUE);

      _seekHess2T(T,evals,evecs,evalDiffThresh,ridge);

      ELL_3V_LERP(gfrontleft, newcoord[2], gbfl, gtfl);
      ELL_3V_LERP(gbackleft, newcoord[2], gbbl, gtbl);
      ELL_3V_LERP(gleft, newcoord[1], gfrontleft, gbackleft);

      ELL_3V_LERP(gfrontright, newcoord[2], gbfr, gtfr);
      ELL_3V_LERP(gbackright, newcoord[2], gbbr, gtbr);
      ELL_3V_LERP(gright, newcoord[1], gfrontright, gbackright);

      ELL_3V_LERP(g, newcoord[0], gleft, gright);

      ell_3mv_mul_d(Tg, T, g);
      ELL_3V_SUB(diff,Tg,g);
      newdist = ELL_3V_DOT(diff,diff);

      if (newdist<eps) {
        ELL_3V_COPY(coord, newcoord); /* update for output */
        return 0; /* we are on the surface */
      }

      if (iter==0 || newdist<=dist-0.5*alpha*optgradsqr) {
        accept=1;
        dist = newdist;
      } else {
        alpha*=gamma;
      }
    }

    if (!accept)
      return 3; /* could not find a valid stepsize */
    coord[0] -= alpha*optgrad[0];
    coord[1] -= alpha*optgrad[1];
    coord[2] -= alpha*optgrad[2];

    if (iter==maxiter-1)
      break;

    /* compute a new optgrad from derivatives of T and g */
    ELL_3V_SUB(gder, gright, gleft); /* x dir */
    ELL_3M_SUB(Hder, Hright, Hleft);
    _seekHessder2Tder(Tder, Hder, evals, evecs, evalDiffThresh, ridge);

    ell_3mv_mul_d(Tpg,Tder,g);
    ell_3mv_mul_d(Tgp,T,gder);
    optgrad[0]=ELL_3V_DOT(Tpg,Tg)+ELL_3V_DOT(Tgp,Tg)-
      ELL_3V_DOT(Tpg,g)-ELL_3V_DOT(Tgp,g)-
      ELL_3V_DOT(Tg,gder)+ELL_3V_DOT(g,gder);

    ELL_3M_LERP(Hfront, coord[0], Hfrontleft, Hfrontright);
    ELL_3M_LERP(Hback, coord[0], Hbackleft, Hbackright);
    ELL_3M_SUB(Hder, Hback, Hfront); /* y dir */
    _seekHessder2Tder(Tder, Hder, evals, evecs, evalDiffThresh, ridge);

    ELL_3V_LERP(gfront, coord[0], gfrontleft, gfrontright);
    ELL_3V_LERP(gback, coord[0], gbackleft, gbackright);
    ELL_3V_SUB(gder, gback, gfront);

    ell_3mv_mul_d(Tpg,Tder,g);
    ell_3mv_mul_d(Tgp,T,gder);
    optgrad[1]=ELL_3V_DOT(Tpg,Tg)+ELL_3V_DOT(Tgp,Tg)-
      ELL_3V_DOT(Tpg,g)-ELL_3V_DOT(Tgp,g)-
      ELL_3V_DOT(Tg,gder)+ELL_3V_DOT(g,gder);

    ELL_3M_LERP(Htopleft, coord[1], Htfl, Htbl);
    ELL_3M_LERP(Htopright, coord[1], Htfr, Htbr);
    ELL_3M_LERP(Hbotleft, coord[1], Hbfl, Hbbl);
    ELL_3M_LERP(Hbotright, coord[1], Hbfr, Hbbr);
    ELL_3M_LERP(Htop, coord[0], Htopleft, Htopright);
    ELL_3M_LERP(Hbot, coord[0], Hbotleft, Hbotright);
    ELL_3M_SUB(Hder, Htop, Hbot); /* z dir */
    _seekHessder2Tder(Tder, Hder, evals, evecs, evalDiffThresh, ridge);

    ELL_3V_LERP(gtopleft, coord[1], gtfl, gtbl);
    ELL_3V_LERP(gtopright, coord[1], gtfr, gtbr);
    ELL_3V_LERP(gbotleft, coord[1], gbfl, gbbl);
    ELL_3V_LERP(gbotright, coord[1], gbfr, gbbr);
    ELL_3V_LERP(gtop, coord[0], gtopleft, gtopright);
    ELL_3V_LERP(gbot, coord[0], gbotleft, gbotright);
    ELL_3V_SUB(gder, gtop, gbot);

    ell_3mv_mul_d(Tpg,Tder,g);
    ell_3mv_mul_d(Tgp,T,gder);
    optgrad[2]=ELL_3V_DOT(Tpg,Tg)+ELL_3V_DOT(Tgp,Tg)-
      ELL_3V_DOT(Tpg,g)-ELL_3V_DOT(Tgp,g)-
      ELL_3V_DOT(Tg,gder)+ELL_3V_DOT(g,gder);
  } while (iter++<maxiter);

  return 2; /* hit maxiter */
}
