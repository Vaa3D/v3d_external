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

enum {
  flagUnknown,
  flagData,
  flagType,
  flagSamples,
  flagLowerInside,
  flagNormalsFind,
  flagStrength,
  flagStrengthUse,
  flagItemValue,
  flagItemStrength,
  flagItemNormal,
  flagItemGradient,
  flagItemEigensystem,
  flagItemHess,
  flagIsovalue,
  flagEvalDiffThresh,

  flagNinEtAl,
  flagAnswerPointers,
  flagSxSySz,
  flagReverse,
  flagTxfNormal,
  flagSlabCacheAlloc,
  flagSclDerived,
  flagSpanSpaceHist,

  flagResult,
  flagLast
};

typedef struct {
  int evti[12];  /* edge vertex index */
  double (*scllup)(const void *, size_t);
  unsigned int esIdx,  /* eigensystem index */
    zi;     /* slice index we're currently on */
  int modeSign;
  const void *scldata;
  airArray *xyzwArr, *normArr, *indxArr;
} baggage;

/* extract.c: This one is also needed in textract.c: */
extern void
_seekIdxProbe(seekContext *sctx, baggage *bag,
              double xi, double yi, double zi);

/* textract.c: Some routines that are also used in descend.c */
extern void
_seekHess2T(double *T, const double *evals, const double *evecs,
            const double evalDiffThresh, const char ridge);
extern void
_seekHessder2Tder(double *Tder, const double *hessder, const double *evals,
                  const double *evecs, const double evalDiffThresh,
                  const char ridge);

extern int
_seekShuffleProbeT(seekContext *sctx, baggage *bag);

extern int
_seekTriangulateT(seekContext *sctx, baggage *bag, limnPolyData *lpld);
