/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah
  Copyright (C) 2011  Thomas Schultz

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


#include "limn.h"

int
limnPolyDataSpiralTubeWrap(limnPolyData *pldOut, const limnPolyData *pldIn,
                           unsigned int infoBitFlag, Nrrd *nvertmap,
                           unsigned int tubeFacet, unsigned int endFacet,
                           double radius) {
  static const char me[]="limnPolyDataSpiralTubeWrap";
  double *cost, *sint;
  unsigned int tubeVertNum = 0, tubeIndxNum = 0, primIdx, pi, *vertmap;
  unsigned int inVertTotalIdx = 0, outVertTotalIdx = 0, outIndxIdx = 0;
  int color;
  airArray *mop;

  if (!( pldOut && pldIn )) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if ((1 << limnPrimitiveLineStrip) != limnPolyDataPrimitiveTypes(pldIn)) {
    biffAddf(LIMN, "%s: sorry, can only handle %s primitives", me,
             airEnumStr(limnPrimitive, limnPrimitiveLineStrip));
    return 1;
  }
  for (primIdx=0; primIdx<pldIn->primNum; primIdx++) {
    unsigned int tvni, tini;
    if (endFacet) {
      tvni = tubeFacet*(2*endFacet + pldIn->icnt[primIdx]) + 1;
      tini = 2*tubeFacet*(2*endFacet + pldIn->icnt[primIdx] + 1) -2;
    } else {
      tvni = tubeFacet*(2 + pldIn->icnt[primIdx]);
      tini = 2*tubeFacet*(1 + pldIn->icnt[primIdx]);
    }
    tubeVertNum += tvni;
    tubeIndxNum += tini;
  }
  if (limnPolyDataAlloc(pldOut,
                        /* sorry have to have normals, even if they weren't
                           asked for, because currently they're used as part
                           of vertex position calc */
                        (infoBitFlag | (1 << limnPolyDataInfoNorm)),
                        tubeVertNum, tubeIndxNum, pldIn->primNum)) {
    biffAddf(LIMN, "%s: trouble allocating output", me);
    return 1;
  }
  if (nvertmap) {
    if (nrrdMaybeAlloc_va(nvertmap, nrrdTypeUInt, 1,
                          AIR_CAST(size_t, tubeVertNum))) {
      biffMovef(LIMN, NRRD, "%s: trouble allocating vert map", me);
      return 1;
    }
    vertmap = AIR_CAST(unsigned int *, nvertmap->data);
  } else {
    vertmap = NULL;
  }
  color = ((infoBitFlag & (1 << limnPolyDataInfoRGBA))
           && (limnPolyDataInfoBitFlag(pldIn) & (1 << limnPolyDataInfoRGBA)));

  mop = airMopNew();
  cost = AIR_CAST(double *, calloc(tubeFacet, sizeof(double)));
  sint = AIR_CAST(double *, calloc(tubeFacet, sizeof(double)));
  airMopAdd(mop, cost, airFree, airMopAlways);
  airMopAdd(mop, sint, airFree, airMopAlways);
  if (!(cost && sint)) {
    biffAddf(LIMN, "%s: couldn't allocate lookup tables", me);
    airMopError(mop); return 1;
  }
  for (pi=0; pi<tubeFacet; pi++) {
    double angle;
    angle = AIR_AFFINE(0, pi, tubeFacet, 0, 2*AIR_PI);
    cost[pi] = cos(angle);
    sint[pi] = sin(angle);
  }
  for (primIdx=0; primIdx<pldIn->primNum; primIdx++) {
    unsigned int inVertIdx;
    pldOut->type[primIdx] = limnPrimitiveTriangleStrip;
    if (endFacet) {
      pldOut->icnt[primIdx] =
        2*tubeFacet*(2*endFacet + pldIn->icnt[primIdx] + 1) - 2;
    } else {
      pldOut->icnt[primIdx] =
        2*tubeFacet*(1 + pldIn->icnt[primIdx]);
    }

    for (inVertIdx=0;
         inVertIdx<pldIn->icnt[primIdx];
         inVertIdx++) {
      unsigned int forwIdx, backIdx, tubeEndIdx;
      double tang[3], tmp, scl, step, perp[3], pimp[3];
      /* inVrt = pldIn->vert + pldIn->indx[inVertTotalIdx]; */
      if (0 == inVertIdx) {
        forwIdx = inVertTotalIdx+1;
        backIdx = inVertTotalIdx;
        scl = 1;
      } else if (pldIn->icnt[primIdx]-1 == inVertIdx) {
        forwIdx = inVertTotalIdx;
        backIdx = inVertTotalIdx-1;
        scl = 1;
      } else {
        forwIdx = inVertTotalIdx+1;
        backIdx = inVertTotalIdx-1;
        scl = 0.5;
      }
      if (1 == pldIn->icnt[primIdx]) {
        ELL_3V_SET(tang, 0, 0, 1); /* completely arbitrary, as it must be */
        step = 0;
      } else {
        ELL_3V_SUB(tang,
                   pldIn->xyzw + 4*forwIdx,
                   pldIn->xyzw + 4*backIdx);
        ELL_3V_NORM(tang, tang, step);
        step *= scl;
      }
      if (0 == inVertIdx || 1 == pldIn->icnt[primIdx]) {
        ell_3v_perp_d(perp, tang);
      } else {
        /* transport last perp forwards */
        double dot;
        dot = ELL_3V_DOT(perp, tang);
        ELL_3V_SCALE_ADD2(perp, 1.0, perp, -dot, tang);
      }
      ELL_3V_NORM(perp, perp, tmp);
      ELL_3V_CROSS(pimp, perp, tang);
      /* (perp, pimp, tang) is a left-handed frame, on purpose */
      /*  limnVrt *outVrt; */
      /* -------------------------------------- BEGIN initial endcap */
      if (0 == inVertIdx) {
        unsigned int startIdx, ei;
        startIdx = outVertTotalIdx;
        if (endFacet) {
          for (ei=0; ei<endFacet; ei++) {
            for (pi=0; pi<tubeFacet; pi++) {
              double costh, sinth, cosph, sinph, phi, theta;
              phi = (AIR_AFFINE(0, ei, endFacet, 0, AIR_PI/2)
                     + AIR_AFFINE(0, pi, tubeFacet,
                                  0, AIR_PI/2)/endFacet);
              theta = AIR_AFFINE(0, pi, tubeFacet, 0.0, 2*AIR_PI);
              cosph = cos(phi);
              sinph = sin(phi);
              costh = cos(theta);
              sinth = sin(theta);
              ELL_3V_SCALE_ADD3_TT(pldOut->norm + 3*outVertTotalIdx, float,
                                   -cosph, tang,
                                   costh*sinph, perp,
                                   sinth*sinph, pimp);
              ELL_3V_SCALE_ADD3_TT(pldOut->xyzw + 4*outVertTotalIdx, float,
                                   1, pldIn->xyzw + 4*inVertTotalIdx,
                                   -step/2, tang,
                                   radius,
                                   pldOut->norm + 3*outVertTotalIdx);
              (pldOut->xyzw + 4*outVertTotalIdx)[3] = 1.0;
              if (vertmap) {
                vertmap[outVertTotalIdx] = inVertTotalIdx;
              }
              if (color) {
                ELL_4V_COPY(pldOut->rgba + 4*outVertTotalIdx,
                            pldIn->rgba + 4*inVertTotalIdx);

              }
              outVertTotalIdx++;
            }
          }
          for (pi=1; pi<tubeFacet; pi++) {
            pldOut->indx[outIndxIdx++] = startIdx;
            pldOut->indx[outIndxIdx++] = startIdx + pi;
          }
          for (ei=0; ei<endFacet; ei++) {
            /* at the highest ei we're actually linking with the first
               row of vertices at the start of the tube */
            for (pi=0; pi<tubeFacet; pi++) {
              pldOut->indx[outIndxIdx++] = (startIdx + pi
                                            + (ei + 0)*tubeFacet);
              pldOut->indx[outIndxIdx++] = (startIdx + pi
                                            + (ei + 1)*tubeFacet);
            }
          }
        } else {
          /* no endcap, open tube */
          for (pi=0; pi<tubeFacet; pi++) {
            double costh, sinth, theta;
            theta = AIR_AFFINE(0, pi, tubeFacet, 0.0, 2*AIR_PI);
            costh = cos(theta);
            sinth = sin(theta);
            ELL_3V_SCALE_ADD2_TT(pldOut->norm + 3*outVertTotalIdx, float,
                                 costh, perp,
                                 sinth, pimp);
            ELL_3V_SCALE_ADD3_TT(pldOut->xyzw + 4*outVertTotalIdx, float,
                                 1, pldIn->xyzw + 4*inVertTotalIdx,
                                 -step/2 + step/(2*tubeFacet), tang,
                                 radius, pldOut->norm + 3*outVertTotalIdx);
            (pldOut->xyzw + 4*outVertTotalIdx)[3] = 1.0;
            if (vertmap) {
              vertmap[outVertTotalIdx] = inVertTotalIdx;
            }
            if (color) {
              ELL_4V_COPY(pldOut->rgba + 4*outVertTotalIdx,
                          pldIn->rgba + 4*inVertTotalIdx);

            }
            outVertTotalIdx++;
          }
          for (pi=0; pi<tubeFacet; pi++) {
            pldOut->indx[outIndxIdx++] = (startIdx + pi + 0*tubeFacet);
            pldOut->indx[outIndxIdx++] = (startIdx + pi + 1*tubeFacet);
          }
        }
      } /* if (0 == inVertIdx) */
      /* -------------------------------------- END initial endcap */
      for (pi=0; pi<tubeFacet; pi++) {
        double shift, cosa, sina;
        shift = AIR_AFFINE(-0.5, pi, tubeFacet-0.5, -step/2, step/2);
        cosa = cost[pi];
        sina = sint[pi];
        /* outVrt = pldOut->vert + outVertTotalIdx; */
        ELL_3V_SCALE_ADD2_TT(pldOut->norm + 3*outVertTotalIdx, float,
                             cosa, perp, sina, pimp);
        ELL_3V_SCALE_ADD3_TT(pldOut->xyzw + 4*outVertTotalIdx, float,
                             1, pldIn->xyzw + 4*inVertTotalIdx,
                             radius,
                             pldOut->norm + 3*outVertTotalIdx,
                             shift, tang);
        (pldOut->xyzw + 4*outVertTotalIdx)[3] = 1.0;
        pldOut->indx[outIndxIdx++] = outVertTotalIdx;
        pldOut->indx[outIndxIdx++] = outVertTotalIdx + tubeFacet;
        if (vertmap) {
          vertmap[outVertTotalIdx] = inVertTotalIdx;
        }
        if (color) {
          ELL_4V_COPY(pldOut->rgba + 4*outVertTotalIdx,
                      pldIn->rgba + 4*inVertTotalIdx);

        }
        outVertTotalIdx++;
      }
      tubeEndIdx = outVertTotalIdx;
      /* -------------------------------------- BEGIN final endcap */
      if (inVertIdx == pldIn->icnt[primIdx]-1) {
        unsigned int ei;
        if (endFacet) {
          for (ei=0; ei<endFacet; ei++) {
            for (pi=0; pi<tubeFacet; pi++) {
              double costh, sinth, cosph, sinph, phi, theta;
              phi = (AIR_AFFINE(0, ei, endFacet, AIR_PI/2, AIR_PI)
                     + AIR_AFFINE(0, pi, tubeFacet,
                                  0, AIR_PI/2)/endFacet);
              theta = AIR_AFFINE(0, pi, tubeFacet, 0.0, 2*AIR_PI);
              cosph = cos(phi);
              sinph = sin(phi);
              costh = cos(theta);
              sinth = sin(theta);
              /* outVrt = pldOut->vert + outVertTotalIdx; */
              ELL_3V_SCALE_ADD3_TT(pldOut->norm + 3*outVertTotalIdx, float,
                                   -cosph, tang,
                                   costh*sinph, perp,
                                   sinth*sinph, pimp);
              ELL_3V_SCALE_ADD3_TT(pldOut->xyzw + 4*outVertTotalIdx, float,
                                   1, pldIn->xyzw + 4*inVertTotalIdx,
                                   step/2, tang,
                                   radius,
                                   pldOut->norm + 3*outVertTotalIdx);
              (pldOut->xyzw + 4*outVertTotalIdx)[3] = 1.0;
              if (vertmap) {
                vertmap[outVertTotalIdx] = inVertTotalIdx;
              }
              if (color) {
                ELL_4V_COPY(pldOut->rgba + 4*outVertTotalIdx,
                            pldIn->rgba + 4*inVertTotalIdx);

              }
              outVertTotalIdx++;
            }
          }
          /* outVrt = pldOut->vert + outVertTotalIdx; */
          ELL_3V_COPY_TT(pldOut->norm + 3*outVertTotalIdx, float, tang);
          ELL_3V_SCALE_ADD3_TT(pldOut->xyzw + 4*outVertTotalIdx, float,
                               1, pldIn->xyzw + 4*inVertTotalIdx,
                               step/2, tang,
                               radius,
                               pldOut->norm + 3*outVertTotalIdx);
          (pldOut->xyzw + 4*outVertTotalIdx)[3] = 1.0;
          if (vertmap) {
            vertmap[outVertTotalIdx] = inVertTotalIdx;
          }
          outVertTotalIdx++;
          for (ei=0; ei<endFacet-1; ei++) {
            for (pi=0; pi<tubeFacet; pi++) {
              pldOut->indx[outIndxIdx++] = (tubeEndIdx + pi
                                            + (ei + 0)*tubeFacet);
              pldOut->indx[outIndxIdx++] = (tubeEndIdx + pi
                                            + (ei + 1)*tubeFacet);
            }
          }
          for (pi=0; pi<tubeFacet; pi++) {
            pldOut->indx[outIndxIdx++] = (tubeEndIdx + pi
                                          + (endFacet - 1)*tubeFacet);
            pldOut->indx[outIndxIdx++] = (tubeEndIdx
                                          + (endFacet - 0)*tubeFacet);
          }
        } else {
          /* no endcap, open tube */
          for (pi=0; pi<tubeFacet; pi++) {
            double costh, sinth, theta;
            theta = AIR_AFFINE(0, pi, tubeFacet, 0.0, 2*AIR_PI);
            costh = cos(theta);
            sinth = sin(theta);
            ELL_3V_SCALE_ADD2_TT(pldOut->norm + 3*outVertTotalIdx, float,
                                 costh, perp,
                                 sinth, pimp);
            ELL_3V_SCALE_ADD3_TT(pldOut->xyzw + 4*outVertTotalIdx, float,
                                 1, pldIn->xyzw + 4*inVertTotalIdx,
                                 step/2 - step/(2*tubeFacet), tang,
                                 radius, pldOut->norm + 3*outVertTotalIdx);
            (pldOut->xyzw + 4*outVertTotalIdx)[3] = 1.0;
            if (vertmap) {
              vertmap[outVertTotalIdx] = inVertTotalIdx;
            }
            if (color) {
              ELL_4V_COPY(pldOut->rgba + 4*outVertTotalIdx,
                          pldIn->rgba + 4*inVertTotalIdx);

            }
            outVertTotalIdx++;
          }
        }
      } /* if (inVertIdx == pldIn->icnt[primIdx]-1) */
      /* -------------------------------------- END final endcap */
      inVertTotalIdx++;
    }
  }

  airMopOkay(mop);
  return 0;
}

