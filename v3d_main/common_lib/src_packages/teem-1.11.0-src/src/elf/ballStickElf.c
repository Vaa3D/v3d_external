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

#if TEEM_LEVMAR
#include <levmar.h>
#endif

/* Routines for estimating the ball-and-multi-stick model from single-shell
 * DWI data. Implements the ideas from
 *
 * T. Schultz, C.-F. Westin, G. Kindlmann: Multi-Diffusion-Tensor
 * Fitting via Spherical Deconvolution: A Unifying Framework. MICCAI
 * 2010, Part I, LNCS 6361, pp. 673-680. Springer, 2010
 */

/* elfKernelStick:
 *
 * Computes the deconvolution kernel corresponding to a single stick, for
 * desired ESH order, bd (b value times diffusivity) and b0 (non-DWI signal)
 * If delta!=0, deconvolves to delta peak, else to rank-1 peak
 *
 * returns 0 on success, 1 if order is not supported
 */
int elfKernelStick_f(float *kernel, unsigned int order, float bd,
                     float b0, int delta) {
  double ebd=exp(bd);
  double embd=exp(-bd);
  double sbd=sqrt(bd);
  double erfsbd=airErf(sbd);
  double spi=sqrt(AIR_PI);
  kernel[0]=AIR_CAST(float, b0*AIR_PI*erfsbd/sbd);
  if (order>=2) {
    kernel[1]=AIR_CAST(float, -b0/(4.0*bd*sbd)*embd*sqrt(5*AIR_PI)*
                       (6*sbd+(-3+2*bd)*ebd*spi*erfsbd));
    if (order>=4) {
      kernel[2]=AIR_CAST(float, b0/(32.0*bd*bd*sbd)*embd*spi*
                         (-30*sbd*(21+2*bd)+9*(35+4*bd*(-5+bd))*
                          ebd*spi*erfsbd));
      if (order>=6) { /* At order 6, noise starts to take over! */
        kernel[3]=AIR_CAST(float, b0/(128*bd*bd*bd*sbd)*embd*sqrt(13.0)*
                           (-42*sbd*(165+4*bd*(5+bd))*spi-
                            5*(-693+378*bd-84*bd*bd+8*bd*bd*bd)*
                            ebd*AIR_PI*erfsbd));
        if (order>=8) {
          kernel[4]=AIR_CAST(float, b0/(2048*bd*bd*bd*bd*sbd)*embd*sqrt(17.0)*
                             (-6*sbd*(225225+2*bd*(15015+2*bd*(1925+62*bd)))*spi+
                              35*(19305+8*bd*(-1287+bd*(297+2*(-18+bd)*bd)))*
                              ebd*AIR_PI*erfsbd));
          if (order>8)
            return 1;
        }
      }
    }
  }
  if (delta) {
    return tijk_esh_make_kernel_delta_f(kernel,kernel,order);
  }
  return tijk_esh_make_kernel_rank1_f(kernel,kernel,order);
}

/* elfBallStickODF:
 *
 * Deconvolves single-shell DWI measurements to an ODF, using a kernel derived
 * from the ball-and-stick model
 *
 * Output:
 *  odf   - ESH coefficients of computed ODF
 *  fiso  - if non-NULL, estimated isotropic volume fraction
 *  d     - if non-NULL, estimated ADC
 * Input:
 *  dwi   - DWI measurement and parameters
 *  T     - matrix computed by elfESHEstimMatrix
 *  order - desired ESH order of odf
 *  delta - whether to use delta peak (set to 0 when using elfBallStickPredict)
 *
 * Returns 0 on success
 *         1 if order is not supported
 *         2 if all DWIs were larger than the B0 image
 */
