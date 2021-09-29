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

#include "gage.h"
#include "privateGage.h"

/*
** based on: T. Lindeberg. "Effective Scale: A Natural Unit For
** Measuring Scale-Space Lifetime" IEEE PAMI, 15:1068-1074 (1993)
**
** which includes tau(tee) as equation (29),
** here taking A'' == 0 and B'' == 1, with
** a0 and a1 as defined by eqs (22) and (23)
**
** Used MiniMaxApproximation[] from Mathematica to get functions, but
** the specific functions and their domains could certainly be
** improved upon.  Also, the absence of conversions directly between
** tau and sigma is quite unfortunate: going through tee loses
** precision and takes more time.
*/
double
gageTauOfTee(double tee) {
  double tau;

  if (tee < 0) {
    tau = 0;
  } else if (tee < 1.49807) {
    /* mmtau0tweaked */
    tau = (tee*(0.2756644487429131 + tee*(0.10594329088466668 + tee*(0.05514331911165778 + (0.021449249669475232 + 0.004417835440932558*tee)*tee))))/
      (1.0 + tee*(-0.08684532328108877 + tee*(0.1792830876099199 + tee*(0.07468999631784223 + (0.012123550696192354 + 0.0021535864222409365*tee)*tee))));
  } else if (tee < 4.96757) {
    /* mmtau1 */
    tau = (0.0076145275813930356 + tee*(0.24811886965997867 + tee*(0.048329025380584194 +
                                                                   tee*(0.04227260554167517 + (0.0084221516844712 + 0.0092075782656669*tee)*tee))))/
      (1.0 + tee*(-0.43596678272093764 + tee*(0.38077975530585234 + tee*(-0.049133766853683175 + (0.030319379462443567 + 0.0034126333151669654*tee)*tee))));
  } else if (tee < 15.4583) {
    /* mmtau2 */
    tau = (-0.2897145176074084 + tee*(1.3527948686285203 + tee*(-0.47099157589904095 +
           tee*(-0.16031981786376195 + (-0.004820970155881798 - 4.149777202275125e-6*tee)*tee))))/
      (1.0 + tee*(0.3662508612514773 + tee*(-0.5357849572367938 + (-0.0805122462310566 - 0.0015558889784971902*tee)*tee)));
  } else if (tee < 420.787) {
    /* mmtau3 */
    tau = (-4.2037874383990445e9 + tee*(2.838805157541766e9 + tee*(4.032410315406513e8 + tee*(5.392017876788518e6 + tee*(9135.49750298428 + tee)))))/
      (tee*(2.326563899563907e9 + tee*(1.6920560224321905e8 + tee*(1.613645012626063e6 + (2049.748257887103 + 0.1617034516398788*tee)*tee))));
  } else {
    /* lindtau = eq (33) in paper */
    tau = 0.53653222368715360118 + log(tee)/2.0 + log(1.0 - 1.0/(8.0*tee));
  }
  return tau;
}

