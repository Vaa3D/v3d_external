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

#define N -1

/*
** NOTE: The idea for this table originated with Raul San Jose Estepar;
** GLK recomputed it optimizing for 3D recon, but
** NOTE: there are probably still be bugs in this; look at the
** "HEY: bug?" notes below, the same problem occurs elsewhere
**
** Basic indexing idea: [sigma max][total # samples][which sample]
**
** "sigma max" can't be 0; smallest value is 1
** ==> index with (sigma max)-1
** biggest value is GAGE_OPTIMSIG_SIGMA_MAX,
** ==> biggest index is GAGE_OPTIMSIG_SIGMA_MAX-1
** ==> allocate for GAGE_OPTIMSIG_SIGMA_MAX
**
** "total # samples" can't be 0, or 1, smallest value is 2
** ==> index with (total # samples)-2
** biggest value is GAGE_OPTIMSIG_SAMPLES_MAXNUM
** ==> biggest index is GAGE_OPTIMSIG_SAMPLES_MAXNUM-2
** ==> allocate for GAGE_OPTIMSIG_SAMPLES_MAXNUM-1
**
** "which sample" ranges from 0 to GAGE_OPTIMSIG_SAMPLES_MAXNUM-1
** ==> allocate for GAGE_OPTIMSIG_SAMPLES_MAXNUM
*/
static double
_optimSigTable[GAGE_OPTIMSIG_SIGMA_MAX][GAGE_OPTIMSIG_SAMPLES_MAXNUM-1][GAGE_OPTIMSIG_SAMPLES_MAXNUM] = {
  {
    {0,1,N,N,N,N,N,N,N,N,N},
    {0,0.5279398,1,N,N,N,N,N,N,N,N},
    {0,0.30728838,0.59967405,1,N,N,N,N,N,N,N},
    {0,0.25022203,0.47050092,0.69525677,1,N,N,N,N,N,N},
    {0,0.17127343,0.39234546,0.56356072,0.75660759,1,N,N,N,N,N},
    {0,0.16795139,0.37100673,0.51324213,0.65655005,0.81952846,1,N,N,N,N},
    {0,0.1662873,0.34969759,0.46556041,0.55324608,0.68717259,0.83465695,1,N,N,N},
    {0,0.12720504,0.22565289,0.28316727,0.44209728,0.58615023,0.75034028,0.87391609,1,N,N},
    {0,0.12836272 /* HEY: bug? should be < 0.12720504 */,0.22926401,0.27715567,0.43546647,0.56471503,0.69411868,0.80830419,0.89314467,1,N},
    {0,0.13169055 /* HEY: bug? should be < 0.12720504 */,0.23498112,0.26570156,0.42672107,0.54272485,0.62969965,0.73375762,0.76996493,0.89293921,1}
  }, {
    {0,2,N,N,N,N,N,N,N,N,N},
    {0,0.75118494,2,N,N,N,N,N,N,N,N},
    {0,0.55478472,1.1535828,2,N,N,N,N,N,N,N},
    {0,0.49007216,0.8412028,1.308665,2,N,N,N,N,N,N},
    {0,0.29460263,0.57445061,0.93797231,1.368475,2,N,N,N,N,N},
    {0,0.2506085,0.49080029,0.73882496,1.069332,1.4497081,2,N,N,N,N},
    {0,0.18255657,0.42056954,0.62766695,0.87999368,1.1692151,1.5175625,2,N,N,N},
    {0,0.17582123,0.40522173,0.58696139,0.79624867,1.0485514,1.2950466,1.5977446,2,N,N},
    {0,0.17304537,0.39376548,0.56427032,0.75127059,0.96672511,1.187861,1.4141362,1.6921321,2,N},
    {0,0.16970521,0.38116929,0.53575242,0.69498152,0.88430929,1.0844854,1.2899524,1.5211773,1.7645421,2}
  }, {
    {0,3,N,N,N,N,N,N,N,N,N},
    {0,0.92324787,3,N,N,N,N,N,N,N,N},
    {0,0.59671402,1.3871731,3,N,N,N,N,N,N,N},
    {0,0.53303385,1.0274624,1.6725048,3,N,N,N,N,N,N},
    {0,0.47298154,0.79659319,1.2379739,1.8434249,3,N,N,N,N,N},
    {0,0.29337707,0.56664073,0.94871783,1.3666322,1.949043,3,N,N,N,N},
    {0,0.25583801,0.52919179,0.78387552,1.1250161,1.516176,2.0632432,3,N,N,N},
    {0,0.25013804,0.48255014,0.72428173,1.0308567,1.3638159,1.7629964,2.2885511,3,N,N},
    {0,0.25038671,0.46448985,0.67336935,0.94502586,1.2324173,1.5780864,1.9883285,2.5002999,3,N},
    {0,0.25034565,0.44725224,0.63590652,0.8669008,1.1130947,1.3942779,1.7180597,2.1408446,2.5466051,3}
  }, {
    {0,4,N,N,N,N,N,N,N,N,N},
    {0,1.0342592,4,N,N,N,N,N,N,N,N},
    {0,0.6341188,1.5414433,4,N,N,N,N,N,N,N},
    {0,0.5523203,1.1400089,1.9595566,4,N,N,N,N,N,N},
    {0,0.51082283,0.91567439,1.4275582,2.2504199,4,N,N,N,N,N},
    {0,0.46390373,0.76406777,1.1620381,1.6579833,2.470386,4,N,N,N,N},
    {0,0.29957226,0.58226484,0.90447241,1.318499,1.8011117,2.5972142,4,N,N,N},
    {0,0.29072434,0.5657317,0.8687849,1.2413157,1.7351674,2.2752147,3.1038468,4,N,N},
    {0,0.25000414,0.5027808,0.75375289,1.0744231,1.4267329,1.8665372,2.4665236,3.2203004,4,N},
    {0,0.19010291,0.44269502,0.66081244,0.95829803,1.2627038,1.6005131,2.0043969,2.6440792,3.2979164,4}
  }, {
    {0,5,N,N,N,N,N,N,N,N,N},
    {0,1.1176668,5,N,N,N,N,N,N,N,N},
    {0,0.66791451,1.6688319,5,N,N,N,N,N,N,N},
    {0,0.56513244,1.2151262,2.2046661,5,N,N,N,N,N,N},
    {0,0.51955444,0.96157616,1.5293243,2.5639,5,N,N,N,N,N},
    {0,0.50639188,0.83235806,1.2596023,1.8475783,2.8751452,5,N,N,N,N},
    {0,0.30821687,0.60048282,1.0057166,1.4351804,2.0372179,3.0747592,5,N,N,N},
    {0,0.28437388,0.560866,0.92278755,1.3049414,1.7620444,2.4607313,3.5198457,5,N,N},
    {0,0.26883101,0.53947717,0.84076571,1.1986721,1.6077875,2.165575,2.9591467,3.931181,5,N},
    {0,0.25029126,0.50162876,0.75587535,1.0861237,1.4452776,1.8865763,2.5002809,3.2476835,4.0337272,5}
  }, {
    {0,6,N,N,N,N,N,N,N,N,N},
    {0,1.185726,6,N,N,N,N,N,N,N,N},
    {0,0.69637311,1.7772807,6,N,N,N,N,N,N,N},
    {0,0.57470578,1.2709187,2.4227901,6,N,N,N,N,N,N},
    {0,0.52996641,1.0128419,1.632214,2.8718762,6,N,N,N,N,N},
    {0,0.50426048,0.87729794,1.3428378,2.0053113,3.2981832,6,N,N,N,N},
    {0,0.46658435,0.76617205,1.1726109,1.6950468,2.5514688,4.1463666,6,N,N,N},
    {0,0.50030917,0.78596908,1.1486269,1.5887094,2.2150676,3.2805684,4.4828262,6,N,N},
    {0,0.27919531,0.56878412,0.88591647,1.2631332,1.7201432,2.3851209,3.392889,4.6255312,6,N},
    {0,0.25088972,0.50369233,0.78494686,1.1030188,1.482311,1.9812444,2.6906328,3.734978,4.7532525,6}
  }, {
    {0,7,N,N,N,N,N,N,N,N,N},
    {0,1.2437892,7,N,N,N,N,N,N,N,N},
    {0,0.72099203,1.8771845,7,N,N,N,N,N,N,N},
    {0,0.58251196,1.3139123,2.6157444,7,N,N,N,N,N,N},
    {0,0.5371021,1.0473768,1.7166929,3.1448426,7,N,N,N,N,N},
    {0,0.51312029,0.92989367,1.4221185,2.2125893,3.6739931,7,N,N,N,N},
    {0,0.50083971,0.84841007,1.2561073,1.8532455,2.8668625,4.7535434,7,N,N,N},
    {0,0.3375614,0.63945627,1.0301709,1.4884938,2.073761,3.1614799,5.0744987,7,N,N},
    {0,0.29428458,0.58668923,0.93714356,1.3736334,1.8300356,2.6405344,3.9042048,5.3097196,7,N},
    {0,0.25234449,0.52068585,0.79422623,1.1273863,1.5991755,2.1453006,2.8984315,4.1899557,5.4597921,7}
  }, {
    {0,8,N,N,N,N,N,N,N,N,N},
    {0,1.2942501,8,N,N,N,N,N,N,N,N},
    {0,0.74332041,1.9693407,8,N,N,N,N,N,N,N},
    {0,0.58823597,1.3483386,2.7880962,8,N,N,N,N,N,N},
    {0,0.56661958,1.2263036,1.9593971,3.6037345,8,N,N,N,N,N},
    {0,0.52106231,0.97026396,1.486012,2.3670862,4.1632919,8,N,N,N,N},
    {0,0.50727636,0.86810225,1.3293955,2.0115428,3.1358411,5.3943086,8,N,N,N},
    {0,0.47202346,0.77812189,1.1608884,1.6648751,2.4694417,3.9094045,5.7665443,8,N,N},
    {0,0.37446901,0.66116196,1.038642,1.4625595,2.0528309,2.9814169,4.4429126,5.9815373,8,N},
    {0,0.26310974,0.54373014,0.84282249,1.2090484,1.6551158,2.3275802,3.3196113,4.7216973,6.1578932,8}
  }, {
    {0,9,N,N,N,N,N,N,N,N,N},
    {0,1.3413963,9,N,N,N,N,N,N,N,N},
    {0,0.76222414,2.0542119,9,N,N,N,N,N,N,N},
    {0,0.59559792,1.3777219,2.946173,9,N,N,N,N,N,N},
    {0,0.56240517,1.1527119,1.9145473,3.6841569,9,N,N,N,N,N},
    {0,0.52387071,0.98832464,1.5376476,2.5417714,4.4669261,9,N,N,N,N},
    {0,0.50359035,0.87327009,1.3558764,2.0646384,3.3180211,5.9420524,9,N,N,N},
    {0,0.50140077,0.83020425,1.256588,1.7709454,2.7100441,4.4434023,6.3934889,9,N,N},
    {0,0.36521655,0.65757704,1.0627806,1.5081434,2.1497617,3.1920822,4.870122,6.6418982,9,N},
    {0,0.31160679,0.59032226,0.94745982,1.3620865,1.8115216,2.6007423,3.8324564,5.2064519,6.8468728,9}
  }, {
    {0,10,N,N,N,N,N,N,N,N,N},
    {0,1.3838946,10,N,N,N,N,N,N,N,N},
    {0,0.77946955,2.1342247,10,N,N,N,N,N,N,N},
    {0,0.60070014,1.4040204,3.0944126,10,N,N,N,N,N,N},
    {0,0.55609542,1.1508646,1.9495349,3.9375696,10,N,N,N,N,N},
    {0,0.5350194,1.031119,1.6607633,2.8520992,5.4718146,10,N,N,N,N},
    {0,0.5083549,0.90783268,1.4059756,2.1796026,3.571064,6.5497985,10,N,N,N},
    {0,0.50199872,0.85233968,1.2647815,1.8777326,2.8592849,4.7821364,7.0110598,10,N,N},
    {0,0.46663594,0.75212663,1.1302133,1.6134665,2.3560972,3.6558499,5.3189116,7.2945781,10,N},
    {0,0.3789258,0.64023608,1.0374272,1.4685256,2.0717783,3.0241971,4.2591534,5.6669927,7.5286098,10}
  }, {
    {0,11,N,N,N,N,N,N,N,N,N},
    {0,1.4234025,11,N,N,N,N,N,N,N,N},
    {0,0.79513794,2.2098076,11,N,N,N,N,N,N,N},
    {0,0.60728961,1.4287171,3.2358651,11,N,N,N,N,N,N},
    {0,0.55890071,1.165283,2.0149148,4.1530919,11,N,N,N,N,N},
    {0,0.55071467,1.0660659,1.7177736,3.0094495,6.0395317,11,N,N,N,N},
    {0,0.5066433,0.89661205,1.4050072,2.2117786,3.7080047,7.0954437,11,N,N,N},
    {0,0.50242329,0.86727452,1.3264461,1.9118301,2.9509099,5.1184769,7.624764,11,N,N},
    {0,0.47785854,0.78873962,1.1769236,1.6880652,2.4978926,4.0288033,5.7288432,7.9420485,11,N},
    {0,0.50532979,0.79486167,1.1706896,1.6148115,2.2648265,3.3499777,4.5595574,6.116312,8.2049971,11}
  }
};

