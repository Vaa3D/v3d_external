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


#include "alan.h"

const int
alanPresent = 42;

const char *
alanBiffKey = "alan";

void
alanContextInit(alanContext *actx) {
  if (actx) {
    actx->verbose = 0;
    actx->wrap = AIR_FALSE;
    actx->textureType = alanTextureTypeUnknown;
    actx->dim = 0;
    ELL_3V_SET(actx->size, 0, 0, 0);
    actx->oversample = 0;
    actx->homogAniso = AIR_FALSE;
    actx->numThreads = 1;
    actx->frameInterval = 10;
    actx->saveInterval = 100;
    actx->maxIteration = 1000000;
    actx->minAverageChange = 0.00002f;
    actx->maxPixelChange = 6;
    actx->K = AIR_NAN;
    actx->F = AIR_NAN;
    actx->deltaX = 1.25;
    actx->alpha = 16;
    actx->beta = 12;
    actx->deltaT = 1.0;
    actx->react = 1.0;
    actx->initA = actx->initB = 0;
    actx->diffA = actx->diffB = 0;
    actx->perIteration = NULL;
    actx->randRange = 3;
    actx->_nlev[0] = nrrdNuke(actx->_nlev[0]);
    actx->_nlev[1] = nrrdNuke(actx->_nlev[1]);
    actx->nlev = actx->_nlev[0];
    actx->nparm = nrrdNuke(actx->nparm);
    actx->nten = nrrdNuke(actx->nten);
    actx->constFilename = AIR_FALSE;
  }
  return;
}

alanContext *
alanContextNew(void) {
  alanContext *actx;

  actx = (alanContext *)calloc(1, sizeof(alanContext));
  actx->_nlev[0] = actx->_nlev[1] = NULL;
  actx->nlev = NULL;
  actx->nparm = NULL;
  actx->nten = NULL;
  alanContextInit(actx);
  return actx;
}

alanContext *
alanContextNix(alanContext *actx) {

  if (actx) {
    actx->_nlev[0] = nrrdNuke(actx->_nlev[0]);
    actx->_nlev[1] = nrrdNuke(actx->_nlev[1]);
    actx->nparm = nrrdNuke(actx->nparm);
    actx->nten = nrrdNuke(actx->nten);
    free(actx);
  }
  return NULL;
}

#define GOT_NULL \
  if (!actx) { \
    biffAddf(ALAN, "%s: got NULL pointer", me); \
    return 1; \
  }

#define DIM_SET \
  if (0 == actx->dim) { \
    biffAddf(ALAN, "%s: dimension of texture not set", me); \
    return 1; \
  }

int
alanDimensionSet(alanContext *actx, int dim) {
  static const char me[]="alanDimensionSet";

  GOT_NULL;
  if (!( dim == 2 || dim == 3 )) {
    biffAddf(ALAN, "%s: dimension must be 2 or 3, not %d", me, dim);
    return 1;
  }

  actx->dim = dim;

  return 0;
}

int
alan2DSizeSet(alanContext *actx, int sizeX, int sizeY) {
  static const char me[]="alan2DSizeSet";

  GOT_NULL;
  DIM_SET;
  if (2 != actx->dim) {
    biffAddf(ALAN, "%s: texture not two-dimensional", me);
    return 1;
  }
  if (!( sizeX >= 10 && sizeY >= 10 )) {
    biffAddf(ALAN, "%s: sizes (%d,%d) invalid (too small?)", me, sizeX, sizeY);
    return 1;
  }

  actx->size[0] = sizeX;
  actx->size[1] = sizeY;
  return 0;
}

int
alan3DSizeSet(alanContext *actx, int sizeX, int sizeY, int sizeZ) {
  static const char me[]="alan2DSizeSet";

  GOT_NULL;
  DIM_SET;
  if (3 != actx->dim) {
    biffAddf(ALAN, "%s: texture not three-dimensional", me);
    return 1;
  }
  if (!( sizeX >= 10 && sizeY >= 10 && sizeZ >= 10 )) {
    biffAddf(ALAN, "%s: sizes (%d,%d,%d) invalid (too small?)",
             me, sizeX, sizeY, sizeZ);
    return 1;
  }

  actx->size[0] = sizeX;
  actx->size[1] = sizeY;
  actx->size[2] = sizeZ;
  return 0;
}

