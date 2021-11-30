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

#include "elf.h"

/* Creates an elfMaximaContext, which can then be used to find all
 * maxima of a symmetric even-order 3D tensor of the given type.
 * Returns NULL if type is not even-order 3D symmetric.
 *
 * The initial maximum sampling is at the vertices of a subdivided
 * icosahedron - level=3 (321 unique directions) should be
 * sufficient. Larger levels reduce the risk of missing one of two (or
 * more) very close maxima, at increased computational cost. */
elfMaximaContext *elfMaximaContextNew(const tijk_type *type,
                                      unsigned int level) {
  elfMaximaContext *retval;
  limnPolyData *sphere;
  unsigned int vert;
  if (type==NULL || type->dim!=3 || type->sym==NULL || type->order%2!=0)
    return NULL; /* elfMaxima cannot be used with this tensor type */
  sphere = limnPolyDataNew();
  limnPolyDataIcoSphere(sphere, 0, level);
  retval = (elfMaximaContext*) malloc(sizeof(elfMaximaContext));
  retval->num = sphere->xyzwNum;
  retval->type = type;
  retval->parm = NULL;
  retval->refine = 1;
  /* extract neighborhood info (needed to take discrete maxima) */
  limnPolyDataNeighborArray(&(retval->neighbors), &(retval->nbstride), sphere);
  /* copy over vertices in single and double precision */
  retval->vertices_f = (float*) malloc(sizeof(float)*3*(sphere->xyzwNum/2));
  for (vert=0; vert<sphere->xyzwNum; vert+=2) {
    ELL_3V_COPY(retval->vertices_f+3*(vert/2), sphere->xyzw+4*vert);
  }
  retval->vertices_d = NULL;
  sphere=limnPolyDataNix(sphere);
  return retval;
}

elfMaximaContext *elfMaximaContextNix(elfMaximaContext *emc) {
  if (emc!=NULL) {
    free(emc->neighbors);
    free(emc->vertices_f);
    if (emc->vertices_d!=NULL)
      free(emc->vertices_d);
    if (emc->parm!=NULL)
      tijk_refine_rank1_parm_nix(emc->parm);
    free(emc);
  }
  return NULL;
}

/* can be used to set the parameters for refining the found local
 * maxima. Note that the elfMaximaContext will "take over possession"
 * of the parm struct, i.e., it will be nix'ed along with the context
 * or when setting another parm */
void elfMaximaParmSet(elfMaximaContext *emc,
                      tijk_refine_rank1_parm *parm) {
  if (emc!=NULL) {
    if (emc->parm!=NULL)
      tijk_refine_rank1_parm_nix(emc->parm);
    emc->parm=parm;
  }
}

/* By default, discrete maxima are refined via optimization on the sphere.
 * Set this to zero for faster, but less accurate results. */
void elfMaximaRefineSet(elfMaximaContext *emc, int refine) {
  if (emc!=NULL) {
    emc->refine = refine;
  }
}

/* Finds discrete maxima of the tensor (based on emc) and refines them,
 * storing magnitudes in (malloc'ed) *ls and *vs. Returns the number of
 * distinct maxima, or -1 on error. ls are sorted in descending order.
 */
int elfMaximaFind_d(double **ls, double **vs, const double *ten,
                    elfMaximaContext *emc) {
  unsigned int i;
  int retval;
  double *vals;
  airHeap *heap;
  if (ls==NULL || vs==NULL || ten==NULL || emc==NULL)
    return -1;
  if (emc->vertices_d==NULL) { /* we need to allocate these */
    emc->vertices_d = (double*) malloc(sizeof(double)*3*(emc->num/2));
    for (i=0; i<emc->num/2; i++) {
      ELL_3V_COPY(emc->vertices_d+3*i, emc->vertices_f+3*i);
    }
  }
  /* evaluate all unique directions */
  vals = (double*) malloc(sizeof(double)*(emc->num/2));
  for (i=0; i<emc->num/2; i++) {
    vals[i]=(*emc->type->sym->s_form_d)(ten, emc->vertices_d+3*i);
  }
  heap = airHeapNew(sizeof(double)*3, 20);
  /* identify discrete maxima */
  for (i=0; i<emc->num/2; i++) {
    unsigned int ni=0;
    int ismax=1, nb;
    while (ni<emc->nbstride && (nb=emc->neighbors[emc->nbstride*2*i+ni])!=-1) {
      if (vals[i]<=vals[nb/2]) {
        ismax=0;
        break;
      }
      ni++;
    }
    if (ismax) {
      double s=vals[i], v[3];
      ELL_3V_COPY(v,emc->vertices_d+3*i);
      if (emc->refine) /* refine further */
        tijk_refine_max_3d_d(&s, v, ten, emc->type, emc->parm);
      /* add to heap */
      airHeapInsert(heap, -s, v);
    }
  }
  /* allocate arrays and return what we have found */
  retval=airHeapLength(heap);
  if (retval>0) {
    *ls = (double*) malloc(sizeof(double)*retval);
    *vs = (double*) malloc(sizeof(double)*3*retval);
    for (i=0; i<(unsigned int)retval; i++) {
      (*ls)[i]=-airHeapFrontPop(heap, (*vs)+3*i);
    }
  }
  heap=airHeapNix(heap);
  free(vals);
  return retval;
}

/* Mostly copy-pasted from above :-/
 */
int elfMaximaFind_f(float **ls, float **vs, const float *ten,
                    elfMaximaContext *emc) {
  unsigned int i;
  int retval;
  float *vals;
  airHeap *heap;
  if (ls==NULL || vs==NULL || ten==NULL || emc==NULL)
    return -1;
  /* evaluate all unique directions */
  vals = (float*) malloc(sizeof(float)*(emc->num/2));
  for (i=0; i<emc->num/2; i++) {
    vals[i]=(*emc->type->sym->s_form_f)(ten, emc->vertices_f+3*i);
  }
  heap = airHeapNew(sizeof(float)*3, 20);
  /* identify discrete maxima */
  for (i=0; i<emc->num/2; i++) {
    unsigned int ni=0;
    int ismax=1, nb;
    while (ni<emc->nbstride && (nb=emc->neighbors[emc->nbstride*2*i+ni])!=-1) {
      if (vals[i]<=vals[nb/2]) {
        ismax=0;
        break;
      }
      ni++;
    }
    if (ismax) {
      float s=vals[i], v[3];
      ELL_3V_COPY(v,emc->vertices_f+3*i);
      if (emc->refine) /* refine further */
        tijk_refine_max_3d_f(&s, v, ten, emc->type, emc->parm);
      /* add to heap */
      airHeapInsert(heap, -s, v);
    }
  }
  /* allocate arrays and return what we have found */
  retval=airHeapLength(heap);
  if (retval>0) {
    *ls = (float*) malloc(sizeof(float)*retval);
    *vs = (float*) malloc(sizeof(float)*3*retval);
    for (i=0; i<(unsigned int)retval; i++) {
      (*ls)[i]=AIR_CAST(float,-airHeapFrontPop(heap, (*vs)+3*i));
    }
  }
  heap=airHeapNix(heap);
  free(vals);
  return retval;
}