int
gageOptimSigSet(double *scale, unsigned int num, unsigned int sigmaMax) {
  static const char me[]="gageOptimSigSet";
  unsigned int si;

  if (!scale) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (!AIR_IN_CL(2, num, GAGE_OPTIMSIG_SAMPLES_MAXNUM)) {
    biffAddf(GAGE,
             "%s: requested # sigma samples %u not in known range [2,%u]",
             me, num, GAGE_OPTIMSIG_SAMPLES_MAXNUM);
    return 1;
  }
  if (!AIR_IN_CL(1, sigmaMax, GAGE_OPTIMSIG_SIGMA_MAX)) {
    biffAddf(GAGE, "%s: requested sigma max %u not in known range [1,%u]",
             me, sigmaMax, GAGE_OPTIMSIG_SIGMA_MAX);
    return 1;
  }

  for (si=0; si<num; si++) {
    scale[si] = _optimSigTable[sigmaMax-1][num-2][si];
  }
  return 0;
}

gageOptimSigParm *
gageOptimSigParmNew(unsigned int sampleNumMax) {
  gageOptimSigParm *parm;

  parm = AIR_CAST(gageOptimSigParm *, calloc(1, sizeof(gageOptimSigParm)));
  if (parm) {
    unsigned int si;
    parm->dim = 0;
    parm->sampleNumMax = sampleNumMax;
    parm->plotting = AIR_FALSE;
    parm->tentRecon = AIR_FALSE;
    parm->sigmatru = NULL;
    parm->truth = NULL;
    parm->ntruth = nrrdNew();
    parm->nerr = nrrdNew();
    parm->ntruline = nrrdNew();
    parm->ninterp = nrrdNew();
    parm->ndiff = nrrdNew();
    parm->scalePos = AIR_CAST(double *, calloc(sampleNumMax, sizeof(double)));
    parm->step = AIR_CAST(double *, calloc(sampleNumMax, sizeof(double)));
    parm->nsampvol = AIR_CAST(Nrrd **, calloc(sampleNumMax, sizeof(Nrrd *)));
    for (si=0; si<sampleNumMax; si++) {
      parm->nsampvol[si] = nrrdNew();
    }
    parm->pvl = NULL;
    parm->pvlSS = AIR_CAST(gagePerVolume **,
                           calloc(sampleNumMax, sizeof(gagePerVolume *)));
    parm->gctx = gageContextNew();
  }
  return parm;
}

