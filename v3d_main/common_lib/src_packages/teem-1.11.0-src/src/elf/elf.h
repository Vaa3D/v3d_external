/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2011, 2010, 2009 Thomas Schultz

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

#ifndef ELF_HAS_BEEN_INCLUDED
#define ELF_HAS_BEEN_INCLUDED

#include <teem/air.h>
#include <teem/nrrd.h>
#include <teem/ell.h>
#include <teem/limn.h>
#include <teem/tijk.h>
#include <teem/ten.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(tijk_EXPORTS) || defined(teem_EXPORTS)
#    define ELF_EXPORT extern __declspec(dllexport)
#  else
#    define ELF_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define ELF_EXPORT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* glyphElf.c */
ELF_EXPORT const int elfPresent;
ELF_EXPORT float elfGlyphHOME(limnPolyData *glyph, const char antipodal,
                              const float *ten, const tijk_type *type,
                              char *isdef, const char normalize);

ELF_EXPORT float elfGlyphPolar(limnPolyData *glyph, const char antipodal,
                               const float *ten, const tijk_type *type,
                               char *isdef, const char clamp,
                               const char normalize,
                               const unsigned char *posColor,
                               const unsigned char *negColor);

ELF_EXPORT int elfColorGlyphMaxima(limnPolyData *glyph, const char antipodal,
                                   const int *neighbors, unsigned int nbstride,
                                   const float *ten, const tijk_type *type,
                                   const char modulate, const float gamma);

/*
********** elfMaximaContext
**
** Allows us to precompute and store information needed to find all maxima
** of a given symmetric 3D tensor type. Should only be used through elfMaxima*
*/
typedef struct {
  unsigned int num;
  const tijk_type *type;
  tijk_refine_rank1_parm *parm;
  int refine;
  int *neighbors;
  unsigned int nbstride;
  float *vertices_f; /* we're only storing the non-redundant ones */
  double *vertices_d; /* only filled when needed */
} elfMaximaContext;

/* maximaElf.c */
ELF_EXPORT elfMaximaContext *elfMaximaContextNew(const tijk_type *type,
                                                 unsigned int level);
ELF_EXPORT elfMaximaContext *elfMaximaContextNix(elfMaximaContext *emc);
ELF_EXPORT void elfMaximaParmSet(elfMaximaContext *emc,
                                 tijk_refine_rank1_parm *parm);
ELF_EXPORT void elfMaximaRefineSet(elfMaximaContext *emc, int refine);
ELF_EXPORT int elfMaximaFind_d(double **ls, double **vs, const double *ten,
                               elfMaximaContext *emc);
ELF_EXPORT int elfMaximaFind_f(float **ls, float **vs, const float *ten,
                               elfMaximaContext *emc);

/* ESHEstimElf.c */
ELF_EXPORT void elfCart2Thetaphi_f(float *thetaphi, const float *dirs,
                                   unsigned int ct);
ELF_EXPORT int elfESHEstimMatrix_f(float *T, float *H, unsigned int order,
                                   const float *thetaphi,
                                   unsigned int ct, float lambda, float *w);

/* ballStickElf.c */

/* elfSingleShellDWI:
 *
 * Collects the parameters and measurements of a single-shell DWI experiment
 */
typedef struct {
  float b0;           /* unweighted measurement */
  float b;            /* b value */
  float *dwis;        /* diffusion-weighted measurements */
  float *grads;       /* normalized gradient vectors (Cartesian 3D) */
  unsigned int dwino; /* number of dwis */
} elfSingleShellDWI;

ELF_EXPORT int elfKernelStick_f(float *kernel, unsigned int order, float bd,
                                float b0, int delta);
ELF_EXPORT int elfBallStickODF_f(float *odf, float *fiso, float *d,
                                 const elfSingleShellDWI *dwi,
                                 const float *T, unsigned int order, int delta);

/* elfBallStickParms:
 *
 * Collects the parameters associated with a ball-and-multi-stick model
 * (up to three sticks), as well as debugging information
 */
typedef struct {
  float d; /* ADC */
  unsigned int fiberct;
  float fs[4]; /* fs[0]==fiso */
  float vs[9];
  /* remaining fields are for statistics/debugging */
  int stopreason;
  double sqrerr;
  double itr;
} elfBallStickParms;

ELF_EXPORT int elfBallStickPredict_f(elfBallStickParms *parms, float *odf,
                                     const tijk_type *type, unsigned int k,
                                     float d, float fiso);
ELF_EXPORT int elfBallStickOptimize_f(elfBallStickParms *parms,
                                      const elfSingleShellDWI *dwi);

#ifdef __cplusplus
}
#endif

#endif /* ELF_HAS_BEEN_INCLUDED */
