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

#ifndef PULL_HAS_BEEN_INCLUDED
#define PULL_HAS_BEEN_INCLUDED

#include <teem/air.h>
#include <teem/hest.h>
#include <teem/biff.h>
#include <teem/nrrd.h>
#include <teem/ell.h>
#include <teem/gage.h>
#include <teem/limn.h>
#include <teem/ten.h>

/*
** This library was created to implement the research and methods of:
**
** Gordon L. Kindlmann, Ra{\'u}l San Jos{\'e} Est{\'e}par,
** Stephen M. Smith, Carl-Fredrik Westin.
** Sampling and Visualizing Creases with Scale-Space Particles.
** IEEE Trans. on Visualization and Computer Graphics,
** 15(6):1415-1424 (2009)
**
** Further information and usage examples:
** http://people.cs.uchicago.edu/~glk/ssp/
**
** The library is still being actively developed to support research on
** particle systems for general feature sampling.  At some point, for
** example, it should subsume the "push" library for glyph packing in
** tensor fields.
*/

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(pull_EXPORTS) || defined(teem_EXPORTS)
#    define PULL_EXPORT extern __declspec(dllexport)
#  else
#    define PULL_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define PULL_EXPORT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PULL pullBiffKey
#define PULL_THREAD_MAXNUM 512
#define PULL_VOLUME_MAXNUM 4
#define PULL_POINT_NEIGH_INCR 16
#define PULL_BIN_MAXNUM 40000000 /* sanity check on max number bins */
#define PULL_PHIST 0
#define PULL_HINTER 0
#define PULL_TANCOVAR 1

/*
******** pullInfo enum
**
** all the things that might be *learned* about the local neighborhood
** that are used as ingredients in the computation of particle motion.
** This info was originally learned only from gage, but now (according
** to value of pullSource) can come from other kinds of information.
**
** There are multiple scalars (and associated) derivatives that can
** be used for dynamics:
** - Inside: just for nudging things to stay inside a mask
** - Height: value for computer-vision-y features of ridges, valleys,
**   and edges.  Setting pullInfoHeight as a constraint does valley
**   sampling (flip the sign to get ridges), based on the various
**   "tangents" Setting pullInfoHeightLaplacian as a constraint
*    does zero-crossing edge detection.
** - Isovalue: just for implicit surfaces f=0
** - Strength: some measure of feature strength, with the assumption
**   that it can't be analytically differentiated in space or scale.
*/
enum {
  pullInfoUnknown,            /*  0 */
  pullInfoTensor,             /*  1: [7] tensor here */
  pullInfoTensorInverse,      /*  2: [7] inverse of tensor here */
  pullInfoHessian,            /*  3: [9] hessian used for force distortion */
  pullInfoInside,             /*  4: [1] containment scalar */
  pullInfoInsideGradient,     /*  5: [3] containment vector */
  pullInfoHeight,             /*  6: [1] for gravity, and edge and crease
                                         feature detection */
  pullInfoHeightGradient,     /*  7: [3] */
  pullInfoHeightHessian,      /*  8: [9] */
  pullInfoHeightLaplacian,    /*  9: [1] for zero-crossing edge detection */
  pullInfoSeedPreThresh,      /* 10: [1] scalar for pre-thresholding seeding,
                                 so that points can be quickly eliminated
                                 (e.g. prior to constraint satisfaction) */
  pullInfoSeedThresh,         /* 11: [1] scalar for thresholding seeding */
  pullInfoLiveThresh,         /* 12: [1] scalar for thresholding extant
                                 particles, AND for future additions from
                                 population control */
  pullInfoLiveThresh2,        /* 13: [1] another pullInfoLiveThresh */
  pullInfoLiveThresh3,        /* 14: [1] yet another pullInfoLiveThresh */
  pullInfoTangent1,           /* 15: [3] first tangent to motion allowed
                                 for constraint satisfaction */
  pullInfoTangent2,           /* 16: [3] second tangent to motion allowed
                                 for constraint satisfaction */
  pullInfoNegativeTangent1,   /* 17: [3] like the tangents, but with a negated
                                 objective function */
  pullInfoNegativeTangent2,   /* 18: [3] second negative tangent */
  pullInfoIsovalue,           /* 19: [1] for isosurface extraction */
  pullInfoIsovalueGradient,   /* 20: [3] */
  pullInfoIsovalueHessian,    /* 21: [9] */
  pullInfoStrength,           /* 22: [1] */
  pullInfoQuality,            /* 23: [1] */
  pullInfoLast
};
#define PULL_INFO_MAX            23

/*
********* pullProp* enum: the various properties of particles in the system
**
** These are things that are not learned from the image data, but are
** descriptions of a particle, its neighborhood of particles in the
** system, and the current state of system computation.  As of revision
** 5080, these may refer to information *computed* from the per-particle
** info, but which may not itself be saved as a field in pullPoint.
**
** consider adding: dot between normalized directions of force and movmt
*/
enum {
  pullPropUnknown,            /*  0: nobody knows */
  pullPropIdtag,              /*  1: [1] idtag (unsigned int) */
  pullPropIdCC,               /*  2: [1] idcc (unsigned int) */
  pullPropEnergy,             /*  3: [1] energy from last iteration */
  pullPropStepEnergy,         /*  4: [1] step size for minimizing energy */
  pullPropStepConstr,         /*  5: [1] step size for constraint satis. */
  pullPropStuck,              /*  6: [1] how many iters its been stuck */
  pullPropPosition,           /*  7: [4] position */
  pullPropForce,              /*  8: [4] force accumulation */
  pullPropNeighDistMean,      /*  9: [1] "mean distance" to neighbors */
  pullPropScale,              /* 10: [1] scale position */
  pullPropNeighCovar,         /* 11: [10] unique coeffs of covariance matrix
                                 of offsets to all interacting neighbors,
                                 in rs-normalized space, updated only during
                                 pullProcessModeNeighLearn. The layout is:
                                 0:xx 1:xy 2:xz 3:xs
                                 .    4:yy 5:yz 6:ys
                                 .         7:zz 8:zs
                                 .              9:ss */
  pullPropNeighCovar7Ten,     /* 12: [7] spatial 3x3 submatrix of covariance,
                                 formatted as ten-compatible 7-tensor */
  pullPropNeighTanCovar,      /* 13: [6] covariance of "tangents" of neighbors,
                                 (e.g. pullInfoTangent1 for crease surfaces)
                                 including point itself */
  pullPropNeighInterNum,      /* 14: [1] # neighbors last iter */
  pullPropNeighCovarTrace,    /* 15: [1] trace of NeighCovar */
  pullPropNeighCovarDet,      /* 16: [1] det of NeighCovar */
  pullPropStability,          /* 17: [1] some measure of NeighCovar */
  pullPropLast
};
#define PULL_PROP_MAX            17