gageOptimSigParm *
gageOptimSigParmNix(gageOptimSigParm *parm) {

  if (parm) {
    unsigned int si;
    airFree(parm->sigmatru);
    nrrdNuke(parm->ntruth);
    nrrdNuke(parm->nerr);
    nrrdNix(parm->ntruline);
    nrrdNuke(parm->ninterp);
    nrrdNuke(parm->ndiff);
    airFree(parm->scalePos);
    airFree(parm->step);
    for (si=0; si<parm->sampleNumMax; si++) {
      nrrdNuke(parm->nsampvol[si]);
    }
    airFree(parm->nsampvol);
    airFree(parm->pvlSS);
    gageContextNix(parm->gctx);
    airFree(parm);
  }
  return NULL;
}

static void
_volTrueBlur(Nrrd *nvol, double sigma, gageOptimSigParm *parm) {
  double *vol, xv, yv, zv;
  unsigned int xi, yi, zi;
  NrrdKernel *dg;
  double kparm[NRRD_KERNEL_PARMS_NUM], xrad, yrad, zrad;

  vol = AIR_CAST(double *, nvol->data);
  xrad = (nvol->axis[0].size + 1)/2 - 1;
  yrad = (nvol->axis[1].size + 1)/2 - 1;
  zrad = (nvol->axis[2].size + 1)/2 - 1;
  dg = nrrdKernelDiscreteGaussian;
  kparm[0] = sigma;
  kparm[1] = parm->cutoff;
  for (zi=0; zi<parm->sz; zi++) {
    zv = (parm->dim >= 2
          ? dg->eval1_d(AIR_CAST(double, zi) - zrad, kparm)
          : 1);
    for (yi=0; yi<parm->sy; yi++) {
      yv = (parm->dim >= 3
            ? dg->eval1_d(AIR_CAST(double, yi) - yrad, kparm)
            : 1);
      for (xi=0; xi<parm->sx; xi++) {
        xv = dg->eval1_d(AIR_CAST(double, xi) - xrad, kparm);
        vol[xi + parm->sx*(yi + parm->sy*zi)] = xv*yv*zv;
      }
    }
  }
  return;
}

