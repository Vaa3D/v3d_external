/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2010, 2009 Thomas Schultz

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

const int
elfPresent = 42;

/* Glyphs for higher-order tensors */

/* Helper routine to estimate normals in a triangle soup in which antipodal
 * vertices are subsequent */
static void
estimateNormalsAntipodal (limnPolyData *glyph, const char normalize) {
  unsigned int faceno=glyph->indxNum/3;
  unsigned int *faces=glyph->indx;
  unsigned int f;
  memset(glyph->norm,0,sizeof(float)*3*glyph->xyzwNum);
  for (f=0; f<faceno/2; f++) {
    float diff1[3],diff2[3],cross[3];
    ELL_3V_SUB(diff1,glyph->xyzw+4*faces[3*f+1],
               glyph->xyzw+4*faces[3*f]);
    ELL_3V_SUB(diff2,glyph->xyzw+4*faces[3*f+2],
               glyph->xyzw+4*faces[3*f]);
    ELL_3V_CROSS(cross,diff1,diff2);
    ELL_3V_INCR(glyph->norm+3*faces[3*f],cross);
    ELL_3V_INCR(glyph->norm+3*faces[3*f+1],cross);
    ELL_3V_INCR(glyph->norm+3*faces[3*f+2],cross);
    /* same for anti-face */
    if (faces[3*f]%2==0)
      ELL_3V_SUB(glyph->norm+3*faces[3*f]+3,
                 glyph->norm+3*faces[3*f]+3,cross);
    else
      ELL_3V_SUB(glyph->norm+3*faces[3*f]-3,
                 glyph->norm+3*faces[3*f]-3,cross);
    if (faces[3*f+1]%2==0)
      ELL_3V_SUB(glyph->norm+3*faces[3*f+1]+3,
                 glyph->norm+3*faces[3*f+1]+3,cross);
    else
      ELL_3V_SUB(glyph->norm+3*faces[3*f+1]-3,
                 glyph->norm+3*faces[3*f+1]-3,cross);
    if (faces[3*f+2]%2==0)
      ELL_3V_SUB(glyph->norm+3*faces[3*f+2]+3,
                 glyph->norm+3*faces[3*f+2]+3,cross);
    else
      ELL_3V_SUB(glyph->norm+3*faces[3*f+2]-3,
                 glyph->norm+3*faces[3*f+2]-3,cross);
  }
  if (normalize) {
    float len;
    unsigned int i;
    for (i=0; i<glyph->normNum; i++) {
      ELL_3V_NORM_TT(glyph->norm + 3*i, float, glyph->norm + 3*i, len);
    }
  }
}

/*
******** elfGlyphPolar
**
** Turns a unit sphere into a polar plot that depicts a given tensor.
**
** Input:
** glyph is expected to represent the unit sphere
** antipodal can be set to a non-zero value if antipodal points on the sphere
**           are subsequent in the input. It will lead to faster processing.
**           (this can be used together with limnPolyDataIcoSphere)
** ten is an input tensor of type type
** clamp - if nonzero, negative values will be clamped to zero
** normalize - if nonzero, surface normals will be rescaled to unit length
** posColor[4] - RGBA color code for positive values (if desired, else NULL)
** negColor[4] - assumed to be non-NULL if posColor is non-NULL
**
** Output:
** glyph is the polar plot that corresponds to ten.
** If the input shape was anything other than a unit sphere, the output
** shape is undefined
** Normals are only updated when they were allocated in the input
** When colors were present in the input, they are replaced by a color
** coding of sign (if posColor!=NULL) or a pointwise XYZ-RGB map (else)
** When isdef!=NULL, *isdef is set to 0 if we found evidence that the given
** input tensor is not positive definite
** The return value is the radius of the glyph's bounding sphere
*/
float
elfGlyphPolar(limnPolyData *glyph, const char antipodal,
       const float *ten, const tijk_type *type,
       char *isdef, const char clamp, const char normalize,
       const unsigned char *posColor,
       const unsigned char *negColor) {
  float *verts=glyph->xyzw;
  float max=0;
  unsigned int i, infoBitFlag;
  char def=1;
  infoBitFlag = limnPolyDataInfoBitFlag(glyph);
  for (i=0; i<glyph->xyzwNum; i++) {
    float val=(*type->sym->s_form_f)(ten,verts);

    /* if RGBA is allocated, take care of coloring */
    if (infoBitFlag & (1 << limnPolyDataInfoRGBA)) {
      if (posColor!=NULL) {
        /* color by sign */
        if (val<0) {
          ELL_4V_COPY(glyph->rgba+4*i, negColor);
          if (antipodal) ELL_4V_COPY(glyph->rgba+4*i+4, negColor);
        } else {
          ELL_4V_COPY(glyph->rgba+4*i, posColor);
          if (antipodal) ELL_4V_COPY(glyph->rgba+4*i+4, posColor);
        }
      } else {
        /* RGB encode the vertex coordinates */
        ELL_4V_SET_TT(glyph->rgba+4*i, unsigned char,
                      255*fabs(verts[0]), 255*fabs(verts[1]),
                      255*fabs(verts[2]), 255);
        if (antipodal) {
          ELL_4V_COPY(glyph->rgba+4*i+4,glyph->rgba+4*i);
        }
      }
    }

    if (val<0) {
      def=0;
      if (clamp) val=0;
      else val=-val;
    }
    if (val>max) max=val;
    ELL_3V_SCALE(verts,val,verts);
    if (antipodal) {
      ELL_3V_SCALE(verts+4,-1.0f,verts);
      verts+=4; i++;
    }
    verts+=4;
  }
  if (isdef!=NULL) *isdef=def;
  if (infoBitFlag & (1 << limnPolyDataInfoNorm)) {
    /* take care of normals */
    if (antipodal &&
        glyph->primNum==1 &&
        glyph->type[0]==limnPrimitiveTriangles) {
      /* we can use our specialized, more efficient code */
      estimateNormalsAntipodal(glyph, normalize);
    } else { /* use standard limn routine */
      limnPolyDataVertexNormals(glyph);
    }
  }
  return max;
}

