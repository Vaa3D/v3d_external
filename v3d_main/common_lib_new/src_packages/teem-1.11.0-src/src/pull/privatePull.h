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

#ifdef __cplusplus
extern "C" {
#endif

/*
** These heuristics/parameters have been adjusted, and don't seem to need
** any further tweaking in order to work for all the datasets used so far
*/

/* size/allocation increment for per-bin airArray */
#define _PULL_BIN_INCR 32

/* size/allocation increment for pullTrace airArray in pullTraceMulti */
#define _PULL_TRACE_MULTI_INCR 1024

/* this has to be big enough to do experiments where binning is turned off */
#define _PULL_NEIGH_MAXNUM 4096

/* used by pullBinsPointMaybeAdd; don't add a point if its (normalized)
   distance to an existing point is less than this */
#define _PULL_BINNING_MAYBE_ADD_THRESH 0.1

/* only try adding a point if the normalized neighbor offset sum is
   greater than this (making this too small only wastes time, by descending
   and testing a point that can't help reduce energy */
#define _PULL_NEIGH_OFFSET_SUM_THRESH 0.2

/* how far to place new points from isolated points (as a fraction of
   radiusSpace), when not using an energy with an explicit well */
#define _PULL_NEWPNT_DIST 0.6

/* scaling factor between point->neighDistMean and distance cap; higher
   values allow for more adventurous explorations . . . */
#define _PULL_DIST_CAP_RSNORM 2.0

/* travel distance limit in terms of voxelSizeSpace and voxelSizeScale */
#define _PULL_DIST_CAP_VOXEL 1.5

/* where along s axis to probe energySpecS to see if its attractive or
   repulsive along scale */
#define _PULL_TARGET_DIM_S_PROBE 0.05

/* tentative new points aren't allowed to move further than this (in
   rs-normalized space) from the original old point */
#define _PULL_NEWPNT_STRAY_DIST 1.3

/* fraction of bboxMax[3]-bboxMin[3] to use as step along scale
   for discrete differencing needed to find the gradient (for
   moving along scale) and 2nd derivative (for learning gamma)
   of strength
*/
#define _PULL_STRENGTH_ENERGY_DELTA_SCALE 0.0002

/* maximum distance, in multiples of voxelSizeSpace, that a point is
   allowed to move between start and end of constraint satisfaction */
#define _PULL_CONSTRAINT_TRAVEL_MAX 2

/* max reasonable value of iterParm->constraintIterMax */
#define _PULL_CONSTRAINT_ITER_MAX 50

/* the main _iterate() function does progress indication if the number of
   points is larger than this */
#define _PULL_PROGRESS_POINT_NUM_MIN 100

/* limit on # times we allow random or halton (non-ppv) seeding to fail */
#define _PULL_RANDOM_SEED_TRY_MAX 1000000

/* limit on stepEnergy */
#define _PULL_STEP_ENERGY_MAX FLT_MAX

/* resolution of histogram of (r,s) coords of interactions ("hinter") */
#define _PULL_HINTER_SIZE 601

/* initPull.c */
extern void _pullInitParmInit(pullInitParm *initParm);
extern int _pullInitParmCheck(pullInitParm *iparm);

/* parmPull.c */
extern void _pullIterParmInit(pullIterParm *iterParm);
extern int _pullIterParmCheck(pullIterParm *iterParm);
extern void _pullSysParmInit(pullSysParm *sysParm);
extern int _pullSysParmCheck(pullSysParm *sysParm);
extern void _pullFlagInit(pullFlag *flag);

/* volumePull.c */
extern pullVolume *_pullVolumeCopy(const pullVolume *pvol);
extern int _pullVolumeSetup(pullContext *pctx);
extern int _pullInsideBBox(pullContext *pctx, double pos[4]);
extern unsigned int _pullVolumeIndex(const pullContext *pctx,
                                     const char *volName);

/* infoPull.c */
extern unsigned int _pullInfoLen[PULL_INFO_MAX+1];
extern void (*_pullInfoCopy[10])(double *, const double *);
extern int _pullInfoSetup(pullContext *pctx);

/* contextPull.c */
extern int _pullContextCheck(pullContext *pctx);

/* taskPull.c */
extern pullTask *_pullTaskNew(pullContext *pctx, int threadIdx);
extern pullTask *_pullTaskNix(pullTask *task);
extern int _pullTaskSetup(pullContext *pctx);
extern void _pullTaskFinish(pullContext *pctx);

/* actionPull.c */
extern double _pullPrayCorner[2][2][3];
extern size_t _pullPrayRes[2];
extern double _pullDistLimit(pullTask *task, pullPoint *point);
extern double _pullEnergyFromPoints(pullTask *task, pullBin *bin,
                                    pullPoint *point,
                                    /* output */
                                    double egradSum[4]);
extern double _pullPointEnergyTotal(pullTask *task, pullBin *bin,
                                    pullPoint *point, int ignoreImage,
                                    double force[4]);
extern int _pullPointProcessDescent(pullTask *task, pullBin *bin,
                                    pullPoint *point, int ignoreImage);
extern double _pullEnergyInterParticle(pullContext *pctx,
                                       pullPoint *me, const pullPoint *she,
                                       double spaceDist, double scaleDist,
                                       double egrad[4]);

/* constraints.c */
extern int _pullConstraintSatisfy(pullTask *task, pullPoint *point,
                                  double travelMax,
                                  int *constrFailP);
extern void _pullConstraintTangent(pullTask *task, pullPoint *point,
                                   double proj[9]);
extern int _pullConstraintDim(const pullContext *pctx);

/* pointPull.c */
#if PULL_PHIST
extern void _pullPointHistInit(pullPoint *point);
extern void _pullPointHistAdd(pullPoint *point, int cond);
#else
#define _pullPointHistInit(p)    /* no-op */
#define _pullPointHistAdd(p, c)  /* no-op */
#endif
extern double _pullStepInterAverage(const pullContext *pctx);
extern double _pullStepConstrAverage(const pullContext *pctx);
extern double _pullEnergyTotal(const pullContext *pctx);
extern void _pullPointStepEnergyScale(pullContext *pctx, double scale);
extern int _pullPointSetup(pullContext *pctx);
extern void _pullPointFinish(pullContext *pctx);

/* popcntl.c */
extern int _pullPointProcessNeighLearn(pullTask *task, pullBin *bin,
                                       pullPoint *point);
extern int _pullPointProcessAdding(pullTask *task, pullBin *bin,
                                   pullPoint *point);
extern int _pullPointProcessNixing(pullTask *task, pullBin *bin,
                                   pullPoint *point);
extern int _pullIterFinishNeighLearn(pullContext *pctx);
extern int _pullIterFinishAdding(pullContext *pctx);
extern int _pullIterFinishNixing(pullContext *pctx);
extern void _pullNixTheNixed(pullContext *pctx);

/* binningPull.c */
extern void _pullBinInit(pullBin *bin);
extern void _pullBinDone(pullBin *bin);
extern pullBin *_pullBinLocate(pullContext *pctx, double *pos);
extern void _pullBinPointRemove(pullContext *pctx, pullBin *bin, int loseIdx);
extern int _pullBinSetup(pullContext *pctx);
extern int _pullIterFinishDescent(pullContext *pctx);
extern void _pullBinFinish(pullContext *pctx);

/* corePull.c */
extern int _pullVerbose;
extern int _pullProcess(pullTask *task);
extern void *_pullWorker(void *_task);
extern int _pullIterate(pullContext *pctx, int mode);

#ifdef __cplusplus
}
#endif