int
gageOptimSigTruthSet(gageOptimSigParm *parm,
                     unsigned int dim,
                     double sigmaMax, double cutoff,
                     unsigned int measrSampleNum) {
  static const char me[]="gageOptimSigTruthSet";
  char doneStr[AIR_STRLEN_SMALL];
  double kparm[NRRD_KERNEL_PARMS_NUM], tauMax;
  unsigned int support, ii;

  if (!parm) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (!AIR_IN_CL(1, dim, 3)) {
    biffAddf(GAGE, "%s: dim %u not 1, 2, or 3", me, dim);
    return 1;
  }
  if (!(sigmaMax > 0 && cutoff > 0)) {
    biffAddf(GAGE, "%s: sigmaMax %g, cutoff %g not both > 0", me,
             sigmaMax, cutoff);
    return 1;
  }
  if (!(measrSampleNum >= 3)) {
    biffAddf(GAGE, "%s: measrSampleNum %u not >= 3", me, measrSampleNum);
    return 1;
  }

  parm->dim = dim;
  kparm[0] = parm->sigmaMax = sigmaMax;
  kparm[1] = parm->cutoff = cutoff;
  parm->measrSampleNum = measrSampleNum;
  support = AIR_ROUNDUP(nrrdKernelDiscreteGaussian->support(kparm));
  /* may later allow different shaped volumes */
  parm->sx = parm->sy = parm->sz = 2*support - 1;
  fprintf(stderr, "!%s: support = %u, vol size = %u\n", me, support, parm->sx);
  airFree(parm->sigmatru);
  parm->sigmatru = AIR_CAST(double *, calloc(measrSampleNum, sizeof(double)));
  if (!parm->sigmatru) {
    biffAddf(GAGE, "%s: couldn't alloc sigmatru buffer", me);
    return 1;
  }
  if (nrrdMaybeAlloc_va(parm->ntruth, nrrdTypeDouble, 4,
                        AIR_CAST(size_t, parm->sx),
                        AIR_CAST(size_t, parm->sy),
                        AIR_CAST(size_t, parm->sz),
                        AIR_CAST(size_t, measrSampleNum))
      || nrrdMaybeAlloc_va(parm->nerr, nrrdTypeDouble, 1,
                           AIR_CAST(size_t, measrSampleNum))
      /* ntruline->data will be re-set willy-nilly */
      || nrrdWrap_va(parm->ntruline, parm->ntruth->data,
                     parm->ntruth->type, 3,
                     AIR_CAST(size_t, parm->sx),
                     AIR_CAST(size_t, parm->sy),
                     AIR_CAST(size_t, parm->sz))
      || nrrdMaybeAlloc_va(parm->ninterp, nrrdTypeDouble, 3,
                           AIR_CAST(size_t, parm->sx),
                           AIR_CAST(size_t, parm->sy),
                           AIR_CAST(size_t, parm->sz))
      || nrrdMaybeAlloc_va(parm->ndiff, nrrdTypeDouble, 3,
                           AIR_CAST(size_t, parm->sx),
                           AIR_CAST(size_t, parm->sy),
                           AIR_CAST(size_t, parm->sz))) {
    biffMovef(GAGE, NRRD, "%s: couldn't allocate truth", me);
    return 1;
  }
  parm->truth = AIR_CAST(double *, parm->ntruth->data);
  nrrdAxisInfoSet_va(parm->ntruth, nrrdAxisInfoSpacing,
                     1.0, 1.0, 1.0, AIR_NAN);
  nrrdAxisInfoSet_va(parm->ntruline, nrrdAxisInfoSpacing,
                     1.0, 1.0, 1.0);
  nrrdAxisInfoSet_va(parm->ninterp, nrrdAxisInfoSpacing,
                     1.0, 1.0, 1.0);
  nrrdAxisInfoSet_va(parm->ndiff, nrrdAxisInfoSpacing,
                     1.0, 1.0, 1.0);
  for (ii=0; ii<parm->sampleNumMax; ii++) {
    if (nrrdMaybeAlloc_va(parm->nsampvol[ii], nrrdTypeDouble, 3,
                          AIR_CAST(size_t, parm->sx),
                          AIR_CAST(size_t, parm->sy),
                          AIR_CAST(size_t, parm->sz))) {
      biffMovef(GAGE, NRRD, "%s: couldn't allocate vol[%u]", me, ii);
      return 1;
    }
    nrrdAxisInfoSet_va(parm->nsampvol[ii], nrrdAxisInfoSpacing,
                       1.0, 1.0, 1.0);
  }
  fprintf(stderr, "%s: computing reference blurrings ...       ", me);
  tauMax =  gageTauOfSig(parm->sigmaMax);
  for (ii=0; ii<parm->measrSampleNum; ii++) {
    double sigma;
    if (!(ii % 10)) {
      fprintf(stderr, "%s", airDoneStr(0, ii, parm->measrSampleNum, doneStr));
      fflush(stderr);
    }
    parm->ntruline->data = parm->truth + ii*parm->sx*parm->sy*parm->sz;
    if (parm->plotting) {
      sigma = AIR_AFFINE(0, ii, parm->measrSampleNum-1, 0.0, parm->sigmaMax);
    } else {
      double tau;
      tau = AIR_AFFINE(0, ii, parm->measrSampleNum-1, 0.0, tauMax);
      sigma = gageSigOfTau(tau);
    }
    parm->sigmatru[ii] = sigma;
    _volTrueBlur(parm->ntruline, sigma, parm);
  }
  fprintf(stderr, "%s\n", airDoneStr(0, ii, parm->measrSampleNum, doneStr));
  return 0;
}

