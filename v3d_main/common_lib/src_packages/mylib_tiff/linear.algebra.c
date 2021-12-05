/*****************************************************************************************\
*                                                                                         *
*  Matrix inversion, determinants, and linear equations via LU-decomposition              *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  April 2007                                                                    *
*  Mod   :  June 2008 -- Added TDG's and Cubic Spline to enable snakes and curves         *
*           Dec 2008 -- Refined TDG's and cubic splines to Decompose/Solve paradigm       *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "utilities.h"
#include "linear.algebra.h"

#define TINY 1.0e-20

static inline double pivot_check(double pivot)
{ if (pivot == 0.)
    fprintf(stderr,"Singular Matrix!\n");
  else if (fabs(pivot) <= TINY)
    { fprintf(stderr,"Warning: pivot magnitude is less than %g, setting to this value\n",TINY);
      if (pivot < 0.)
        pivot = -TINY;
      else
        pivot = TINY;
    }
  return (pivot);
}


/****************************************************************************************
 *                                                                                      *
 *  TRI-BAND SYSTEM AND CUBIC SPLINE SOLVER                                             *
 *                                                                                      *
 ****************************************************************************************/

//  Triband_Decompose effectively precomputes the inverse of the tridiagonal system [ a, b, c].
//    It does so in a private storage area it shares with Triband_Solve which solves the system
//    [ a, b, c ] x = v returning the answer in v.  Triband_Decompose does not affect the values of
//    a, b, and c.  Triband_Solve can be called repeatedly with different vectors v to solve the
//    same system with different constraints over and over again.  The routines handle the
//    circular case where a[0] != 0 or c[n-1] != 0 albeit a little less efficiently.

//  For the non-circular case, the Thompson algorithm is used where [ a, b, c ] is turned into
//    an upper triangular system.  We unconventionally split this algorithm into the bits that
//    don't depend on v and store those in the precomputation step.  The operation count of 7n
//    for the solution phase is as good as that with a custom LU-decompose.

//  To solve the circular case, we use the Sherman-Morrison formula (Recipes, pg 73).  This
//    involves tweaking the original matrix a bit before decomposing it as a non-circular
//    matrix and then solving two systems with this modified matrix and combining them.  The
//    nice thing is that one system does not depend on v and can be solved in the decomposition
//    phase.  The operation count for the solve stage ends up being an impressive 10n.

//  On a MacBook Pro w. 2.33GHz Intel Core 2 Duo, gcc -O4 compile, times for problems
//    with N = 10,000 are as follows:
//
//      Triband_Decompose, non-circular   .21ms
//      Triband_Decompose, circular       .28ms
//      Triband_Solve, non-circular       .12ms
//      Triband_Solve, circular           .14ms

static double   *TDG_Decom = NULL;
static Dimn_Type TDG_Size = 0;
static int       TDG_Circular;

int Triband_Decompose(Double_Matrix *tri_vector)
{ static Indx_Type wmax = 0;

  Dimn_Type n;
  double   *a, *b, *c;

  double *ap, *bp, *cp;
  double  b0,  bn;

  if (tri_vector->ndims != 2 || tri_vector->dims[1] != 3 || tri_vector->type != FLOAT64)
    { fprintf(stderr,"Matrix is not a 3 x n double array (Triband_Decompose)\n");
      exit (1);
    }

  n = tri_vector->dims[0];

  if (n < 2)
    { fprintf(stderr,"Matrix has second dimension less than 2! (Triband_Decompose)\n");
      exit (1);
    }

  a = AFLOAT64(tri_vector);
  b = a + n;
  c = b + n;

  if (4*((Indx_Type) n) >= wmax)
    { wmax = 4.8*n + 100;
      TDG_Decom = (double *) Guarded_Realloc(TDG_Decom,sizeof(double)*wmax,Program_Name());
    }

  ap = TDG_Decom;
  bp = ap + n;
  cp = bp + n;

  TDG_Size     = n;
  TDG_Circular = (a[0] != 0. || c[n-1] != 0.);

  // For circular case, let g = [ -b0 ..0.. cn ]^T and h = [ 1 ..0.. -a0/b0 ]
  //   Let A' = A - g * h which by the choice of g and h is non-cyclic

  if (TDG_Circular)
    { b0 = b[0];     //  Save the elements of b that are about to be trashed
      bn = b[n-1];

      if (b0 == 0.)
        return (1);

      b[0]   += b0;
      b[n-1] += (a[0]*c[n-1])/b0;
    }

  { double    wm1, ai, bi;  //  prepare the inverse of non-circular A
    Dimn_Type i;            //    (sneaky: a[0] and c[n-1] values play no part in forming a result
                            //     even though we didn't zero them in the circular case)
    wm1 = 0.;
    for (i = 0; i < n; i++)
      { ap[i] = ai = -a[i];
  
        bi = pivot_check(b[i] + ai*wm1);
        if (bi == 0.)
          { if (TDG_Circular)
              { b[0]   = b0;   //  Restore the elements of b that got trashed in the circular case
                b[n-1] = bn;
              }
            return (1);
          }
  
        bp[i] = bi = 1./bi;
        cp[i] = wm1 = c[i] * bi;
      }
  }

  if (TDG_Circular)      //  for circular case, solve system A' * z = g (take advantage of 0's)

    { double *z = cp + n;

      { double    zm1;
        Dimn_Type i;

        z[0] = zm1 = -.5;            //  sneaky: know g[0] * bp[0] = -b0 * (1./2*b0) = -.5
        for (i = 1; i < n-1; i++)
          z[i] = zm1 *= ap[i]*bp[i];
        z[n-1] = zm1 = (c[n-1] + ap[n-1]*zm1) * bp[n-1];
  
        for (i = n-1; i-- > 0; )
          z[i] = zm1 = z[i] - cp[i]*zm1;
      }

      b[0]   = b0;     //  Restore the elements of b that got trashed
      b[n-1] = bn;
    }

  return (0);
}