double
gageTeeOfTau(double tau) {
  double tee;

  /* the number of branches here is not good; needs re-working */
  if (tau < 0) {
    tee = 0;
  } else if (tau < 0.611262) {
    /* mmtee0tweaked */
    tee = (tau*(3.6275987317285265 + tau*(11.774700160760132 + tau*(4.52406587856803 + tau*(-14.125688866786549 + tau*(-0.725387283317479 + 3.5113122862478865*tau))))))/
      (1.0 + tau*(4.955066250765395 + tau*(4.6850073321973404 + tau*(-6.407987550661679 + tau*(-6.398430668865182 + 5.213709282093169*tau)))));
  } else if (tau < 1.31281) {
    /* mmtee1 */
    tee = (1.9887378739371435e49 + tau*(-2.681749984485673e50 + tau*(-4.23360463718195e50 + tau*(2.09694591123974e51 + tau*(-2.7561518523389087e51 + (1.661629137055526e51 - 4.471073383223687e50*tau)*tau)))))/
      (1.0 + tau*(-5.920734745050949e50 + tau*(1.580953446553531e51 + tau*(-1.799463907469813e51 + tau*(1.0766702953985062e51 + tau*(-3.57278667155516e50 + 5.008335824520649e49*tau))))));
  } else if (tau < 1.64767) {
    /* mmtee2 */
    tee = (7.929177830383403 + tau*(-26.12773195115971 + tau*(40.13296225515305 + tau*(-25.041659428733585 + 11.357596970027744*tau))))/
      (1.0 + tau*(-2.3694595653302377 + tau*(7.324354882915464 + (-3.5335141717471314 + 0.4916661013041915*tau)*tau)));
  } else if (tau < 1.88714) {
    /* mmtee3 */
    tee = (0.8334252264680793 + tau*(-0.2388940380698891 + (0.6057616935583752 - 0.01610044688317929*tau)*tau))/(1.0 + tau*(-0.7723301124908083 + (0.21283962841683607 - 0.020834957466407206*tau)*tau));
  } else if (tau < 2.23845) {
    /* mmtee4 */
    tee = (0.6376900379835665 + tau*(0.3177131886056259 + (0.1844114646774132 + 0.2001613331260136*tau)*tau))/(1.0 + tau*(-0.6685635461372561 + (0.15860524381878136 - 0.013304300252332686*tau)*tau));
  } else if (tau < 2.6065) {
    /* mmtee5 */
    tee = (1.3420027677612982 + (-0.939215712453483 + 0.9586140009249253*tau)*tau)/(1.0 + tau*(-0.6923014141351673 + (0.16834190074776287 - 0.014312833444962668*tau)*tau));
  } else if (tau < 3.14419) {
    /* mmtee6 */
    tee = (tau*(190.2181493338235 + tau*(-120.16652155353106 + 60.*tau)))/(76.13355144582292 + tau*(-42.019121363472614 + (8.023304636521623 - 0.5281725039404653*tau)*tau));
  } else {
    /* lindtee = lindtau^{-1} */
    double ee;
    ee = exp(2.0*tau);
    tee = 0.0063325739776461107152*(27.0*ee + 2*AIR_PI*AIR_PI + 3.0*sqrt(81.0*ee*ee + 12*ee*AIR_PI*AIR_PI));
  }
  return tee;
}

double
gageSigOfTau(double tau) {

  return sqrt(gageTeeOfTau(tau));
}

double
gageTauOfSig(double sig) {

  return gageTauOfTee(sig*sig);
}

double
gageStackWtoI(gageContext *ctx, double swrl, int *outside) {
  double si;

  if (ctx && ctx->parm.stackUse && outside) {
    unsigned int sidx;
    if (swrl < ctx->stackPos[0]) {
      /* we'll extrapolate from stackPos[0] and [1] */
      sidx = 0;
      *outside = AIR_TRUE;
    } else if (swrl > ctx->stackPos[ctx->pvlNum-2]) {
      /* extrapolate from stackPos[ctx->pvlNum-3] and [ctx->pvlNum-2];
         gageStackPerVolumeAttach ensures that we there are at least two
         blurrings pvls & one base pvl ==> pvlNum >= 3 ==> pvlNum-3 >= 0 */
      sidx = ctx->pvlNum-3;
      *outside = AIR_TRUE;
    } else {
      /* HEY: stupid linear search */
      for (sidx=0; sidx<ctx->pvlNum-2; sidx++) {
        if (AIR_IN_CL(ctx->stackPos[sidx], swrl, ctx->stackPos[sidx+1])) {
          break;
        }
      }
      if (sidx == ctx->pvlNum-2) {
        /* search failure */
        *outside = AIR_FALSE;
        return AIR_NAN;
      }
      *outside = AIR_FALSE;
    }
    si = AIR_AFFINE(ctx->stackPos[sidx], swrl, ctx->stackPos[sidx+1],
                    sidx, sidx+1);
  } else {
    si = AIR_NAN;
  }
  return si;
}