int elfBallStickODF_f(float *odf, float *fiso, float *d,
                      const elfSingleShellDWI *dwi,
                      const float *T, unsigned int order, int delta)
{
  unsigned int C = tijk_esh_len[order/2], k, l;
  float mean=0, _origd=1e-20f, _d=_origd;
  float *odfp = odf;
  float isovf0, isovf1, _fiso;
  float kernel[10]; /* should be tijk_max_esh_order/2+1,
                     * but ANSI C doesn't allow that :/ */

  if (order>tijk_max_esh_order || order%2!=0)
    return 1;

  /* use T to estimate ESH coefficients for given signal values */
  for (k=0; k<C; k++) {
    *odfp = 0.0;
    for (l=0; l<dwi->dwino; l++)
      *odfp += T[k*dwi->dwino+l]*dwi->dwis[l];
    odfp++;
  }

  /* guess d and fiso based on the data */
  for (k=0; k<dwi->dwino; k++) {
    float thisd = AIR_CAST(float, -log(dwi->dwis[k]/dwi->b0)/dwi->b);
    if (dwi->dwis[k]!=0 && thisd>_d) _d=thisd;
    mean += dwi->dwis[k];
  }
  mean /= dwi->dwino;
  isovf0 = AIR_CAST(float, 0.5*sqrt(AIR_PI/(dwi->b*_d))
                    *airErf(sqrt(dwi->b*_d)));
  isovf1 = AIR_CAST(float, exp(-dwi->b*_d));
  _fiso = AIR_CAST(float, AIR_AFFINE(isovf0,mean/dwi->b0,isovf1, 0.0, 1.0));
  _fiso=AIR_CLAMP(0.01f,_fiso,0.99f);
  if (fiso!=NULL) *fiso=_fiso;
  if (d!=NULL) *d=_d;

  /* create deconvolution kernel */
  elfKernelStick_f(kernel, order, dwi->b*_d, dwi->b0, delta);

  /* remove estimated isotropic part from the signal */
  odf[0] -= AIR_CAST(float, dwi->b0 * _fiso * 2*sqrt(AIR_PI)*exp(-dwi->b*_d));

  /* deconvolve */
  tijk_esh_deconvolve_f(odf, odf, kernel, order);

  if (_d==_origd) return 2;
  else return 0;
}

/* elfBallStickPredict:
 *
 * Based on a rank-k decomposition of the given odf (given as a
 * symmetric tensor of type "type"), sets vs and fs[1..k] of parms. d
 * (ADC) and fiso are used to set d and fs[0].
 * Returns 0 upon success, 1 upon error
 */
int elfBallStickPredict_f(elfBallStickParms *parms, float *odf,
                          const tijk_type *type, unsigned int k,
                          float d, float fiso) {
  tijk_refine_rankk_parm *rparm;
  unsigned int i;
  float totalfs=0;

  if (k>3) return 1;

  rparm=tijk_refine_rankk_parm_new();
  rparm->pos=1;
  if (tijk_approx_rankk_3d_f(parms->fs+1, parms->vs, NULL,
                             odf, type, k, rparm)) {
    rparm=tijk_refine_rankk_parm_nix(rparm);
    return 1;
  }
  rparm=tijk_refine_rankk_parm_nix(rparm);

  parms->d = d;
  parms->fiberct = k;
  parms->fs[0]=fiso;
  /* normalize fs[1..3] */
  for (i=1; i<=k; i++)
    totalfs+=parms->fs[i];
  for (i=1; i<=k; i++)
    parms->fs[i]*=(1.0f-parms->fs[0])/totalfs;

  return 0;
}

#if TEEM_LEVMAR
/* Callbacks for levmar-based optimization of ball-and-stick model */

/* pp[0]: log(d) pp[1]: VF1 pp[2]: theta1 pp[3]: phi1
 * pp[4]: VF2 pp[5]: theta2 pp[6]: phi2
 * pp[7]: VF3 pp[8]: theta3 pp[9]: phi3
 * one-/two-stick-models may only use a subset of these */
static void
_levmarBallStickCB(double *pp, double *xx, int mm, int nn, void *_data) {
  int ii, maxk=(mm-1)/3;
  elfSingleShellDWI *data = (elfSingleShellDWI *) _data;
  float *egrad;
  double adc=exp(pp[0]);
  double dirs[3][3];
  int k;
  double vfs[4]={0.0,0.0,0.0,0.0}, sumvfs;

  for (k=0; k<maxk; k++) {
    double stheta=sin(pp[2+3*k]);
    ELL_3V_SET(dirs[k], cos(pp[3+3*k])*stheta,
               sin(pp[3+3*k])*stheta,
               cos(pp[2+3*k]));
  }

  for (k=0; k<maxk; k++) {
    if (pp[1+3*k]>=0) vfs[k+1]=1.0-0.5*exp(-pp[1+3*k]);
    else vfs[k+1]=0.5*exp(pp[1+3*k]);
  }
  sumvfs=vfs[1]+vfs[2]+vfs[3];
  if (sumvfs>1.0) {
    vfs[1]/=sumvfs; vfs[2]/=sumvfs; vfs[3]/=sumvfs;
  } else {
    vfs[0]=1.0-sumvfs;
  }

  egrad = data->grads;
  for (ii=0; ii<nn; ii++) {
    xx[ii] = vfs[0]*exp(-data->b*adc); /* ball compartment */
    for (k=0; k<maxk; k++) {
      double dp=ELL_3V_DOT(egrad,dirs[k]);
      xx[ii] += vfs[k+1]*exp(-data->b*adc*dp*dp);
    }
    xx[ii]*=data->b0;
    egrad+=3;
  }
}