static void
_volInterp(Nrrd *ninterp, double scale, gageOptimSigParm *parm) {
  double *interp, scaleIdx;
  const double *answer;
  unsigned int xi, yi, zi;
  int outside;

  scaleIdx = gageStackWtoI(parm->gctx, scale, &outside);
  answer = gageAnswerPointer(parm->gctx, parm->pvl, gageSclValue);
  interp = AIR_CAST(double *, ninterp->data);
  for (zi=0; zi<parm->sz; zi++) {
    for (yi=0; yi<parm->sy; yi++) {
      for (xi=0; xi<parm->sx; xi++) {
        gageStackProbe(parm->gctx, xi, yi, zi, scaleIdx);
        interp[xi + parm->sx*(yi + parm->sy*zi)] = answer[0];
      }
    }
  }
  return;
}

static double
_errSingle(gageOptimSigParm *parm, unsigned int sigmaIdx) {
  double *interp, *truline, *diff, ret;
  size_t ii, nn;

  _volInterp(parm->ninterp, parm->sigmatru[sigmaIdx], parm);
  interp = AIR_CAST(double *, parm->ninterp->data);
  nn = parm->sx*parm->sy*parm->sz;
  truline = parm->truth + sigmaIdx*nn;
  diff = AIR_CAST(double *, parm->ndiff->data);
  for (ii=0; ii<nn; ii++) {
    diff[ii] = truline[ii] - interp[ii];
  }
  if (0) {
    char fname[AIR_STRLEN_SMALL];
    sprintf(fname, "interp-%03u.nrrd", sigmaIdx);
    nrrdSave(fname, parm->ninterp, NULL);
  }
  nrrdMeasureLine[parm->volMeasr](&ret, nrrdTypeDouble,
                                  diff, nrrdTypeDouble,
                                  nn, AIR_NAN, AIR_NAN);
  return ret;
}