/* Straightforward implementation of Laplacian+HC mesh smoothing, as
 * described by Vollmer et al., Improved Laplacian Smoothing of Noisy
 * Surface Meshes, Eurographics/CGF 18(3), 1999
 *
 * pld is the polydata you want smoothed
 * neighbors / idx is from limnPolyDataNeighborArrayComp
 * alpha / beta are parameters of the smoothing (try a=0,b=0.5)
 * iter is the number of iterations you want to run (try iter=10)
 *
 * Returns -1 and leaves a message on biff upon error.
 */
int
limnPolyDataSmoothHC(limnPolyData *pld, int *neighbors, int *idx,
                     double alpha, double beta, int iter) {
  static const char me[]="limnPolyDataSmoothHC";
  float *orig, *in, *out, *b;
  unsigned int v;
  int i, nb;
  airArray *mop;
  mop=airMopNew();
  if (pld==NULL || neighbors==NULL || idx==NULL) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    airMopError(mop); return -1;
  }
  if (alpha<0 || alpha>1 || beta<0 || beta>1) {
    biffAddf(LIMN, "%s: alpha/beta outside parameter range [0,1]", me);
    airMopError(mop); return -1;
  }
  orig = in = pld->xyzw;
  out = (float*) malloc(sizeof(float)*4*pld->xyzwNum);
  if (out==NULL) {
    biffAddf(LIMN, "%s: couldn't allocate output buffer", me);
    airMopError(mop); return -1;
  }
  airMopAdd(mop, out, airFree, airMopOnError);
  b = (float*) malloc(sizeof(float)*4*pld->xyzwNum);
  if (b==NULL) {
    biffAddf(LIMN, "%s: couldn't allocate buffer b", me);
    airMopError(mop); return -1;
  }
  airMopAdd(mop, b, airFree, airMopAlways);

  for (i=0; i<iter; i++) {
    /* Laplacian smoothing / compute bs */
    for (v=0; v<pld->xyzwNum; v++) {
      int p=4*v;
      if (idx[v]==idx[v+1]) {
        ELL_4V_COPY(out+p, in+p);
      } else {
        ELL_4V_SET(out+p,0,0,0,0);
        for (nb=idx[v]; nb<idx[v+1]; nb++) {
          ELL_4V_INCR(out+p, in+4*neighbors[nb]);
        }
        ELL_4V_SCALE_TT(out+p, float, 1.0/(idx[v+1]-idx[v]), out+p);
      }
      ELL_4V_SET_TT(b+p, float, out[p]-(alpha*orig[p]+(1-alpha)*in[p]),
                    out[p+1]-(alpha*orig[p+1]+(1-alpha)*in[p+1]),
                    out[p+2]-(alpha*orig[p+2]+(1-alpha)*in[p+2]),
                    out[p+3]-(alpha*orig[p+3]+(1-alpha)*in[p+3]));
    }
    /* HC correction step */
    for (v=0; v<pld->xyzwNum; v++) {
      int p=4*v;
      if (idx[v]<idx[v+1]) {
        float avgb[4]={0,0,0,0};
        for (nb=idx[v]; nb<idx[v+1]; nb++) {
          ELL_4V_INCR(avgb, b+4*neighbors[nb]);
        }
        ELL_4V_SCALE_TT(avgb, float, 1.0/(idx[v+1]-idx[v]), avgb);
        ELL_4V_LERP_TT(avgb, float, beta, b+p, avgb);
        ELL_4V_SUB(out+p,out+p,avgb);
      }
    }
    if (i==0 && iter>1) {
      in = out;
      out = (float*) malloc(sizeof(float)*4*pld->xyzwNum);
    } else { /* swap */
      float *tmp = in; in = out; out = tmp;
    }
  }

  if (iter>1)
    airFree(out);
  airFree(pld->xyzw);
  pld->xyzw = in;
  airMopOkay(mop);
  return 0;
}