/*
** the components of a point's status that are set as a bitflag
** in point->status
*/
enum {
  pullStatusUnknown,             /* 0: nobody knows */
  pullStatusStuck,               /* 1: couldn't move to decrease energy */
#define PULL_STATUS_STUCK_BIT  (1<< 1)
  pullStatusNewbie,              /* 2: not binned, testing if the system
                                    would be better with me in it */
#define PULL_STATUS_NEWBIE_BIT (1<< 2)
  pullStatusNixMe,               /* 3: nix me at the *end* of this iter,
                                    and don't look at me for energy
                                    during this iteration */
#define PULL_STATUS_NIXME_BIT  (1<< 3)
  pullStatusEdge,                /* 4: at the spatial edge of one of the
                                    volumes: gage had to invent values for
                                    some samples in the kernel support */
#define PULL_STATUS_EDGE_BIT   (1<< 4)
  pullStatusLast
};

/*
******** pullInterType* enum
**
** the different types of scale-space interaction that can happen
** in scale-space.  The descriptions here overlook the normalization
** by radiusScale and radiusSpace
*/
enum {
  pullInterTypeUnknown,      /* 0 */
  pullInterTypeJustR,        /* 1: phi(r,s) = phi_r(r) */
  pullInterTypeUnivariate,   /* 2: phi(r,s) = phi_r(u); u = sqrt(r*r+s*s) */
  pullInterTypeSeparable,    /* 3: phi(r,s) = phi_r(r)*phi_s(s) */
  pullInterTypeAdditive,     /* 4: phi(r,s) = beta*phi_r(r)*win(s)
                                              + (1-beta)*win(r)*phi_s(s) */
  pullInterTypeLast
};
#define PULL_INTER_TYPE_MAX     4

/*
******** pullEnergyType* enum
**
** the different shapes of potential energy profiles that can be used
*/
enum {
  pullEnergyTypeUnknown,             /* 0 */
  pullEnergyTypeSpring,              /* 1 */
  pullEnergyTypeGauss,               /* 2 */
  pullEnergyTypeBspln,               /* 3 */
  pullEnergyTypeButterworth,         /* 4 */
  pullEnergyTypeCotan,               /* 5 */
  pullEnergyTypeCubic,               /* 6 */
  pullEnergyTypeQuartic,             /* 7 */
  pullEnergyTypeCubicWell,           /* 8 */
  pullEnergyTypeBetterCubicWell,     /* 9 */
  pullEnergyTypeQuarticWell,         /* 10 */
  pullEnergyTypeHepticWell,          /* 11 */
  pullEnergyTypeZero,                /* 12 */
  pullEnergyTypeButterworthParabola, /* 13 */
  pullEnergyTypeLast
};
#define PULL_ENERGY_TYPE_MAX            13
#define PULL_ENERGY_PARM_NUM 3

enum {
  pullProcessModeUnknown,      /* 0 */
  pullProcessModeDescent,      /* 1 */
  pullProcessModeNeighLearn,   /* 2 */
  pullProcessModeAdding,       /* 3 */
  pullProcessModeNixing,       /* 4 */
  pullProcessModeLast
};
#define PULL_PROCESS_MODE_MAX     4

/*
** the conditions under which a point may find itself at some position
*/
enum {
  pullCondUnknown,            /* 0 */
  pullCondOld,                /* 1 */
  pullCondConstraintSatA,     /* 2 */
  pullCondConstraintSatB,     /* 3 */
  pullCondEnergyTry,          /* 4 */
  pullCondConstraintFail,     /* 5 */
  pullCondEnergyBad,          /* 6 */
  pullCondNew,                /* 7 */
  pullCondLast
};

/*
** the places that "info" can be learned from.  Originally this was
** strictly from gage and no where else; it can be handy to allow it
** to originate from different places as well, but still flow through
** the channels now organized around pullInfo
*/
enum {
  pullSourceUnknown,  /* 0 */
  pullSourceGage,     /* 1: measured from gage */
  pullSourceProp,     /* 2: copied from a pullProp */
  pullSourceLast
};
#define PULL_SOURCE_MAX  2

/*
** the different kinds of computations and entities that one can
** count, for book-keeping and meta-optimization purposes
*/
enum {
  pullCountUnknown,             /*  0 */
  pullCountDescent,             /*  1 */
  pullCountTestStep,            /*  2 */
  pullCountEnergyFromImage,     /*  3 */
  pullCountForceFromImage,      /*  4 */
  pullCountEnergyFromPoints,    /*  5 */
  pullCountForceFromPoints,     /*  6 */
  pullCountProbe,               /*  7 */
  pullCountConstraintSatisfy,   /*  8 */
  pullCountAdding,              /*  9 */
  pullCountNixing,              /* 10 */
  pullCountPointsStuck,         /* 11 */
  pullCountPoints,              /* 12 */
  pullCountCC,                  /* 13 */
  pullCountIteration,           /* 14 */
  pullCountLast
};
#define PULL_COUNT_MAX             14

/*
** reasons for pullTraceSet to stop (or go nowhere)
*/
enum {
  pullTraceStopUnknown,         /* 0 */
  pullTraceStopSpeeding,        /* 1 */
  pullTraceStopConstrFail,      /* 2 */
  pullTraceStopBounds,          /* 3 */
  pullTraceStopLength,          /* 4 */
  pullTraceStopStub,            /* 5 */
  pullTraceStopLast
};
#define PULL_TRACE_STOP_MAX        5