Double_Vector *Triband_Solve(Double_Vector *R(M(values)))
{ Dimn_Type n;
  double   *v;
  double   *ap, *bp, *cp;

  if (values->ndims != 1 || values->type != FLOAT64)
    { fprintf(stderr,"Vector is not a 1 dimensional double array (TDG_Value)\n");
      exit (1);
    }
  
  n = values->dims[0];
  v = AFLOAT64(values);

  if (TDG_Size != n)
    { fprintf(stderr,"Vector length does not match last system decomposition (TDG_Value)\n");
      exit (1);
    }

  ap = TDG_Decom;
  bp = ap + n;
  cp = bp + n;

  { double    vm1;             //  solve core system
    Dimn_Type i;

    vm1 = 0.;
    for (i = 0; i < n; i++)
      v[i] = vm1 = (v[i] + ap[i]*vm1) * bp[i];
  
    for (i = n-1; i-- > 0; )
      v[i] = vm1 = v[i] - cp[i]*vm1;
  }

  if (TDG_Circular)       //  for circular case, v += ( h*v / (1 + h*z) ) z

    { double *z = cp + n;

      { double    hn, gm;
        Dimn_Type i;

        hn = 2.*ap[0]*bp[0];  // sneaky: h[n-1] = -a0/b0 = 2.*(-a0)*(1/(2*b0)) = 2.*ap[0]*bp[0]
        gm = (v[0] + hn*v[n-1]) / (1. + z[0] + hn*z[n-1]);

        for (i = 0; i < n; i++)
          v[i] -= gm * z[i];
      }
    }
}


/*
    Given control points v1, v2, ... vn, compute the Bezier displacements for cubic
    splines between them such that (a) the points are interpolated, and (b) 1st and
    2nd order continuous.  If the curve is to begin at v1 and end at vn then these
    are the solution [d] to the tridiagonal system at left, and if the curve is to be
    closed then they are the solution to the system at right.

       | 2 1        | d1     v2-v1         | 4 1     1 |  d1     v2-vn
       | 1 4 1      | d2     v3-v1         | 1 4 1     |  d2     v3-v1
       |   1 4 1    | d3     v4-v2         |   1 4 1   |  d3     v4-v2
       |     .....  |      =               |     ..... |      =
       |      1 4 1 | dn-1   vn-vn-2       |     1 4 1 | dn-1   vn-vn-2
       |        1 2 | dn     vn-vn-1       | 1     1 4 | dn     v1-vn-1

    We use a tailored version of the TDG routines to return the desired displacements
    in the array v.  The required decompose step is cached and reused with each call if
    n and circular are unchanged.  Op counts are 5n for the non-circular case and 7n for
    the circular case
*/

//  On a MacBook Pro w. 2.33GHz Intel Core 2 Duo, gcc -O4 compile, times for problems
//    with N = 10,000 are as follows:
//
//      Cubic_Spline_Slopes, non-circular, decompose   .18ms
//      Cubic_Spline_Slopes, circular, decompose       .26ms
//      Cubic_Spline_Slopes, non-circular, solve       .11ms
//      Cubic_Spline_Slopes, circular, solve           .13ms