static double
_errTotal(gageOptimSigParm *parm) {
  unsigned int ii;
  double *err, ret;

  for (ii=0; ii<parm->sampleNum; ii++) {
    parm->gctx->stackPos[ii] = parm->scalePos[ii];
  }
  err = AIR_CAST(double *, parm->nerr->data);
  for (ii=0; ii<parm->measrSampleNum; ii++) {
    err[ii] = _errSingle(parm, ii);
  }
  nrrdMeasureLine[parm->lineMeasr](&ret, nrrdTypeDouble,
                                   err, nrrdTypeDouble,
                                   parm->measrSampleNum,
                                   AIR_NAN, AIR_NAN);
  if (0) {
    static unsigned int call;
    char fname[AIR_STRLEN_SMALL];
    sprintf(fname, "err-%04u.nrrd", call);
    nrrdSave(fname, parm->nerr, NULL);
    call++;
  }
  return ret;
}

static int
_gageSetup(gageOptimSigParm *parm) {
  static const char me[]="_gageSetup";
  double kparm[NRRD_KERNEL_PARMS_NUM];
  int E;

  if (parm->gctx) {
    gageContextNix(parm->gctx);
  }
  parm->gctx = gageContextNew();
  gageParmSet(parm->gctx, gageParmVerbose, 0);
  gageParmSet(parm->gctx, gageParmRenormalize, AIR_FALSE);
  gageParmSet(parm->gctx, gageParmCheckIntegrals, AIR_FALSE);
  gageParmSet(parm->gctx, gageParmOrientationFromSpacing, AIR_TRUE);
  gageParmSet(parm->gctx, gageParmStackUse, AIR_TRUE);
  E = 0;
  if (!E) E |= !(parm->pvl = gagePerVolumeNew(parm->gctx, parm->nsampvol[0],
                                              gageKindScl));
  if (!E) E |= gageStackPerVolumeNew(parm->gctx, parm->pvlSS,
                                     AIR_CAST(const Nrrd*const*,
                                              parm->nsampvol),
                                     parm->sampleNum, gageKindScl);
  if (!E) E |= gageStackPerVolumeAttach(parm->gctx, parm->pvl, parm->pvlSS,
                                        parm->scalePos, parm->sampleNum);
  kparm[0] = 1;
  if (!E) E |= gageKernelSet(parm->gctx, gageKernel00,
                             nrrdKernelTent, kparm);
  if (parm->tentRecon) {
    if (!E) E |= gageKernelSet(parm->gctx, gageKernelStack,
                               nrrdKernelTent, kparm);
  } else {
    if (!E) E |= gageKernelSet(parm->gctx, gageKernelStack,
                               nrrdKernelHermiteScaleSpaceFlag, kparm);
  }
  if (!E) E |= gageQueryItemOn(parm->gctx, parm->pvl, gageSclValue);
  if (!E) E |= gageUpdate(parm->gctx);
  if (E) {
    biffAddf(GAGE, "%s: problem setting up gage", me);
    return 1;
  }
  return 0;
}

static void
_scalePosSet(gageOptimSigParm *parm, unsigned int ii, double sigma) {

  parm->scalePos[ii] = sigma;
  _volTrueBlur(parm->nsampvol[ii], parm->scalePos[ii], parm);
  gagePointReset(&(parm->gctx->point));
}

static char *
_timefmt(char tstr[AIR_STRLEN_MED], double deltim) {

  if (deltim < 60) {
    sprintf(tstr, "%g secs", deltim);
    return tstr;
  }
  deltim /= 60;
  if (deltim < 60) {
    sprintf(tstr, "%g mins", deltim);
    return tstr;
  }
  deltim /= 60;
  if (deltim < 24) {
    sprintf(tstr, "%g hours", deltim);
    return tstr;
  }
  deltim /= 24;
  if (deltim < 7) {
    sprintf(tstr, "%g days", deltim);
    return tstr;
  }
  deltim /= 7;
  sprintf(tstr, "%g weeks", deltim);
  return tstr;
}