/*
** Defines how par-particle information can be learned.  This is
** typically via measurements in the image by gage, but other sources
** are possible (as indicated by the source field).
**
** There is a basic design decision in whether to make pullInfos the
** most general representation of information (including referring to
** pullProps), or, whether pullProps should be the general thing,
** which can then refer to some pullInfo.  In the long run, these
** should probably be merged.  In the short term, there is better
** infrastructure for representing and parsing pullInfos, so that has
** been leveraged to include other things as well.
*/
typedef struct pullInfoSpec_t {
  /* ------ INPUT ------ */
  int info,                     /* from the pullInfo* enum: what is the
                                   "info" that this infospec defines */
    source;                     /* from the pullSource* enum: where does
                                   this information come from */
  char *volName;                /* volume name */
  int item,                     /* which gage item (for pullSourceGage) */
    prop;                       /* which pull property (for pullSourceProp) */
  double scale,                 /* scaling factor (including sign) */
    zero;                       /* for height and inside: where is zero,
                                   for seedThresh, threshold value */
  int constraint;               /* (for scalar items) minimizing this
                                   is a constraint to enforce per-point
                                   per-iteration, not merely a contribution
                                   to the point's energy */
  /* ------ INTERNAL ------ */
  unsigned int volIdx;          /* which volume (for pullSourceGage) */
} pullInfoSpec;

/*
******** pullPoint
**
*/
typedef struct pullPoint_t {
  unsigned int idtag,         /* unique point ID */
    idCC;                     /* id for connected component analysis */
  struct pullPoint_t **neighPoint; /* list of neighboring points */
  unsigned int neighPointNum;
  airArray *neighPointArr;    /* airArray around neighPoint and neighNum
                                 (no callbacks used here) */
  double neighDistMean;       /* average of distance to neighboring
                                 points with whom this point interacted,
                                 in rs-normalized space */
  float neighCovar[10],       /* unique coeffs in 4x4 covariance matrix of
                                 neighbors with whom this point interacted */
#if PULL_TANCOVAR
    neighTanCovar[6],         /* covariance of "tangent" info of neighbors */
#endif
    stability;                /* the scalar stability measure */
  unsigned int neighInterNum, /* number of particles with which I had some
                                 non-zero interaction on last iteration */
    stuckIterNum;             /* how many iterations I've been stuck */
#if PULL_PHIST
  double *phist;              /* history of positions tried in the last iter,
                                 in sets of 5 doubles: (x,y,z,t,info) */
  unsigned int phistNum;      /* number of positions stored */
  airArray *phistArr;         /* airArray around phist */
#endif
  int status;                 /* bit-flag of status info */
  double pos[4],              /* position in space and scale */
    energy,                   /* energy accumulator for this iteration */
    force[4],                 /* force accumulator for this iteration */
    stepEnergy,               /* step size for energy minimization */
    stepConstr,               /* step size for constraint satisfaction;
                                 HEY: this doens't seem to be really used? */
    info[1];                  /* all information learned from gage that matters
                                 for particle dynamics.  This is sneakily
                                 allocated for *more*, depending on needs,
                                 so this MUST be last field */
} pullPoint;

/*
******** pullBin
**
** the data structure for doing spatial binning.
*/
typedef struct pullBin_t {
  pullPoint **point;         /* dyn. alloc. array of point pointers */
  unsigned int pointNum;     /* # of points in this bin */
  airArray *pointArr;        /* airArray around point and pointNum
                                (no callbacks used here) */
  struct pullBin_t **neighBin;  /* NULL-terminated list of all
                                   neighboring bins, including myself */
} pullBin;

/*
******** pullEnergy
**
** the functions which determine inter-point forces
**
** NOTE: the eval() function probably does NOT check to see it was passed
** a non-NULL pointer into which to store the derivative of energy ("denr")
**
** Thu Apr 10 12:40:08 EDT 2008: nixed the "support" function, since it
** was annoying to deal with variable support potentials.  Now everything
** cuts off at dist=1.  You can still use the parm vector to change the
** shape inside the support.
*/
typedef struct {
  char name[AIR_STRLEN_SMALL];
  unsigned int parmNum;
  double (*well)(double *wx, const double parm[PULL_ENERGY_PARM_NUM]);
  double (*eval)(double *denr, double dist,
                 const double parm[PULL_ENERGY_PARM_NUM]);
} pullEnergy;

typedef struct {
  const pullEnergy *energy;
  double parm[PULL_ENERGY_PARM_NUM];
} pullEnergySpec;

/*
** In the interests of simplicity (and with the cost of some redundancy),
** this is going to copied per-task, which is why it contains the gageContext
** The idea is that the first of these is somehow set up by the user
** or something, and the rest of them are created within pull per-task.
*/
typedef struct {
  int verbose;                 /* blah blah blah */
  char *name;                  /* how the volume will be identified
                                  (like its a variable name) */
  const gageKind *kind;
  const Nrrd *ninSingle;       /* don't own */
  const Nrrd *const *ninScale; /* don't own;
                                  NOTE: only one of ninSingle and ninScale
                                  can be non-NULL */
  unsigned int scaleNum;       /* number of scale-space samples (volumes) */
  double *scalePos;            /* location of all samples in scale */
  int scaleDerivNorm;          /* normalize derivatives based on scale */
  double scaleDerivNormBias;   /* bias on derivative normalization by scale */
  NrrdKernelSpec *ksp00,       /* for sampling tensor field */
    *ksp11,                    /* for gradient of mask, other 1st derivs */
    *ksp22,                    /* for 2nd derivatives */
    *kspSS;                    /* for reconstructing from scale-space
                                  samples */
  gageQuery pullValQuery;      /* if this is a pullValGageKind volume,
                                  then we don't have a real gageContext,
                                  and we have to manage our own query */
  gageContext *gctx;           /* do own, and set based on info here */
  gagePerVolume *gpvl,         /* stupid gage API . . . */
    **gpvlSS;                  /* stupid gage API . . . */
  int seedOnly,                /* volume only required for seeding, for
                                  either pullInfoSeedThresh or
                                  pullInfoSeedPreThresh */
    forSeedPreThresh;          /* we learn pullInfoSeedPreThresh from this */
} pullVolume;

