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

#ifndef MEET_HAS_BEEN_INCLUDED
#define MEET_HAS_BEEN_INCLUDED

#include <stdio.h>
#include <string.h>

/* TEEM_LIB_LIST */
#include <teem/air.h>
#include <teem/hest.h>
#include <teem/biff.h>
#include <teem/nrrd.h>
#include <teem/ell.h>
#include <teem/unrrdu.h>
#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
#  include <teem/alan.h>
#endif
#include <teem/moss.h>
#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
#  include <teem/tijk.h>
#endif
#include <teem/gage.h>
#include <teem/dye.h>
#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
#  include <teem/bane.h>
#endif
#include <teem/limn.h>
#include <teem/echo.h>
#include <teem/hoover.h>
#include <teem/seek.h>
#include <teem/ten.h>
#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
#  include <teem/elf.h>
#endif
#include <teem/pull.h>
#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
#  include <teem/coil.h>
#  include <teem/push.h>
#endif
#include <teem/mite.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(meet_EXPORTS) || defined(teem_EXPORTS)
#    define MEET_EXPORT extern __declspec(dllexport)
#  else
#    define MEET_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define MEET_EXPORT extern
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define MEET meetBiffKey

/* enumall.c: (not really a descriptive filename) */
MEET_EXPORT const int meetPresent;
MEET_EXPORT const char *meetBiffKey;
MEET_EXPORT const airEnum **meetAirEnumAll(void);
MEET_EXPORT void meetAirEnumAllPrint(FILE *file);
MEET_EXPORT int meetAirEnumAllCheck(void);
MEET_EXPORT const char *const meetTeemLibs[];

/* meetNrrd.c */
MEET_EXPORT const NrrdKernel **meetNrrdKernelAll(void);
MEET_EXPORT int meetNrrdKernelAllCheck(void);

/* meetGage.c */
MEET_EXPORT gageKind *meetGageKindParse(const char *str);
MEET_EXPORT const gageKind *meetConstGageKindParse(const char *str);
MEET_EXPORT hestCB *meetHestGageKind;
MEET_EXPORT hestCB *meetHestConstGageKind;

/*
******** meetPullVol
**
** this is information that somehow precedes the full pullVolume; it
** is more likely to have been by the user, and is not everything that
** a pullVolume is ("Vol" vs "Volume").  Does provide a way of parsing
** a string to specify all this info (useful for command-line).  Note
** that the nrrds inside are owned by this, in contrast to the
** pullVolume.
**
** One constraint that motivates putting this in meet (instead of
** pull) is that this has to be general WRT gageKind; and will
** ultimately rely on meetConstGageKindParse.  So, either
** meetConstGageKindParse gets passed to pull as a callback, or this
** functionality moves up to meet.  Also, given the weird leeching
** tricks that are being enabled here, perhaps it is better for this
** to be properly outside of pull.
*/
typedef struct {
  const gageKind *kind;
  char *fileName,
    *volName;
  int derivNormSS,             /* normalize derivatives based on scale */
    uniformSS,                 /* uniform sampling along scale */
    optimSS,                   /* optimal (non-uniform) sampling of scale */
    leeching,                  /* non-zero iff using the same nin and ninSS
                                  as another meetPullVol (so as to avoid
                                  redundant copies in memory) */
    recomputedSS;              /* (OUTPUT) non-zero if meetPullVolLoadMulti
                                  had to recompute these, versus being read
                                  from disk */
  unsigned int numSS;
  double rangeSS[2], derivNormBiasSS, *posSS;
  Nrrd *nin;                             /* we DO own */
  Nrrd **ninSS;                          /* we DO own */
} meetPullVol;

/*
******** meetPullInfo
**
** information that helps define a pullInfoSpec.
**
** The original reason that the pullInfo struct itself isn't being
** used here is that we want a way of parsing and storing this
** information from some source (e.g. command-line via hest), at a
** point where we do not yet know, or not have access to all the
** volumes and their gageKinds.  So, the gageItem information is
** stored here only as a string, rather than the real integral item
** number.
*/
typedef struct {
  int info,                    /* which pullInfo is being defined */
    source,                    /* the source (from pullSource* enum) */
    prop,                      /* which property (if pullSourceProp) */
    constraint;                /* this info should be a constraint */
  char *volName,               /* name of volume from which info is measured */
    *itemStr;                  /* which item in that volume gives the info */
  double zero, scale;          /* affine mapping of scalar info */
} meetPullInfo;

/* meetPull.c */
MEET_EXPORT meetPullVol *meetPullVolNew(void);
MEET_EXPORT int meetPullVolParse(meetPullVol *mpv, const char *str);
MEET_EXPORT int meetPullVolLeechable(const meetPullVol *orig,
                                     const meetPullVol *lchr);
MEET_EXPORT meetPullVol *meetPullVolNix(meetPullVol *pvol);
MEET_EXPORT hestCB *meetHestPullVol;
MEET_EXPORT int meetPullVolLoadMulti(meetPullVol **mpv, unsigned int mpvNum,
                                     char *cachePath, NrrdKernelSpec *kSSblur,
                                     int boundary, double padValue,
                                     int verbose);
MEET_EXPORT int meetPullVolAddMulti(pullContext *pctx,
                                    meetPullVol **mpv, unsigned int mpvNum,
                                    const NrrdKernelSpec *k00,
                                    const NrrdKernelSpec *k11,
                                    const NrrdKernelSpec *k22,
                                    const NrrdKernelSpec *kSSrecon);
MEET_EXPORT meetPullInfo *meetPullInfoNew(void);
MEET_EXPORT meetPullInfo *meetPullInfoNix(meetPullInfo *minf);
MEET_EXPORT int meetPullInfoParse(meetPullInfo *minf, const char *str);
MEET_EXPORT hestCB *meetHestPullInfo;
MEET_EXPORT int meetPullInfoAddMulti(pullContext *pctx,
                                     meetPullInfo **minf,
                                     unsigned int minfNum);


#ifdef __cplusplus
}
#endif

#endif /* MEET_HAS_BEEN_INCLUDED */