Double_Vector *Cubic_Spline_Slopes(Double_Vector *R(M(values)), int circular)
{ static double    *w = NULL, *u;
  static Indx_Type  wmax = 0;
  static Dimn_Type  nlast = 0;
  static int        clast = 0;

  Dimn_Type  i, n;
  double    *v, wm1, vm1, b, v0, gm;

  if (values->ndims != 1 || values->type != FLOAT64)
    { fprintf(stderr,"Vector is not a 1 dimensional double array (Cubic_Spline_Slopes)\n");
      exit (1);
    }
  
  n = values->dims[0];
  v = AFLOAT64(values);

  if (nlast != n || clast != circular)

    { if (2*((Indx_Type) n) >= wmax)
        { wmax = 2.4*n + 100;
          w    = (double *) Guarded_Realloc(w,sizeof(double)*wmax,Program_Name());
        }

      if (circular)
        b = 3.;
      else
        b = 2.;

      w[0] = wm1 = 1./b;            //  pre-compute that portion of the inversion that is
      for (i = 1; i < n-1; i++)     //    invariant for the given value of n
        w[i] = wm1 = 1. / (4.-wm1);
      w[n-1] = 1. / (b - wm1);

      if (circular)
        { u = w + n;
    
          u[0] = wm1 = w[0];   //  solve system with g = [ 1 0... 1 ] as v, take advantage of 0's
          for (i = 1; i < n-1; i++)
            u[i] = wm1 *= -w[i];
          u[n-1] = wm1 = (1. - wm1) * w[n-1];
    
          for (i = n-1; i-- > 0; )
            u[i] = wm1 = u[i] - w[i]*wm1;
        }
    }

  v0 = vm1 = v[0];
  if (circular)
    v[0] = v[1] - v[n-1];
  else
    v[0] = v[1] - vm1;
  for (i = 1; i < n-1; i++)
    { double vi = v[i];
      v[i] = v[i+1] - vm1;
      vm1  = vi;
    }
  if (circular)
    v[n-1] = v0-vm1;
  else
    v[n-1] -= vm1;

  vm1 = 0.;
  for (i = 0; i < n; i++)
    v[i] = vm1 = (v[i] - vm1) * w[i];

  for (i = n-1; i-- > 0; )
    vm1 = (v[i] -= w[i]*vm1);

  if (circular)
    { u = w + n;

      gm = (v[0] + v[n-1]) / (1. + u[0]  + u[n-1]);   // h.v / (1 + h.u)

      for (i = 0; i < n; i++)
        v[i] -= gm * u[i];
    }

  nlast = n;
  clast = circular;
}


/****************************************************************************************
 *                                                                                      *
 *  PENTA-BAND SYSTEM SOLVER                                                            *
 *                                                                                      *
 ****************************************************************************************/

//  Pentaband_Decompose effectively precomputes the inverse of the penta-diagonal system
//    [ a, b, c, d, e].  It does so in a private storage area it shares with Pentaband_Solve
//    which solves the system [ a, b, c, d, e ] x = v returning the answer in v.
//    Pentaband_Decompose does not affect the values of a, b, c, d, and e.  Pentaband_Solve
//    can be called repeatedly with different vectors v to solve the same system with different
//    constraints over and over again.  The routines handle the circular case albeit a little
//    less efficiently.

//  For the non-circular case, the Thompson algorithm is used where [ a, b, c, d, e ] is turned into
//    an upper triangular system.  We unconventionally split this algorithm into the bits that
//    don't depend on v and store those in the precomputation step.  The operation count of 11n
//    for the solution phase is as good as that with a custom LU-decompose.

//  To solve the circular case, we use the Woodbury formula (Recipes, pg 73) that is a
//    generalization of the Sherman-Morrison formula.  Its use involves tweaking the original
//    matrix a bit before decomposing it as a non-circular  matrix and then solving three systems
//    with this modified matrix and taking a linear combination thereof where the coefficents
//    depend on the two systems that do not depend on v.  As for the tridiagonal case, the nice
//    thing is that only one system depends on v and the other two can be solved in the
//    decomposition phase.  The operation count for the solve stage ends up being an
//    impressive 16n.

//  On a MacBook Pro w. 2.33GHz Intel Core 2 Duo, gcc -O4 compile, times for problems
//    with N = 10,000 are as follows:
//
//      Pentaband_Decompose, non-circular   .25ms
//      Pentaband_Decompose, circular       .52ms
//      Pentaband_Solve, non-circular       .17ms
//      Pentaband_Solve, circular           .23ms

static double   *PDG_Decom = NULL;
static Dimn_Type PDG_Size  = 0;
static int       PDG_Circular;