double
gageStackItoW(gageContext *ctx, double si, int *outside) {
  unsigned int sidx;
  double swrl, sfrac;

  if (ctx && ctx->parm.stackUse && outside) {
    if (si < 0) {
      sidx = 0;
      *outside = AIR_TRUE;
    } else if (si > ctx->pvlNum-2) {
      sidx = ctx->pvlNum-3;
      *outside = AIR_TRUE;
    } else {
      sidx = AIR_CAST(unsigned int, si);
      *outside = AIR_FALSE;
    }
    sfrac = si - sidx;
    swrl = AIR_AFFINE(0, sfrac, 1, ctx->stackPos[sidx], ctx->stackPos[sidx+1]);
    /*
    fprintf(stderr, "!%s: si %g (%u) --> %u + %g --> [%g,%g] -> %g\n", me,
            si, ctx->pvlNum, sidx, sfrac,
            ctx->stackPos[sidx], ctx->stackPos[sidx+1], swrl);
    */
  } else {
    swrl = AIR_NAN;
  }
  return swrl;
}

/*
** this is a little messy: the given pvlStack array has to be allocated
** by the caller to hold blNum gagePerVolume pointers, BUT, the values
** of pvlStack[i] shouldn't be set to anything: as with gagePerVolumeNew(),
** gage allocates the pervolume itself.
*/
int
gageStackPerVolumeNew(gageContext *ctx,
                      gagePerVolume **pvlStack,
                      const Nrrd *const *nblur, unsigned int blNum,
                      const gageKind *kind) {
  static const char me[]="gageStackPerVolumeNew";
  unsigned int blIdx;

  if (!( ctx && pvlStack && nblur && kind )) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (!blNum) {
    biffAddf(GAGE, "%s: need non-zero num", me);
    return 1;
  }

  for (blIdx=0; blIdx<blNum; blIdx++) {
    if (!( pvlStack[blIdx] = gagePerVolumeNew(ctx, nblur[blIdx], kind) )) {
      biffAddf(GAGE, "%s: on pvl %u of %u", me, blIdx, blNum);
      return 1;
    }
  }

  return 0;
}

/*
** the "base" pvl is the LAST pvl, ctx->pvl[pvlNum-1]
*/
int
gageStackPerVolumeAttach(gageContext *ctx, gagePerVolume *pvlBase,
                         gagePerVolume **pvlStack, const double *stackPos,
                         unsigned int blNum) {
  static const char me[]="gageStackPerVolumeAttach";
  unsigned int blIdx;

  if (!(ctx && pvlBase && pvlStack && stackPos)) {
    biffAddf(GAGE, "%s: got NULL pointer %p %p %p %p", me,
             AIR_VOIDP(ctx), AIR_VOIDP(pvlBase),
             AIR_VOIDP(pvlStack), AIR_CVOIDP(stackPos));
    return 1;
  }
  if (!( blNum >= 2 )) {
    /* this constraint is important for the logic of stack reconstruction:
       minimum number of node-centered samples is 2, and the number of
       pvls has to be at least 3 (two blurrings + one base pvl) */
    biffAddf(GAGE, "%s: need at least two samples along stack", me);
    return 1;
  }
  if (ctx->pvlNum) {
    biffAddf(GAGE, "%s: can't have pre-existing volumes (%u) "
             "prior to stack attachment", me, ctx->pvlNum);
    return 1;
  }
  for (blIdx=0; blIdx<blNum; blIdx++) {
    if (!AIR_EXISTS(stackPos[blIdx])) {
      biffAddf(GAGE, "%s: stackPos[%u] = %g doesn't exist", me, blIdx,
               stackPos[blIdx]);
      return 1;
    }
    if (blIdx < blNum-1) {
      if (!( stackPos[blIdx] < stackPos[blIdx+1] )) {
        biffAddf(GAGE, "%s: stackPos[%u] = %g not < stackPos[%u] = %g", me,
                 blIdx, stackPos[blIdx], blIdx+1, stackPos[blIdx+1]);
        return 1;
      }
    }
  }

  /* the base volume is LAST, after all the stack samples */
  for (blIdx=0; blIdx<blNum; blIdx++) {
    if (gagePerVolumeAttach(ctx, pvlStack[blIdx])) {
      biffAddf(GAGE, "%s: on pvl %u of %u", me, blIdx, blNum);
      return 1;
    }
  }
  if (gagePerVolumeAttach(ctx, pvlBase)) {
    biffAddf(GAGE, "%s: on base pvl", me);
    return 1;
  }

  airFree(ctx->stackPos);
  airFree(ctx->stackFsl);
  airFree(ctx->stackFw);
  ctx->stackPos = AIR_CALLOC(blNum, double);
  ctx->stackFsl = AIR_CALLOC(blNum, double);
  ctx->stackFw = AIR_CALLOC(blNum, double);
  if (!( ctx->stackPos && ctx->stackFsl && ctx->stackFw )) {
    biffAddf(GAGE, "%s: couldn't allocate stack buffers (%p %p %p)", me,
             AIR_CAST(void *, ctx->stackPos),
             AIR_CAST(void *, ctx->stackFsl),
             AIR_CAST(void *, ctx->stackFw));
    return 1;
  }
  for (blIdx=0; blIdx<blNum; blIdx++) {
    ctx->stackPos[blIdx] = stackPos[blIdx];
  }

  return 0;
}