/*
******** elfGlyphHOME
**
** Turns a unit sphere into a HOME glyph that depicts a given tensor.
**
** Input:
** glyph is expected to represent the unit sphere
** antipodal can be set to a non-zero value if antipodal points on the sphere
**           are subsequent in the input. It will lead to faster processing.
**           (this can be used together with limnPolyDataIcoSphere)
** ten is an input tensor of type type
** normalize - if nonzero, normals will be rescaled to unit length
**
** Output:
** glyph is the HOME glyph that corresponds to ten.
** If the input shape was anything other than a unit sphere, the output
** shape is undefined
** If the input tensor does not have a rank-k decomposition with positive
** coefficients, the output shape may self-intersect
** Normals are only updated when they were allocated in the input
** When colors were present in the input, they are replaced by a
** pointwise XYZ-RGB map
** When isdef!=NULL, *isdef is set to 0 if we found evidence that the given
** input tensor is not positive definite
** The return value is the radius of the glyph's bounding sphere, or -1
** upon error (odd tensor order; HOME glyph is only defined for even orders)
*/
float
elfGlyphHOME(limnPolyData *glyph, const char antipodal,
             const float *ten, const tijk_type *type,
             char *isdef, const char normalize) {
  float *verts=glyph->xyzw;
  float max=0;
  unsigned int i, infoBitFlag;
  char def=1;
  if (type->order%2==1) /* sanity check */
    return -1;
  infoBitFlag = limnPolyDataInfoBitFlag(glyph);
  for (i=0; i<glyph->xyzwNum; i++) {
    float HOMEpos[3], len;
    (*type->sym->v_form_f)(HOMEpos,ten,verts);
    if (ELL_3V_DOT(HOMEpos,verts)<0) def=0;
    ELL_3V_COPY(verts,HOMEpos);
    len=AIR_CAST(float, ELL_3V_LEN(HOMEpos));
    if (len>max) max=len;

    /* if RGBA is allocated, take care of coloring */
    if (infoBitFlag & (1 << limnPolyDataInfoRGBA)) {
      float c[3];
      if (len>1e-18)
        ELL_3V_SET_TT(c,float,fabs(verts[0]/len),
                      fabs(verts[1]/len),fabs(verts[2]/len));
      else
        ELL_3V_SET(c,0,0,0);
      /* RGB encode the vertex coordinates */
      ELL_4V_SET_TT(glyph->rgba+4*i, unsigned char,
                    255*c[0], 255*c[1], 255*c[2], 255);
      if (antipodal) {
        ELL_4V_COPY(glyph->rgba+4*i+4,glyph->rgba+4*i);
      }
    }

    if (antipodal) {
      ELL_3V_SCALE(verts+4,-1.0f,verts);
      verts+=4; i++;
    }
    verts+=4;
  }
  if (isdef!=NULL) *isdef=def;
  if (infoBitFlag & (1 << limnPolyDataInfoNorm)) {
    /* take care of normals */
    if (antipodal &&
        glyph->primNum==1 &&
        glyph->type[0]==limnPrimitiveTriangles) {
      /* we can use our specialized faster code */
      estimateNormalsAntipodal(glyph, normalize);
    } else { /* use standard limn routine */
      limnPolyDataVertexNormals(glyph);
    }
  }
  return max;
}