int Pentaband_Decompose(Double_Matrix *penta_vector)
{ static Indx_Type wmax = 0;

  Dimn_Type  n;
  double    *a, *b, *c, *d, *e;

  double *ap, *bp, *cp, *dp, *ep;
  double  h0m, h0n, h1n, p0, p1;
  double  c0, c1, cm, cn, dm, bn;

  if (penta_vector->ndims != 2 || penta_vector->dims[1] != 5 || penta_vector->type != FLOAT64)
    { fprintf(stderr,"Matrix is not a 5 x n double array (Pentaband_Decompose)\n");
      exit (1);
    }

  n = penta_vector->dims[0];

  if (n < 3)
    { fprintf(stderr,"Matrix has second dimension less than 3! (Pentaband_Decompose)\n");
      exit (1);
    }

  a = AFLOAT64(penta_vector);
  b = a + n;
  c = b + n;
  d = c + n;
  e = d + n;

  if (7*((Indx_Type) n)+8 >= wmax)
    { wmax = 8.4*n + 100;
      PDG_Decom = (double *) Guarded_Realloc(PDG_Decom,sizeof(double)*wmax,Program_Name());
    }

  ap = PDG_Decom;
  bp = ap + n;
  cp = bp + n;
  dp = cp + n;
  ep = dp + n;

  PDG_Size     = n;
  PDG_Circular = (a[0] != 0. || a[1] != 0. || b[0] != 0. ||
                  d[n-1] != 0. || e[n-1] != 0. || e[n-2] != 0.);

  //  For the circular case, let G = [ -p0   0 0... en-2 dn-1 ]^T and H = [ 1 0 0... a0/-p0 b0/-p0 ]
  //                                 [   0 -p1 0...    0 en-1 ]           [ 0 1 0...    0   a1/-p1 ]
  //    Let A' = A - G * H which by the choice of G and H is a non-circular matrix

  if (PDG_Circular)
    { c0 = c[0];     //  Save the elements of b, c, and d that are about to be trashed
      c1 = c[1];
      cm = c[n-2];
      cn = c[n-1];
      dm = d[n-2];
      bn = b[n-1];

      p0 = c0;
      p1 = c1 - b[1]*(d[0]/c0);   // 2nd pivot of the unalterred matrix, should != 0 if invertable

      if (p0 == 0 || p1 == 0)
        return (1);

      h0m = a[0]/p0;
      h0n = b[0]/p0;
      h1n = a[1]/p1;

      c[0]   += p0;
      c[1]   += p1;
      c[n-1] += h0n*d[n-1] + h1n*e[n-1];
      c[n-2] += h0m*e[n-2];
      d[n-2] += h0n*e[n-2];
      b[n-1] += h0m*d[n-1];
    }

  { double    dm2, em2;      //  prepare the inverse of non-circular A
    double    dm1, em1;
    double    ai, bi, ci;
    Dimn_Type i;

    em1 = em2 = 0.;
    dm1 = dm2 = 0.;
    for (i = 0; i < n; i++)
      { ap[i] = ai = a[i];
  
        bp[i] = bi = b[i] - ai*dm2; 
        ci = pivot_check(c[i] - (ai*em2 + bi*dm1));
        if (ci == 0.)
          { if (PDG_Circular)
              { c[0]   = c0;     //  Restore the elements of b, c, and d that got
                c[1]   = c1;     //    trashed in the circular case
                c[n-2] = cm;
                c[n-1] = cn;
                d[n-2] = dm;
                b[n-1] = bn;
              }
            return (1);
          }
  
        em2 = em1;
        dm2 = dm1;
  
        cp[i] = ci = 1./ci;
        dp[i] = dm1 = (d[i] - bi*em1) * ci;
        ep[i] = em1 = e[i] * ci;
      }
  }
  
  if (PDG_Circular)   //  Prepare for solving by computing and storing: z1, z2, and pv s.t.
                      //    A * zi = gi and pv = (I + H*Z)^-1 * H where Z = [ z1 z2 ]
    { double *z1 = ep + n;
      double *z2 = z1 + n;
      double *pv = z2 + n;

      { double    ui, um1, um2;          //  Solve A * z1 = u1 and A * z2 = u2
        double    wi, wm1, wm2;
        Dimn_Type i;

        z1[0] = um2 = -.5;      //  sneaky: know g1[0] * cp[0] = -c0 * (1./2*c0) = -.5
        z2[0] = wm2 =  .0;
        z1[1] = um1 =  .5*bp[1]*cp[1];
        z2[1] = wm1 = -p1*cp[1];
        for (i = 2; i < n-2; i++)
          { z1[i] = ui = - (ap[i]*um2 + bp[i]*um1) * cp[i];
            um2  = um1;
            um1  = ui;
            z2[i] = wi = - (ap[i]*wm2 + bp[i]*wm1) * cp[i];
            wm2  = wm1;
            wm1  = wi;
          }
        z1[n-2] = ui  = (e[n-2] - (ap[n-2]*um2 + bp[n-2]*um1)) * cp[n-2];
        z2[n-2] = wi  = (       - (ap[n-2]*wm2 + bp[n-2]*wm1)) * cp[n-2];
        z1[n-1] = um1 = (d[n-1] - (ap[n-1]*um1 + bp[n-1]*ui )) * cp[n-1];
        z2[n-1] = wm1 = (e[n-1] - (ap[n-1]*wm1 + bp[n-1]*wi )) * cp[n-1];

        um2 = wm2 = 0.;
        for (i = n-1; i-- > 0; )
          { z1[i] = ui = z1[i] - (dp[i]*um1 + ep[i]*um2);
            um2  = um1;
            um1  = ui;
            z2[i] = wi = z2[i] - (dp[i]*wm1 + ep[i]*wm2);
            wm2  = wm1;
            wm1  = wi;
          }
      }

      { double p11, p12, p21, p22;
        double fac, piv;
        double q11, q12, q21, q22;

        p11 = 1. + z1[0] - (h0m*z1[n-2] + h0n*z1[n-1]);   //  [ p11 p12 ] = P = I + H * Z
        p12 =      z2[0] - (h0m*z2[n-2] + h0n*z2[n-1]);   //  [ p21 p22 ]
        p21 =      z1[1] -                h1n*z1[n-1];
        p22 = 1. + z2[1] -                h1n*z2[n-1];

        fac = -p21/p11;             //  Q = P^-1  (does this need pivoting?)
        piv = p22 + p12*fac;
        q21 = fac / piv; 
        q22 = 1. / piv;
        q11 = (1. - p12*q21) / p11;
        q12 = -p12*q22 / p11;

        pv[0] = q11;                //  [ pv0 pv1  ...0...  -pv2 -pv3 ]  = Q * H = PV
        pv[1] = q12;                //  [ pv4 pv5  ...0...  -pv6 -pv7 ]
        pv[2] = q11*h0m;
        pv[3] = q11*h0n + q12*h1n;
        pv[4] = q21;
        pv[5] = q22;
        pv[6] = q21*h0m;
        pv[7] = q21*h0n + q22*h1n;
      }

      c[0]   = c0;     //  Restore the elements of b, c, and d that got trashed
      c[1]   = c1;
      c[n-2] = cm;
      c[n-1] = cn;
      d[n-2] = dm;
      b[n-1] = bn;
    }

  return (0);
}