/*
** _gageStackBaseIv3Fill
**
** after the individual iv3's in the stack have been filled, this does
** the across-stack filtering to fill the iv3 of pvl[pvlNum-1] (the
** "base" pvl)
*/
int
_gageStackBaseIv3Fill(gageContext *ctx) {
  static const char me[]="_gageStackBaseIv3Fill";
  unsigned int fd, pvlIdx, cacheIdx, cacheLen, baseIdx, valLen;

  fd = 2*ctx->radius;
  /* the "base" pvl is the LAST pvl */
  baseIdx = ctx->pvlNum - 1;
  cacheLen = fd*fd*fd*ctx->pvl[0]->kind->valLen;
  if (ctx->verbose > 2) {
    fprintf(stderr, "%s: cacheLen = %u\n", me, cacheLen);
  }
  if (nrrdKernelHermiteScaleSpaceFlag  == ctx->ksp[gageKernelStack]->kernel) {
    unsigned int xi, yi, zi, blurIdx, valIdx, fdd;
    double xx, *iv30, *iv31, sigma0, sigma1;

    fdd = fd*fd;
    /* initialize the output iv3 to all zeros, since we won't be
       usefully setting the values on the boundary (the boundary which
       is required in the rest of the stack's iv3s in order to do the
       laplacian-based spline recon), and we can't have any
       non-existent values creeping in.  We shouldn't need to do any
       kind of nrrdBoundaryBleed thing here, because the kernel
       weights really should be zero on the boundary. */
    for (cacheIdx=0; cacheIdx<cacheLen; cacheIdx++) {
      ctx->pvl[baseIdx]->iv3[cacheIdx] = 0;
    }

    /* find the interval in the pre-blurred volumes containing the
       desired scale location */
    for (pvlIdx=0; pvlIdx<ctx->pvlNum-1; pvlIdx++) {
      if (ctx->stackFw[pvlIdx]) {
        /* has to be non-zero somewhere, since _gageLocationSet()
           gives an error if there aren't non-zero stackFw[i] */
        break;
      }
    }
    /* so no way that pvlIdx == pvlNum-1 */
    if (pvlIdx == ctx->pvlNum-2) {
      /* pvlNum-2 is pvl index of last pre-blurred volume */
      /* gageStackPerVolumeAttach() enforces getting at least two
         pre-blurred volumes --> pvlNum >= 3 --> blurIdx >= 0 */
      blurIdx = pvlIdx-1;
      xx = 1;
    } else {
      blurIdx = pvlIdx;
      /* by design, the hermite non-kernel generates the same values as
         the tent kernel (with scale forced == 1), so we can use those
         to control the interpolation */
      xx = 1 - ctx->stackFw[pvlIdx];
    }
    iv30 = ctx->pvl[blurIdx]->iv3;
    iv31 = ctx->pvl[blurIdx+1]->iv3;
    sigma0 = ctx->stackPos[blurIdx];
    sigma1 = ctx->stackPos[blurIdx+1];
    valLen = ctx->pvl[baseIdx]->kind->valLen;
    for (valIdx=0; valIdx<valLen; valIdx++) {
      unsigned iii;
      double val0, val1, drv0, drv1, lapl0, lapl1, aa, bb, cc, dd;
      for (zi=1; zi<fd-1; zi++) {
        for (yi=1; yi<fd-1; yi++) {
          for (xi=1; xi<fd-1; xi++) {
            /* note that iv3 axis ordering is x, y, z, tuple */
            iii = xi + fd*(yi + fd*(zi + fd*valIdx));
            val0 = iv30[iii];
            val1 = iv31[iii];
            lapl0 = (iv30[iii + 1]   + iv30[iii - 1] +
                     iv30[iii + fd]  + iv30[iii - fd] +
                     iv30[iii + fdd] + iv30[iii - fdd] - 6*val0);
            lapl1 = (iv31[iii + 1]   + iv31[iii - 1] +
                     iv31[iii + fd]  + iv31[iii - fd] +
                     iv31[iii + fdd] + iv31[iii - fdd] - 6*val1);
            /* the (sigma1 - sigma0) factor is needed to convert the
               derivative with respect to sigma (sigma*lapl) into the
               derivative with respect to xx */
            drv0 = sigma0*lapl0*(sigma1 - sigma0);
            drv1 = sigma1*lapl1*(sigma1 - sigma0);
            /* Hermite spline coefficients, thanks Mathematica */
            aa = drv0 + drv1 + 2*val0 - 2*val1;
            bb = -2*drv0 - drv1 - 3*val0 + 3*val1;
            cc = drv0;
            dd = val0;
            ctx->pvl[baseIdx]->iv3[iii] = dd + xx*(cc + xx*(bb + aa*xx));
          }
        }
      }
    }
  } else {
    /* we're doing simple convolution-based recon on the stack */
    /* NOTE we are treating the 4D fd*fd*fd*valLen iv3 as a big 1-D array */
    double wght, val;
    for (cacheIdx=0; cacheIdx<cacheLen; cacheIdx++) {
      val = 0;
      for (pvlIdx=0; pvlIdx<ctx->pvlNum-1; pvlIdx++) {
        wght = ctx->stackFw[pvlIdx];
        val += (wght
                ? wght*ctx->pvl[pvlIdx]->iv3[cacheIdx]
                : 0);
      }
      ctx->pvl[baseIdx]->iv3[cacheIdx] = val;
    }
  }
  return 0;
}