static int
_optsigrun(gageOptimSigParm *parm) {
  static const char me[]="_optsigrun";
  char tstr[AIR_STRLEN_MED];
  unsigned int iter, pnt;
  double lastErr, newErr, sigeps, oppor, lastPos, backoff, decavg, time0;
  int badStep;

  time0 = airTime();
  lastErr = _errTotal(parm);
  fprintf(stderr, "%s: (%s for initial error measr)\n", me,
          _timefmt(tstr, airTime() - time0));
  newErr = AIR_NAN;
  decavg = parm->sampleNum; /* hack */
  /* meaningful discrete difference for looking at error gradient is
     bounded by the resolution of the sampling we're doing along scale */
  sigeps = parm->sigmatru[1]/10;
  oppor = 1.3333;
  backoff = 0.25;
  for (pnt=1; pnt<parm->sampleNum-1; pnt++) {
    parm->step[pnt] = 10;
  }
  for (iter=0; iter<parm->maxIter; iter++) {
    double limit, err1, grad, delta;
    unsigned int tryi;
    int zerodelta, esgn;
    esgn = 2*AIR_CAST(int, airRandInt(2)) - 1;
    pnt = 1 + (iter % (parm->sampleNum-2));
    lastPos = parm->scalePos[pnt];
    fprintf(stderr, "%s: ***** iter %u; [[ err %g ]] %s\n",
            me, iter, lastErr, _timefmt(tstr, airTime() - time0));
    limit = AIR_MIN((parm->scalePos[pnt] - parm->scalePos[pnt-1])/3,
                    (parm->scalePos[pnt+1] - parm->scalePos[pnt])/3);
    fprintf(stderr, ". pnt %u: pos %g, step %g\n",
            pnt, lastPos, parm->step[pnt]);
    fprintf(stderr, ". limit = min((%g-%g)/3,(%g-%g)/3) = %g\n",
            parm->scalePos[pnt], parm->scalePos[pnt-1],
            parm->scalePos[pnt+1], parm->scalePos[pnt], limit);
    _scalePosSet(parm, pnt, lastPos + esgn*sigeps);
    err1 = _errTotal(parm);
    _scalePosSet(parm, pnt, lastPos);
    grad = (err1 - lastErr)/(esgn*sigeps);
    fprintf(stderr, ". grad = %g\n", grad);
    delta = -grad*parm->step[pnt];
    if (!AIR_EXISTS(delta)) {
      biffAddf(GAGE, "%s: got non-exist delta %g on iter %u (pnt %u) err %g",
               me, delta, iter, pnt, lastErr);
      return 1;
    }
    if (AIR_ABS(delta) > limit) {
      parm->step[pnt] *= limit/AIR_ABS(delta);
      fprintf(stderr, ". step *= %g/%g -> %g\n",
              limit, AIR_ABS(delta), parm->step[pnt]);
      delta = -grad*parm->step[pnt];
    }
    fprintf(stderr, ". delta = %g\n", delta);
    tryi = 0;
    badStep = AIR_FALSE;
    do {
      if (tryi == parm->maxIter) {
        biffAddf(GAGE, "%s: confusion (tryi %u) on iter %u (pnt %u) err %g",
                 me, tryi, iter, pnt, lastErr);
        return 1;
      }
      if (!delta) {
        fprintf(stderr, "... try %u: delta = 0; nothing to do\n", tryi);
        newErr = lastErr;
        zerodelta = AIR_TRUE;
      } else {
        zerodelta = AIR_FALSE;
        _scalePosSet(parm, pnt, lastPos + delta);
        newErr = _errTotal(parm);
        badStep = newErr > lastErr;
        fprintf(stderr, "... try %u: pos[%u] %g + %g = %g;\n"
                "%s: err %g %s %g\n",
                tryi, pnt, lastPos, delta,
                parm->scalePos[pnt],
               badStep ? "*BAD*" : "good",
               newErr, newErr > lastErr ? ">" : "<=", lastErr);
        if (badStep) {
          parm->step[pnt] *= backoff;
          if (parm->step[pnt] < sigeps/1000) {
            /* step got so small its stupid to be moving this point */
            fprintf(stderr, "... !! step %g < %g pointlessly small, "
                    "moving on\n", parm->step[pnt], sigeps/1000);
            _scalePosSet(parm, pnt, lastPos);
            newErr = lastErr;
            badStep = AIR_FALSE;
          } else {
            delta = -grad*parm->step[pnt];
          }
        }
      }
      tryi++;
    } while (badStep);
    if (!zerodelta) {
      /* don't update decavg if we moved on because slope was EXACTLY zero */
      decavg = AIR_AFFINE(0, 1, parm->sampleNum,
                          decavg, (lastErr - newErr)/lastErr);
      parm->step[pnt] *= oppor;
    }
    if (decavg <= parm->convEps) {
      fprintf(stderr, "%s: converged (%g <= %g) after %u iters\n", me,
              decavg, parm->convEps, iter);
      break;
    } else {
      fprintf(stderr, "%s: _____ iter %u done; decavg = %g > %g\n", me,
              iter, decavg, parm->convEps);
    }
    lastErr = newErr;
  }
  if (iter == parm->maxIter) {
    biffAddf(GAGE, "%s: failed to converge (%g > %g) after %u iters\n", me,
             decavg, parm->convEps, iter);
    return 1;
  }
  parm->finalErr = lastErr;
  return 0;
}