Double_Vector *Pentaband_Solve(Double_Vector *R(M(values)))
{ double    *v, *ap, *bp, *cp, *dp, *ep, *u;
  double    vm1, vm2, vi;
  Dimn_Type i, n;

  if (values->ndims != 1 || values->type != FLOAT64)
    { fprintf(stderr,"Vector is not a 1 dimensional double array (PDG_Value)\n");
      exit (1);
    }
  
  n = values->dims[0];
  v = AFLOAT64(values);

  if (PDG_Size != n)
    { fprintf(stderr,"Vector length does not match last system decomposition (PDG_Value)\n");
      exit (1);
    }

  ap = PDG_Decom;
  bp = ap + n;
  cp = bp + n;
  dp = cp + n;
  ep = dp + n;

  vm1  = vm2  = 0.;
  for (i = 0; i < n; i++)
    { v[i] = vi = (v[i] - (ap[i]*vm2 + bp[i]*vm1)) * cp[i];
      vm2  = vm1;
      vm1  = vi;
    }

  vm2 = 0.;
  for (i = n-1; i-- > 0; )
    { v[i] = vi = v[i] - (dp[i]*vm1 + ep[i]*vm2);
      vm2  = vm1;
      vm1  = vi;
    }

  if (PDG_Circular)        //  v += Z * (PV * v);
    { double *z1 = ep + n;
      double *z2 = z1 + n;
      double *pv = z2 + n;

      double f1 = (pv[0]*v[0] + pv[1]*v[1]) - (pv[2]*v[n-2] + pv[3]*v[n-1]);
      double f2 = (pv[4]*v[0] + pv[5]*v[1]) - (pv[6]*v[n-2] + pv[7]*v[n-1]);
      int    i;

      for (i = 0; i < n; i++)
        v[i] = v[i] - (f1*z1[i] + f2*z2[i]);
    }
}


/****************************************************************************************
 *                                                                                      *
 *  LU-FACTORIZATION SYSTEM SOLVER                                                      *
 *                                                                                      *
 ****************************************************************************************/

double *get_lu_vector(Dimn_Type n, char *routine)
{ static double   *LU_Decom = NULL;
  static Indx_Type LU_Size  = 0;

  if (n == 0)
    { free (LU_Decom);
      LU_Decom = NULL;
      LU_Size  = 0;
    }
  if (n > LU_Size)
    { LU_Size  = 1.2*n + 100;
      LU_Decom = (double *) Guarded_Realloc(LU_Decom,(sizeof(double)+sizeof(double *))*LU_Size,
                                            routine);
    }
  return (LU_Decom);
}

static inline uint64 lu_factor_psize(LU_Factor *f)
{ return (f->lu_mat->dims[0]*sizeof(int)); }


typedef struct __LU_Factor
  { struct __LU_Factor *next;
    struct __LU_Factor *prev;
    int                 refcnt;
    int                 psize;
    LU_Factor           lu_factor;
  } _LU_Factor;

static _LU_Factor *Free_LU_Factor_List = NULL;
static _LU_Factor *Use_LU_Factor_List  = NULL;
static _LU_Factor  LU_Factor_Proto;

static int LU_Factor_Offset = ((char *) &(LU_Factor_Proto.lu_factor)) - ((char *) &LU_Factor_Proto);
static int LU_Factor_Inuse  = 0;

int LU_Factor_Refcount(LU_Factor *lu_factor)
{ _LU_Factor *object = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  return (object->refcnt);
}

static inline void allocate_lu_factor_perm(LU_Factor *lu_factor, int psize, char *routine)
{ _LU_Factor *object = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  if (object->psize < psize)
    { lu_factor->perm = Guarded_Realloc(lu_factor->perm,psize,routine);
      object->psize = psize;
    }
}

static inline int sizeof_lu_factor_perm(LU_Factor *lu_factor)
{ _LU_Factor *object = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  return (object->psize);
}

static inline LU_Factor *new_lu_factor(int psize, char *routine)
{ _LU_Factor *object;
  LU_Factor  *lu_factor;

  if (Free_LU_Factor_List == NULL)
    { object = (_LU_Factor *) Guarded_Realloc(NULL,sizeof(_LU_Factor),routine);
      lu_factor = &(object->lu_factor);
      object->psize = 0;
      lu_factor->perm = NULL;
    }
  else
    { object = Free_LU_Factor_List;
      Free_LU_Factor_List = object->next;
      lu_factor = &(object->lu_factor);
    }
  LU_Factor_Inuse += 1;
  object->refcnt = 1;
  if (Use_LU_Factor_List != NULL)
    Use_LU_Factor_List->prev = object;
  object->next = Use_LU_Factor_List;
  object->prev = NULL;
  Use_LU_Factor_List = object;
  lu_factor->lu_mat = NULL;
  allocate_lu_factor_perm(lu_factor,psize,routine);
  return (lu_factor);
}