/*
******** gageStackProbe()
*/
int
gageStackProbe(gageContext *ctx,
               double xi, double yi, double zi, double stackIdx) {
  static const char me[]="gageStackProbe";

  if (!ctx) {
    return 1;
  }
  if (!ctx->parm.stackUse) {
    if (ctx->parm.generateErrStr) {
      sprintf(ctx->errStr, "%s: can't probe stack without parm.stackUse", me);
    } else {
      strcpy(ctx->errStr, _GAGE_NON_ERR_STR);
    }
    ctx->errNum = gageErrStackUnused;
    return 1;
  }
  return _gageProbe(ctx, xi, yi, zi, stackIdx);
}

int
gageStackProbeSpace(gageContext *ctx,
                    double xx, double yy, double zz, double ss,
                    int indexSpace, int clamp) {
  static const char me[]="gageStackProbeSpace";

  if (!ctx) {
    return 1;
  }
  if (!ctx->parm.stackUse) {
    if (ctx->parm.generateErrStr) {
      sprintf(ctx->errStr, "%s: can't probe stack without parm.stackUse", me);
    } else {
      strcpy(ctx->errStr, _GAGE_NON_ERR_STR);
    }
    ctx->errNum = gageErrStackUnused;
    return 1;
  }
  return _gageProbeSpace(ctx, xx, yy, zz, ss, indexSpace, clamp);
}