static void
_levmarBallStickJacCB(double *p, double *jac, int m, int n, void *_data) {
  elfSingleShellDWI *data = (elfSingleShellDWI *) _data;
  float *egrad;
  double *j;
  int i,k, maxk=(m-1)/3;
  double adc=exp(p[0]);
  double dirs[3][3];
  double stheta[3], ctheta[3], sphi[3], cphi[3];

  /* pre-compute volume fractions */
  double _vfs[4]={0.0,0.0,0.0,0.0}, vfs[4], vfssum, svfssum;
  for (k=0; k<maxk; k++) {
    if (p[1+3*k]>=0) _vfs[k+1]=1.0-0.5*exp(-p[1+3*k]);
    else _vfs[k+1]=0.5*exp(p[1+3*k]);
  }
  vfssum=_vfs[1]+_vfs[2]+_vfs[3];
  svfssum=vfssum*vfssum;
  if (vfssum>1.0) {
    vfs[0]=0;
    vfs[1]=_vfs[1]/vfssum; vfs[2]=_vfs[2]/vfssum; vfs[3]=_vfs[3]/vfssum;
  } else {
    vfs[0]=1.0-vfssum;
    ELL_3V_COPY(vfs+1,_vfs+1);
  }

  /* directions */
  for (k=0; k<maxk; k++) {
    stheta[k]=sin(p[2+3*k]); ctheta[k]=cos(p[2+3*k]);
    sphi[k]=sin(p[3+3*k]); cphi[k]=cos(p[3+3*k]);
    dirs[k][0]=cphi[k]*stheta[k];
    dirs[k][1]=sphi[k]*stheta[k];
    dirs[k][2]=ctheta[k];
  }

  egrad = data->grads;
  j=jac;
  for (i=0; i<n; i++) { /* iterate over all directions */
    double dps[3];
    double pred[4]; /* to match vfs, pred[0] is the ball compartment */
    pred[0]=data->b0*exp(-data->b*adc);
    for (k=0; k<maxk; k++) { /* we need these first */
      dps[k]=ELL_3V_DOT(egrad,dirs[k]);
      pred[k+1]=data->b0*exp(-data->b*adc*dps[k]*dps[k]);
    }
    j[0]=vfs[0]*pred[0]*(-data->b*adc); /* ball contribution */
    for (k=0; k<maxk; k++) {
      int ik; /* inner loop over k */
      j[0]+=vfs[k+1]*pred[k+1]*(-data->b*adc*dps[k]*dps[k]);
      if (vfssum<1.0) {
        if (_vfs[k+1]>=0.5)
          j[1+3*k]=(1.0-_vfs[k+1])*(-pred[0]+pred[k+1]);
        else
          j[1+3*k]=_vfs[k+1]*(-pred[0]+pred[k+1]);
      } else {
        j[1+3*k]=0;
        for (ik=0; ik<maxk; ik++) {
          j[1+3*k]+=_vfs[ik+1]*(pred[k+1]-pred[ik+1]);
        }
        if (_vfs[k+1]>=0.5)
          j[1+3*k]*=(1.0-_vfs[k+1])/svfssum;
        else
          j[1+3*k]*=_vfs[k+1]/svfssum;
      }
      j[2+3*k]=vfs[k+1]*pred[k+1]*
        (-2*data->b*adc*dps[k]*(egrad[0]*ctheta[k]*cphi[k]+
                                egrad[1]*ctheta[k]*sphi[k]-
                                egrad[2]*stheta[k]));
      j[3+3*k]=vfs[k+1]*pred[k+1]*
        (-2*data->b*adc*dps[k]*(-egrad[0]*stheta[k]*sphi[k]+
                                egrad[1]*stheta[k]*cphi[k]));
    }
    egrad+=3;
    j+=m;
  }
}
#endif /* levmar support */

/* elfBallStickOptimize:
 *
 * Based on an initial guess of parms, use Levenberg-Marquardt optimization
 * to improve the fit w.r.t. the given DWI data.
 * Requires teem to be compiled with levmar support
 * Returns 0 upon success
 *         1 if fiberct outside {1,2,3}
 *         2 if levmar returned an error
 *         3 if levmar support is missing in this version of teem
 */