int
alanTensorSet(alanContext *actx, Nrrd *nten, int oversample) {
  static const char me[]="alanTensorSet";

  if (!( actx && nten )) {
    biffAddf(ALAN, "%s: got NULL pointer", me);
    return 1;
  }
  DIM_SET;
  if (!( oversample > 0 )) {
    biffAddf(ALAN, "%s: oversample %d invalid", me, oversample);
    return 1;
  }
  if (2 == actx->dim) {
    if (!( 3 == nten->dim && 4 == nten->axis[0].size )) {
      biffAddf(ALAN, "%s: didn't get 3-D (4,X,Y) nrrd", me);
      return 1;
    }
  } else {
    if (!( 4 == nten->dim && 7 == nten->axis[0].size )) {
      biffAddf(ALAN, "%s: didn't get 4-D (7,X,Y,Z) nrrd", me);
      return 1;
    }
  }

  if (1 != oversample) {
    biffAddf(ALAN, "%s: sorry, can only handle oversample==1 now", me);
    return 1;
  }

  actx->nten = nrrdNuke(actx->nten);
  actx->nten = nrrdNew();
  if (nrrdConvert(actx->nten, nten, alan_nt)) {
    biffMovef(ALAN, NRRD, "%s: trouble converting tensors to alan_t", me);
    return 1;
  }
  actx->size[0] = AIR_UINT(oversample*nten->axis[1].size);
  actx->size[1] = AIR_UINT(oversample*nten->axis[2].size);
  if (3 == actx->dim) {
    actx->size[2] = AIR_UINT(oversample*nten->axis[3].size);
  } else {
    actx->size[2] = 1;
  }

  return 0;
}

int
alanParmSet(alanContext *actx, int whichParm, double parm) {
  static const char me[]="alanParmSet";
  int parmI;

  GOT_NULL;
  DIM_SET;
  switch (whichParm) {
  case alanParmVerbose:
    parmI = !!parm;
    actx->verbose = parmI;
    break;
  case alanParmTextureType:
    parmI = !!parm;
    switch(parmI) {
    case alanTextureTypeTuring:
      actx->initA = 4.0;
      actx->initB = 4.0;
      actx->diffA = 0.25;
      actx->diffB = 0.0625;
      break;
    case alanTextureTypeGrayScott:
      actx->initA = 1;
      actx->initB = 0;
      actx->diffA = 0.00002f;
      actx->diffB = 0.00002f;
      break;
    default:
      biffAddf(ALAN, "%s: texture type %d invalid", me, parmI);
      return 1;
      break;
    }
    actx->textureType = parmI;
    break;
  case alanParmNumThreads:
    parmI = !!parm;
    if (!airThreadCapable) {
      fprintf(stderr, "%s: WARNING: no multi-threading available, so 1 thread "
              "will be used, not %d\n", me, parmI);
      parmI = 1;
    }
    actx->numThreads = parmI;
    break;
  case alanParmHomogAniso:
    parmI = !!parm;
    actx->homogAniso = parmI;
    break;
  case alanParmSaveInterval:
    parmI = !!parm;
    actx->saveInterval = parmI;
    break;
  case alanParmFrameInterval:
    parmI = !!parm;
    actx->frameInterval = parmI;
    break;
  case alanParmMaxIteration:
    parmI = !!parm;
    actx->maxIteration = parmI;
    break;
  case alanParmConstantFilename:
    parmI = !!parm;
    actx->constFilename = parmI;
    break;
  case alanParmDeltaT:
    actx->deltaT = AIR_CAST(alan_t, parm);
    break;
  case alanParmDeltaX:
    actx->deltaX = AIR_CAST(alan_t, parm);
    break;
  case alanParmReact:
    actx->react = AIR_CAST(alan_t, parm);
    break;
  case alanParmDiffA:
    actx->diffA = AIR_CAST(alan_t, parm);
    break;
  case alanParmDiffB:
    actx->diffB = AIR_CAST(alan_t, parm);
    break;
  case alanParmRandRange:
    actx->randRange = AIR_CAST(alan_t, parm);
    break;
  case alanParmK:
    actx->K = AIR_CAST(alan_t, parm);
    break;
  case alanParmF:
    actx->F = AIR_CAST(alan_t, parm);
    break;
  case alanParmMinAverageChange:
    actx->minAverageChange = AIR_CAST(alan_t, parm);
    break;
  case alanParmMaxPixelChange:
    actx->maxPixelChange = AIR_CAST(alan_t, parm);
    break;
  case alanParmAlpha:
    actx->alpha = AIR_CAST(alan_t, parm);
    break;
  case alanParmBeta:
    actx->beta = AIR_CAST(alan_t, parm);
    break;
  case alanParmWrapAround:
    parmI = !!parm;
    actx->wrap = parmI;
    break;
  default:
    biffAddf(ALAN, "%s: parameter %d invalid", me, whichParm);
    return 1;
    break;
  }

  return 0;
}