static inline LU_Factor *copy_lu_factor(LU_Factor *lu_factor)
{ LU_Factor *copy = new_lu_factor(lu_factor_psize(lu_factor),"Copy_LU_Factor");
  void *_perm = copy->perm;
  *copy = *lu_factor;
  if (lu_factor->lu_mat != NULL)
    copy->lu_mat = Copy_Array(lu_factor->lu_mat);
  copy->perm = _perm;
  if (lu_factor->perm != NULL)
    memcpy(copy->perm,lu_factor->perm,lu_factor_psize(lu_factor));
  return (copy);
}

LU_Factor *Copy_LU_Factor(LU_Factor *lu_factor)
{ return ((LU_Factor *) copy_lu_factor(lu_factor)); }

static inline void pack_lu_factor(LU_Factor *lu_factor)
{ _LU_Factor *object  = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  if (lu_factor->lu_mat != NULL)
    Pack_Array(lu_factor->lu_mat);
  if (object->psize > lu_factor_psize(lu_factor))
    { object->psize = lu_factor_psize(lu_factor);
      if (object->psize != 0)
        lu_factor->perm = Guarded_Realloc(lu_factor->perm,
                                          object->psize,"Pack_LU_Factor");
      else
        { free(lu_factor->perm);
          lu_factor->perm = NULL;
        }
    }
}

LU_Factor *Pack_LU_Factor(LU_Factor *lu_factor)
{ pack_lu_factor(lu_factor);
  return (lu_factor);
}

LU_Factor *Inc_LU_Factor(LU_Factor *lu_factor)
{ _LU_Factor *object  = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  object->refcnt += 1;
  return (lu_factor);
}

static inline void free_lu_factor(LU_Factor *lu_factor)
{ _LU_Factor *object  = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  if (--object->refcnt > 0) return;
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released LU_Factor\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_LU_Factor_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_LU_Factor_List;
  Free_LU_Factor_List = object;
  if (lu_factor->lu_mat != NULL)
    Free_Array(lu_factor->lu_mat);
  LU_Factor_Inuse -= 1;
}

void Free_LU_Factor(LU_Factor *lu_factor)
{ free_lu_factor(lu_factor); }

static inline void kill_lu_factor(LU_Factor *lu_factor)
{ _LU_Factor *object  = (_LU_Factor *) (((char *) lu_factor) - LU_Factor_Offset);
  if (--object->refcnt > 0) return;
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released LU_Factor\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_LU_Factor_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  if (object->psize != 0)
    free(lu_factor->perm);
  if (lu_factor->lu_mat != NULL)
    Kill_Array(lu_factor->lu_mat);
  free(((char *) lu_factor) - LU_Factor_Offset);
  LU_Factor_Inuse -= 1;
}

void Kill_LU_Factor(LU_Factor *lu_factor)
{ kill_lu_factor(lu_factor); }

static inline void reset_lu_factor()
{ _LU_Factor *object;
  LU_Factor  *lu_factor;
  while (Free_LU_Factor_List != NULL)
    { object = Free_LU_Factor_List;
      Free_LU_Factor_List = object->next;
      lu_factor = &(object->lu_factor);
      if (object->psize != 0)
        free(lu_factor->perm);
      if (lu_factor->lu_mat != NULL)
        Kill_Array(lu_factor->lu_mat);
      free(object);
    }
}

int LU_Factor_Usage()
{ return (LU_Factor_Inuse); }

void LU_Factor_List(void (*handler)(LU_Factor *))
{ _LU_Factor *a, *b;
  for (a = Use_LU_Factor_List; a != NULL; a = b)
    { b = a->next;
      handler((LU_Factor *) &(a->lu_factor));
    }
}

static inline LU_Factor *read_lu_factor(FILE *input)
{ char name[9];
  fread(name,9,1,input);
  if (strncmp(name,"LU_Factor",9) != 0)
    return (NULL);
  LU_Factor *obj = new_lu_factor(0,"Read_LU_Factor");
  fread(obj,sizeof(LU_Factor),1,input);
  if (obj->lu_mat != NULL)
    obj->lu_mat = Read_Array(input);
  obj->perm = NULL;
  if (lu_factor_psize(obj) != 0)
    { allocate_lu_factor_perm(obj,lu_factor_psize(obj),"Read_LU_Factor");
      fread(obj->perm,lu_factor_psize(obj),1,input);
    }
  return (obj);
}

LU_Factor *Read_LU_Factor(FILE *input)
{ return ((LU_Factor *) read_lu_factor(input)); }

static inline void write_lu_factor(LU_Factor *lu_factor, FILE *output)
{ fwrite("LU_Factor",9,1,output);
  fwrite(lu_factor,sizeof(LU_Factor),1,output);
  if (lu_factor->lu_mat != NULL)
    Write_Array(lu_factor->lu_mat,output);
  if (lu_factor_psize(lu_factor) != 0)
    fwrite(lu_factor->perm,lu_factor_psize(lu_factor),1,output);
}