/*
******** pullTask
**
** The information specific for a thread.
*/
typedef struct pullTask_t {
  struct pullContext_t
    *pctx;                      /* parent's context; not const because the
                                   tasks assign themselves bins to do work */
  pullVolume
    *vol[PULL_VOLUME_MAXNUM];   /* volumes copied from parent */
  const double
    *ans[PULL_INFO_MAX+1];      /* answer *pointers* for all possible infos,
                                   pointing into per-task per-volume gctxs
                                   (or into above per-task pullValAnswer),
                                   OR: NULL if that info is not being used */
  int processMode,              /* what kind of point processing is being
                                   done by this task right now */
    probeSeedPreThreshOnly;     /* hack-ish flag to communicate to pullProbe
                                   that we only care about SeedPreThresh */
  airThread *thread;            /* my thread */
  unsigned int threadIdx;       /* which thread am I */
  airRandMTState *rng;          /* state for my RNG */
  pullPoint *pointBuffer,       /* place for copying point into during
                                   strength ascent computation; can't be
                                   statically allocated because pullPoint
                                   size is known only at run-time */
    **neighPoint;               /* array of point pointers, either all
                                   possible points from neighbor bins, or
                                   last learned interacting neighbors */
  pullPoint **addPoint;         /* points to add before next iter */
  unsigned int addPointNum;     /* # of points to add */
  airArray *addPointArr;        /* airArray around addPoint, addPointNum */
  pullPoint **nixPoint;         /* points to nix before next iter */
  unsigned int nixPointNum;     /* # of points to nix */
  airArray *nixPointArr;        /* airArray around nixPoint, nixPointNum */
  void *returnPtr;              /* for airThreadJoin */
  unsigned int stuckNum;        /* # stuck particles seen by this task */
} pullTask;

/*
******** pullInitMethod* enum
**
** the different ways pull can be initialized
*/
enum {
  pullInitMethodUnknown,       /* 0 */
  pullInitMethodRandom,        /* 1 */
  pullInitMethodHalton,        /* 2 */
  pullInitMethodPointPerVoxel, /* 3 */
  pullInitMethodGivenPos,      /* 4 */
  pullInitMethodLast
};

/*
******** pullInitParm
**
** none of this is directly user-set; set with pullInit*Set function
** (note that there is no pullInit* enum; these values are too diverse)
*/
typedef struct {
  int method;               /* from pullInitMethod* enum */
  int liveThreshUse,        /* use the liveThresh info as a criterion
                               for seeding */
    unequalShapesAllow;     /* when taking in volumes, allow their shapes
                               to be unuequal.  Some confusing usage problems
                               are due to using differently shaped volumes */
  double jitter;            /* w/ PointPerVoxel,
                               how much to jitter index space positions */
  unsigned int numInitial,  /* w/ Random OR Halton, # points to start with */
    haltonStartIndex,       /* w/ Halton, first index to use */
    samplesAlongScaleNum,   /* w/ PointPerVoxel,
                               # of samples along scale (distributed
                               uniformly in scale's *index* space*/
    ppvZRange[2];           /* (hack to permit seeding only in part of
                               volume, when initialization is painfully
                               the main bottleneck) w/ PointPerVoxel,
                               range of indices along Z to do seeding
                               by pointPerVoxel, or, {1,0} to do the
                               whole volume as normal */
  int pointPerVoxel;        /* w/ PointPerVoxel,
                               if this (ppv) is > 0, then use
                               ppv points per voxel. If ppv < 0, then jitter
                               point point every -ppv'th voxel
                               (so ppv=-1 is same as ppv=1) */
  const Nrrd *npos;         /* positions (4xN array) to start with */
} pullInitParm;

/*
******** pullIterParm* enum
**
** parameters related to iterations and their periods
*/
enum {
  pullIterParmUnknown,

  /* if non-zero, minimum number of iterations for whole system. */
  pullIterParmMin,

  /* if non-zero, max number of iterations for whole system.
     if zero: no explicit limit on the number of iterations */
  pullIterParmMax,

  /* if non-zero, max number of iterations we allow something to be
     continuously stuck before nixing it */
  pullIterParmStuckMax,

  /* if non-zero, max number of iterations for enforcing each constraint */
  pullIterParmConstraintMax,

  /* how many intervals to wait between attemps at population control,
     or, 0 to say: "no pop cntl" */
  pullIterParmPopCntlPeriod,

  /* how many iterations for which to run descent on tentative new
     points during population control, so that they end up at a good
     location, at which we can meaningfully test whether adding the
     point will reducing system energy */
  pullIterParmAddDescent,

  /* periodicity with which to call the pullContext->iter_cb */
  pullIterParmCallback,

  /* if non-zero, interval between iters at which output snapshots are saved */
  pullIterParmSnap,

  /* the half-life of energyIncreasePermit, in terms of iterations, or
     0 if no such decay is wanted */
  pullIterParmEnergyIncreasePermitHalfLife,

  pullIterParmLast
};

typedef struct {
  unsigned int min, max,
    popCntlPeriod,
    addDescent,
    constraintMax,
    stuckMax,
    callback,
    snap,
    energyIncreasePermitHalfLife;
} pullIterParm;

/*
******** pullSysParm* enum
**
** various continuous parameters.  It should be understood that the number
** of these is a reflection of the experimentation and exploration that
** went into the creation of these particles systems, rather than a need
** for onerous parameter tweaking in order to get something useful done.
** Except for alpha, beta, and gamma, these have reasonable defaults.
*/
enum {
  pullSysParmUnknown,

  /* balance between particle-image energy E_i and inter-particle energy E_ij,
     the most important parameter in the governing equation of the system.
     alpha = 0: only particle-image; alpha = 1: only inter-particle */
  pullSysParmAlpha,

  /* when using pullInterAdditive ("Phi_2") inter-particle energy, beta
     sets the balance between spatial repulsion and scale attraction.
     (if not using this energy, this is moot)
     beta = 0: only spatial repulsion; beta = 1: only scale attraction */
  pullSysParmBeta,

  /* when energyFromStrength is non-zero: scaling factor on energy
     from strength */
  pullSysParmGamma,

  /* when learning a suitable gamma from data (via pullGammaLearn)
     in the context of separable energy functions, how much to
     scale the learned gamma. Making this greater than 1.0 helps
     let the energy from strength overpower the slight repulsion
     along scale that might exist by the separable phi construction. */
  pullSysParmSeparableGammaLearnRescale,