int elfBallStickOptimize_f(elfBallStickParms *parms,
                           const elfSingleShellDWI *dwi) {
#if TEEM_LEVMAR
  double lmparms[10], *dwis;
  int lmret=0;
  unsigned int k;
  const int parmct=3*parms->fiberct+1;
  const int maxitr=200;
  double opts[LM_OPTS_SZ]={LM_INIT_MU,1e-2,1e-8,1e-8,1e-8};
  double info[9];
  double sumfs;
  double mind=1e-5, minvf=1e-3; /* some minimal values for stability */

  if (parms->fiberct==0 || parms->fiberct>3)
    return 1;

  /* we need a double precision version of the dwis */
  dwis = (double*) malloc(sizeof(double)*dwi->dwino);
  for (k=0; k<dwi->dwino; k++)
    dwis[k]=dwi->dwis[k];

  /* set parameters for optimization */
  lmparms[0]=log(AIR_MAX(mind,parms->d));
  parms->fs[1]=AIR_MAX(minvf,parms->fs[1]);
  if (parms->fs[1]<0.5) lmparms[1]=log(2*parms->fs[1]);
  else lmparms[1]=-log(2.0-2.0*parms->fs[1]);
  lmparms[2]=acos(parms->vs[2]);
  lmparms[3]=atan2(parms->vs[1],parms->vs[0]);
  if (parms->fiberct>1) {
    parms->fs[2]=AIR_MAX(minvf,parms->fs[2]);
    if (parms->fs[2]<0.5) lmparms[4]=log(2*parms->fs[2]);
    else lmparms[4]=-log(2.0-2.0*parms->fs[2]);
    lmparms[5]=acos(parms->vs[5]);
    lmparms[6]=atan2(parms->vs[4],parms->vs[3]);
    if (parms->fiberct>2) {
      parms->fs[3]=AIR_MAX(minvf,parms->fs[3]);
      if (parms->fs[3]<0.5) lmparms[7]=log(2*parms->fs[3]);
      else lmparms[7]=-log(2.0-2.0*parms->fs[3]);
      lmparms[8]=acos(parms->vs[8]);
      lmparms[9]=atan2(parms->vs[7],parms->vs[6]);
    }
  }

  lmret = dlevmar_der(_levmarBallStickCB,_levmarBallStickJacCB,
                      lmparms, dwis,
                      parmct, dwi->dwino, maxitr, opts, info,
                      NULL, NULL, (void*)dwi);
  if (lmret==-1 && (int)info[6]==4) {
    /* try again with larger mu */
    opts[0]*=10;
    lmret = dlevmar_der(_levmarBallStickCB,_levmarBallStickJacCB,
                        lmparms, dwis,
                        parmct, dwi->dwino, maxitr, opts, info,
                        NULL, NULL, (void*)dwi);
  }

  free(dwis); /* no longer needed */

  /* output the results (whether or not levmar signalled an error) */
  parms->d = exp(lmparms[0]);
  parms->fs[0] = 0.0;
  if (lmparms[1]>=0) parms->fs[1]=1.0-0.5*exp(-lmparms[1]);
  else parms->fs[1]=0.5*exp(lmparms[1]);
  if (parms->fiberct>1) {
    if (lmparms[4]>=0) parms->fs[2]=1.0-0.5*exp(-lmparms[4]);
    else parms->fs[2]=0.5*exp(lmparms[4]);
  } else {
    parms->fs[2]=0.0;
  }
  if (parms->fiberct>2) {
    if (lmparms[7]>=0) parms->fs[3]=1.0-0.5*exp(-lmparms[7]);
    else parms->fs[3]=0.5*exp(lmparms[7]);
  } else {
    parms->fs[3]=0.0;
  }
  sumfs = parms->fs[1] + parms->fs[2] + parms->fs[3];
  if (sumfs>1.0) {
    ELL_4V_SCALE(parms->fs, 1.0/sumfs, parms->fs);
  } else {
    parms->fs[0] = 1.0-sumfs;
  }
  for (k=0; k<parms->fiberct; k++) {
    double stheta = sin(lmparms[2+3*k]);
    ELL_3V_SET(parms->vs+3*k,
               stheta*cos(lmparms[3+3*k]),
               stheta*sin(lmparms[3+3*k]),
               cos(lmparms[2+3*k]));
  }

  /* record statistical information */
  parms->stopreason = (int) (info[6])-1;
  parms->sqrerr = info[1];
  parms->itr = info[5];

  if (lmret==-1) return 2;
  return 0;
#else /* no levmar support, out of luck */
  (void) parms; (void) dwi; /* not using the parameters in this case */
  return 3;
#endif
}