void Write_LU_Factor(LU_Factor *lu_factor, FILE *output)
{ write_lu_factor(lu_factor,output); }

void Reset_LU_Factor()
{ reset_lu_factor();
  get_lu_vector(0,NULL);
}

//  m is a square double matrix where the row index moves the fastest.
//    LU_Decompose takes M and produces an LU factorization of m that
//    can then be used to rapidly solve the system for given right hand sides
//    and to compute m's determinant.  The return value is NULL if the matrix
//    is nonsingular.  If the matrix appears unstable (had to use a very nearly
//    zero pivot) then the integer pointed at by stable will be zero, and
//    non-zero otherwise.  m is subsumed and effectively destroyed by the routine.

LU_Factor *G(LU_Decompose)(Double_Matrix *S(M(m)), int *O(stable))
{ LU_Factor      *factor;
  Dimn_Type       n, i, j;
  double        **a, *v;
  int            *p, sign;

  if (m->ndims != 2 || m->type != FLOAT64 || m->dims[0] != m->dims[1])
    { fprintf(stderr,"Matrix is not a square 2D double array (LU_Decompose)\n");
      exit (1);
    }

  n = m->dims[0];
  v = get_lu_vector(n,"LU_Decompose");
  a = (double **) (v + n);

  factor = new_lu_factor(n,"LU_Decompose");
  p      = factor->perm;

  p[0] = 0;
  a[0] = AFLOAT64(m);
  for (i = 1; i < n; i++)
    { a[i] = a[i-1] + n;
      p[i] = i;
    }

  *stable = 1;
  sign    = 1;
  for (i = 0; i < n; i++)  // Find the scale factors for each row in v.
    { double b, f, *r;

      r = a[i];
      b = 0.;
      for (j = 0; j < n; j++)
        { f = fabs(r[j]);
          if (f > b)
            b = f;
        }
      if (b == 0.0)
        { Kill_Array(m);
          Kill_LU_Factor(factor);
          return (NULL);
        }
      v[i] = 1./b;
    }

  for (j = 0; j < n; j++)      //  For each column
    { double    b, s, *r;
      Dimn_Type k, w;

      for (i = 0; i < j; i++)    // Determine U
        { r = a[i];
          s = r[j];
          for (k = 0; k < i; k++)
            s -= r[k]*a[k][j];
          r[j] = s;
        }

      b = -1.;
      for (i = j; i < n; i++)      // Determine L without dividing by pivot, in order to
        { r = a[i];                //   determine who the pivot should be.
          s = r[j];
          for (k = 0; k < j; k++)
            s -= r[k]*a[k][j];
          r[j] = s;

          s = v[i]*fabs(s);        // Update best pivot seen thus far
          if (s > b)
            { b = s;
              w = i;
            }
	}

      if (w != j)                  // Pivot if necessary
        { r    = a[w];
          a[w] = a[j];
          a[j] = r;
          k    = p[w];
          p[w] = p[j];
          p[j] = k;
          sign = -sign;
          v[w] = v[j];
        }

      if (fabs(a[j][j]) < TINY)    // Complete column of L by dividing by selected pivot
        { if (a[j][j] < 0.)
            a[j][j] = -TINY;
          else
            a[j][j] = TINY;
          *stable = 0;
        }
      b = 1./a[j][j];
      for (i = j+1; i < n; i++)
        a[i][j] *= b;
    }

#ifdef DEBUG_LU
  { Dimn_Type i, j;

    printf("\nLU Decomposition\n");
    for (i = 0; i < n; i++)
      { printf("  %2d: ",p[i]);
        for (j = 0; j < n; j++)
          printf(" %8g",a[i][j]);
        printf("\n");
      }
  }
#endif

  factor->sign   = sign;
  factor->lu_mat = m;
  return (factor);
}

void Show_LU_Product(FILE *file, LU_Factor *f)
{ Dimn_Type n, i, j, k;
  int      *p;
  double    u, **a, *d;

  n = f->lu_mat->ndims;
  a = (double **) get_lu_vector(n,"Show_LU_Product");
  d = AFLOAT64(f->lu_mat);
  p = f->perm;

  for (i = 0; i < n; i++)
    a[i] = d + p[i]*n;

  fprintf(file,"\nLU Product:\n");
  for (i = 0; i < n; i++)
    { for (j = 0; j < i; j++)
        { u = 0.;
          for (k = 0; k <= j; k++)
            u += a[i][k] * a[k][j];
          fprintf(file," %g",u);
        }
      for (j = i; j < n; j++)
        { u = a[i][j];
          for (k = 0; k < i; k++)
            u += a[i][k] * a[k][j];
          fprintf(file," %g",u);
        }
      fprintf(file,"\n");
    }
}


//  Given rhs vector b and LU-factorization f, solve the system of equations
//    and return the result in b.
//  To invert a given the LU-decomposition, simply call LU_Solve with
//    b = [ 0^k-1 1 0^n-k] to get the k'th column of the inverse matrix.