  /* to be more selective for pullInfoSeedThresh and
     pullInfoLiveThresh at higher scales, set theta > 0, and the
     effective threshold will be base threshold + theta*scale.
     HOWEVER, the way this is implemented is a hack:
     the probed strength value is decremented by theta*scale */
  pullSysParmTheta,

  /* initial (time) step for dynamics */
  pullSysParmStepInitial,

  /* radius/scaling of inter-particle interactions in the spatial domain */
  pullSysParmRadiusSpace,

  /* radius/scaling of inter-particle interactions in the scale domain */
  pullSysParmRadiusScale,

  /* spatial width of bin, as multiple of pullSysParmRadiusSpace (the
     width along scale is set to pullSysParmRadiusScale). Can't be
     lower than 1, but may be usefully set greater that 1 to reduce
     the total number of bins, especially if caching neighbor lists */
  pullSysParmBinWidthSpace,

  /* probability that we find the true neighbors of the particle, as
     opposed to using a cached list */
  pullSysParmNeighborTrueProb,

  /* probability that we do image probing to find out what's really going on */
  pullSysParmProbeProb,

  /* (>= 1.0) how much to opportunistically scale up step size (for
     energy minimization) with every iteration */
  pullSysParmOpporStepScale,

  /* (< 1.0) when energy goes up instead of down, or when constraint
     satisfaction seems to be going the wrong way, how to scale (down)
     step size */
  pullSysParmBackStepScale,

  /* pseudo-convergence threshold that controls when population control is
     activated (has to be higher than (less strict) energyDecreaseMin */
  pullSysParmEnergyDecreasePopCntlMin,

  /* epsilon amount by which its okay for particle energy to increase,
     in the context of gradient descent */
  pullSysParmEnergyIncreasePermit,

  /* convergence threshold: stop when fractional improvement
     (decrease) in total system energy dips below this */
  pullSysParmEnergyDecreaseMin,

  /* convergence threshold for constraint satisfaction: finished if
     stepsize goes below this times pctx->voxelSizeSpace */
  pullSysParmConstraintStepMin,

  /* spring constant on bbox wall */
  pullSysParmWall,

  /* when doing population control nixing, don't nix a particle if this
     fraction of its neighbors have already been nixed (Section 3.5 of
     paper implies that this value should be 0.5; lower values also work) */
  pullSysParmFracNeighNixedMax,

  pullSysParmLast
};

typedef struct {
  double alpha, beta, gamma, separableGammaLearnRescale, theta, wall,
    radiusSpace, radiusScale, binWidthSpace,
    neighborTrueProb, probeProb,
    stepInitial, opporStepScale, backStepScale, constraintStepMin,
    energyDecreaseMin,
    energyDecreasePopCntlMin,
    energyIncreasePermit,
    fracNeighNixedMax;
} pullSysParm;

/*
******** pullConstraintFail* enum
**
** the various ways constriant satisfaction can fail
*/
enum {
  pullConstraintFailUnknown,        /* 0 */
  pullConstraintFailProjGradZeroA,  /* 1 */
  pullConstraintFailProjGradZeroB,  /* 2 */
  pullConstraintFailIterMaxed,      /* 3 */
  pullConstraintFailTravel,         /* 4 */
  pullConstraintFailLast
};
#define PULL_CONSTRAINT_FAIL_MAX       4

/*
******** pullFlag* enum
**
** the various booleans
*/
enum {
  pullFlagUnknown,

  /* permute points during rebinning between iters, to randomize their
     ordering within their bin (bins are still processed in scanline
     order) */
  pullFlagPermuteOnRebin,

  /* when alpha is exactly zero, probably with the purpose of migrating
     particles along scale towards the scale of maximal strength while
     avoiding all inter-particle interaction, don't try any population
     control */
  pullFlagNoPopCntlWithZeroAlpha,

  /* when learning gamma via Eq. 26 of the paper, also use beta to
     more accurately reflect the 2nd derivative of energy wrt scale */
  pullFlagUseBetaForGammaLearn,

  /* whether or not to deny adding points to bins where there are
     close points already */
  pullFlagRestrictiveAddToBins,

  /* if non-zero, strength is a particle-image energy term that is
     minimized by motion along scale, which in turn requires extra
     probing to determine the strength gradient along scale. */
  pullFlagEnergyFromStrength,

  /* if non-zero, nix points that got near enough to the volume edge
     that gage had to invent values for the kernel support */
  pullFlagNixAtVolumeEdgeSpace,

  /* if non-zero, during initialization, try constraint satisfaction
     (if there is a constraint) before testing whether the seedThresh
     is met.  Doing the constraint will take longer, but a point is
     more likely to meet a threshold based on feature strength */
  pullFlagConstraintBeforeSeedThresh,

  /* do no adding during population control */
  pullFlagNoAdd,

  /* use the targetDim-based "enough" heuristic to guess whether
     adding a point could usefully reduce system energy.  On by
     default; turn this off when using a large-support energy that
     involves more neighbors than the single-neighbor-deep energies
     that are normally used (like cwell:0.66,x or qwell:0.64) */
  pullFlagPopCntlEnoughTest,

  /* no binning: all particles can potentially interact (for debugging) */
  pullFlagBinSingle,

  /* whether to allow codimension-3 (point) constraint manifolds.
     Typical uses of constraints are for extracting lines and surfaces */
  pullFlagAllowCodimension3Constraints,

  /* what we call scale is not sigma but rather the tau of gage */
  pullFlagScaleIsTau,

  /* pullStart should skip initializing the points */
  pullFlagStartSkipsPoints,

  pullFlagLast
};

typedef struct {
  int permuteOnRebin,
    noPopCntlWithZeroAlpha,
    useBetaForGammaLearn,
    restrictiveAddToBins,
    energyFromStrength,
    nixAtVolumeEdgeSpace,
    constraintBeforeSeedThresh,
    popCntlEnoughTest,
    noAdd,
    binSingle,
    allowCodimension3Constraints,
    scaleIsTau,
    startSkipsPoints;
} pullFlag;