/*
******** elfColorGlyphMaxima
**
** Maximum-based coloring of tensor glyphs, as described in Section 4 of
** Schultz/Kindlmann, EuroVis 2010
**
** Input:
** glyph is a tensor glyph (generated by elfGlyph*)
** antipodal can be set to a non-zero value if antipodal points on the sphere
**           are subsequent in the input. It will lead to faster processing.
** neighbors is an array that, for each vertex in glyph, holds the indices
**           of its neighbors (adjacent through an edge). Each vertex has
**           nbstride entries, padded with -1s if it really has less neighbors
** ten is the tensor depicted by the glyph
** type is the tensor type
** modulate - non-zero values lead to modulation by peak sharpness
** gamma - allows you to "bump up" the peak sharpness by the power of gamma
**
** Output:
** glyph is colored according to its maxima
** returns zero upon success (fails if memory cannot be allocated)
*/
int
elfColorGlyphMaxima(limnPolyData *glyph, const char antipodal,
                    const int *neighbors, unsigned int nbstride,
                    const float *ten, const tijk_type *type,
                    const char modulate, const float gamma) {
  float *sqrdist, *verts, *newcol;
  char *processed, *id_ct, *diff_ct;
  int *path;
  unsigned int infoBitFlag, i, j;
  airArray *mop;
  infoBitFlag = limnPolyDataInfoBitFlag(glyph);
  if (limnPolyDataAlloc(glyph, /* make sure we have a color buffer */
                        infoBitFlag | (1 << limnPolyDataInfoRGBA),
                        glyph->xyzwNum, glyph->indxNum, glyph->primNum)) {
    return 1;
  }
  /* do the memory allocation */
  mop = airMopNew();
  if (mop==NULL) return 1;
  sqrdist = (float *) malloc(sizeof(float)*glyph->xyzwNum);
  airMopAdd(mop, sqrdist, airFree, airMopAlways);
  processed = (char *) malloc(sizeof(char)*glyph->xyzwNum);
  airMopAdd(mop, processed, airFree, airMopAlways);
  path = (int *) malloc(sizeof(int)*glyph->xyzwNum);
  airMopAdd(mop, path, airFree, airMopAlways);
  newcol = (float *) malloc(sizeof(float)*3*glyph->xyzwNum);
  airMopAdd(mop, newcol, airFree, airMopAlways);
  id_ct = (char *) malloc(sizeof(char)*glyph->xyzwNum);
  airMopAdd(mop, id_ct, airFree, airMopAlways);
  diff_ct = (char *) malloc(sizeof(char)*glyph->xyzwNum);
  airMopAdd(mop, diff_ct, airFree, airMopAlways);
  if (sqrdist==NULL || processed==NULL || path==NULL || newcol==NULL ||
      id_ct==NULL || diff_ct==NULL) {
    airMopError(mop);
    return 1;
  }

  /* initialize sqrdist / processed / path */
  verts=glyph->xyzw;
  if (antipodal) {
    for (i=0; i<glyph->xyzwNum; i+=2) {
      sqrdist[i]=sqrdist[i+1]=ELL_3V_DOT(verts,verts);
      verts+=8;
    }
  } else {
    for (i=0; i<glyph->xyzwNum; i++) {
      sqrdist[i]=ELL_3V_DOT(verts,verts);
      verts+=4;
    }
  }
  memset(processed,0,sizeof(char)*glyph->xyzwNum);
  memset(path, -1, sizeof(int)*glyph->xyzwNum);

  /* go through all vertices; ascend until we get to a maximum or a
   * previously processed vertex */
  for (i=0; i<glyph->xyzwNum; i++) {
    unsigned int pathno=0;
    int vert=i;
    char foundmax=0;
    unsigned char color[3];

    if (processed[i]) continue;

    path[pathno++]=vert;
    while (!foundmax) {
      int bestnb=-1;
      float maxdist=0;
      for (j=0; j<nbstride; ++j) {
        int nb=neighbors[nbstride*vert+j];
        float dist;
        if (nb==-1) break;
        dist=sqrdist[nb]-sqrdist[vert];
        if (dist>maxdist) {
          maxdist=dist; bestnb=nb;
        }
      }
      if (bestnb==-1) { /* we are at an unprocessed maximum */
        /* find appropriate color */
        float vertdir[3];
        float norm;
        float modfactor=1.0;
        ELL_3V_COPY(vertdir,glyph->xyzw+4*vert);
        norm=AIR_CAST(float, ELL_3V_LEN(vertdir));
        if (norm>1e-18) {
          ELL_3V_SCALE(vertdir,1.0f/norm,vertdir);
          if (modulate) {
            /* modulate by peak strength */
            float hess[7], val, evals[3];

            /* compute second derivatives */
            (*type->sym->hess_f)(hess+1,ten,vertdir);
            val=(*type->sym->s_form_f)(ten,vertdir);
            tenEigensolve_f(evals, NULL, hess);
            if (evals[1]>=0 || val<1e-10) {
              modfactor=0.0;
            } else {
              modfactor=-evals[1]/(type->order*val);
              if (modfactor>1.0) modfactor=1.0;
              else modfactor=AIR_CAST(float, pow(modfactor,gamma));
            }
          }
        } else {
          ELL_3V_SET(vertdir,0,0,0);
        }

        ELL_3V_SET(color,
                   (unsigned char) (AIR_LERP(modfactor, 1.0,
                                             fabs(vertdir[0]))*255),
                   (unsigned char) (AIR_LERP(modfactor, 1.0,
                                             fabs(vertdir[1]))*255),
                   (unsigned char) (AIR_LERP(modfactor, 1.0,
                                             fabs(vertdir[2]))*255));
        foundmax=1;
      } else {
        if (processed[bestnb]) {
          ELL_3V_COPY(color, glyph->rgba+4*bestnb);
          foundmax=1;
        } else { /* add bestnb to the path and proceed */
          path[pathno++]=bestnb;
          vert=bestnb;
        }
      }
    } /* end looking for maximum */

    /* copy color to all vertices on the path */
    for (j=0; j<pathno; ++j) {
      processed[path[j]]=1;
      ELL_4V_SET(glyph->rgba+4*path[j], color[0], color[1], color[2], 255);
      if (antipodal) {
        int antip=path[j]+1;
        if (antip%2==0) antip-=2;
        processed[antip]=1;
        ELL_4V_COPY(glyph->rgba+4*antip, glyph->rgba+4*path[j]);
      }
    }
    if (antipodal) i++; /* we can skip over the next one */
  }

  /* make coloring smoother by averaging */
  memset(newcol,0,sizeof(float)*3*glyph->xyzwNum);
  memset(id_ct,0,sizeof(char)*glyph->xyzwNum);
  memset(diff_ct,0,sizeof(char)*glyph->xyzwNum);

  for (i=0; i<glyph->xyzwNum; i++) {
    for (j=0; j<nbstride; j++) {
      int nb=neighbors[nbstride*i+j];
      if (nb<0) break;
      if ((unsigned int)nb<i) continue; /* process each pair only once */
      if (ELL_3V_EQUAL(glyph->rgba+4*i, glyph->rgba+4*nb)) {
        id_ct[i]++;
        id_ct[nb]++;
      } else {
        ELL_3V_INCR(newcol+3*i, glyph->rgba+4*nb);
        diff_ct[i]++;
        ELL_3V_INCR(newcol+3*nb, glyph->rgba+4*i);
        diff_ct[nb]++;
      }
    }
  }

  for (i=0; i<glyph->xyzwNum; i++) {
    if (diff_ct[i]>0) {
      ELL_3V_SCALE_INCR_TT(newcol+3*i, float, 1.0+id_ct[i],glyph->rgba+4*i);
      ELL_3V_SCALE_TT(glyph->rgba+4*i, unsigned char,
                      1.0/(1.0+id_ct[i]+diff_ct[i]),
                      newcol+3*i);
      if (antipodal) {
        ELL_3V_COPY(glyph->rgba+4*i+4,glyph->rgba+4*i);
        i++;
      }
    }
  }

  airMopOkay(mop);
  return 0;
}