Double_Vector *LU_Solve(Double_Vector *R(M(bv)), LU_Factor *f)
{ double   *x;
  Dimn_Type n, i, j;
  int      *p;
  double   *a, *b, s, *r;

  if (bv->ndims != 1 || bv->type != FLOAT64 || bv->dims[0] != f->lu_mat->dims[0])
    { fprintf(stderr,"B-vector is not a double 1D array of same size as matrix (LU_Solve)\n");
      exit (1);
    }

  n = f->lu_mat->dims[0];
  a = AFLOAT64(f->lu_mat);
  p = f->perm;
  b = AFLOAT64(bv);

  x = get_lu_vector(n,"LU_Solve");

  for (i = 0; i < n; i++)
    { r = a + p[i]*n;
      s = b[p[i]];
      for (j = 0; j < i; j++)
        s -= r[j] * x[j];
      x[i] = s;
    }

  for (i = n; i-- > 0; )
    { r = a + p[i]*n;
      s = x[i]; 
      for (j = i+1; j < n; j++)
        s -= r[j] * b[j];
      b[i] = s/r[i];
    }
}

//  Generate a square identity matrix of size n

Double_Matrix *Identity_Matrix(Dimn_Type n)
{ Dimn_Type      i, j, dims[2];
  Indx_Type      p;
  double        *a;
  Double_Matrix *g;
 
  dims[0] = dims[1] = n;
  g = Make_Array(PLAIN_KIND,FLOAT64,2,dims);

  a = AFLOAT64(g);

  p = 0;
  for (j = 0; j < n; j++)                 //  Set g to the identity matrix
    for (i = 0; i < n; i++)
      if (i == j)
        a[p++] = 1.;
      else
        a[p++] = 0.;

  return (g);
}

//  Transpose a square matrix g and as a convenience return a pointer to it

Double_Matrix *Transpose_Matrix(Double_Matrix *g)
{ Dimn_Type      n, i, j;
  Indx_Type      p, q;
  double        *a;

  n = g->dims[0];
  a = AFLOAT64(g);
 
  p = 0;
  for (j = 0; j < n; j++)                 //  Transpose the result
    { q = j;
      for (i = 0; i < j; i++)
        { double x = a[p];
          a[p++] = a[q];
          a[q] = x;
          q += n;
        }
      p += (n-j);
    }
}

//  Generate the right inverse of the matrix that gave rise to the LU factorization f.
//    That is for matrix A, return matrix A^-1 s.t. A * A^-1 = I.  If transpose is non-zero
//    then the transpose of the right inverse is returned.

Double_Matrix *LU_Invert(LU_Factor *f, int transpose)
{ Dimn_Type      n, i;
  Double_Matrix *g;

  n = f->lu_mat->dims[0];

  g = Identity_Matrix(n);

  for (i = 0; i < n; i++)                 //  Find the inverse of each column in the
    LU_Solve(Get_Array_Plane(g,i),f);     //    corresponding *row* (i.e. the transpose)

  if (!transpose)
    Transpose_Matrix(g);

  return (g);
}

//  Given an LU-factorization f, return the value of the determinant of the
//    original matrix.

double LU_Determinant(LU_Factor *f)
{ Dimn_Type i, n;
  int      *p;
  double   *a, det;

  n = f->lu_mat->dims[0];
  a = AFLOAT64(f->lu_mat);
  p = f->perm;

  det = f->sign;
  for (i = 0; i < n; i++)
    det *= a[p[i]*n+i];
  return (det);
}


/****************************************************************************************
 *                                                                                      *
 *  ORTHOGONAL BASIS                                                                    *
 *                                                                                      *
 ****************************************************************************************/

//  Make an orthogonal rotation matrix that spans the same space as basis.
//    Use a "stable" version of Gram-Schmidt.  NR says should use SVD, but that's
//    a lot more code, so be cautious about stability of output matrix.

Double_Matrix *Orthogonalize_Matrix(Double_Matrix *R(M(basis)))
{ Dimn_Type n;
  Indx_Type n2;
  double   *mag;

  if (basis->ndims != 2 || basis->type != FLOAT64 || basis->dims[0] != basis->dims[1])
    { fprintf(stderr,"basis matrix is not a square double 2D array (Orthogonalize_Array)\n");
      exit (1);
    }

  n   = basis->dims[0];
  n2  = basis->size;
  mag = get_lu_vector(n,"Orthogonalize_Array");

  { float64  *v = (float64 *) (basis->data);
    Indx_Type j, h;
    Dimn_Type i, k;
    double    x, b;

    for (j = 0; j < n2; j += n)
      { for (h = i = 0; h < j; i++, h += n)
          { b = 0;                   // v_j = v_j - proj(v_i) v_j  (= (v_i . v_j) / (v_i . v_i) v_i
            for (k = 0; k < n; k++)
              b += v[h+k] * v[j+k];
            b /= mag[i];
            for (k = 0; k < n; k++)
              x = v[j+k] -= b * v[h+k];
          }

        b = 0;                      // v_j = v_j / ||v_j||
        for (k = 0; k < n; k++)
          { x  = v[j+k];
            b += x*x;
          }
        mag[i] = 1./b;
        b = 1./sqrt(mag[i]);
        for (k = 0; k < n; k++)
          v[j+k] *= b;
      }
  }

  return (basis);
}