/*
******** pullContext
**
** everything for doing one computation
**
*/
typedef struct pullContext_t {
  /* INPUT ----------------------------- */

  pullInitParm initParm;           /* parms for initialization, set with
                                      the pullInit*Set() functions */
  pullIterParm iterParm;           /* parms about iterations and periods, set
                                      with pullIterParmSet() */
  pullSysParm sysParm;             /* continuous parameters for system,
                                      set with pullSysParmSet() */
  pullFlag flag;                   /* all flags, set with pullFlagSet() */
  int verbose;                     /* verbosity level, set with
                                      pullVerboseSet() */
  unsigned int threadNum,          /* number of threads to use, set with
                                      pullThreadNumSet() */
    rngSeed,                       /* seed value for random number generator,
                                      NOT directly related to seed point
                                      placement (as name might suggest),
                                      set with pullRngSeedSet() */
    progressBinMod;                /* if non-zero, progress indication by
                                      printing "." is given when the bin index
                                      is a multiple of this; higher numbers
                                      give less feedback, set with
                                      pullProgressBinModSet() */
  void (*iter_cb)(void *data_cb);  /* callback to call from pullRun() every
                                      iterParm.callback iterations. This and
                                      data_cb are set w/ pullCallbackSet() */
  void *data_cb;                   /* data to pass to callback */
  pullVolume
    *vol[PULL_VOLUME_MAXNUM];      /* the volumes we analyze (we DO OWN),
                                      set by either pullVolumeSingleAdd()
                                      or pullVolumeStackAdd() */
  unsigned int volNum;             /* actual length of vol[] used */
  pullInfoSpec
    *ispec[PULL_INFO_MAX+1];       /* info ii is in effect if ispec[ii] is
                                      non-NULL (and we DO OWN ispec[ii]),
                                      set by pullInfoSpecAdd() */
  int interType;                   /* from the pullInterType* enum.  This
                                      and the energy specs below are set by
                                      pullInterEnergySet() */
  pullEnergySpec *energySpecR,     /* starting point for radial potential
                                      energy function, phi_r */
    *energySpecS,                  /* second energy potential function, for
                                      scale-space behavior, phi_s */
    *energySpecWin;                /* function used to window phi_r along s,
                                      and phi_s along r, for use with
                                      pullInterTypeAdditive */

  /* INTERNAL -------------------------- */

  unsigned int haltonOffset;       /* with pullInitMethodHalton, add this to
                                      the index to sequence generation, to
                                      account for the points previously
                                      generated (which did not meet the
                                      constraint satisfaction) */
  double bboxMin[4], bboxMax[4];   /* scale-space bounding box of all volumes:
                                      region over which binning is defined.
                                      In 3-D space, the bbox is axis aligned,
                                      even when the volume is not so aligned,
                                      which means that some bins might be
                                      under- or un- utilized, oh well.
                                      bboxMin[3] and bboxMax[3] are the
                                      bounds of the volume in *scale* (sigma),
                                      not t, or tau */
  unsigned int infoTotalLen,       /* total length of the info buffers needed,
                                      which determines size of allocated
                                      binPoint */
    infoIdx[PULL_INFO_MAX+1],      /* index of answer within pullPoint->info */
    idtagNext;                     /* next per-point igtag value */
  int haveScale,                   /* non-zero iff one of the volumes is in
                                      scale-space */
    constraint,                    /* if non-zero, we have a constraint to
                                      satisfy, and this is its info number  */
    constraintDim,                 /* dimension of *spatial* constraint
                                      manifold we're working on; or
                                      -1 if unknown/unset */
    targetDim,                     /* dimension of total constraint manifold
                                      which can be different than constraintDim
                                      because of scale-space, and either
                                      repulsive (+1) or attractive (+0)
                                      behavior along scale; or
                                      -1 if unknown/unset */
    finished;                      /* used to signal all threads to return */
  double maxDistSpace,             /* max dist of point-point interaction in
                                      the spatial axes.*/
    maxDistScale,                  /* max dist of point-point interaction
                                      along scale */
    voxelSizeSpace,                /* mean spatial voxel edge length, for
                                      limiting travel distance for descent
                                      and constraint satisfaction */
    voxelSizeScale,                /* mean voxel edge length in space, for
                                      limiting travel (along scale) distance
                                      during descent */
    eipScale;                      /* how to scale energyIncreasePermit
                                      at each iteration, in accordance with
                                      energyIncreasePermitHalfLife */
  pullBin *bin;                    /* volume of bins (see binsEdge, binNum) */
  unsigned int binsEdge[4],        /* # bins along each volume edge,
                                      determined by maxEval and scale */
    binNum,                        /* total # bins in grid */
    binNextIdx;                    /* next bin of points to be processed,
                                      we're done when binNextIdx == binNum */
  unsigned int *tmpPointPerm;      /* storing points during rebinning */
  pullPoint **tmpPointPtr;
  unsigned int tmpPointNum;

  airThreadMutex *binMutex;        /* mutex around bin, needed because bins
                                      are the unit of work for the tasks */
  pullTask **task;                 /* dynamically allocated array of tasks */
  airThreadBarrier *iterBarrierA;  /* barriers between iterations */
  airThreadBarrier *iterBarrierB;  /* barriers between iterations */
#if PULL_HINTER
  Nrrd *nhinter;                   /* 2-D histogram of (r,s)-space relative
                                      locations of interacting particles
                                      (NOT thread safe) */
#endif
  FILE *logAdd;                    /* text-file record of all the particles
                                      that have been added
                                      (NOT thread-safe) */

  /* OUTPUT ---------------------------- */

  double timeIteration,            /* time needed for last (single) iter */
    timeRun,                       /* total time spent in pullRun() */
    energy;                        /* final energy of system */
  unsigned int addNum,             /* # prtls added by PopCntl in last iter */
    nixNum,                        /* # prtls nixed by PopCntl in last iter */
    stuckNum,                      /* # stuck particles in last iter */
    pointNum,                      /* total # particles */
    CCNum,                         /* # connected components */
    iter,                          /* how many iterations were needed */
  /* HEY: this should really be per-task, to be thread-safe!! */
    count[PULL_COUNT_MAX+1];       /* all possible kinds of counts */
} pullContext;