int
gageOptimSigCalculate(gageOptimSigParm *parm,
                      double *scalePos, unsigned int num,
                      int volMeasr, int lineMeasr,
                      double convEps, unsigned int maxIter) {
  static const char me[]="gageOptimSigCalculate";
  unsigned int ii;
  double tauMax;

  if (!( parm && scalePos && num )) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( AIR_IN_CL(1, parm->dim, 3)
         && parm->ntruth->data )) {
    biffAddf(GAGE, "%s: incomplete parm setup?", me);
    return 1;
  }
  if (num > parm->sampleNumMax) {
    biffAddf(GAGE, "%s: parm setup for max %u samples, not %u", me,
             parm->sampleNumMax, num);
    return 1;
  }
  /* copy remaining input parms */
  parm->sampleNum = num;
  parm->volMeasr = volMeasr;
  parm->lineMeasr = lineMeasr;
  parm->maxIter = maxIter;
  parm->convEps = convEps;

  /* initialize the scalePos[] array to uniform samples in tau */
  fprintf(stderr, "%s: initializing samples ... ", me); fflush(stderr);
  tauMax = gageTauOfSig(parm->sigmaMax);
  for (ii=0; ii<parm->sampleNum; ii++) {
    double tau;
    tau = AIR_AFFINE(0, ii, parm->sampleNum-1, 0, tauMax);
    _scalePosSet(parm, ii, gageSigOfTau(tau));
  }
  fprintf(stderr, "done.\n");

  /* set up gage */
  fprintf(stderr, "%s: setting up gage ... \n", me);
  if (_gageSetup(parm)) {
    biffAddf(GAGE, "%s: problem setting up gage", me);
    return 1;
  }
  fprintf(stderr, "%s: gage setup done.\n", me);

  /* run the optimization */
  if (num > 2) {
    if (_optsigrun(parm)) {
      biffAddf(GAGE, "%s: trouble", me);
      return 1;
    }
  } else {
    fprintf(stderr, "%s: num == 2, no optimization, finding error ... ", me);
    fflush(stderr);
    parm->finalErr = _errTotal(parm);
    fprintf(stderr, "done.\n");
  }

  /* save output */
  for (ii=0; ii<num; ii++) {
    scalePos[ii] = parm->scalePos[ii];
  }

  return 0;
}

int
gageOptimSigPlot(gageOptimSigParm *parm, Nrrd *nout,
                 const double *plotPos, unsigned int plotPosNum,
                 int volMeasr, int tentRecon) {
  char me[]="gageOptimSigPlot", doneStr[AIR_STRLEN_SMALL];
  unsigned int ii;
  double *out;

  if (!(parm && nout && plotPos)) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( plotPosNum >= 2 )) {
    biffAddf(GAGE, "%s: need plotPosNum >= 2 (not %u)", me, plotPosNum);
    return 1;
  }
  /* HEY: copy and paste from above */
  if (!( AIR_IN_CL(1, parm->dim, 3)
         && parm->ntruth->data )) {
    biffAddf(GAGE, "%s: incomplete parm setup?", me);
    return 1;
  }
  if (plotPosNum > parm->sampleNumMax) {
    biffAddf(GAGE, "%s: parm setup for max %u samples, not %u", me,
             parm->sampleNumMax, plotPosNum);
    return 1;
  }
  /* copy remaining input parms */
  parm->sampleNum = plotPosNum;
  parm->volMeasr = volMeasr;
  parm->tentRecon = tentRecon;
  if (nrrdMaybeAlloc_va(nout, nrrdTypeDouble, 1,
                        AIR_CAST(size_t, parm->measrSampleNum))) {
    biffMovef(GAGE, NRRD, "%s: trouble allocating output", me);
    return 1;
  }
  out = AIR_CAST(double *, nout->data);

  /* set up requested samples */
  for (ii=0; ii<parm->sampleNum; ii++) {
    _scalePosSet(parm, ii, plotPos[ii]);
  }
  if (_gageSetup(parm)) {
    biffAddf(GAGE, "%s: problem setting up gage", me);
    return 1;
  }
  fprintf(stderr, "%s: working ...       ", me);
  for (ii=0; ii<parm->measrSampleNum; ii++) {
    printf("%s", airDoneStr(0, ii, parm->measrSampleNum, doneStr));
    fflush(stderr);
    out[ii] = _errSingle(parm, ii);
  }
  fprintf(stderr, "%s\n", airDoneStr(0, ii, parm->measrSampleNum, doneStr));

  return 0;
}