/*
******** pullTrace
*/
typedef struct {
  double seedPos[4];    /* where was the seed point */
  /* ------- output ------- */
  Nrrd *nvert,          /* locations of tract vertices */
    *nstrn,             /* if non-NULL, 1-D array of strengths */
    *nvelo;             /* 1-D list of velocities */
  unsigned int seedIdx; /* which index in nvert is for seedpoint */
  int whyStop[2],       /* why backward/forward (0/1) tracing stopped
                           (from pullTraceStop* enum) */
    whyNowhere;         /* why trace never started (from pullTraceStop*) */
} pullTrace;

/*
******** pullTraceMulti
*/
typedef struct {
  pullTrace **trace;
  unsigned int traceNum;
  airArray *traceArr;
} pullTraceMulti;

/*
******** pullPtrPtrUnion
**
** deal with "dereferencing type-punned pointer will
** break strict-aliasing rules"
*/
typedef union {
  pullPoint ***points; /* address of array of point pointers */
  void **v;
} pullPtrPtrUnion;

/* defaultsPull.c */
PULL_EXPORT const int pullPresent;
PULL_EXPORT const int pullPhistEnabled;
PULL_EXPORT const char *pullBiffKey;

/* initPull.c */
PULL_EXPORT int pullInitRandomSet(pullContext *pctx, unsigned int numInitial);
PULL_EXPORT int pullInitHaltonSet(pullContext *pctx, unsigned int numInitial,
                                  unsigned int start);
PULL_EXPORT int pullInitPointPerVoxelSet(pullContext *pctx, int pointPerVoxel,
                                         unsigned int zSlcMin,
                                         unsigned int zSlcMax,
                                         unsigned int alongScaleNum,
                                         double jitter);
PULL_EXPORT int pullInitGivenPosSet(pullContext *pctx, const Nrrd *npos);
PULL_EXPORT int pullInitLiveThreshUseSet(pullContext *pctx, int liveThreshUse);
PULL_EXPORT int pullInitUnequalShapesAllowSet(pullContext *pctx, int allow);

/* parmPull.c */
PULL_EXPORT int pullIterParmSet(pullContext *pctx, int which,
                                unsigned int pval);
PULL_EXPORT int pullSysParmSet(pullContext *pctx, int which,
                               double pval);
PULL_EXPORT int pullFlagSet(pullContext *pctx, int which, int flag);
PULL_EXPORT int pullVerboseSet(pullContext *pctx, int verbose);
PULL_EXPORT int pullThreadNumSet(pullContext *pctx, unsigned int threadNum);
PULL_EXPORT int pullRngSeedSet(pullContext *pctx, unsigned int rngSeed);
PULL_EXPORT int pullProgressBinModSet(pullContext *pctx, unsigned int bmod);
PULL_EXPORT int pullCallbackSet(pullContext *pctx,
                                void (*iter_cb)(void *data_cb),
                                void *data_cb);
PULL_EXPORT int pullInterEnergySet(pullContext *pctx, int interType,
                                   const pullEnergySpec *enspR,
                                   const pullEnergySpec *enspS,
                                   const pullEnergySpec *enspWin);
PULL_EXPORT int pullLogAddSet(pullContext *pctx, FILE *log);

/* energy.c */
PULL_EXPORT const airEnum *const pullInterType;
PULL_EXPORT const airEnum *const pullEnergyType;
PULL_EXPORT const pullEnergy *const pullEnergyUnknown;
PULL_EXPORT const pullEnergy *const pullEnergySpring;
PULL_EXPORT const pullEnergy *const pullEnergyGauss;
PULL_EXPORT const pullEnergy *const pullEnergyBspln;
PULL_EXPORT const pullEnergy *const pullEnergyButterworth;
PULL_EXPORT const pullEnergy *const pullEnergyCotan;
PULL_EXPORT const pullEnergy *const pullEnergyCubic;
PULL_EXPORT const pullEnergy *const pullEnergyQuartic;
PULL_EXPORT const pullEnergy *const pullEnergyCubicWell;
PULL_EXPORT const pullEnergy *const pullEnergyBetterCubicWell;
PULL_EXPORT const pullEnergy *const pullEnergyQuarticWell;
PULL_EXPORT const pullEnergy *const pullEnergyHepticWell;
PULL_EXPORT const pullEnergy *const pullEnergyZero;
PULL_EXPORT const pullEnergy *const pullEnergyButterworthParabola;
PULL_EXPORT const pullEnergy *const pullEnergyAll[PULL_ENERGY_TYPE_MAX+1];
PULL_EXPORT pullEnergySpec *pullEnergySpecNew(void);
PULL_EXPORT void pullEnergySpecSet(pullEnergySpec *ensp,
                                   const pullEnergy *energy,
                                   const double parm[PULL_ENERGY_PARM_NUM]);
PULL_EXPORT void pullEnergySpecCopy(pullEnergySpec *esDst,
                                    const pullEnergySpec *esSrc);
PULL_EXPORT pullEnergySpec *pullEnergySpecNix(pullEnergySpec *ensp);
PULL_EXPORT int pullEnergySpecParse(pullEnergySpec *ensp, const char *str);
PULL_EXPORT hestCB *pullHestEnergySpec;

/* volumePull.c */
PULL_EXPORT pullVolume *pullVolumeNew(void);
PULL_EXPORT pullVolume *pullVolumeNix(pullVolume *vol);
PULL_EXPORT int pullVolumeSingleAdd(pullContext *pctx,
                                    const gageKind *kind,
                                    char *name, const Nrrd *nin,
                                    const NrrdKernelSpec *ksp00,
                                    const NrrdKernelSpec *ksp11,
                                    const NrrdKernelSpec *ksp22);
PULL_EXPORT int pullVolumeStackAdd(pullContext *pctx,
                                   const gageKind *kind,
                                   char *name,
                                   const Nrrd *nin,
                                   const Nrrd *const *ninSS,
                                   double *scalePos,
                                   unsigned int ninNum,
                                   int scaleDerivNorm,
                                   double scaleDerivNormBias,
                                   const NrrdKernelSpec *ksp00,
                                   const NrrdKernelSpec *ksp11,
                                   const NrrdKernelSpec *ksp22,
                                   const NrrdKernelSpec *kspSS);
PULL_EXPORT const pullVolume *pullVolumeLookup(const pullContext *pctx,
                                               const char *volName);
PULL_EXPORT int pullConstraintScaleRange(pullContext *pctx,
                                         double ssrange[2]);

/* enumsPull.c */
PULL_EXPORT const airEnum *const pullInfo;
PULL_EXPORT const airEnum *const pullSource;
PULL_EXPORT const airEnum *const pullProp;
PULL_EXPORT const airEnum *const pullProcessMode;
PULL_EXPORT const airEnum *const pullTraceStop;
PULL_EXPORT const airEnum *const pullCount;
PULL_EXPORT const airEnum *const pullConstraintFail;

/* infoPull.c */
PULL_EXPORT unsigned int pullPropLen(int prop);
PULL_EXPORT unsigned int pullInfoLen(int info);
PULL_EXPORT pullInfoSpec *pullInfoSpecNew(void);
PULL_EXPORT pullInfoSpec *pullInfoSpecNix(pullInfoSpec *ispec);
PULL_EXPORT int pullInfoSpecAdd(pullContext *pctx, pullInfoSpec *ispec);
PULL_EXPORT int pullInfoGet(Nrrd *ninfo, int info, pullContext *pctx);
PULL_EXPORT int pullInfoSpecSprint(char str[AIR_STRLEN_LARGE],
                                   const pullContext *pctx,
                                   const pullInfoSpec *ispec);

/* contextPull.c */
PULL_EXPORT pullContext *pullContextNew(void);
PULL_EXPORT pullContext *pullContextNix(pullContext *pctx);
PULL_EXPORT int pullOutputGet(Nrrd *nPosOut, Nrrd *nTenOut,
                              Nrrd *nStrengthOut,
                              const double scaleVec[3], double scaleRad,
                              pullContext *pctx);
PULL_EXPORT int pullOutputGetFilter(Nrrd *nPosOut, Nrrd *nTenOut,
                                    Nrrd *nStrengthOut,
                                    const double scaleVec[3], double scaleRad,
                                    pullContext *pctx,
                                    unsigned int idtagMin,
                                    unsigned int idtagMax);
PULL_EXPORT int pullPositionHistoryGet(limnPolyData *pld, pullContext *pctx);
PULL_EXPORT int pullPropGet(Nrrd *nprop, int prop, pullContext *pctx);

/* pointPull.c */
PULL_EXPORT int pullPointInitializePerVoxel(const pullContext *pctx,
                                            const unsigned int pointIdx,
                                            pullPoint *point,
                                            pullVolume *scaleVol,
                                            int *createFailP);
PULL_EXPORT int pullPointInitializeRandomOrHalton(pullContext *pctx,
                                                  const unsigned int pointIdx,
                                                  pullPoint *point,
                                                  pullVolume *scaleVol);
PULL_EXPORT int pullPointInitializeGivenPos(pullContext *pctx,
                                            const double *posData,
                                            const unsigned int pointIdx,
                                            pullPoint *point,
                                            int *createFailP);
PULL_EXPORT double pullPointScalar(const pullContext *pctx,
                                   const pullPoint *point, int sclInfo,
                                   double grad[4], double hess[9]);
PULL_EXPORT unsigned int pullPointNumber(const pullContext *pctx);
PULL_EXPORT unsigned int pullPointNumberFilter(const pullContext *pctx,
                                               unsigned int idtagMin,
                                               unsigned int idtagMax);
PULL_EXPORT pullPoint *pullPointNew(pullContext *pctx);
PULL_EXPORT pullPoint *pullPointNix(pullPoint *pnt);
PULL_EXPORT int pullProbe(pullTask *task, pullPoint *point);

/* binningPull.c */
PULL_EXPORT int pullBinsPointAdd(pullContext *pctx, pullPoint *point,
                                 /* output */
                                 pullBin **binUsed);
PULL_EXPORT int pullBinsPointMaybeAdd(pullContext *pctx, pullPoint *point,
                                      /* output */
                                      pullBin **binUsed, int *added);

/* trace.c */
PULL_EXPORT pullTrace *pullTraceNew(void);
PULL_EXPORT pullTrace *pullTraceNix(pullTrace *pts);
PULL_EXPORT size_t pullTraceMultiSizeof(const pullTraceMulti *mtrc);
PULL_EXPORT int pullTraceSet(pullContext *pctx, pullTrace *trc,
                             int recordStrength,
                             double scaleDelta, double halfScaleWin,
                             double velocityMax, unsigned int arrIncr,
                             const double seedPos[4]);
PULL_EXPORT pullTraceMulti *pullTraceMultiNew(void);
PULL_EXPORT pullTraceMulti *pullTraceMultiNix(pullTraceMulti *mtrc);
PULL_EXPORT int pullTraceMultiAdd(pullTraceMulti *mtrc, pullTrace *trc,
                                  int *addedP);
PULL_EXPORT int pullTraceMultiFilterConcaveDown(Nrrd *nfilt,
                                                const pullTraceMulti *mtrc,
                                                double winLenFrac);
PULL_EXPORT int pullTraceMultiPlotAdd(Nrrd *nplot,
                                      const pullTraceMulti *mtrc,
                                      const Nrrd *nfilt,
                                      unsigned int trcIdxMin,
                                      unsigned int trcNum);
PULL_EXPORT int pullTraceMultiWrite(FILE *file, const pullTraceMulti *mtrc);
PULL_EXPORT int pullTraceMultiRead(pullTraceMulti *mtrc, FILE *file);

/* actionPull.c */
PULL_EXPORT int pullEnergyPlot(pullContext *pctx, Nrrd *nplot,
                               double xx, double yy, double zz,
                               unsigned int res);
PULL_EXPORT int pullBinProcess(pullTask *task, unsigned int myBinIdx);
PULL_EXPORT int pullGammaLearn(pullContext *pctx);

/* corePull.c */
PULL_EXPORT int pullStart(pullContext *pctx);
PULL_EXPORT int pullRun(pullContext *pctx);
PULL_EXPORT int pullFinish(pullContext *pctx);

/* ccPull.c */
PULL_EXPORT int pullCCFind(pullContext *pctx);
PULL_EXPORT int pullCCMeasure(pullContext *pctx, Nrrd *nmeas,
                              int measrInfo, double rho);
PULL_EXPORT int pullCCSort(pullContext *pctx, int measrInfo, double rho);

#ifdef __cplusplus
}
#endif

#endif /* PULL_HAS_BEEN_INCLUDED */
